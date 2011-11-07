/*
 * gedit-gio-document-saver.c
 * This file is part of gedit
 *
 * Copyright (C) 2005-2006 - Paolo Borelli and Paolo Maggi
 * Copyright (C) 2007 - Paolo Borelli, Paolo Maggi, Steve Frécinaux
 * Copyright (C) 2008 - Jesse van den Kieboom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the gedit Team, 2005-2006. See the AUTHORS file for a
 * list of people on the gedit Team.
 * See the ChangeLog files for a list of changes.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <glib.h>
#include <gio/gio.h>
#include <string.h>

#include "gedit-gio-document-saver.h"
#include "gedit-document-input-stream.h"
#include "gedit-debug.h"

#define WRITE_CHUNK_SIZE 8192

typedef struct
{
	GeditGioDocumentSaver *saver;
	gchar 		       buffer[WRITE_CHUNK_SIZE];
	GCancellable 	      *cancellable;
	gboolean	       tried_mount;
	gssize		       written;
	gssize		       read;
	GError                *error;
} AsyncData;

#define REMOTE_QUERY_ATTRIBUTES G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE "," \
				G_FILE_ATTRIBUTE_TIME_MODIFIED
				
#define GEDIT_GIO_DOCUMENT_SAVER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
							  GEDIT_TYPE_GIO_DOCUMENT_SAVER, \
							  GeditGioDocumentSaverPrivate))

static void	     gedit_gio_document_saver_save		    (GeditDocumentSaver *saver,
								     GTimeVal           *old_mtime);
static goffset	     gedit_gio_document_saver_get_file_size	    (GeditDocumentSaver *saver);
static goffset	     gedit_gio_document_saver_get_bytes_written	    (GeditDocumentSaver *saver);


static void 	    check_modified_async 			    (AsyncData          *async);

struct _GeditGioDocumentSaverPrivate
{
	GTimeVal		  old_mtime;

	goffset			  size;
	goffset			  bytes_written;

	GFile			 *gfile;
	GCancellable		 *cancellable;
	GOutputStream		 *stream;
	GInputStream		 *input;
	
	GError                   *error;
};

G_DEFINE_TYPE(GeditGioDocumentSaver, gedit_gio_document_saver, GEDIT_TYPE_DOCUMENT_SAVER)

static void
gedit_gio_document_saver_dispose (GObject *object)
{
	GeditGioDocumentSaverPrivate *priv = GEDIT_GIO_DOCUMENT_SAVER (object)->priv;

	if (priv->cancellable != NULL)
	{
		g_cancellable_cancel (priv->cancellable);
		g_object_unref (priv->cancellable);
		priv->cancellable = NULL;
	}

	if (priv->gfile != NULL)
	{
		g_object_unref (priv->gfile);
		priv->gfile = NULL;
	}

	if (priv->error != NULL)
	{
		g_error_free (priv->error);
		priv->error = NULL;
	}

	if (priv->stream != NULL)
	{
		g_object_unref (priv->stream);
		priv->stream = NULL;
	}

	if (priv->input != NULL)
	{
		g_object_unref (priv->input);
		priv->input = NULL;
	}

	G_OBJECT_CLASS (gedit_gio_document_saver_parent_class)->dispose (object);
}

static AsyncData *
async_data_new (GeditGioDocumentSaver *gvsaver)
{
	AsyncData *async;
	
	async = g_slice_new (AsyncData);
	async->saver = gvsaver;
	async->cancellable = g_object_ref (gvsaver->priv->cancellable);

	async->tried_mount = FALSE;
	async->written = 0;
	async->read = 0;

	async->error = NULL;

	return async;
}

static void
async_data_free (AsyncData *async)
{
	g_object_unref (async->cancellable);

	if (async->error)
	{
		g_error_free (async->error);
	}

	g_slice_free (AsyncData, async);
}

static void 
gedit_gio_document_saver_class_init (GeditGioDocumentSaverClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GeditDocumentSaverClass *saver_class = GEDIT_DOCUMENT_SAVER_CLASS (klass);

	object_class->dispose = gedit_gio_document_saver_dispose;

	saver_class->save = gedit_gio_document_saver_save;
	saver_class->get_file_size = gedit_gio_document_saver_get_file_size;
	saver_class->get_bytes_written = gedit_gio_document_saver_get_bytes_written;

	g_type_class_add_private (object_class, sizeof(GeditGioDocumentSaverPrivate));
}

static void
gedit_gio_document_saver_init (GeditGioDocumentSaver *gvsaver)
{
	gvsaver->priv = GEDIT_GIO_DOCUMENT_SAVER_GET_PRIVATE (gvsaver);

	gvsaver->priv->cancellable = g_cancellable_new ();
	gvsaver->priv->error = NULL;
}

static void
remote_save_completed_or_failed (GeditGioDocumentSaver *gvsaver, 
				 AsyncData 	       *async)
{
	gedit_document_saver_saving (GEDIT_DOCUMENT_SAVER (gvsaver),
				     TRUE,
				     gvsaver->priv->error);

	if (async)
		async_data_free (async);
}

static void
async_failed (AsyncData *async,
	      GError    *error)
{
	g_propagate_error (&async->saver->priv->error, error);
	remote_save_completed_or_failed (async->saver, async);
}

/* BEGIN NOTE:
 *
 * This fixes an issue in GOutputStream that applies the atomic replace
 * save strategy. The stream moves the written file to the original file
 * when the stream is closed. However, there is no way currently to tell
 * the stream that the save should be aborted (there could be a
 * conversion error). The patch explicitly closes the output stream
 * in all these cases with a GCancellable in the cancelled state, causing
 * the output stream to close, but not move the file. This makes use
 * of an implementation detail in the local gio file stream and should be
 * properly fixed by adding the appropriate API in gio. Until then, at least
 * we prevent data corruption for now.
 *
 * Relevant bug reports:
 *
 * Bug 615110 - write file ignore encoding errors (gedit)
 * https://bugzilla.mate.org/show_bug.cgi?id=615110
 *
 * Bug 602412 - g_file_replace does not restore original file when there is
 *              errors while writing (glib/gio)
 * https://bugzilla.mate.org/show_bug.cgi?id=602412
 */
