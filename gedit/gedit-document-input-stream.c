/*
 * gedit-document-input-stream.c
 * This file is part of gedit
 *
 * Copyright (C) 2010 - Ignacio Casal Quinteiro
 *
 * gedit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * gedit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gedit; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#include "config.h"

#include <glib.h>
#include <gio/gio.h>
#include <string.h>
#include "gedit-document-input-stream.h"
#include "gedit-enum-types.h"

/* NOTE: never use async methods on this stream, the stream is just
 * a wrapper around GtkTextBuffer api so that we can use GIO Stream
 * methods, but the undelying code operates on a GtkTextBuffer, so
 * there is no I/O involved and should be accessed only by the main
 * thread */


G_DEFINE_TYPE (GeditDocumentInputStream, gedit_document_input_stream, G_TYPE_INPUT_STREAM);

struct _GeditDocumentInputStreamPrivate
{
	GtkTextBuffer *buffer;
	GtkTextMark   *pos;
	gint           bytes_partial;

	GeditDocumentNewlineType newline_type;

	guint newline_added : 1;
	guint is_initialized : 1;
};

enum
{
	PROP_0,
	PROP_BUFFER,
	PROP_NEWLINE_TYPE
};

static gssize     gedit_document_input_stream_read     (GInputStream      *stream,
							void              *buffer,
							gsize              count,
							GCancellable      *cancellable,
							GError           **error);
static gboolean   gedit_document_input_stream_close    (GInputStream      *stream,
							GCancellable      *cancellable,
							GError           **error);

static void
gedit_document_input_stream_set_property (GObject      *object,
					  guint         prop_id,
					  const GValue *value,
					  GParamSpec   *pspec)
{
	GeditDocumentInputStream *stream = GEDIT_DOCUMENT_INPUT_STREAM (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			stream->priv->buffer = GTK_TEXT_BUFFER (g_value_get_object (value));
			break;

		case PROP_NEWLINE_TYPE:
			stream->priv->newline_type = g_value_get_enum (value);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gedit_document_input_stream_get_property (GObject    *object,
					  guint       prop_id,
					  GValue     *value,
					  GParamSpec *pspec)
{
	GeditDocumentInputStream *stream = GEDIT_DOCUMENT_INPUT_STREAM (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_value_set_object (value, stream->priv->buffer);
			break;

		case PROP_NEWLINE_TYPE:
			g_value_set_enum (value, stream->priv->newline_type);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gedit_document_input_stream_class_init (GeditDocumentInputStreamClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GInputStreamClass *stream_class = G_INPUT_STREAM_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GeditDocumentInputStreamPrivate));

	gobject_class->get_property = gedit_document_input_stream_get_property;
	gobject_class->set_property = gedit_document_input_stream_set_property;

	stream_class->read_fn = gedit_document_input_stream_read;
	stream_class->close_fn = gedit_document_input_stream_close;

	g_object_class_install_property (gobject_class,
					 PROP_BUFFER,
					 g_param_spec_object ("buffer",
							      "Buffer",
							      "The buffer which is read",
							      GTK_TYPE_TEXT_BUFFER,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY));

	/**
	 * GeditDocumentInputStream:newline-type:
	 *
	 * The :newline-type property determines what is considered
	 * as a line ending when reading complete lines from the stream.
	 */ 
	g_object_class_install_property (gobject_class,
					 PROP_NEWLINE_TYPE,
					 g_param_spec_enum ("newline-type",
							    "Newline type",
							    "The accepted types of line ending",
							    GEDIT_TYPE_DOCUMENT_NEWLINE_TYPE,
							    GEDIT_DOCUMENT_NEWLINE_TYPE_LF,
							    G_PARAM_READWRITE |
							    G_PARAM_STATIC_NAME |
							    G_PARAM_STATIC_BLURB |
							    G_PARAM_CONSTRUCT_ONLY));
}

static void
gedit_document_input_stream_init (GeditDocumentInputStream *stream)
{
	stream->priv = G_TYPE_INSTANCE_GET_PRIVATE (stream,
						    GEDIT_TYPE_DOCUMENT_INPUT_STREAM,
						    GeditDocumentInputStreamPrivate);
}

static gsize
get_new_line_size (GeditDocumentInputStream *stream)
{
	gsize ret;

	switch (stream->priv->newline_type)
	{
		case GEDIT_DOCUMENT_NEWLINE_TYPE_CR:
		case GEDIT_DOCUMENT_NEWLINE_TYPE_LF:
			ret = 1;
			break;

		case GEDIT_DOCUMENT_NEWLINE_TYPE_CR_LF:
			ret = 2;
			break;

		default:
			g_warn_if_reached ();
			ret = 1;
			break;
	}

	return ret;
}

/**
 * gedit_document_input_stream_new:
 * @buffer: a #GtkTextBuffer
 *
 * Reads the data from @buffer.
 *
 * Returns: a new #GInputStream to read @buffer
 */
GInputStream *
gedit_document_input_stream_new (GtkTextBuffer           *buffer,
				 GeditDocumentNewlineType type)
{
	GeditDocumentInputStream *stream;

	g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), NULL);

	stream = g_object_new (GEDIT_TYPE_DOCUMENT_INPUT_STREAM,
			       "buffer", buffer,
			       "newline-type", type,
			       NULL);

	return G_INPUT_STREAM (stream);
}

