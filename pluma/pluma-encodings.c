/*
 * pluma-encodings.c
 * This file is part of pluma
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
 * Modified by the pluma Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <glib/gi18n.h>

#include "pluma-encodings.h"


struct _PlumaEncoding
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

  PLUMA_ENCODING_ISO_8859_1,
  PLUMA_ENCODING_ISO_8859_2,
  PLUMA_ENCODING_ISO_8859_3,
  PLUMA_ENCODING_ISO_8859_4,
  PLUMA_ENCODING_ISO_8859_5,
  PLUMA_ENCODING_ISO_8859_6,
  PLUMA_ENCODING_ISO_8859_7,
  PLUMA_ENCODING_ISO_8859_8,
  PLUMA_ENCODING_ISO_8859_9,
  PLUMA_ENCODING_ISO_8859_10,
  PLUMA_ENCODING_ISO_8859_13,
  PLUMA_ENCODING_ISO_8859_14,
  PLUMA_ENCODING_ISO_8859_15,
  PLUMA_ENCODING_ISO_8859_16,

  PLUMA_ENCODING_UTF_7,
  PLUMA_ENCODING_UTF_16,
  PLUMA_ENCODING_UTF_16_BE,
  PLUMA_ENCODING_UTF_16_LE,
  PLUMA_ENCODING_UTF_32,
  PLUMA_ENCODING_UCS_2,
  PLUMA_ENCODING_UCS_4,

  PLUMA_ENCODING_ARMSCII_8,
  PLUMA_ENCODING_BIG5,
  PLUMA_ENCODING_BIG5_HKSCS,
  PLUMA_ENCODING_CP_866,

  PLUMA_ENCODING_EUC_JP,
  PLUMA_ENCODING_EUC_JP_MS,
  PLUMA_ENCODING_CP932,
  PLUMA_ENCODING_EUC_KR,
  PLUMA_ENCODING_EUC_TW,

  PLUMA_ENCODING_GB18030,
  PLUMA_ENCODING_GB2312,
  PLUMA_ENCODING_GBK,
  PLUMA_ENCODING_GEOSTD8,

  PLUMA_ENCODING_IBM_850,
  PLUMA_ENCODING_IBM_852,
  PLUMA_ENCODING_IBM_855,
  PLUMA_ENCODING_IBM_857,
  PLUMA_ENCODING_IBM_862,
  PLUMA_ENCODING_IBM_864,

  PLUMA_ENCODING_ISO_2022_JP,
  PLUMA_ENCODING_ISO_2022_KR,
  PLUMA_ENCODING_ISO_IR_111,
  PLUMA_ENCODING_JOHAB,
  PLUMA_ENCODING_KOI8_R,
  PLUMA_ENCODING_KOI8__R,
  PLUMA_ENCODING_KOI8_U,
  
  PLUMA_ENCODING_SHIFT_JIS,
  PLUMA_ENCODING_TCVN,
  PLUMA_ENCODING_TIS_620,
  PLUMA_ENCODING_UHC,
  PLUMA_ENCODING_VISCII,

  PLUMA_ENCODING_WINDOWS_1250,
  PLUMA_ENCODING_WINDOWS_1251,
  PLUMA_ENCODING_WINDOWS_1252,
  PLUMA_ENCODING_WINDOWS_1253,
  PLUMA_ENCODING_WINDOWS_1254,
  PLUMA_ENCODING_WINDOWS_1255,
  PLUMA_ENCODING_WINDOWS_1256,
  PLUMA_ENCODING_WINDOWS_1257,
  PLUMA_ENCODING_WINDOWS_1258,

  PLUMA_ENCODING_LAST,

  PLUMA_ENCODING_UTF_8,
  PLUMA_ENCODING_UNKNOWN
  
} PlumaEncodingIndex;

static const PlumaEncoding utf8_encoding =  {
	PLUMA_ENCODING_UTF_8,
	"UTF-8",
	N_("Unicode")
};

/* initialized in pluma_encoding_lazy_init() */
static PlumaEncoding unknown_encoding = {
	PLUMA_ENCODING_UNKNOWN,
	NULL, 
	NULL 
};

