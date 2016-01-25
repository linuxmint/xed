/*
 * xedit-encodings.c
 * This file is part of xedit
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
 * Modified by the xedit Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <glib/gi18n.h>

#include "xedit-encodings.h"


struct _XeditEncoding
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

  XEDIT_ENCODING_ISO_8859_1,
  XEDIT_ENCODING_ISO_8859_2,
  XEDIT_ENCODING_ISO_8859_3,
  XEDIT_ENCODING_ISO_8859_4,
  XEDIT_ENCODING_ISO_8859_5,
  XEDIT_ENCODING_ISO_8859_6,
  XEDIT_ENCODING_ISO_8859_7,
  XEDIT_ENCODING_ISO_8859_8,
  XEDIT_ENCODING_ISO_8859_9,
  XEDIT_ENCODING_ISO_8859_10,
  XEDIT_ENCODING_ISO_8859_13,
  XEDIT_ENCODING_ISO_8859_14,
  XEDIT_ENCODING_ISO_8859_15,
  XEDIT_ENCODING_ISO_8859_16,

  XEDIT_ENCODING_UTF_7,
  XEDIT_ENCODING_UTF_16,
  XEDIT_ENCODING_UTF_16_BE,
  XEDIT_ENCODING_UTF_16_LE,
  XEDIT_ENCODING_UTF_32,
  XEDIT_ENCODING_UCS_2,
  XEDIT_ENCODING_UCS_4,

  XEDIT_ENCODING_ARMSCII_8,
  XEDIT_ENCODING_BIG5,
  XEDIT_ENCODING_BIG5_HKSCS,
  XEDIT_ENCODING_CP_866,

  XEDIT_ENCODING_EUC_JP,
  XEDIT_ENCODING_EUC_JP_MS,
  XEDIT_ENCODING_CP932,
  XEDIT_ENCODING_EUC_KR,
  XEDIT_ENCODING_EUC_TW,

  XEDIT_ENCODING_GB18030,
  XEDIT_ENCODING_GB2312,
  XEDIT_ENCODING_GBK,
  XEDIT_ENCODING_GEOSTD8,

  XEDIT_ENCODING_IBM_850,
  XEDIT_ENCODING_IBM_852,
  XEDIT_ENCODING_IBM_855,
  XEDIT_ENCODING_IBM_857,
  XEDIT_ENCODING_IBM_862,
  XEDIT_ENCODING_IBM_864,

  XEDIT_ENCODING_ISO_2022_JP,
  XEDIT_ENCODING_ISO_2022_KR,
  XEDIT_ENCODING_ISO_IR_111,
  XEDIT_ENCODING_JOHAB,
  XEDIT_ENCODING_KOI8_R,
  XEDIT_ENCODING_KOI8__R,
  XEDIT_ENCODING_KOI8_U,
  
  XEDIT_ENCODING_SHIFT_JIS,
  XEDIT_ENCODING_TCVN,
  XEDIT_ENCODING_TIS_620,
  XEDIT_ENCODING_UHC,
  XEDIT_ENCODING_VISCII,

  XEDIT_ENCODING_WINDOWS_1250,
  XEDIT_ENCODING_WINDOWS_1251,
  XEDIT_ENCODING_WINDOWS_1252,
  XEDIT_ENCODING_WINDOWS_1253,
  XEDIT_ENCODING_WINDOWS_1254,
  XEDIT_ENCODING_WINDOWS_1255,
  XEDIT_ENCODING_WINDOWS_1256,
  XEDIT_ENCODING_WINDOWS_1257,
  XEDIT_ENCODING_WINDOWS_1258,

  XEDIT_ENCODING_LAST,

  XEDIT_ENCODING_UTF_8,
  XEDIT_ENCODING_UNKNOWN
  
} XeditEncodingIndex;

static const XeditEncoding utf8_encoding =  {
	XEDIT_ENCODING_UTF_8,
	"UTF-8",
	N_("Unicode")
};

/* initialized in xedit_encoding_lazy_init() */
static XeditEncoding unknown_encoding = {
	XEDIT_ENCODING_UNKNOWN,
	NULL, 
	NULL 
};