gsize
gedit_document_input_stream_get_total_size (GeditDocumentInputStream *stream)
{
	g_return_val_if_fail (GEDIT_IS_DOCUMENT_INPUT_STREAM (stream), 0);

	return gtk_text_buffer_get_char_count (stream->priv->buffer);
}

gsize
gedit_document_input_stream_tell (GeditDocumentInputStream *stream)
{
	g_return_val_if_fail (GEDIT_IS_DOCUMENT_INPUT_STREAM (stream), 0);

	/* FIXME: is this potentially inefficient? If yes, we could keep
	   track of the offset internally, assuming the mark doesn't move
	   during the operation */
	if (!stream->priv->is_initialized)
	{
		return 0;
	}
	else
	{
		GtkTextIter iter;

		gtk_text_buffer_get_iter_at_mark (stream->priv->buffer,
						  &iter,
						  stream->priv->pos);
		return gtk_text_iter_get_offset (&iter);
	}
}

static const gchar *
get_new_line (GeditDocumentInputStream *stream)
{
	const gchar *ret;

	switch (stream->priv->newline_type)
	{
		case GEDIT_DOCUMENT_NEWLINE_TYPE_CR:
			ret = "\r";
			break;

		case GEDIT_DOCUMENT_NEWLINE_TYPE_LF:
			ret = "\n";
			break;

		case GEDIT_DOCUMENT_NEWLINE_TYPE_CR_LF:
			ret = "\r\n";
			break;

		default:
			g_warn_if_reached ();
			ret = "\n";
			break;
	}

	return ret;
}

