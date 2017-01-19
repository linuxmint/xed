/*
 * document-output-stream.c
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


#include "xed-document-output-stream.h"
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <string.h>

static void
test_consecutive_write (const gchar *inbuf,
			const gchar *outbuf,
			gsize write_chunk_len,
			XedDocumentNewlineType newline_type)
{
	XedDocument *doc;
	GOutputStream *out;
	gsize len;
	gssize n, w;
	GError *err = NULL;
	gchar *b;
	XedDocumentNewlineType type;
	GSList *encodings = NULL;

	doc = xed_document_new ();
	encodings = g_slist_prepend (encodings, (gpointer)xed_encoding_get_utf8 ());
	out = xed_document_output_stream_new (doc, encodings);

	n = 0;

	do
	{
		len = MIN (write_chunk_len, strlen (inbuf + n));
		w = g_output_stream_write (out, inbuf + n, len, NULL, &err);
		g_assert_cmpint (w, >=, 0);
		g_assert_no_error (err);

		n += w;
	} while (w != 0);

	g_assert(g_output_stream_flush (out, NULL, &err) == TRUE);
	g_assert_no_error (err);

	g_object_get (G_OBJECT (doc), "text", &b, NULL);

	g_assert_cmpstr (inbuf, ==, b);
	g_free (b);

	type = xed_document_output_stream_detect_newline_type (XED_DOCUMENT_OUTPUT_STREAM (out));
	g_assert (type == newline_type);

	g_output_stream_close (out, NULL, &err);
	g_assert_no_error (err);

	g_object_get (G_OBJECT (doc), "text", &b, NULL);

	g_assert_cmpstr (outbuf, ==, b);
	g_free (b);

	g_assert (gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (doc)) == FALSE);

	g_object_unref (doc);
	g_object_unref (out);
}

static void
test_empty ()
{
	test_consecutive_write ("", "", 10, XED_DOCUMENT_NEWLINE_TYPE_DEFAULT);
	test_consecutive_write ("\r\n", "", 10, XED_DOCUMENT_NEWLINE_TYPE_CR_LF);
	test_consecutive_write ("\r", "", 10, XED_DOCUMENT_NEWLINE_TYPE_CR);
	test_consecutive_write ("\n", "", 10, XED_DOCUMENT_NEWLINE_TYPE_LF);
}

static void
test_consecutive ()
{
	test_consecutive_write ("hello\nhow\nare\nyou", "hello\nhow\nare\nyou", 3,
				XED_DOCUMENT_NEWLINE_TYPE_LF);
	test_consecutive_write ("hello\rhow\rare\ryou", "hello\rhow\rare\ryou", 3,
				XED_DOCUMENT_NEWLINE_TYPE_CR);
	test_consecutive_write ("hello\r\nhow\r\nare\r\nyou", "hello\r\nhow\r\nare\r\nyou", 3,
				XED_DOCUMENT_NEWLINE_TYPE_CR_LF);
}

static void
test_consecutive_tnewline ()
{
	test_consecutive_write ("hello\nhow\nare\nyou\n", "hello\nhow\nare\nyou", 3,
				XED_DOCUMENT_NEWLINE_TYPE_LF);
	test_consecutive_write ("hello\rhow\rare\ryou\r", "hello\rhow\rare\ryou", 3,
				XED_DOCUMENT_NEWLINE_TYPE_CR);
	test_consecutive_write ("hello\r\nhow\r\nare\r\nyou\r\n", "hello\r\nhow\r\nare\r\nyou", 3,
				XED_DOCUMENT_NEWLINE_TYPE_CR_LF);
}

static void
test_big_char ()
{
	test_consecutive_write ("\343\203\200\343\203\200", "\343\203\200\343\203\200", 2,
				XED_DOCUMENT_NEWLINE_TYPE_LF);
}

/* SMART CONVERSION */

#define TEXT_TO_CONVERT "this is some text to make the tests"
#define TEXT_TO_GUESS "hello \xe6\x96\x87 world"

static void
print_hex (gchar *ptr, gint len)
{
	gint i;

	for (i = 0; i < len; ++i)
	{
		g_printf ("\\x%02x", (unsigned char)ptr[i]);
	}

	g_printf ("\n");
}

static gchar *
get_encoded_text (const gchar         *text,
                  gsize                nread,
                  const XedEncoding *to,
                  const XedEncoding *from,
                  gsize               *bytes_written_aux,
                  gboolean             care_about_error)
{
	GCharsetConverter *converter;
	gchar *out, *out_aux;
	gsize bytes_read, bytes_read_aux;
	gsize bytes_written;
	GConverterResult res;
	GError *err;

	converter = g_charset_converter_new (xed_encoding_get_charset (to),
					     xed_encoding_get_charset (from),
					     NULL);

	out = g_malloc (200);
	out_aux = g_malloc (200);
	err = NULL;
	bytes_read_aux = 0;
	*bytes_written_aux = 0;

	if (nread == -1)
	{
		nread = strlen (text);
	}

	do
	{
		res = g_converter_convert (G_CONVERTER (converter),
		                           text + bytes_read_aux,
		                           nread,
		                           out_aux,
		                           200,
		                           G_CONVERTER_INPUT_AT_END,
		                           &bytes_read,
		                           &bytes_written,
		                           &err);
		memcpy (out + *bytes_written_aux, out_aux, bytes_written);
		bytes_read_aux += bytes_read;
		*bytes_written_aux += bytes_written;
		nread -= bytes_read;
	} while (res != G_CONVERTER_FINISHED && res != G_CONVERTER_ERROR);

	if (care_about_error)
	{
		g_assert_no_error (err);
	}
	else if (err)
	{
		g_printf ("** You don't care, but there was an error: %s", err->message);
		return NULL;
	}

	out[*bytes_written_aux] = '\0';

	if (!g_utf8_validate (out, *bytes_written_aux, NULL) && !care_about_error)
	{
		if (!care_about_error)
		{
			return NULL;
		}
		else
		{
			g_assert_not_reached ();
		}
	}

	return out;
}