static void
cancel_output_stream_ready_cb (GOutputStream *stream,
                               GAsyncResult  *result,
                               AsyncData     *async)
{
	GError *error;

	g_output_stream_close_finish (stream, result, NULL);

	/* check cancelled state manually */
	if (g_cancellable_is_cancelled (async->cancellable) || async->error == NULL)
	{
		async_data_free (async);
		return;
	}

	error = async->error;
	async->error = NULL;

	async_failed (async, error);
}

static void
cancel_output_stream (AsyncData *async)
{
	GCancellable *cancellable;

	gedit_debug_message (DEBUG_SAVER, "Cancel output stream");

	cancellable = g_cancellable_new ();
	g_cancellable_cancel (cancellable);

	g_output_stream_close_async (async->saver->priv->stream,
				     G_PRIORITY_HIGH,
				     cancellable,
				     (GAsyncReadyCallback)cancel_output_stream_ready_cb,
				     async);

	g_object_unref (cancellable);
}

static void
cancel_output_stream_and_fail (AsyncData *async,
                               GError    *error)
{

	gedit_debug_message (DEBUG_SAVER, "Cancel output stream and fail");

	g_propagate_error (&async->error, error);
	cancel_output_stream (async);
}

/*
 * END NOTE
 */

static void
remote_get_info_cb (GFile        *source,
		    GAsyncResult *res,
		    AsyncData    *async)
{
	GeditGioDocumentSaver *saver;
	GFileInfo *info;
	GError *error = NULL;

	gedit_debug (DEBUG_SAVER);

	/* check cancelled state manually */
	if (g_cancellable_is_cancelled (async->cancellable))
	{
		async_data_free (async);
		return;
	}
	
	saver = async->saver;

	gedit_debug_message (DEBUG_SAVER, "Finished query info on file");
	info = g_file_query_info_finish (source, res, &error);

	if (info != NULL)
	{
		if (GEDIT_DOCUMENT_SAVER (saver)->info != NULL)
			g_object_unref (GEDIT_DOCUMENT_SAVER (saver)->info);

		GEDIT_DOCUMENT_SAVER (saver)->info = info;
	}
	else
	{
		gedit_debug_message (DEBUG_SAVER, "Query info failed: %s", error->message);
		g_propagate_error (&saver->priv->error, error);
	}

	remote_save_completed_or_failed (saver, async);
}