static gsize
read_line (GeditDocumentInputStream *stream,
	   gchar                    *outbuf,
	   gsize                     space_left)
{
	GtkTextIter start, next, end;
	gchar *buf;
	gint bytes; /* int since it's what iter_get_offset returns */
	gsize bytes_to_write, newline_size, read;
	const gchar *newline;
	gboolean is_last;

	gtk_text_buffer_get_iter_at_mark (stream->priv->buffer,
					  &start,
					  stream->priv->pos);

	if (gtk_text_iter_is_end (&start))
		return 0;

	end = next = start;
	newline = get_new_line (stream);

	/* Check needed for empty lines */
	if (!gtk_text_iter_ends_line (&end))
		gtk_text_iter_forward_to_line_end (&end);

	gtk_text_iter_forward_line (&next);

	buf = gtk_text_iter_get_slice (&start, &end);

	/* the bytes of a line includes also the newline, so with the
	   offsets we remove the newline and we add the new newline size */
	bytes = gtk_text_iter_get_bytes_in_line (&start) - stream->priv->bytes_partial;

	/* bytes_in_line includes the newlines, so we remove that assuming that
	   they are single byte characters */
	bytes = bytes - (gtk_text_iter_get_offset (&next) - gtk_text_iter_get_offset (&end));
	is_last = gtk_text_iter_is_end (&end);

	/* bytes_to_write contains the amount of bytes we would like to write.
	   This means its the amount of bytes in the line (without the newline
	   in the buffer) + the amount of bytes for the newline we want to
	   write (newline_size) */
	bytes_to_write = bytes;

	/* do not add the new newline_size for the last line */
	newline_size = get_new_line_size (stream);
	if (!is_last)
		bytes_to_write += newline_size;

	if (bytes_to_write > space_left)
	{
		gchar *ptr;
		gint char_offset;
		gint written;
		gsize to_write;

		/* Here the line does not fit in the buffer, we thus write
		   the amount of bytes we can still fit, storing the position
		   for the next read with the mark. Do not try to write the
		   new newline in this case, it will be handled in the next
		   iteration */
		to_write = MIN (space_left, bytes);
		ptr = buf;
		written = 0;
		char_offset = 0;

		while (written < to_write)
		{
			gint w;

			ptr = g_utf8_next_char (ptr);
			w = (ptr - buf);
			if (w > to_write)
			{
				break;
			}
			else
			{
				written = w;
				++char_offset;
			}
		}

		memcpy (outbuf, buf, written);

		/* Note: offset is one past what we wrote */
		gtk_text_iter_forward_chars (&start, char_offset);
		stream->priv->bytes_partial += written;
		read = written;
	}
	else
	{
		/* First just copy the bytes without the newline */
		memcpy (outbuf, buf, bytes);

		/* Then add the newline, but not for the last line */
		if (!is_last)
		{
			memcpy (outbuf + bytes, newline, newline_size);
		}

		start = next;
		stream->priv->bytes_partial = 0;
		read = bytes_to_write;
	}

	gtk_text_buffer_move_mark (stream->priv->buffer,
				   stream->priv->pos,
				   &start);

	g_free (buf);
	return read;
}

static gssize
gedit_document_input_stream_read (GInputStream  *stream,
				  void          *buffer,
				  gsize          count,
				  GCancellable  *cancellable,
				  GError       **error)
{
	GeditDocumentInputStream *dstream;
	GtkTextIter iter;
	gssize space_left, read, n;

	dstream = GEDIT_DOCUMENT_INPUT_STREAM (stream);

	if (count < 6)
	{
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NO_SPACE,
				     "Not enougth space in destination");
		return -1;
	}

	if (g_cancellable_set_error_if_cancelled (cancellable, error))
		return -1;

	/* Initialize the mark to the first char in the text buffer */
	if (!dstream->priv->is_initialized)
	{
		gtk_text_buffer_get_start_iter (dstream->priv->buffer, &iter);
		dstream->priv->pos = gtk_text_buffer_create_mark (dstream->priv->buffer,
								 NULL,
								 &iter,
								 FALSE);

		dstream->priv->is_initialized = TRUE;
	}

	space_left = count;
	read = 0;

	do
	{
		n = read_line (dstream, buffer + read, space_left);
		read += n;
		space_left -= n;
	} while (space_left > 0 && n != 0 && dstream->priv->bytes_partial == 0);

	/* Make sure that non-empty files are always terminated with \n (see bug #95676).
	 * Note that we strip the trailing \n when loading the file */
	gtk_text_buffer_get_iter_at_mark (dstream->priv->buffer,
					  &iter,
					  dstream->priv->pos);

	if (gtk_text_iter_is_end (&iter) &&
	    !gtk_text_iter_is_start (&iter))
	{
		gssize newline_size;

		newline_size = get_new_line_size (dstream);

		if (space_left >= newline_size &&
		    !dstream->priv->newline_added)
		{
			const gchar *newline;

			newline = get_new_line (dstream);

			memcpy (buffer + read, newline, newline_size);

			read += newline_size;
			dstream->priv->newline_added = TRUE;
		}
	}

	return read;
}

static gboolean
gedit_document_input_stream_close (GInputStream  *stream,
				   GCancellable  *cancellable,
				   GError       **error)
{
	GeditDocumentInputStream *dstream = GEDIT_DOCUMENT_INPUT_STREAM (stream);

	dstream->priv->newline_added = FALSE;

	if (dstream->priv->is_initialized)
	{
		gtk_text_buffer_delete_mark (dstream->priv->buffer, dstream->priv->pos);
	}

	return TRUE;
}
