/*
 * xed-document-output-stream.c
 * This file is part of xed
 *
 * Copyright (C) 2010 - Ignacio Casal Quinteiro
 *
 * xed is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xed is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xed; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "config.h"

#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <errno.h>
#include "xed-document-output-stream.h"
#include "xed-debug.h"

/* NOTE: never use async methods on this stream, the stream is just
 * a wrapper around GtkTextBuffer api so that we can use GIO Stream
 * methods, but the undelying code operates on a GtkTextBuffer, so
 * there is no I/O involved and should be accessed only by the main
 * thread */

#define XED_DOCUMENT_OUTPUT_STREAM_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object),\
                                                       XED_TYPE_DOCUMENT_OUTPUT_STREAM,\
                                                       XedDocumentOutputStreamPrivate))

#define MAX_UNICHAR_LEN 6

struct _XedDocumentOutputStreamPrivate
{
    XedDocument *doc;
    GtkTextIter  pos;

    gchar *buffer;
    gsize buflen;

    /* Encoding detection */
    GIConv iconv;
    GCharsetConverter *charset_conv;

    GSList *encodings;
    GSList *current_encoding;

    guint is_utf8 : 1;
    guint use_first : 1;

    guint is_initialized : 1;
    guint is_closed : 1;
};

enum
{
    PROP_0,
    PROP_DOCUMENT
};

G_DEFINE_TYPE (XedDocumentOutputStream, xed_document_output_stream, G_TYPE_OUTPUT_STREAM)

static gssize   xed_document_output_stream_write (GOutputStream *stream,
                                                  const void    *buffer,
                                                  gsize          count,
                                                  GCancellable   *cancellable,
                                                  GError        **error);

static gboolean xed_document_output_stream_flush (GOutputStream *stream,
                                                  GCancellable  *cancellable,
                                                  GError        **error);

static gboolean xed_document_output_stream_close (GOutputStream *stream,
                                                  GCancellable  *cancellable,
                                                  GError       **error);

