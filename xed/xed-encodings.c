/*
 * xed-encodings.c
 * This file is part of xed
 *
 * Copyright (C) 2002-2005 Paolo Maggi 
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
 * Modified by the xed Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the xed Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <glib/gi18n.h>

#include "xed-encodings.h"


struct _XedEncoding
{
	gint   index;
	const gchar *charset;
	const gchar *name;
};

/* 
 * The original versions of the following tables are taken from profterm 
 *
 * Copyright (C) 2002 Red Hat, Inc.
 */

typedef enum
{

  XED_ENCODING_ISO_8859_1,
  XED_ENCODING_ISO_8859_2,
  XED_ENCODING_ISO_8859_3,
  XED_ENCODING_ISO_8859_4,
  XED_ENCODING_ISO_8859_5,
  XED_ENCODING_ISO_8859_6,
  XED_ENCODING_ISO_8859_7,
  XED_ENCODING_ISO_8859_8,
  XED_ENCODING_ISO_8859_9,
  XED_ENCODING_ISO_8859_10,
  XED_ENCODING_ISO_8859_13,
  XED_ENCODING_ISO_8859_14,
  XED_ENCODING_ISO_8859_15,
  XED_ENCODING_ISO_8859_16,

  XED_ENCODING_UTF_7,
  XED_ENCODING_UTF_16,
  XED_ENCODING_UTF_16_BE,
  XED_ENCODING_UTF_16_LE,
  XED_ENCODING_UTF_32,
  XED_ENCODING_UCS_2,
  XED_ENCODING_UCS_4,

  XED_ENCODING_ARMSCII_8,
  XED_ENCODING_BIG5,
  XED_ENCODING_BIG5_HKSCS,
  XED_ENCODING_CP_866,

  XED_ENCODING_EUC_JP,
  XED_ENCODING_EUC_JP_MS,
  XED_ENCODING_CP932,
  XED_ENCODING_EUC_KR,
  XED_ENCODING_EUC_TW,

  XED_ENCODING_GB18030,
  XED_ENCODING_GB2312,
  XED_ENCODING_GBK,
  XED_ENCODING_GEOSTD8,

  XED_ENCODING_IBM_850,
  XED_ENCODING_IBM_852,
  XED_ENCODING_IBM_855,
  XED_ENCODING_IBM_857,
  XED_ENCODING_IBM_862,
  XED_ENCODING_IBM_864,

  XED_ENCODING_ISO_2022_JP,
  XED_ENCODING_ISO_2022_KR,
  XED_ENCODING_ISO_IR_111,
  XED_ENCODING_JOHAB,
  XED_ENCODING_KOI8_R,
  XED_ENCODING_KOI8__R,
  XED_ENCODING_KOI8_U,
  
  XED_ENCODING_SHIFT_JIS,
  XED_ENCODING_TCVN,
  XED_ENCODING_TIS_620,
  XED_ENCODING_UHC,
  XED_ENCODING_VISCII,

  XED_ENCODING_WINDOWS_1250,
  XED_ENCODING_WINDOWS_1251,
  XED_ENCODING_WINDOWS_1252,
  XED_ENCODING_WINDOWS_1253,
  XED_ENCODING_WINDOWS_1254,
  XED_ENCODING_WINDOWS_1255,
  XED_ENCODING_WINDOWS_1256,
  XED_ENCODING_WINDOWS_1257,
  XED_ENCODING_WINDOWS_1258,

  XED_ENCODING_LAST,

  XED_ENCODING_UTF_8,
  XED_ENCODING_UNKNOWN
  
} XedEncodingIndex;

static const XedEncoding utf8_encoding =  {
	XED_ENCODING_UTF_8,
	"UTF-8",
	N_("Unicode")
};

/* initialized in xed_encoding_lazy_init() */
static XedEncoding unknown_encoding = {
	XED_ENCODING_UNKNOWN,
	NULL, 
	NULL 
};