static const XeditEncoding encodings [] = {

  { XEDIT_ENCODING_ISO_8859_1,
    "ISO-8859-1", N_("Western") },
  { XEDIT_ENCODING_ISO_8859_2,
   "ISO-8859-2", N_("Central European") },
  { XEDIT_ENCODING_ISO_8859_3,
    "ISO-8859-3", N_("South European") },
  { XEDIT_ENCODING_ISO_8859_4,
    "ISO-8859-4", N_("Baltic") },
  { XEDIT_ENCODING_ISO_8859_5,
    "ISO-8859-5", N_("Cyrillic") },
  { XEDIT_ENCODING_ISO_8859_6,
    "ISO-8859-6", N_("Arabic") },
  { XEDIT_ENCODING_ISO_8859_7,
    "ISO-8859-7", N_("Greek") },
  { XEDIT_ENCODING_ISO_8859_8,
    "ISO-8859-8", N_("Hebrew Visual") },
  { XEDIT_ENCODING_ISO_8859_9,
    "ISO-8859-9", N_("Turkish") },
  { XEDIT_ENCODING_ISO_8859_10,
    "ISO-8859-10", N_("Nordic") },
  { XEDIT_ENCODING_ISO_8859_13,
    "ISO-8859-13", N_("Baltic") },
  { XEDIT_ENCODING_ISO_8859_14,
    "ISO-8859-14", N_("Celtic") },
  { XEDIT_ENCODING_ISO_8859_15,
    "ISO-8859-15", N_("Western") },
  { XEDIT_ENCODING_ISO_8859_16,
    "ISO-8859-16", N_("Romanian") },

  { XEDIT_ENCODING_UTF_7,
    "UTF-7", N_("Unicode") },
  { XEDIT_ENCODING_UTF_16,
    "UTF-16", N_("Unicode") },
  { XEDIT_ENCODING_UTF_16_BE,
    "UTF-16BE", N_("Unicode") },
  { XEDIT_ENCODING_UTF_16_LE,
    "UTF-16LE", N_("Unicode") },
  { XEDIT_ENCODING_UTF_32,
    "UTF-32", N_("Unicode") },
  { XEDIT_ENCODING_UCS_2,
    "UCS-2", N_("Unicode") },
  { XEDIT_ENCODING_UCS_4,
    "UCS-4", N_("Unicode") },

  { XEDIT_ENCODING_ARMSCII_8,
    "ARMSCII-8", N_("Armenian") },
  { XEDIT_ENCODING_BIG5,
    "BIG5", N_("Chinese Traditional") },
  { XEDIT_ENCODING_BIG5_HKSCS,
    "BIG5-HKSCS", N_("Chinese Traditional") },
  { XEDIT_ENCODING_CP_866,
    "CP866", N_("Cyrillic/Russian") },

  { XEDIT_ENCODING_EUC_JP,
    "EUC-JP", N_("Japanese") },
  { XEDIT_ENCODING_EUC_JP_MS,
    "EUC-JP-MS", N_("Japanese") },
  { XEDIT_ENCODING_CP932,
    "CP932", N_("Japanese") },

  { XEDIT_ENCODING_EUC_KR,
    "EUC-KR", N_("Korean") },
  { XEDIT_ENCODING_EUC_TW,
    "EUC-TW", N_("Chinese Traditional") },

  { XEDIT_ENCODING_GB18030,
    "GB18030", N_("Chinese Simplified") },
  { XEDIT_ENCODING_GB2312,
    "GB2312", N_("Chinese Simplified") },
  { XEDIT_ENCODING_GBK,
    "GBK", N_("Chinese Simplified") },
  { XEDIT_ENCODING_GEOSTD8,
    "GEORGIAN-ACADEMY", N_("Georgian") }, /* FIXME GEOSTD8 ? */

  { XEDIT_ENCODING_IBM_850,
    "IBM850", N_("Western") },
  { XEDIT_ENCODING_IBM_852,
    "IBM852", N_("Central European") },
  { XEDIT_ENCODING_IBM_855,
    "IBM855", N_("Cyrillic") },
  { XEDIT_ENCODING_IBM_857,
    "IBM857", N_("Turkish") },
  { XEDIT_ENCODING_IBM_862,
    "IBM862", N_("Hebrew") },
  { XEDIT_ENCODING_IBM_864,
    "IBM864", N_("Arabic") },

  { XEDIT_ENCODING_ISO_2022_JP,
    "ISO-2022-JP", N_("Japanese") },
  { XEDIT_ENCODING_ISO_2022_KR,
    "ISO-2022-KR", N_("Korean") },
  { XEDIT_ENCODING_ISO_IR_111,
    "ISO-IR-111", N_("Cyrillic") },
  { XEDIT_ENCODING_JOHAB,
    "JOHAB", N_("Korean") },
  { XEDIT_ENCODING_KOI8_R,
    "KOI8R", N_("Cyrillic") },
  { XEDIT_ENCODING_KOI8__R,
    "KOI8-R", N_("Cyrillic") },
  { XEDIT_ENCODING_KOI8_U,
    "KOI8U", N_("Cyrillic/Ukrainian") },
  
  { XEDIT_ENCODING_SHIFT_JIS,
    "SHIFT_JIS", N_("Japanese") },
  { XEDIT_ENCODING_TCVN,
    "TCVN", N_("Vietnamese") },
  { XEDIT_ENCODING_TIS_620,
    "TIS-620", N_("Thai") },
  { XEDIT_ENCODING_UHC,
    "UHC", N_("Korean") },
  { XEDIT_ENCODING_VISCII,
    "VISCII", N_("Vietnamese") },

  { XEDIT_ENCODING_WINDOWS_1250,
    "WINDOWS-1250", N_("Central European") },
  { XEDIT_ENCODING_WINDOWS_1251,
    "WINDOWS-1251", N_("Cyrillic") },
  { XEDIT_ENCODING_WINDOWS_1252,
    "WINDOWS-1252", N_("Western") },
  { XEDIT_ENCODING_WINDOWS_1253,
    "WINDOWS-1253", N_("Greek") },
  { XEDIT_ENCODING_WINDOWS_1254,
    "WINDOWS-1254", N_("Turkish") },
  { XEDIT_ENCODING_WINDOWS_1255,
    "WINDOWS-1255", N_("Hebrew") },
  { XEDIT_ENCODING_WINDOWS_1256,
    "WINDOWS-1256", N_("Arabic") },
  { XEDIT_ENCODING_WINDOWS_1257,
    "WINDOWS-1257", N_("Baltic") },
  { XEDIT_ENCODING_WINDOWS_1258,
    "WINDOWS-1258", N_("Vietnamese") }
};