static const PlumaEncoding encodings [] = {

  { PLUMA_ENCODING_ISO_8859_1,
    "ISO-8859-1", N_("Western") },
  { PLUMA_ENCODING_ISO_8859_2,
   "ISO-8859-2", N_("Central European") },
  { PLUMA_ENCODING_ISO_8859_3,
    "ISO-8859-3", N_("South European") },
  { PLUMA_ENCODING_ISO_8859_4,
    "ISO-8859-4", N_("Baltic") },
  { PLUMA_ENCODING_ISO_8859_5,
    "ISO-8859-5", N_("Cyrillic") },
  { PLUMA_ENCODING_ISO_8859_6,
    "ISO-8859-6", N_("Arabic") },
  { PLUMA_ENCODING_ISO_8859_7,
    "ISO-8859-7", N_("Greek") },
  { PLUMA_ENCODING_ISO_8859_8,
    "ISO-8859-8", N_("Hebrew Visual") },
  { PLUMA_ENCODING_ISO_8859_9,
    "ISO-8859-9", N_("Turkish") },
  { PLUMA_ENCODING_ISO_8859_10,
    "ISO-8859-10", N_("Nordic") },
  { PLUMA_ENCODING_ISO_8859_13,
    "ISO-8859-13", N_("Baltic") },
  { PLUMA_ENCODING_ISO_8859_14,
    "ISO-8859-14", N_("Celtic") },
  { PLUMA_ENCODING_ISO_8859_15,
    "ISO-8859-15", N_("Western") },
  { PLUMA_ENCODING_ISO_8859_16,
    "ISO-8859-16", N_("Romanian") },

  { PLUMA_ENCODING_UTF_7,
    "UTF-7", N_("Unicode") },
  { PLUMA_ENCODING_UTF_16,
    "UTF-16", N_("Unicode") },
  { PLUMA_ENCODING_UTF_16_BE,
    "UTF-16BE", N_("Unicode") },
  { PLUMA_ENCODING_UTF_16_LE,
    "UTF-16LE", N_("Unicode") },
  { PLUMA_ENCODING_UTF_32,
    "UTF-32", N_("Unicode") },
  { PLUMA_ENCODING_UCS_2,
    "UCS-2", N_("Unicode") },
  { PLUMA_ENCODING_UCS_4,
    "UCS-4", N_("Unicode") },

  { PLUMA_ENCODING_ARMSCII_8,
    "ARMSCII-8", N_("Armenian") },
  { PLUMA_ENCODING_BIG5,
    "BIG5", N_("Chinese Traditional") },
  { PLUMA_ENCODING_BIG5_HKSCS,
    "BIG5-HKSCS", N_("Chinese Traditional") },
  { PLUMA_ENCODING_CP_866,
    "CP866", N_("Cyrillic/Russian") },

  { PLUMA_ENCODING_EUC_JP,
    "EUC-JP", N_("Japanese") },
  { PLUMA_ENCODING_EUC_JP_MS,
    "EUC-JP-MS", N_("Japanese") },
  { PLUMA_ENCODING_CP932,
    "CP932", N_("Japanese") },

  { PLUMA_ENCODING_EUC_KR,
    "EUC-KR", N_("Korean") },
  { PLUMA_ENCODING_EUC_TW,
    "EUC-TW", N_("Chinese Traditional") },

  { PLUMA_ENCODING_GB18030,
    "GB18030", N_("Chinese Simplified") },
  { PLUMA_ENCODING_GB2312,
    "GB2312", N_("Chinese Simplified") },
  { PLUMA_ENCODING_GBK,
    "GBK", N_("Chinese Simplified") },
  { PLUMA_ENCODING_GEOSTD8,
    "GEORGIAN-ACADEMY", N_("Georgian") }, /* FIXME GEOSTD8 ? */

  { PLUMA_ENCODING_IBM_850,
    "IBM850", N_("Western") },
  { PLUMA_ENCODING_IBM_852,
    "IBM852", N_("Central European") },
  { PLUMA_ENCODING_IBM_855,
    "IBM855", N_("Cyrillic") },
  { PLUMA_ENCODING_IBM_857,
    "IBM857", N_("Turkish") },
  { PLUMA_ENCODING_IBM_862,
    "IBM862", N_("Hebrew") },
  { PLUMA_ENCODING_IBM_864,
    "IBM864", N_("Arabic") },

  { PLUMA_ENCODING_ISO_2022_JP,
    "ISO-2022-JP", N_("Japanese") },
  { PLUMA_ENCODING_ISO_2022_KR,
    "ISO-2022-KR", N_("Korean") },
  { PLUMA_ENCODING_ISO_IR_111,
    "ISO-IR-111", N_("Cyrillic") },
  { PLUMA_ENCODING_JOHAB,
    "JOHAB", N_("Korean") },
  { PLUMA_ENCODING_KOI8_R,
    "KOI8R", N_("Cyrillic") },
  { PLUMA_ENCODING_KOI8__R,
    "KOI8-R", N_("Cyrillic") },
  { PLUMA_ENCODING_KOI8_U,
    "KOI8U", N_("Cyrillic/Ukrainian") },
  
  { PLUMA_ENCODING_SHIFT_JIS,
    "SHIFT_JIS", N_("Japanese") },
  { PLUMA_ENCODING_TCVN,
    "TCVN", N_("Vietnamese") },
  { PLUMA_ENCODING_TIS_620,
    "TIS-620", N_("Thai") },
  { PLUMA_ENCODING_UHC,
    "UHC", N_("Korean") },
  { PLUMA_ENCODING_VISCII,
    "VISCII", N_("Vietnamese") },

  { PLUMA_ENCODING_WINDOWS_1250,
    "WINDOWS-1250", N_("Central European") },
  { PLUMA_ENCODING_WINDOWS_1251,
    "WINDOWS-1251", N_("Cyrillic") },
  { PLUMA_ENCODING_WINDOWS_1252,
    "WINDOWS-1252", N_("Western") },
  { PLUMA_ENCODING_WINDOWS_1253,
    "WINDOWS-1253", N_("Greek") },
  { PLUMA_ENCODING_WINDOWS_1254,
    "WINDOWS-1254", N_("Turkish") },
  { PLUMA_ENCODING_WINDOWS_1255,
    "WINDOWS-1255", N_("Hebrew") },
  { PLUMA_ENCODING_WINDOWS_1256,
    "WINDOWS-1256", N_("Arabic") },
  { PLUMA_ENCODING_WINDOWS_1257,
    "WINDOWS-1257", N_("Baltic") },
  { PLUMA_ENCODING_WINDOWS_1258,
    "WINDOWS-1258", N_("Vietnamese") }
};