static void
close_async_ready_get_info_cb (GOutputStream *stream,
			       GAsyncResult  *res,
			       AsyncData     *async)
{
	GError *error = NULL;

	gedit_debug (DEBUG_SAVER);

	/* check cancelled state manually */
	if (g_cancellable_is_cancelled (async->cancellable))
	{
		async_data_free (async);
		return;
	}
	
	gedit_debug_message (DEBUG_SAVER, "Finished closing stream");
	
	if (!g_output_stream_close_finish (stream, res, &error))
	{
		gedit_debug_message (DEBUG_SAVER, "Closing stream error: %s", error->message);

		async_failed (async, error);
		return;
	}

	/* get the file info: note we cannot use 
	 * g_file_output_stream_query_info_async since it is not able to get the
	 * content type etc, beside it is not supported by gvfs.
	 * I'm not sure this is actually necessary, can't we just use
	 * g_content_type_guess (since we have the file name and the data)
	 */
	gedit_debug_message (DEBUG_SAVER, "Query info on file");
	g_file_query_info_async (async->saver->priv->gfile,
			         REMOTE_QUERY_ATTRIBUTES,
			         G_FILE_QUERY_INFO_NONE,
			         G_PRIORITY_HIGH,
			         async->cancellable,
			         (GAsyncReadyCallback) remote_get_info_cb,
			         async);
}

static void
write_complete (AsyncData *async)
{
	GError *error = NULL;

	/* first we close the input stream */
	gedit_debug_message (DEBUG_SAVER, "Close input stream");
	if (!g_input_stream_close (async->saver->priv->input,
				   async->cancellable, &error))
	{
		gedit_debug_message (DEBUG_SAVER, "Closing input stream error: %s", error->message);
		cancel_output_stream_and_fail (async, error);
		return;
	}

	/* now we close the output stream */
	gedit_debug_message (DEBUG_SAVER, "Close output stream");
	g_output_stream_close_async (async->saver->priv->stream,
				     G_PRIORITY_HIGH,
				     async->cancellable,
				     (GAsyncReadyCallback)close_async_ready_get_info_cb,
				     async);
}

/* prototype, because they call each other... isn't C lovely */
static void read_file_chunk (AsyncData *async);
static void write_file_chunk (AsyncData *async);

static void
async_write_cb (GOutputStream *stream,
		GAsyncResult  *res,
		AsyncData     *async)
{
	GeditGioDocumentSaver *gvsaver;
	gssize bytes_written;
	GError *error = NULL;

	gedit_debug (DEBUG_SAVER);

	/* Check cancelled state manually */
	if (g_cancellable_is_cancelled (async->cancellable))
	{
		cancel_output_stream (async);
		return;
	}

	bytes_written = g_output_stream_write_finish (stream, res, &error);

	gedit_debug_message (DEBUG_SAVER, "Written: %" G_GSSIZE_FORMAT, bytes_written);

	if (bytes_written == -1)
	{
		gedit_debug_message (DEBUG_SAVER, "Write error: %s", error->message);
		cancel_output_stream_and_fail (async, error);
		return;
	}

	gvsaver = async->saver;
	async->written += bytes_written;

	/* write again */
	if (async->written != async->read)
	{
		write_file_chunk (async);
		return;
	}

	/* note that this signal blocks the write... check if it isn't
	 * a performance problem
	 */
	gedit_document_saver_saving (GEDIT_DOCUMENT_SAVER (gvsaver),
				     FALSE,
				     NULL);

	read_file_chunk (async);
}

static void
write_file_chunk (AsyncData *async)
{
	GeditGioDocumentSaver *gvsaver;

	gedit_debug (DEBUG_SAVER);

	gvsaver = async->saver;

	g_output_stream_write_async (G_OUTPUT_STREAM (gvsaver->priv->stream),
				     async->buffer + async->written,
				     async->read - async->written,
				     G_PRIORITY_HIGH,
				     async->cancellable,
				     (GAsyncReadyCallback) async_write_cb,
				     async);
}

static void
read_file_chunk (AsyncData *async)
{
	GeditGioDocumentSaver *gvsaver;
	GeditDocumentInputStream *dstream;
	GError *error = NULL;

	gedit_debug (DEBUG_SAVER);

	gvsaver = async->saver;
	async->written = 0;

	/* we use sync methods on doc stream since it is in memory. Using async
	   would be racy and we can endup with invalidated iters */
	async->read = g_input_stream_read (gvsaver->priv->input,
					   async->buffer,
					   WRITE_CHUNK_SIZE,
					   async->cancellable,
					   &error);

	if (error != NULL)
	{
		cancel_output_stream_and_fail (async, error);
		return;
	}

	/* Check if we finished reading and writing */
	if (async->read == 0)
	{
		write_complete (async);
		return;
	}

	/* Get how many chars have been read */
	dstream = GEDIT_DOCUMENT_INPUT_STREAM (gvsaver->priv->input);
	gvsaver->priv->bytes_written = gedit_document_input_stream_tell (dstream);

	write_file_chunk (async);
}