static void
xedit_encoding_lazy_init (void)
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

const XeditEncoding *
xedit_encoding_get_from_charset (const gchar *charset)
{
	gint i;

	g_return_val_if_fail (charset != NULL, NULL);

	xedit_encoding_lazy_init ();

	if (charset == NULL)
		return NULL;

	if (g_ascii_strcasecmp (charset, "UTF-8") == 0)
		return xedit_encoding_get_utf8 ();

	i = 0; 
	while (i < XEDIT_ENCODING_LAST)
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

const XeditEncoding *
xedit_encoding_get_from_index (gint idx)
{
	g_return_val_if_fail (idx >= 0, NULL);

	if (idx >= XEDIT_ENCODING_LAST)
		return NULL;

	xedit_encoding_lazy_init ();

	return &encodings[idx];
}

const XeditEncoding *
xedit_encoding_get_utf8 (void)
{
	xedit_encoding_lazy_init ();

	return &utf8_encoding;
}

const XeditEncoding *
xedit_encoding_get_current (void)
{
	static gboolean initialized = FALSE;
	static const XeditEncoding *locale_encoding = NULL;

	const gchar *locale_charset;

	xedit_encoding_lazy_init ();

	if (initialized != FALSE)
		return locale_encoding;

	if (g_get_charset (&locale_charset) == FALSE) 
	{
		g_return_val_if_fail (locale_charset != NULL, &utf8_encoding);
		
		locale_encoding = xedit_encoding_get_from_charset (locale_charset);
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
xedit_encoding_to_string (const XeditEncoding* enc)
{
	g_return_val_if_fail (enc != NULL, NULL);
	
	xedit_encoding_lazy_init ();

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
xedit_encoding_get_charset (const XeditEncoding* enc)
{
	g_return_val_if_fail (enc != NULL, NULL);

	xedit_encoding_lazy_init ();

	g_return_val_if_fail (enc->charset != NULL, NULL);

	return enc->charset;
}

const gchar *
xedit_encoding_get_name (const XeditEncoding* enc)
{
	g_return_val_if_fail (enc != NULL, NULL);

	xedit_encoding_lazy_init ();

	return (enc->name == NULL) ? _("Unknown") : _(enc->name);
}

/* These are to make language bindings happy. Since Encodings are
 * const, copy() just returns the same pointer and fres() doesn't
 * do nothing */

XeditEncoding *
xedit_encoding_copy (const XeditEncoding *enc)
{
	g_return_val_if_fail (enc != NULL, NULL);

	return (XeditEncoding *) enc;
}

void 
xedit_encoding_free (XeditEncoding *enc)
{
	g_return_if_fail (enc != NULL);
}

/**
 * xedit_encoding_get_type:
 * 
 * Retrieves the GType object which is associated with the
 * #XeditEncoding class.
 * 
 * Return value: the GType associated with #XeditEncoding.
 **/
GType 
xedit_encoding_get_type (void)
{
	static GType our_type = 0;

	if (!our_type)
		our_type = g_boxed_type_register_static (
			"XeditEncoding",
			(GBoxedCopyFunc) xedit_encoding_copy,
			(GBoxedFreeFunc) xedit_encoding_free);

	return our_type;
} 