static const XedEncoding encodings [] = {

  { XED_ENCODING_ISO_8859_1,
    "ISO-8859-1", N_("Western") },
  { XED_ENCODING_ISO_8859_2,
   "ISO-8859-2", N_("Central European") },
  { XED_ENCODING_ISO_8859_3,
    "ISO-8859-3", N_("South European") },
  { XED_ENCODING_ISO_8859_4,
    "ISO-8859-4", N_("Baltic") },
  { XED_ENCODING_ISO_8859_5,
    "ISO-8859-5", N_("Cyrillic") },
  { XED_ENCODING_ISO_8859_6,
    "ISO-8859-6", N_("Arabic") },
  { XED_ENCODING_ISO_8859_7,
    "ISO-8859-7", N_("Greek") },
  { XED_ENCODING_ISO_8859_8,
    "ISO-8859-8", N_("Hebrew Visual") },
  { XED_ENCODING_ISO_8859_9,
    "ISO-8859-9", N_("Turkish") },
  { XED_ENCODING_ISO_8859_10,
    "ISO-8859-10", N_("Nordic") },
  { XED_ENCODING_ISO_8859_13,
    "ISO-8859-13", N_("Baltic") },
  { XED_ENCODING_ISO_8859_14,
    "ISO-8859-14", N_("Celtic") },
  { XED_ENCODING_ISO_8859_15,
    "ISO-8859-15", N_("Western") },
  { XED_ENCODING_ISO_8859_16,
    "ISO-8859-16", N_("Romanian") },

  { XED_ENCODING_UTF_7,
    "UTF-7", N_("Unicode") },
  { XED_ENCODING_UTF_16,
    "UTF-16", N_("Unicode") },
  { XED_ENCODING_UTF_16_BE,
    "UTF-16BE", N_("Unicode") },
  { XED_ENCODING_UTF_16_LE,
    "UTF-16LE", N_("Unicode") },
  { XED_ENCODING_UTF_32,
    "UTF-32", N_("Unicode") },
  { XED_ENCODING_UCS_2,
    "UCS-2", N_("Unicode") },
  { XED_ENCODING_UCS_4,
    "UCS-4", N_("Unicode") },

  { XED_ENCODING_ARMSCII_8,
    "ARMSCII-8", N_("Armenian") },
  { XED_ENCODING_BIG5,
    "BIG5", N_("Chinese Traditional") },
  { XED_ENCODING_BIG5_HKSCS,
    "BIG5-HKSCS", N_("Chinese Traditional") },
  { XED_ENCODING_CP_866,
    "CP866", N_("Cyrillic/Russian") },

  { XED_ENCODING_EUC_JP,
    "EUC-JP", N_("Japanese") },
  { XED_ENCODING_EUC_JP_MS,
    "EUC-JP-MS", N_("Japanese") },
  { XED_ENCODING_CP932,
    "CP932", N_("Japanese") },

  { XED_ENCODING_EUC_KR,
    "EUC-KR", N_("Korean") },
  { XED_ENCODING_EUC_TW,
    "EUC-TW", N_("Chinese Traditional") },

  { XED_ENCODING_GB18030,
    "GB18030", N_("Chinese Simplified") },
  { XED_ENCODING_GB2312,
    "GB2312", N_("Chinese Simplified") },
  { XED_ENCODING_GBK,
    "GBK", N_("Chinese Simplified") },
  { XED_ENCODING_GEOSTD8,
    "GEORGIAN-ACADEMY", N_("Georgian") }, /* FIXME GEOSTD8 ? */

  { XED_ENCODING_IBM_850,
    "IBM850", N_("Western") },
  { XED_ENCODING_IBM_852,
    "IBM852", N_("Central European") },
  { XED_ENCODING_IBM_855,
    "IBM855", N_("Cyrillic") },
  { XED_ENCODING_IBM_857,
    "IBM857", N_("Turkish") },
  { XED_ENCODING_IBM_862,
    "IBM862", N_("Hebrew") },
  { XED_ENCODING_IBM_864,
    "IBM864", N_("Arabic") },

  { XED_ENCODING_ISO_2022_JP,
    "ISO-2022-JP", N_("Japanese") },
  { XED_ENCODING_ISO_2022_KR,
    "ISO-2022-KR", N_("Korean") },
  { XED_ENCODING_ISO_IR_111,
    "ISO-IR-111", N_("Cyrillic") },
  { XED_ENCODING_JOHAB,
    "JOHAB", N_("Korean") },
  { XED_ENCODING_KOI8_R,
    "KOI8R", N_("Cyrillic") },
  { XED_ENCODING_KOI8__R,
    "KOI8-R", N_("Cyrillic") },
  { XED_ENCODING_KOI8_U,
    "KOI8U", N_("Cyrillic/Ukrainian") },
  
  { XED_ENCODING_SHIFT_JIS,
    "SHIFT_JIS", N_("Japanese") },
  { XED_ENCODING_TCVN,
    "TCVN", N_("Vietnamese") },
  { XED_ENCODING_TIS_620,
    "TIS-620", N_("Thai") },
  { XED_ENCODING_UHC,
    "UHC", N_("Korean") },
  { XED_ENCODING_VISCII,
    "VISCII", N_("Vietnamese") },

  { XED_ENCODING_WINDOWS_1250,
    "WINDOWS-1250", N_("Central European") },
  { XED_ENCODING_WINDOWS_1251,
    "WINDOWS-1251", N_("Cyrillic") },
  { XED_ENCODING_WINDOWS_1252,
    "WINDOWS-1252", N_("Western") },
  { XED_ENCODING_WINDOWS_1253,
    "WINDOWS-1253", N_("Greek") },
  { XED_ENCODING_WINDOWS_1254,
    "WINDOWS-1254", N_("Turkish") },
  { XED_ENCODING_WINDOWS_1255,
    "WINDOWS-1255", N_("Hebrew") },
  { XED_ENCODING_WINDOWS_1256,
    "WINDOWS-1256", N_("Arabic") },
  { XED_ENCODING_WINDOWS_1257,
    "WINDOWS-1257", N_("Baltic") },
  { XED_ENCODING_WINDOWS_1258,
    "WINDOWS-1258", N_("Vietnamese") }
};