static void
async_replace_ready_callback (GFile        *source,
			      GAsyncResult *res,
			      AsyncData    *async)
{
	GeditGioDocumentSaver *gvsaver;
	GeditDocumentSaver *saver;
	GCharsetConverter *converter;
	GFileOutputStream *file_stream;
	GError *error = NULL;

	gedit_debug (DEBUG_SAVER);

	/* Check cancelled state manually */
	if (g_cancellable_is_cancelled (async->cancellable))
	{
		async_data_free (async);
		return;
	}
	
	gvsaver = async->saver;
	saver = GEDIT_DOCUMENT_SAVER (gvsaver);
	file_stream = g_file_replace_finish (source, res, &error);
	
	/* handle any error that might occur */
	if (!file_stream)
	{
		gedit_debug_message (DEBUG_SAVER, "Opening file failed: %s", error->message);
		async_failed (async, error);
		return;
	}

	/* FIXME: manage converter error? */
	gedit_debug_message (DEBUG_SAVER, "Encoding charset: %s",
			     gedit_encoding_get_charset (saver->encoding));

	if (saver->encoding != gedit_encoding_get_utf8 ())
	{
		converter = g_charset_converter_new (gedit_encoding_get_charset (saver->encoding),
						     "UTF-8",
						     NULL);
		gvsaver->priv->stream = g_converter_output_stream_new (G_OUTPUT_STREAM (file_stream),
								       G_CONVERTER (converter));

		g_object_unref (file_stream);
		g_object_unref (converter);
	}
	else
	{
		gvsaver->priv->stream = G_OUTPUT_STREAM (file_stream);
	}
	
	gvsaver->priv->input = gedit_document_input_stream_new (GTK_TEXT_BUFFER (saver->document),
								saver->newline_type);

	gvsaver->priv->size = gedit_document_input_stream_get_total_size (GEDIT_DOCUMENT_INPUT_STREAM (gvsaver->priv->input));

	read_file_chunk (async);
}

static void
begin_write (AsyncData *async)
{
	GeditGioDocumentSaver *gvsaver;
	GeditDocumentSaver *saver;
	gboolean backup;

	gedit_debug_message (DEBUG_SAVER, "Start replacing file contents");

	/* For remote files we simply use g_file_replace_async. There is no
	 * backup as of yet
	 */
	gvsaver = async->saver;
	saver = GEDIT_DOCUMENT_SAVER (gvsaver);

	/* Do not make backups for remote files so they do not clutter remote systems */
	backup = (saver->keep_backup && gedit_document_is_local (saver->document));

	gedit_debug_message (DEBUG_SAVER, "File contents size: %" G_GINT64_FORMAT, gvsaver->priv->size);
	gedit_debug_message (DEBUG_SAVER, "Calling replace_async");
	gedit_debug_message (DEBUG_SAVER, backup ? "Keep backup" : "Discard backup");

	g_file_replace_async (gvsaver->priv->gfile,
			      NULL,
			      backup,
			      G_FILE_CREATE_NONE,
			      G_PRIORITY_HIGH,
			      async->cancellable,
			      (GAsyncReadyCallback) async_replace_ready_callback,
			      async);
}

static void
mount_ready_callback (GFile        *file,
		      GAsyncResult *res,
		      AsyncData    *async)
{
	GError *error = NULL;
	gboolean mounted;

	gedit_debug (DEBUG_SAVER);

	/* manual check for cancelled state */
	if (g_cancellable_is_cancelled (async->cancellable))
	{
		async_data_free (async);
		return;
	}

	mounted = g_file_mount_enclosing_volume_finish (file, res, &error);
	
	if (!mounted)
	{
		async_failed (async, error);
	}
	else
	{
		/* try again to get the modified state */
		check_modified_async (async);
	}
}

static void
recover_not_mounted (AsyncData *async)
{
	GeditDocument *doc;
	GMountOperation *mount_operation;
	
	gedit_debug (DEBUG_LOADER);

	doc = gedit_document_saver_get_document (GEDIT_DOCUMENT_SAVER (async->saver));
	mount_operation = _gedit_document_create_mount_operation (doc);

	async->tried_mount = TRUE;
	g_file_mount_enclosing_volume (async->saver->priv->gfile,
				       G_MOUNT_MOUNT_NONE,
				       mount_operation,
				       async->cancellable,
				       (GAsyncReadyCallback) mount_ready_callback,
				       async);

	g_object_unref (mount_operation);
}