static void
pluma_encoding_lazy_init (void)
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

const PlumaEncoding *
pluma_encoding_get_from_charset (const gchar *charset)
{
	gint i;

	g_return_val_if_fail (charset != NULL, NULL);

	pluma_encoding_lazy_init ();

	if (charset == NULL)
		return NULL;

	if (g_ascii_strcasecmp (charset, "UTF-8") == 0)
		return pluma_encoding_get_utf8 ();

	i = 0; 
	while (i < PLUMA_ENCODING_LAST)
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

const PlumaEncoding *
pluma_encoding_get_from_index (gint idx)
{
	g_return_val_if_fail (idx >= 0, NULL);

	if (idx >= PLUMA_ENCODING_LAST)
		return NULL;

	pluma_encoding_lazy_init ();

	return &encodings[idx];
}

const PlumaEncoding *
pluma_encoding_get_utf8 (void)
{
	pluma_encoding_lazy_init ();

	return &utf8_encoding;
}

const PlumaEncoding *
pluma_encoding_get_current (void)
{
	static gboolean initialized = FALSE;
	static const PlumaEncoding *locale_encoding = NULL;

	const gchar *locale_charset;

	pluma_encoding_lazy_init ();

	if (initialized != FALSE)
		return locale_encoding;

	if (g_get_charset (&locale_charset) == FALSE) 
	{
		g_return_val_if_fail (locale_charset != NULL, &utf8_encoding);
		
		locale_encoding = pluma_encoding_get_from_charset (locale_charset);
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
pluma_encoding_to_string (const PlumaEncoding* enc)
{
	g_return_val_if_fail (enc != NULL, NULL);
	
	pluma_encoding_lazy_init ();

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
pluma_encoding_get_charset (const PlumaEncoding* enc)
{
	g_return_val_if_fail (enc != NULL, NULL);

	pluma_encoding_lazy_init ();

	g_return_val_if_fail (enc->charset != NULL, NULL);

	return enc->charset;
}

const gchar *
pluma_encoding_get_name (const PlumaEncoding* enc)
{
	g_return_val_if_fail (enc != NULL, NULL);

	pluma_encoding_lazy_init ();

	return (enc->name == NULL) ? _("Unknown") : _(enc->name);
}

/* These are to make language bindings happy. Since Encodings are
 * const, copy() just returns the same pointer and fres() doesn't
 * do nothing */

PlumaEncoding *
pluma_encoding_copy (const PlumaEncoding *enc)
{
	g_return_val_if_fail (enc != NULL, NULL);

	return (PlumaEncoding *) enc;
}

void 
pluma_encoding_free (PlumaEncoding *enc)
{
	g_return_if_fail (enc != NULL);
}

/**
 * pluma_encoding_get_type:
 * 
 * Retrieves the GType object which is associated with the
 * #PlumaEncoding class.
 * 
 * Return value: the GType associated with #PlumaEncoding.
 **/
GType 
pluma_encoding_get_type (void)
{
	static GType our_type = 0;

	if (!our_type)
		our_type = g_boxed_type_register_static (
			"PlumaEncoding",
			(GBoxedCopyFunc) pluma_encoding_copy,
			(GBoxedFreeFunc) pluma_encoding_free);

	return our_type;
} 

