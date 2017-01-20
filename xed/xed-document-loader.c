/*
 * xed-document-loader.c
 * This file is part of xed
 *
 * Copyright (C) 2005 - Paolo Maggi
 * Copyright (C) 2007 - Paolo Maggi, Steve Frécinaux
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/*
 * Modified by the xed Team, 2005-2007. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gio/gio.h>

#include "xed-document-loader.h"
#include "xed-document-output-stream.h"
#include "xed-debug.h"
#include "xed-metadata-manager.h"
#include "xed-utils.h"
#include "xed-marshal.h"
#include "xed-enum-types.h"
#include "xed-settings.h"

#ifndef ENABLE_GVFS_METADATA
#include "xed_metadata-manager.h"
#endif

typedef struct
{
    XedDocumentLoader *loader;
    GCancellable *cancellable;

    gssize read;
    gboolean tried_mount;
}AsyncData;

enum
{
    LOADING,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

enum
{
    PROP_0,
    PROP_DOCUMENT,
    PROP_LOCATION,
    PROP_ENCODING,
    PROP_NEWLINE_TYPE
};

#define READ_CHUNK_SIZE 8192
#define REMOTE_QUERY_ATTRIBUTES G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE "," \
                                G_FILE_ATTRIBUTE_STANDARD_TYPE "," \
                                G_FILE_ATTRIBUTE_TIME_MODIFIED "," \
                                G_FILE_ATTRIBUTE_STANDARD_SIZE "," \
                                G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE "," \
                                XED_METADATA_ATTRIBUTE_ENCODING

#define XED_DOCUMENT_LOADER_GET_PRIVATE(object) \
               (G_TYPE_INSTANCE_GET_PRIVATE ((object), \
                XED_TYPE_DOCUMENT_LOADER,   \
                XedDocumentLoaderPrivate))

static void open_async_read (AsyncData *async);

struct _XedDocumentLoaderPrivate
{
    GSettings *enc_settings;

    XedDocument *document;
    gboolean used;

    /* Info on the current file */
    GFileInfo *info;
    GFile *location;
    const XedEncoding *encoding;
    const XedEncoding *auto_detected_encoding;
    XedDocumentNewlineType auto_detected_newline_type;

    goffset bytes_read;

    /* Handle for remote files */
    GCancellable *cancellable;
    GInputStream *stream;
    GOutputStream *output;

    gchar buffer[READ_CHUNK_SIZE];

    GError *error;
};

G_DEFINE_TYPE(XedDocumentLoader, xed_document_loader, G_TYPE_OBJECT)