static void
check_modification_callback (GFile        *source,
			     GAsyncResult *res,
			     AsyncData    *async)
{
	GeditGioDocumentSaver *gvsaver;
	GError *error = NULL;
	GFileInfo *info;

	gedit_debug (DEBUG_SAVER);

	/* manually check cancelled state */
	if (g_cancellable_is_cancelled (async->cancellable))
	{
		async_data_free (async);
		return;
	}
	
	gvsaver = async->saver;
	info = g_file_query_info_finish (source, res, &error);
	if (info == NULL)
	{
		if (error->code == G_IO_ERROR_NOT_MOUNTED && !async->tried_mount)
		{
			recover_not_mounted (async);
			g_error_free (error);
			return;
		}
		
		/* it's perfectly fine if the file doesn't exist yet */
		if (error->code != G_IO_ERROR_NOT_FOUND)
		{
			gedit_debug_message (DEBUG_SAVER, "Error getting modification: %s", error->message);

			async_failed (async, error);
			return;
		}
	}

	/* check if the mtime is > what we know about it (if we have it) */
	if (info != NULL && g_file_info_has_attribute (info,
				       G_FILE_ATTRIBUTE_TIME_MODIFIED))
	{
		GTimeVal mtime;
		GTimeVal old_mtime;

		g_file_info_get_modification_time (info, &mtime);
		old_mtime = gvsaver->priv->old_mtime;

		if ((old_mtime.tv_sec > 0 || old_mtime.tv_usec > 0) &&
		    (mtime.tv_sec != old_mtime.tv_sec || mtime.tv_usec != old_mtime.tv_usec) &&
		    (GEDIT_DOCUMENT_SAVER (gvsaver)->flags & GEDIT_DOCUMENT_SAVE_IGNORE_MTIME) == 0)
		{
			gedit_debug_message (DEBUG_SAVER, "File is externally modified");
			g_set_error (&gvsaver->priv->error,
				     GEDIT_DOCUMENT_ERROR,
				     GEDIT_DOCUMENT_ERROR_EXTERNALLY_MODIFIED,
				     "Externally modified");

			remote_save_completed_or_failed (gvsaver, async);
			g_object_unref (info);

			return;
		}
	}

	if (info != NULL)
		g_object_unref (info);

	/* modification check passed, start write */
	begin_write (async);
}

static void
check_modified_async (AsyncData *async)
{
	gedit_debug_message (DEBUG_SAVER, "Check externally modified");

	g_file_query_info_async (async->saver->priv->gfile, 
				 G_FILE_ATTRIBUTE_TIME_MODIFIED,
				 G_FILE_QUERY_INFO_NONE,
				 G_PRIORITY_HIGH,
				 async->cancellable,
				 (GAsyncReadyCallback) check_modification_callback,
				 async);
}

static gboolean
save_remote_file_real (GeditGioDocumentSaver *gvsaver)
{
	AsyncData *async;

	gedit_debug_message (DEBUG_SAVER, "Starting gio save");
	
	/* First find out if the file is modified externally. This requires
	 * a stat, but I don't think we can do this any other way
	 */
	async = async_data_new (gvsaver);
	
	check_modified_async (async);
	
	/* return false to stop timeout */
	return FALSE;
}

static void
gedit_gio_document_saver_save (GeditDocumentSaver *saver,
			       GTimeVal           *old_mtime)
{
	GeditGioDocumentSaver *gvsaver = GEDIT_GIO_DOCUMENT_SAVER (saver);

	gvsaver->priv->old_mtime = *old_mtime;
	gvsaver->priv->gfile = g_file_new_for_uri (saver->uri);

	/* saving start */
	gedit_document_saver_saving (saver, FALSE, NULL);

	g_timeout_add_full (G_PRIORITY_HIGH,
			    0,
			    (GSourceFunc) save_remote_file_real,
			    gvsaver,
			    NULL);
}

static goffset
gedit_gio_document_saver_get_file_size (GeditDocumentSaver *saver)
{
	return GEDIT_GIO_DOCUMENT_SAVER (saver)->priv->size;
}

static goffset
gedit_gio_document_saver_get_bytes_written (GeditDocumentSaver *saver)
{
	return GEDIT_GIO_DOCUMENT_SAVER (saver)->priv->bytes_written;
}