static GSList *
get_all_encodings ()
{
	GSList *encs = NULL;
	gint i = 0;

	while (TRUE)
	{
		const XedEncoding *enc;

		enc = xed_encoding_get_from_index (i);

		if (enc == NULL)
			break;

		encs = g_slist_prepend (encs, (gpointer)enc);
		i++;
	}

	return encs;
}

static gchar *
do_test (const gchar *test_in,
         const gchar *enc,
         GSList      *encodings,
         gsize        nread,
         const XedEncoding **guessed)
{
	XedDocument *doc;
	GOutputStream *out;
	GError *err = NULL;
	GtkTextIter start, end;
	gchar *text;

	if (enc != NULL)
	{
		encodings = NULL;
		encodings = g_slist_prepend (encodings, (gpointer)xed_encoding_get_from_charset (enc));
	}

	doc = xed_document_new ();
	encodings = g_slist_prepend (encodings, (gpointer)xed_encoding_get_utf8 ());
	out = xed_document_output_stream_new (doc, encodings);

	g_output_stream_write (out, test_in, nread, NULL, &err);
	g_assert_no_error (err);

	g_output_stream_flush (out, NULL, &err);
	g_assert_no_error (err);

	g_output_stream_close (out, NULL, &err);
	g_assert_no_error (err);

	if (guessed != NULL)
		*guessed = xed_document_output_stream_get_guessed (XED_DOCUMENT_OUTPUT_STREAM (out));

	gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (doc), &start, &end);
	text = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (doc),
	                                 &start,
	                                 &end,
	                                 FALSE);

	g_object_unref (doc);
	g_object_unref (out);

	return text;
}

static void
test_utf8_utf8 ()
{
	gchar *aux;

	aux = do_test (TEXT_TO_CONVERT, "UTF-8", NULL, strlen (TEXT_TO_CONVERT), NULL);
	g_assert_cmpstr (aux, ==, TEXT_TO_CONVERT);

	aux = do_test ("foobar\xc3\xa8\xc3\xa8\xc3\xa8zzzzzz", "UTF-8", NULL, 18, NULL);
	g_assert_cmpstr (aux, ==, "foobar\xc3\xa8\xc3\xa8\xc3\xa8zzzzzz");

	aux = do_test ("foobar\xc3\xa8\xc3\xa8\xc3\xa8zzzzzz", "UTF-8", NULL, 12, NULL);
	g_assert_cmpstr (aux, ==, "foobar\xc3\xa8\xc3\xa8\xc3\xa8");

	/* FIXME: Use the utf8 stream for a fallback? */
	//do_test_with_error ("\xef\xbf\xbezzzzzz", encs, G_IO_ERROR_FAILED);
}

static void
test_empty_conversion ()
{
	const XedEncoding *guessed;
	gchar *out;
	GSList *encodings = NULL;

	/* testing the case of an empty file and list of encodings with no
	   utf-8. In this case, the smart converter cannot determine the right
	   encoding (because there is no input), but should still default to
	   utf-8 for the detection */
	encodings = g_slist_prepend (encodings, (gpointer)xed_encoding_get_from_charset ("UTF-16"));
	encodings = g_slist_prepend (encodings, (gpointer)xed_encoding_get_from_charset ("ISO-8859-15"));

	out = do_test ("", NULL, encodings, 0, &guessed);

	g_assert_cmpstr (out, ==, "");

	g_assert (guessed == xed_encoding_get_utf8 ());
}

static void
test_guessed ()
{
	GSList *encs = NULL;
	gchar *aux, *aux2, *fail;
	gsize aux_len, fail_len;
	const XedEncoding *guessed;

	aux = get_encoded_text (TEXT_TO_GUESS, -1,
	                        xed_encoding_get_from_charset ("UTF-16"),
	                        xed_encoding_get_from_charset ("UTF-8"),
	                        &aux_len,
	                        TRUE);

	fail = get_encoded_text (aux, aux_len,
	                         xed_encoding_get_from_charset ("UTF-8"),
	                         xed_encoding_get_from_charset ("ISO-8859-15"),
	                         &fail_len,
	                         FALSE);

	g_assert (fail == NULL);

	/* ISO-8859-15 should fail */
	encs = g_slist_append (encs, (gpointer)xed_encoding_get_from_charset ("ISO-8859-15"));
	encs = g_slist_append (encs, (gpointer)xed_encoding_get_from_charset ("UTF-16"));

	aux2 = do_test (aux, NULL, encs, aux_len, &guessed);

	g_assert (guessed == xed_encoding_get_from_charset ("UTF-16"));
}

int main (int   argc,
          char *argv[])
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/document-output-stream/empty", test_empty);

	g_test_add_func ("/document-output-stream/consecutive", test_consecutive);
	g_test_add_func ("/document-output-stream/consecutive_tnewline", test_consecutive_tnewline);
	g_test_add_func ("/document-output-stream/big-char", test_big_char);

	g_test_add_func ("/document-output-stream/smart conversion: utf8-utf8", test_utf8_utf8);
	g_test_add_func ("/document-output-stream/smart conversion: guessed", test_guessed);
	g_test_add_func ("/document-output-stream/smart conversion: empty", test_empty_conversion);

	return g_test_run ();
}