static void
xed_document_output_stream_set_property (GObject      *object,
                                         guint         prop_id,
                                         const GValue *value,
                                         GParamSpec   *pspec)
{
    XedDocumentOutputStream *stream = XED_DOCUMENT_OUTPUT_STREAM (object);

    switch (prop_id)
    {
        case PROP_DOCUMENT:
            stream->priv->doc = XED_DOCUMENT (g_value_get_object (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_document_output_stream_get_property (GObject    *object,
                                         guint       prop_id,
                                         GValue     *value,
                                         GParamSpec *pspec)
{
    XedDocumentOutputStream *stream = XED_DOCUMENT_OUTPUT_STREAM (object);

    switch (prop_id)
    {
        case PROP_DOCUMENT:
            g_value_set_object (value, stream->priv->doc);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_document_output_stream_dispose (GObject *object)
{
    XedDocumentOutputStream *stream = XED_DOCUMENT_OUTPUT_STREAM (object);

    if (stream->priv->iconv != NULL)
    {
        g_iconv_close (stream->priv->iconv);
        stream->priv->iconv = NULL;
    }

    if (stream->priv->charset_conv != NULL)
    {
        g_object_unref (stream->priv->charset_conv);
        stream->priv->charset_conv = NULL;
    }

    G_OBJECT_CLASS (xed_document_output_stream_parent_class)->dispose (object);
}

static void
xed_document_output_stream_finalize (GObject *object)
{
    XedDocumentOutputStream *stream = XED_DOCUMENT_OUTPUT_STREAM (object);

    g_free (stream->priv->buffer);
    g_slist_free (stream->priv->encodings);

    G_OBJECT_CLASS (xed_document_output_stream_parent_class)->finalize (object);
}

static void
xed_document_output_stream_constructed (GObject *object)
{
    XedDocumentOutputStream *stream = XED_DOCUMENT_OUTPUT_STREAM (object);

    if (!stream->priv->doc)
    {
        g_critical ("This should never happen, a problem happened constructing the Document Output Stream!");
        return;
    }

    /* Init the undoable action */
    gtk_source_buffer_begin_not_undoable_action (GTK_SOURCE_BUFFER (stream->priv->doc));
    /* clear the buffer */
    gtk_text_buffer_set_text (GTK_TEXT_BUFFER (stream->priv->doc), "", 0);
    gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (stream->priv->doc), FALSE);

    gtk_source_buffer_end_not_undoable_action (GTK_SOURCE_BUFFER (stream->priv->doc));
}

static void
xed_document_output_stream_class_init (XedDocumentOutputStreamClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GOutputStreamClass *stream_class = G_OUTPUT_STREAM_CLASS (klass);

    object_class->get_property = xed_document_output_stream_get_property;
    object_class->set_property = xed_document_output_stream_set_property;
    object_class->dispose = xed_document_output_stream_dispose;
    object_class->finalize = xed_document_output_stream_finalize;
    object_class->constructed = xed_document_output_stream_constructed;

    stream_class->write_fn = xed_document_output_stream_write;
    stream_class->flush = xed_document_output_stream_flush;
    stream_class->close_fn = xed_document_output_stream_close;

    g_object_class_install_property (object_class,
                                     PROP_DOCUMENT,
                                     g_param_spec_object ("document",
                                                          "Document",
                                                          "The document which is written",
                                                          XED_TYPE_DOCUMENT,
                                                          G_PARAM_READWRITE |
                                                          G_PARAM_CONSTRUCT_ONLY));

    g_type_class_add_private (object_class, sizeof (XedDocumentOutputStreamPrivate));
}

static void
xed_document_output_stream_init (XedDocumentOutputStream *stream)
{
    stream->priv = XED_DOCUMENT_OUTPUT_STREAM_GET_PRIVATE (stream);

    stream->priv->buffer = NULL;
    stream->priv->buflen = 0;

    stream->priv->charset_conv = NULL;
    stream->priv->encodings = NULL;
    stream->priv->current_encoding = NULL;

    stream->priv->is_initialized = FALSE;
    stream->priv->is_closed = FALSE;
    stream->priv->is_utf8 = FALSE;
    stream->priv->use_first = FALSE;
}

static const XedEncoding *
get_encoding (XedDocumentOutputStream *stream)
{
    if (stream->priv->current_encoding == NULL)
    {
        stream->priv->current_encoding = stream->priv->encodings;
    }
    else
    {
        stream->priv->current_encoding = g_slist_next (stream->priv->current_encoding);
    }

    if (stream->priv->current_encoding != NULL)
    {
        return (const XedEncoding *)stream->priv->current_encoding->data;
    }

    return NULL;
}

static gboolean
try_convert (GCharsetConverter *converter,
             const void        *inbuf,
             gsize              inbuf_size)
{
    GError *err;
    gsize bytes_read, nread;
    gsize bytes_written, nwritten;
    GConverterResult res;
    gchar *out;
    gboolean ret;
    gsize out_size;

    if (inbuf == NULL || inbuf_size == 0)
    {
        return FALSE;
    }

    err = NULL;
    nread = 0;
    nwritten = 0;
    out_size = inbuf_size * 4;
    out = g_malloc (out_size);

    do
    {
        res = g_converter_convert (G_CONVERTER (converter),
                                  (gchar *)inbuf + nread,
                                  inbuf_size - nread,
                                  (gchar *)out + nwritten,
                                  out_size - nwritten,
                                  G_CONVERTER_INPUT_AT_END,
                                  &bytes_read,
                                  &bytes_written,
                                  &err);

        nread += bytes_read;
        nwritten += bytes_written;
    } while (res != G_CONVERTER_FINISHED && res != G_CONVERTER_ERROR && err == NULL);

    if (err != NULL)
    {
        if (err->code == G_CONVERT_ERROR_PARTIAL_INPUT)
        {
            /* FIXME We can get partial input while guessing the
              encoding because we just take some amount of text
              to guess from. */
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }

        g_error_free (err);
    }
    else
    {
        ret = TRUE;
    }

    /* FIXME: Check the remainder? */
    if (ret == TRUE && !g_utf8_validate (out, nwritten, NULL))
    {
        ret = FALSE;
    }

    g_free (out);

    return ret;
}

static GCharsetConverter *
guess_encoding (XedDocumentOutputStream *stream,
                const void              *inbuf,
                gsize                    inbuf_size)
{
    GCharsetConverter *conv = NULL;

    if (inbuf == NULL || inbuf_size == 0)
    {
        stream->priv->is_utf8 = TRUE;
        return NULL;
    }

    if (stream->priv->encodings != NULL && stream->priv->encodings->next == NULL)
    {
        stream->priv->use_first = TRUE;
    }

    /* We just check the first block */
    while (TRUE)
    {
        const XedEncoding *enc;

        if (conv != NULL)
        {
            g_object_unref (conv);
            conv = NULL;
        }

        /* We get an encoding from the list */
        enc = get_encoding (stream);

        /* if it is NULL we didn't guess anything */
        if (enc == NULL)
        {
            break;
        }

        xed_debug_message (DEBUG_UTILS, "trying charset: %s",
                           xed_encoding_get_charset (stream->priv->current_encoding->data));

        if (enc == xed_encoding_get_utf8 ())
        {
            gsize remainder;
            const gchar *end;

            if (g_utf8_validate (inbuf, inbuf_size, &end) || stream->priv->use_first)
            {
                stream->priv->is_utf8 = TRUE;
                break;
            }

            /* Check if the end is less than one char */
            remainder = inbuf_size - (end - (gchar *)inbuf);
            if (remainder < 6)
            {
                stream->priv->is_utf8 = TRUE;
                break;
            }

            continue;
        }

        conv = g_charset_converter_new ("UTF-8", xed_encoding_get_charset (enc), NULL);

        /* If we tried all encodings we use the first one */
        if (stream->priv->use_first)
        {
            break;
        }

        /* Try to convert */
        if (try_convert (conv, inbuf, inbuf_size))
        {
            break;
        }
    }

    if (conv != NULL)
    {
        g_converter_reset (G_CONVERTER (conv));
    }

    return conv;
 }

static XedDocumentNewlineType
get_newline_type (GtkTextIter *end)
{
    XedDocumentNewlineType res;
    GtkTextIter copy;
    gunichar c;

    copy = *end;
    c = gtk_text_iter_get_char (&copy);

    if (g_unichar_break_type (c) == G_UNICODE_BREAK_CARRIAGE_RETURN)
    {
        if (gtk_text_iter_forward_char (&copy) &&
            g_unichar_break_type (gtk_text_iter_get_char (&copy)) == G_UNICODE_BREAK_LINE_FEED)
        {
            res = XED_DOCUMENT_NEWLINE_TYPE_CR_LF;
        }
        else
        {
            res = XED_DOCUMENT_NEWLINE_TYPE_CR;
        }
    }
    else
    {
        res = XED_DOCUMENT_NEWLINE_TYPE_LF;
    }

    return res;
}

GOutputStream *
xed_document_output_stream_new (XedDocument *doc,
                                GSList      *candidate_encodings)
{
    XedDocumentOutputStream *stream;

    stream = g_object_new (XED_TYPE_DOCUMENT_OUTPUT_STREAM, "document", doc, NULL);

    stream->priv->encodings = g_slist_copy (candidate_encodings);

    return G_OUTPUT_STREAM (stream);
}

XedDocumentNewlineType
xed_document_output_stream_detect_newline_type (XedDocumentOutputStream *stream)
{
    XedDocumentNewlineType type;
    GtkTextIter iter;

    g_return_val_if_fail (XED_IS_DOCUMENT_OUTPUT_STREAM (stream), XED_DOCUMENT_NEWLINE_TYPE_DEFAULT);

    type = XED_DOCUMENT_NEWLINE_TYPE_DEFAULT;

    gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (stream->priv->doc), &iter);

    if (gtk_text_iter_ends_line (&iter) || gtk_text_iter_forward_to_line_end (&iter))
    {
        type = get_newline_type (&iter);
    }

    return type;
}

const XedEncoding *
xed_document_output_stream_get_guessed (XedDocumentOutputStream *stream)
{
    g_return_val_if_fail (XED_IS_DOCUMENT_OUTPUT_STREAM (stream), NULL);

    if (stream->priv->current_encoding != NULL)
    {
        return (const XedEncoding *)stream->priv->current_encoding->data;
    }
    else if (stream->priv->is_utf8 || !stream->priv->is_initialized)
    {
        /* If it is not initialized we assume that we are trying to convert
           the empty string */
        return xed_encoding_get_utf8 ();
    }

    return NULL;
}

guint
xed_document_output_stream_get_num_fallbacks (XedDocumentOutputStream *stream)
{
    g_return_val_if_fail (XED_IS_DOCUMENT_OUTPUT_STREAM (stream), FALSE);

    if (stream->priv->charset_conv == NULL)
    {
        return FALSE;
    }

    return g_charset_converter_get_num_fallbacks (stream->priv->charset_conv) != 0;
}

/* If the last char is a newline, remove it from the buffer (otherwise
   GtkTextView shows it as an empty line). See bug #324942. */
static void
remove_ending_newline (XedDocumentOutputStream *stream)
{
    GtkTextIter end;
    GtkTextIter start;

    gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (stream->priv->doc), &end);
    start = end;

    gtk_text_iter_set_line_offset (&start, 0);

    if (gtk_text_iter_ends_line (&start) && gtk_text_iter_backward_line (&start))
    {
        if (!gtk_text_iter_ends_line (&start))
        {
            gtk_text_iter_forward_to_line_end (&start);
        }

        /* Delete the empty line which is from 'start' to 'end' */
        gtk_text_buffer_delete (GTK_TEXT_BUFFER (stream->priv->doc), &start, &end);
    }
}

static void
end_append_text_to_document (XedDocumentOutputStream *stream)
{
    remove_ending_newline (stream);

    gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (stream->priv->doc), FALSE);

    gtk_source_buffer_end_not_undoable_action (GTK_SOURCE_BUFFER (stream->priv->doc));
}

static gssize
xed_document_output_stream_write (GOutputStream *stream,
                                  const void    *buffer,
                                  gsize          count,
                                  GCancellable  *cancellable,
                                  GError        **error)
{
    XedDocumentOutputStream *ostream;
    gchar *text;
    gsize len;
    gboolean freetext = FALSE;
    const gchar *end;
    gboolean valid;
    gsize nvalid;
    gsize remainder;

    if (g_cancellable_set_error_if_cancelled (cancellable, error))
    {
        return -1;
    }

    ostream = XED_DOCUMENT_OUTPUT_STREAM (stream);

    if (!ostream->priv->is_initialized)
    {
        ostream->priv->charset_conv = guess_encoding (ostream, buffer, count);

        /* If we still have the previous case is that we didn't guess
          anything */
        if (ostream->priv->charset_conv == NULL && !ostream->priv->is_utf8)
        {
            /* FIXME: Add a different domain when we kill xed_convert */
            g_set_error_literal (error, XED_DOCUMENT_ERROR,
                                 XED_DOCUMENT_ERROR_ENCODING_AUTO_DETECTION_FAILED,
                                 _("It is not possible to detect the encoding automatically"));
            return -1;
        }

        /* Do not initialize iconv if we are not going to conver anything */
        if (!ostream->priv->is_utf8)
        {
            gchar *from_charset;

            /* Initialize iconv */
            g_object_get (G_OBJECT (ostream->priv->charset_conv), "from-charset", &from_charset, NULL);

            ostream->priv->iconv = g_iconv_open ("UTF-8", from_charset);

            if (ostream->priv->iconv == (GIConv)-1)
            {
                if (errno == EINVAL)
                {
                    g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                                 _("Conversion from character set '%s' to 'UTF-8' is not supported"),
                                 from_charset);
                }
                else
                {
                    g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                                 _("Could not open converter from '%s' to 'UTF-8'"),
                                 from_charset);
                }

                g_free (from_charset);

                return -1;
            }

            g_free (from_charset);
        }

        /* Init the undoable action */
        gtk_source_buffer_begin_not_undoable_action (GTK_SOURCE_BUFFER (ostream->priv->doc));

        gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (ostream->priv->doc), &ostream->priv->pos);
        ostream->priv->is_initialized = TRUE;
    }

    if (ostream->priv->buflen > 0)
    {
        len = ostream->priv->buflen + count;
        text = g_new (gchar , len + 1);

        memcpy (text, ostream->priv->buffer, ostream->priv->buflen);
        memcpy (text + ostream->priv->buflen, buffer, count);

        text[len] = '\0';

        g_free (ostream->priv->buffer);

        ostream->priv->buffer = NULL;
        ostream->priv->buflen = 0;

        freetext = TRUE;
    }
    else
    {
        text = (gchar *) buffer;
        len = count;
    }

    if (!ostream->priv->is_utf8)
    {
        gchar *conv_text;
        gsize conv_read;
        gsize conv_written;
        GError *err = NULL;

        if (ostream->priv->iconv == NULL)
        {
            g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_INITIALIZED, _("Invalid object, not initialized"));

            if (freetext)
            {
                g_free (text);
            }

            return -1;
        }

        /* If we reached here is because we need to convert the text so, we
          convert it with the charset converter */
        conv_text = g_convert_with_iconv (text,
                                         len,
                                         ostream->priv->iconv,
                                         &conv_read,
                                         &conv_written,
                                         &err);

        if (freetext)
        {
           g_free (text);
        }

        if (err != NULL)
        {
            remainder = len - conv_read;

            /* Store the partial char for the next conversion */
            if (err->code == G_CONVERT_ERROR_ILLEGAL_SEQUENCE &&
               remainder < MAX_UNICHAR_LEN &&
               (g_utf8_get_char_validated (text + conv_read, remainder) == (gunichar)-2))
            {
                ostream->priv->buffer = g_strndup (text + conv_read, remainder);
                ostream->priv->buflen = remainder;
            }
            else
            {
                /* Something went wrong with the conversion,
                   propagate the error and finish */
                g_propagate_error (error, err);
                g_free (conv_text);

                return -1;
            }
        }

        text = conv_text;
        len = conv_written;
        freetext = TRUE;
    }


    /* validate */
    valid = g_utf8_validate (text, len, &end);
    nvalid = end - text;

    /* Avoid keeping a CRLF across two buffers. */
    if (valid && len > 1 && end[-1] == '\r')
    {
        valid = FALSE;
        end--;
    }

    if (!valid)
    {
        // gsize nvalid = end - text;
        remainder = len - nvalid;
        gunichar ch;

        if ((remainder < MAX_UNICHAR_LEN) &&
            ((ch = g_utf8_get_char_validated (text + nvalid, remainder)) == (gunichar)-2 ||
             ch == (gunichar)'\r'))
        {
            ostream->priv->buffer = g_strndup (end, remainder);
            ostream->priv->buflen = remainder;
            len -= remainder;
        }
        else
        {
            /* TODO: we could escape invalid text and tag it in red
             * and make the doc readonly.
             */
            g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA, _("Invalid UTF-8 sequence in input"));

            if (freetext)
            {
                g_free (text);
            }

            return -1;
        }
    }

    gtk_text_buffer_insert (GTK_TEXT_BUFFER (ostream->priv->doc), &ostream->priv->pos, text, len);

    if (freetext)
    {
        g_free (text);
    }

    return count;
}

static gboolean
xed_document_output_stream_flush (GOutputStream *stream,
                                  GCancellable  *cancellable,
                                  GError        **error)
{
    XedDocumentOutputStream *ostream = XED_DOCUMENT_OUTPUT_STREAM (stream);

    /* Flush deferred data if some. */
    if (!ostream->priv->is_closed && ostream->priv->is_initialized &&
        ostream->priv->buflen > 0 &&
        xed_document_output_stream_write (stream, "", 0, cancellable, error) == -1)
    {
        return FALSE;
    }

    return TRUE;
}

static gboolean
xed_document_output_stream_close (GOutputStream *stream,
                                  GCancellable  *cancellable,
                                  GError       **error)
{
    XedDocumentOutputStream *ostream = XED_DOCUMENT_OUTPUT_STREAM (stream);

    if (!ostream->priv->is_closed && ostream->priv->is_initialized)
    {
        end_append_text_to_document (ostream);
        ostream->priv->is_closed = TRUE;
    }

    if (ostream->priv->buflen > 0)
    {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA, _("Incomplete UTF-8 sequence in input"));
        return FALSE;
    }

    return TRUE;
}