static void
xed_encoding_lazy_init (void)
{
	static gboolean initialized = FALSE;
	const gchar *locale_charset;

	if (initialized)
		return;

	if (g_get_charset (&locale_charset) == FALSE)
	{
		unknown_encoding.charset = g_strdup (locale_charset);
	}

	initialized = TRUE;
}

const XedEncoding *
xed_encoding_get_from_charset (const gchar *charset)
{
	gint i;

	g_return_val_if_fail (charset != NULL, NULL);

	xed_encoding_lazy_init ();

	if (charset == NULL)
		return NULL;

	if (g_ascii_strcasecmp (charset, "UTF-8") == 0)
		return xed_encoding_get_utf8 ();

	i = 0; 
	while (i < XED_ENCODING_LAST)
	{
		if (g_ascii_strcasecmp (charset, encodings[i].charset) == 0)
			return &encodings[i];
      
		++i;
	}

	if (unknown_encoding.charset != NULL)
	{
		if (g_ascii_strcasecmp (charset, unknown_encoding.charset) == 0)
			return &unknown_encoding;
	}

	return NULL;
}

const XedEncoding *
xed_encoding_get_from_index (gint idx)
{
	g_return_val_if_fail (idx >= 0, NULL);

	if (idx >= XED_ENCODING_LAST)
		return NULL;

	xed_encoding_lazy_init ();

	return &encodings[idx];
}

const XedEncoding *
xed_encoding_get_utf8 (void)
{
	xed_encoding_lazy_init ();

	return &utf8_encoding;
}

const XedEncoding *
xed_encoding_get_current (void)
{
	static gboolean initialized = FALSE;
	static const XedEncoding *locale_encoding = NULL;

	const gchar *locale_charset;

	xed_encoding_lazy_init ();

	if (initialized != FALSE)
		return locale_encoding;

	if (g_get_charset (&locale_charset) == FALSE) 
	{
		g_return_val_if_fail (locale_charset != NULL, &utf8_encoding);
		
		locale_encoding = xed_encoding_get_from_charset (locale_charset);
	}
	else
	{
		locale_encoding = &utf8_encoding;
	}
	
	if (locale_encoding == NULL)
	{
		locale_encoding = &unknown_encoding;
	}

	g_return_val_if_fail (locale_encoding != NULL, NULL);

	initialized = TRUE;

	return locale_encoding;
}

gchar *
xed_encoding_to_string (const XedEncoding* enc)
{
	g_return_val_if_fail (enc != NULL, NULL);
	
	xed_encoding_lazy_init ();

	g_return_val_if_fail (enc->charset != NULL, NULL);

	if (enc->name != NULL)
	{
	    	return g_strdup_printf ("%s (%s)", _(enc->name), enc->charset);
	}
	else
	{
		if (g_ascii_strcasecmp (enc->charset, "ANSI_X3.4-1968") == 0)
			return g_strdup_printf ("US-ASCII (%s)", enc->charset);
		else
			return g_strdup (enc->charset);
	}
}

const gchar *
xed_encoding_get_charset (const XedEncoding* enc)
{
	g_return_val_if_fail (enc != NULL, NULL);

	xed_encoding_lazy_init ();

	g_return_val_if_fail (enc->charset != NULL, NULL);

	return enc->charset;
}

const gchar *
xed_encoding_get_name (const XedEncoding* enc)
{
	g_return_val_if_fail (enc != NULL, NULL);

	xed_encoding_lazy_init ();

	return (enc->name == NULL) ? _("Unknown") : _(enc->name);
}

/* These are to make language bindings happy. Since Encodings are
 * const, copy() just returns the same pointer and fres() doesn't
 * do nothing */

XedEncoding *
xed_encoding_copy (const XedEncoding *enc)
{
	g_return_val_if_fail (enc != NULL, NULL);

	return (XedEncoding *) enc;
}

void 
xed_encoding_free (XedEncoding *enc)
{
	g_return_if_fail (enc != NULL);
}

/**
 * xed_encoding_get_type:
 * 
 * Retrieves the GType object which is associated with the
 * #XedEncoding class.
 * 
 * Return value: the GType associated with #XedEncoding.
 **/
GType 
xed_encoding_get_type (void)
{
	static GType our_type = 0;

	if (!our_type)
		our_type = g_boxed_type_register_static (
			"XedEncoding",
			(GBoxedCopyFunc) xed_encoding_copy,
			(GBoxedFreeFunc) xed_encoding_free);

	return our_type;
} 

