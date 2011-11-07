/*
 * smart-converter.c
 * This file is part of gedit
 *
 * Copyright (C) 2009 - Ignacio Casal Quinteiro
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


#include "gedit-smart-charset-converter.h"
#include "gedit-encodings.h"
#include <gio/gio.h>
#include <glib.h>
#include <string.h>

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
		  const GeditEncoding *to,
		  const GeditEncoding *from,
		  gsize               *bytes_written_aux,
		  gboolean             care_about_error)
{
	GCharsetConverter *converter;
	gchar *out, *out_aux;
	gsize bytes_read, bytes_read_aux;
	gsize bytes_written;
	GConverterResult res;
	GError *err;

	converter = g_charset_converter_new (gedit_encoding_get_charset (to),
					     gedit_encoding_get_charset (from),
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
		const GeditEncoding *enc;

		enc = gedit_encoding_get_from_index (i);

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
         const GeditEncoding **guessed)
{
	GeditSmartCharsetConverter *converter;
	gchar *out, *out_aux;
	gsize bytes_read, bytes_read_aux;
	gsize bytes_written, bytes_written_aux;
	GConverterResult res;
	GError *err;

	if (enc != NULL)
	{
		encodings = NULL;
		encodings = g_slist_prepend (encodings, (gpointer)gedit_encoding_get_from_charset (enc));
	}

	converter = gedit_smart_charset_converter_new (encodings);

	out = g_malloc (200);
	out_aux = g_malloc (200);
	err = NULL;
	bytes_read_aux = 0;
	bytes_written_aux = 0;

	do
	{
		res = g_converter_convert (G_CONVERTER (converter),
		                           test_in + bytes_read_aux,
		                           nread,
		                           out_aux,
		                           200,
		                           G_CONVERTER_INPUT_AT_END,
		                           &bytes_read,
		                           &bytes_written,
		                           &err);
		memcpy (out + bytes_written_aux, out_aux, bytes_written);
		bytes_read_aux += bytes_read;
		bytes_written_aux += bytes_written;
		nread -= bytes_read;
	} while (res != G_CONVERTER_FINISHED && res != G_CONVERTER_ERROR);

	g_assert_no_error (err);
	out[bytes_written_aux] = '\0';

	if (guessed != NULL)
		*guessed = gedit_smart_charset_converter_get_guessed (converter);

	return out;
}

static void
do_test_roundtrip (const char *str, const char *charset)
{
	gsize len;
	gchar *buf, *p;
	GInputStream *in, *tmp;
	GCharsetConverter *c1;
	GeditSmartCharsetConverter *c2;
	gsize n, tot;
	GError *err;
	GSList *enc = NULL;

	len = strlen(str);
	buf = g_new0 (char, len);

	in = g_memory_input_stream_new_from_data (str, -1, NULL);

	c1 = g_charset_converter_new (charset, "UTF-8", NULL);

	tmp = in;
	in = g_converter_input_stream_new (in, G_CONVERTER (c1));
	g_object_unref (tmp);
	g_object_unref (c1);

	enc = g_slist_prepend (enc, (gpointer)gedit_encoding_get_from_charset (charset));
	c2 = gedit_smart_charset_converter_new (enc);
	g_slist_free (enc);

	tmp = in;
	in = g_converter_input_stream_new (in, G_CONVERTER (c2));
	g_object_unref (tmp);
	g_object_unref (c2);

	tot = 0;
	p = buf;
	n = len;
	while (TRUE)
	{
		gssize res;

		err = NULL;
		res = g_input_stream_read (in, p, n, NULL, &err);
		g_assert_no_error (err);
		if (res == 0)
		break;

		p += res;
		n -= res;
		tot += res;
	}

	g_assert_cmpint (tot, ==, len);
	g_assert_cmpstr (str, ==, buf);

	g_free (buf);
	g_object_unref (in);
}

static void
test_utf8_utf8 ()
{
	gchar *aux;

	aux = do_test (TEXT_TO_CONVERT, "UTF-8", NULL, strlen (TEXT_TO_CONVERT), NULL);
	g_assert_cmpstr (aux, ==, TEXT_TO_CONVERT);

	aux = do_test ("foobar\xc3\xa8\xc3\xa8\xc3\xa8zzzzzz", "UTF-8", NULL, 18, NULL);
	g_assert_cmpstr (aux, ==, "foobar\xc3\xa8\xc3\xa8\xc3\xa8zzzzzz");

	aux = do_test ("foobar\xc3\xa8\xc3\xa8\xc3\xa8zzzzzz", "UTF-8", NULL, 9, NULL);
	g_assert_cmpstr (aux, ==, "foobar\xc3\xa8\xc3");

	/* FIXME: Use the utf8 stream for a fallback? */
	//do_test_with_error ("\xef\xbf\xbezzzzzz", encs, G_IO_ERROR_FAILED);
}

static void
test_xxx_xxx ()
{
	GSList *encs, *l;

	encs = get_all_encodings ();

	/* Here we just test all encodings it is just to know that the conversions
	   are done ok */
	for (l = encs; l != NULL; l = g_slist_next (l))
	{
		do_test_roundtrip (TEXT_TO_CONVERT, gedit_encoding_get_charset ((const GeditEncoding *)l->data));
	}

	g_slist_free (encs);
}

static void
test_empty ()
{
	const GeditEncoding *guessed;
	gchar *out;
	GSList *encodings = NULL;

	/* testing the case of an empty file and list of encodings with no
	   utf-8. In this case, the smart converter cannot determine the right
	   encoding (because there is no input), but should still default to
	   utf-8 for the detection */
	encodings = g_slist_prepend (encodings, (gpointer)gedit_encoding_get_from_charset ("UTF-16"));
	encodings = g_slist_prepend (encodings, (gpointer)gedit_encoding_get_from_charset ("ISO-8859-15"));

	out = do_test ("", NULL, encodings, 0, &guessed);

	g_assert_cmpstr (out, ==, "");

	g_assert (guessed == gedit_encoding_get_utf8 ());
}

static void
test_guessed ()
{
	GSList *encs = NULL;
	gchar *aux, *aux2, *fail;
	gsize aux_len, fail_len;
	const GeditEncoding *guessed;

	aux = get_encoded_text (TEXT_TO_GUESS, -1,
	                        gedit_encoding_get_from_charset ("UTF-16"),
	                        gedit_encoding_get_from_charset ("UTF-8"),
	                        &aux_len,
	                        TRUE);

	fail = get_encoded_text (aux, aux_len,
	                         gedit_encoding_get_from_charset ("UTF-8"),
	                         gedit_encoding_get_from_charset ("ISO-8859-15"),
	                         &fail_len,
	                         FALSE);

	g_assert (fail == NULL);

	/* ISO-8859-15 should fail */
	encs = g_slist_append (encs, (gpointer)gedit_encoding_get_from_charset ("ISO-8859-15"));
	encs = g_slist_append (encs, (gpointer)gedit_encoding_get_from_charset ("UTF-16"));

	aux2 = do_test (aux, NULL, encs, aux_len, &guessed);

	g_assert (guessed == gedit_encoding_get_from_charset ("UTF-16"));
}

int main (int   argc,
          char *argv[])
{
	g_type_init ();
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/smart-converter/utf8-utf8", test_utf8_utf8);
	//g_test_add_func ("/smart-converter/xxx-xxx", test_xxx_xxx);
	g_test_add_func ("/smart-converter/guessed", test_guessed);
	g_test_add_func ("/smart-converter/empty", test_empty);

	return g_test_run ();
}