static void
xed_document_loader_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
    XedDocumentLoader *loader = XED_DOCUMENT_LOADER (object);

    switch (prop_id)
    {
        case PROP_DOCUMENT:
            g_return_if_fail (loader->priv->document == NULL);
            loader->priv->document = g_value_get_object (value);
            break;
        case PROP_LOCATION:
            g_return_if_fail (loader->priv->location == NULL);
            loader->priv->location = g_value_dup_object (value);
            break;
        case PROP_ENCODING:
            g_return_if_fail (loader->priv->encoding == NULL);
            loader->priv->encoding = g_value_get_boxed (value);
            break;
        case PROP_NEWLINE_TYPE:
            loader->priv->auto_detected_newline_type = g_value_get_enum (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_document_loader_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
    XedDocumentLoader *loader = XED_DOCUMENT_LOADER (object);

    switch (prop_id)
    {
        case PROP_DOCUMENT:
            g_value_set_object (value, loader->priv->document);
            break;
        case PROP_LOCATION:
            g_value_set_object (value, loader->priv->location);
            break;
        case PROP_ENCODING:
            g_value_set_boxed (value, xed_document_loader_get_encoding (loader));
            break;
        case PROP_NEWLINE_TYPE:
            g_value_set_enum (value, loader->priv->auto_detected_newline_type);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_document_loader_dispose (GObject *object)
{
    XedDocumentLoaderPrivate *priv;

    priv = XED_DOCUMENT_LOADER (object)->priv;

    if (priv->cancellable != NULL)
    {
        g_cancellable_cancel (priv->cancellable);
        g_object_unref (priv->cancellable);
        priv->cancellable = NULL;
    }

    if (priv->stream != NULL)
    {
        g_object_unref (priv->stream);
        priv->stream = NULL;
    }

    if (priv->output != NULL)
    {
        g_object_unref (priv->output);
        priv->output = NULL;
    }

    if (priv->error != NULL)
    {
        g_error_free (priv->error);
        priv->error = NULL;
    }

    if (priv->info != NULL)
    {
        g_object_unref (priv->info);
        priv->info = NULL;
    }

    if (priv->location != NULL)
    {
        g_object_unref (priv->location);
        priv->location = NULL;
    }

    g_clear_object (&priv->enc_settings);

    G_OBJECT_CLASS (xed_document_loader_parent_class)->dispose (object);
}

static void
xed_document_loader_class_init (XedDocumentLoaderClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = xed_document_loader_dispose;
    object_class->get_property = xed_document_loader_get_property;
    object_class->set_property = xed_document_loader_set_property;

    g_object_class_install_property (object_class,
                                     PROP_DOCUMENT,
                                     g_param_spec_object ("document",
                                                          "Document",
                                                          "The XedDocument this XedDocumentLoader is associated with",
                                                          XED_TYPE_DOCUMENT,
                                                          G_PARAM_READWRITE |
                                                          G_PARAM_CONSTRUCT_ONLY |
                                                          G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (object_class,
                                     PROP_LOCATION,
                                     g_param_spec_object ("location",
                                                          "LOCATION",
                                                          "The LOCATION this XedDocumentLoader loads the document from",
                                                          G_TYPE_FILE,
                                                          G_PARAM_READWRITE |
                                                          G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (object_class,
                                     PROP_ENCODING,
                                     g_param_spec_boxed ("encoding",
                                                         "Encoding",
                                                         "The encoding of the saved file",
                                                         XED_TYPE_ENCODING,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT_ONLY |
                                                         G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (object_class,
                                     PROP_NEWLINE_TYPE,
                                     g_param_spec_enum ("newline-type",
                                                        "Newline type",
                                                        "The accepted types of line ending",
                                                        XED_TYPE_DOCUMENT_NEWLINE_TYPE,
                                                        XED_DOCUMENT_NEWLINE_TYPE_LF,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));

    signals[LOADING] = g_signal_new ("loading",
                                     G_OBJECT_CLASS_TYPE (object_class),
                                     G_SIGNAL_RUN_LAST,
                                     G_STRUCT_OFFSET (XedDocumentLoaderClass, loading),
                                     NULL, NULL,
                                     xed_marshal_VOID__BOOLEAN_POINTER,
                                     G_TYPE_NONE,
                                     2,
                                     G_TYPE_BOOLEAN,
                                     G_TYPE_POINTER);

    g_type_class_add_private (object_class, sizeof (XedDocumentLoaderPrivate));
}

static void
xed_document_loader_init (XedDocumentLoader *loader)
{
    loader->priv = XED_DOCUMENT_LOADER_GET_PRIVATE (loader);

    loader->priv->used = FALSE;
    loader->priv->auto_detected_newline_type = XED_DOCUMENT_NEWLINE_TYPE_DEFAULT;
    loader->priv->error = NULL;
    loader->priv->enc_settings = g_settings_new ("org.x.editor.preferences.encodings");
}

XedDocumentLoader *
xed_document_loader_new (XedDocument       *doc,
                         GFile             *location,
                         const XedEncoding *encoding)
{
    g_return_val_if_fail (XED_IS_DOCUMENT (doc), NULL);

    return XED_DOCUMENT_LOADER (g_object_new (XED_TYPE_DOCUMENT_LOADER,
                                              "document", doc,
                                              "location", location,
                                              "encoding", encoding,
                                              NULL));
}

static AsyncData *
async_data_new (XedDocumentLoader *loader)
{
    AsyncData *async;

    async = g_slice_new (AsyncData);
    async->loader = loader;
    async->cancellable = g_object_ref (loader->priv->cancellable);
    async->tried_mount = FALSE;

    return async;
}

static void
async_data_free (AsyncData *async)
{
    g_object_unref (async->cancellable);
    g_slice_free (AsyncData, async);
}

static const XedEncoding *
get_metadata_encoding (XedDocumentLoader *loader)
{
    const XedEncoding *enc = NULL;

#ifndef ENABLE_GVFS_METADATA
    gchar *charset;
    GFile *location;
    gchar *uri;

    location = xed_document_loader_get_location (loader);
    uri = g_file_get_uri (location);
    g_object_unref (location);

    charset = xed_metadata_manager_get (uri, "encoding");
    g_free (uri);

    if (charset == NULL)
    {
        return NULL;
    }

    enc = xed_encoding_get_from_charset (charset);

    g_free (charset);
#else
    GFileInfo *info;

    info = xed_document_loader_get_info (loader);

    /* check if encoding was set in the metadata */
    if (g_file_info_has_attribute (info, XED_METADATA_ATTRIBUTE_ENCODING))
    {
        const gchar *charset;

        charset = g_file_info_get_attribute_string (info, XED_METADATA_ATTRIBUTE_ENCODING);

        if (charset == NULL)
        {
            return NULL;
        }

        enc = xed_encoding_get_from_charset (charset);
    }
#endif

    return enc;
}

static void
remote_load_completed_or_failed (XedDocumentLoader *loader,
                                 AsyncData         *async)
{
    xed_document_loader_loading (loader, TRUE, loader->priv->error);

    if (async)
    {
        async_data_free (async);
    }
}

static void
async_failed (AsyncData *async,
              GError    *error)
{
    g_propagate_error (&async->loader->priv->error, error);
    remote_load_completed_or_failed (async->loader, async);
}

static void
close_input_stream_ready_cb (GInputStream *stream,
                             GAsyncResult *res,
                             AsyncData    *async)
{
    GError *error = NULL;

    xed_debug (DEBUG_LOADER);

    /* check cancelled state manually */
    if (g_cancellable_is_cancelled (async->cancellable))
    {
        async_data_free (async);
        return;
    }

    xed_debug_message (DEBUG_SAVER, "Finished closing input stream");

    if (!g_input_stream_close_finish (stream, res, &error))
    {
        xed_debug_message (DEBUG_SAVER, "Closing input stream error: %s", error->message);

        async_failed (async, error);
        return;
    }

    xed_debug_message (DEBUG_SAVER, "Close output stream");
    if (!g_output_stream_close (async->loader->priv->output, async->cancellable, &error))
    {
        async_failed (async, error);
        return;
    }

    /* Check if we needed some fallback char, if so, check if there was
      a previous error and if not set a fallback used error */
   if ((xed_document_output_stream_get_num_fallbacks (XED_DOCUMENT_OUTPUT_STREAM (async->loader->priv->output)) != 0) &&
       async->loader->priv->error == NULL)
   {
       g_set_error_literal (&async->loader->priv->error,
                            XED_DOCUMENT_ERROR,
                            XED_DOCUMENT_ERROR_CONVERSION_FALLBACK,
                            "There was a conversion error and it was "
                            "needed to use a fallback char");
   }

    remote_load_completed_or_failed (async->loader, async);
}

static void
write_complete (AsyncData *async)
{
    if (async->loader->priv->stream)
    {
        g_input_stream_close_async (G_INPUT_STREAM (async->loader->priv->stream),
                                    G_PRIORITY_HIGH,
                                    async->cancellable,
                                    (GAsyncReadyCallback)close_input_stream_ready_cb,
                                    async);
    }
}

/* prototype, because they call each other... isn't C lovely */
static void    read_file_chunk     (AsyncData *async);

static void
write_file_chunk (AsyncData *async)
{
    XedDocumentLoader *loader;
    gssize bytes_written;
    GError *error = NULL;

    loader = async->loader;

    /* we use sync methods on doc stream since it is in memory. Using async
       would be racy and we can endup with invalidated iters */
    bytes_written = g_output_stream_write (G_OUTPUT_STREAM (loader->priv->output),
                                           loader->priv->buffer,
                                           async->read,
                                           async->cancellable,
                                           &error);

    xed_debug_message (DEBUG_SAVER, "Written: %" G_GSSIZE_FORMAT, bytes_written);
    if (bytes_written == -1)
    {
        xed_debug_message (DEBUG_SAVER, "Write error: %s", error->message);
        async_failed (async, error);
        return;
    }

    /* note that this signal blocks the read... check if it isn't
     * a performance problem
     */
    xed_document_loader_loading (loader, FALSE, NULL);

    read_file_chunk (async);
}

static void
async_read_cb (GInputStream *stream,
               GAsyncResult *res,
               AsyncData    *async)
{
    XedDocumentLoader *loader;
    GError *error = NULL;

    xed_debug (DEBUG_LOADER);

    /* manually check cancelled state */
    if (g_cancellable_is_cancelled (async->cancellable))
    {
        async_data_free (async);
        return;
    }

    loader = async->loader;

    async->read = g_input_stream_read_finish (stream, res, &error);

    /* error occurred */
    if (async->read == -1)
    {
        async_failed (async, error);
        return;
    }

    /* Check for the extremely unlikely case where the file size overflows. */
    if (loader->priv->bytes_read + async->read < loader->priv->bytes_read)
    {
        g_set_error (&loader->priv->error,
                     XED_DOCUMENT_ERROR,
                     XED_DOCUMENT_ERROR_TOO_BIG,
                     "File too big");

        async_failed (async, loader->priv->error);
        return;
    }

    /* Bump the size. */
    loader->priv->bytes_read += async->read;

    /* end of the file, we are done! */
    if (async->read == 0)
    {
        /* flush the stream to ensure proper line ending detection */
        g_output_stream_flush (loader->priv->output, NULL, NULL);

        loader->priv->auto_detected_encoding =
            xed_document_output_stream_get_guessed (XED_DOCUMENT_OUTPUT_STREAM (loader->priv->output));

        loader->priv->auto_detected_newline_type =
            xed_document_output_stream_detect_newline_type (XED_DOCUMENT_OUTPUT_STREAM (loader->priv->output));

        write_complete (async);

        return;
    }

    write_file_chunk (async);
}

static void
read_file_chunk (AsyncData *async)
{
    XedDocumentLoader *loader;

    loader = async->loader;

    g_input_stream_read_async (G_INPUT_STREAM (loader->priv->stream),
                               loader->priv->buffer,
                               READ_CHUNK_SIZE,
                               G_PRIORITY_HIGH,
                               async->cancellable,
                               (GAsyncReadyCallback) async_read_cb,
                               async);
}

static GSList *
get_candidate_encodings (XedDocumentLoader *loader)
{
    const XedEncoding *metadata;
    GSList *encodings;
    gchar **enc_strv;

    enc_strv = g_settings_get_strv (loader->priv->enc_settings, XED_SETTINGS_ENCODING_AUTO_DETECTED);
    encodings = _xed_encoding_strv_to_list ((const gchar * const *)enc_strv);
    g_free (enc_strv);

    metadata = get_metadata_encoding (loader);
    if (metadata != NULL)
    {
       encodings = g_slist_prepend (encodings, (gpointer)metadata);
    }

    return encodings;
}

static void
finish_query_info (AsyncData *async)
{
    XedDocumentLoader *loader;
    GInputStream *conv_stream;
    GFileInfo *info;
    GSList *candidate_encodings;

    loader = async->loader;
    info = loader->priv->info;

    /* if it's not a regular file, error out... */
    if (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_STANDARD_TYPE) &&
        g_file_info_get_file_type (info) != G_FILE_TYPE_REGULAR)
    {
        g_set_error (&loader->priv->error,
                     G_IO_ERROR,
                     G_IO_ERROR_NOT_REGULAR_FILE,
                     "Not a regular file");

        remote_load_completed_or_failed (loader, async);

        return;
    }

    conv_stream = g_object_ref (loader->priv->stream);
    g_object_unref (loader->priv->stream);
    loader->priv->stream = conv_stream;

    /* Get the candidate encodings */
    if (loader->priv->encoding == NULL)
    {
        candidate_encodings = get_candidate_encodings (loader);
    }
    else
    {
        candidate_encodings = g_slist_prepend (NULL, (gpointer) loader->priv->encoding);
    }

    /* Output stream */
    loader->priv->output = xed_document_output_stream_new (loader->priv->document, candidate_encodings);
    g_slist_free (candidate_encodings);

    /* start reading */
    read_file_chunk (async);
}

static void
query_info_cb (GFile        *source,
               GAsyncResult *res,
               AsyncData    *async)
{
    GFileInfo *info;
    GError *error = NULL;

    xed_debug (DEBUG_LOADER);

    /* manually check the cancelled state */
    if (g_cancellable_is_cancelled (async->cancellable))
    {
        async_data_free (async);
        return;
    }

    /* finish the info query */
    info = g_file_query_info_finish (async->loader->priv->location, res, &error);

    if (info == NULL)
    {
        /* propagate the error and clean up */
        async_failed (async, error);
        return;
    }

    async->loader->priv->info = info;

    finish_query_info (async);
}

static void
mount_ready_callback (GFile        *file,
                      GAsyncResult *res,
                      AsyncData    *async)
{
    GError *error = NULL;
    gboolean mounted;

    xed_debug (DEBUG_LOADER);

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
        /* try again to open the file for reading */
       open_async_read (async);
    }
}

static void
recover_not_mounted (AsyncData *async)
{
    XedDocument *doc;
    GMountOperation *mount_operation;

    xed_debug (DEBUG_LOADER);

    doc = xed_document_loader_get_document (async->loader);
    mount_operation = _xed_document_create_mount_operation (doc);

    async->tried_mount = TRUE;
    g_file_mount_enclosing_volume (async->loader->priv->location,
                                   G_MOUNT_MOUNT_NONE,
                                   mount_operation,
                                   async->cancellable,
                                   (GAsyncReadyCallback) mount_ready_callback,
                                   async);

    g_object_unref (mount_operation);
}

static void
async_read_ready_callback (GObject      *source,
                           GAsyncResult *res,
                           AsyncData    *async)
{
    GError *error = NULL;
    XedDocumentLoader *loader;

    xed_debug (DEBUG_LOADER);

    /* manual check for cancelled state */
    if (g_cancellable_is_cancelled (async->cancellable))
    {
        async_data_free (async);
        return;
    }

    loader = async->loader;

    loader->priv->stream = G_INPUT_STREAM (g_file_read_finish (loader->priv->location, res, &error));

    if (!loader->priv->stream)
    {
        if (error->code == G_IO_ERROR_NOT_MOUNTED && !async->tried_mount)
        {
            recover_not_mounted (async);
            g_error_free (error);
            return;
        }

        /* Propagate error */
        g_propagate_error (&loader->priv->error, error);
        xed_document_loader_loading (loader, TRUE, loader->priv->error);

        async_data_free (async);
        return;
    }

    /* get the file info: note we cannot use
     * g_file_input_stream_query_info_async since it is not able to get the
     * content type etc, beside it is not supported by gvfs.
     * Using the file instead of the stream is slightly racy, but for
     * loading this is not too bad...
     */
    g_file_query_info_async (loader->priv->location,
                             REMOTE_QUERY_ATTRIBUTES,
                             G_FILE_QUERY_INFO_NONE,
                             G_PRIORITY_HIGH,
                             async->cancellable,
                             (GAsyncReadyCallback) query_info_cb,
                             async);
}

static void
open_async_read (AsyncData *async)
{
    g_file_read_async (async->loader->priv->location,
                       G_PRIORITY_HIGH,
                       async->cancellable,
                       (GAsyncReadyCallback) async_read_ready_callback,
                       async);
}

void
xed_document_loader_loading (XedDocumentLoader *loader,
                             gboolean           completed,
                             GError            *error)
{
    /* the object will be unrefed in the callback of the loading signal
     * (when completed == TRUE), so we need to prevent finalization.
     */
    if (completed)
    {
        g_object_ref (loader);
    }

    g_signal_emit (loader, signals[LOADING], 0, completed, error);

    if (completed)
    {
        if (error == NULL)
        {
            xed_debug_message (DEBUG_LOADER, "load completed");
        }
        else
        {
            xed_debug_message (DEBUG_LOADER, "load failed");
        }

        g_object_unref (loader);
    }
}

void
xed_document_loader_load (XedDocumentLoader *loader)
{
    AsyncData *async;

    xed_debug (DEBUG_LOADER);

    g_return_if_fail (XED_IS_DOCUMENT_LOADER (loader));

    /* the loader can be used just once, then it must be thrown away */
    g_return_if_fail (loader->priv->used == FALSE);
    loader->priv->used = TRUE;

    /* Make sure no load operation is currently running */
    g_return_if_fail (loader->priv->cancellable == NULL);

    /* loading start */
    xed_document_loader_loading (loader, FALSE, NULL);

    loader->priv->cancellable = g_cancellable_new ();
    async = async_data_new (loader);

    open_async_read (async);
}

gboolean
xed_document_loader_cancel (XedDocumentLoader *loader)
{
    xed_debug (DEBUG_LOADER);

    g_return_val_if_fail (XED_IS_DOCUMENT_LOADER (loader), FALSE);

    if (loader->priv->cancellable == NULL)
    {
        return FALSE;
    }

    g_cancellable_cancel (loader->priv->cancellable);

    g_set_error (&loader->priv->error,
                 G_IO_ERROR,
                 G_IO_ERROR_CANCELLED,
                 "Operation cancelled");

    remote_load_completed_or_failed (loader, NULL);

    return TRUE;
}

XedDocument *
xed_document_loader_get_document (XedDocumentLoader *loader)
{
    g_return_val_if_fail (XED_IS_DOCUMENT_LOADER (loader), NULL);

    return loader->priv->document;
}

/* Returns STDIN_URI if loading from stdin */
GFile *
xed_document_loader_get_location (XedDocumentLoader *loader)
{
    g_return_val_if_fail (XED_IS_DOCUMENT_LOADER (loader), NULL);

    return g_file_dup (loader->priv->location);
}

goffset
xed_document_loader_get_bytes_read (XedDocumentLoader *loader)
{
    g_return_val_if_fail (XED_IS_DOCUMENT_LOADER (loader), 0);

    return loader->priv->bytes_read;
}

const XedEncoding *
xed_document_loader_get_encoding (XedDocumentLoader *loader)
{
    g_return_val_if_fail (XED_IS_DOCUMENT_LOADER (loader), NULL);

    if (loader->priv->encoding != NULL)
    {
        return loader->priv->encoding;
    }

    g_return_val_if_fail (loader->priv->auto_detected_encoding != NULL, xed_encoding_get_current ());

    return loader->priv->auto_detected_encoding;
}

XedDocumentNewlineType
xed_document_loader_get_newline_type (XedDocumentLoader *loader)
{
    g_return_val_if_fail (XED_IS_DOCUMENT_LOADER (loader), XED_DOCUMENT_NEWLINE_TYPE_LF);

    return loader->priv->auto_detected_newline_type;
}

GFileInfo *
xed_document_loader_get_info (XedDocumentLoader *loader)
{
    g_return_val_if_fail (XED_IS_DOCUMENT_LOADER (loader), NULL);

    return loader->priv->info;
}
