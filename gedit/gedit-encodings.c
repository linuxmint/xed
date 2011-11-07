/*
 * gedit-encodings.c
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */
 
/*
 * Modified by the gedit Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <glib/gi18n.h>

#include "gedit-encodings.h"


struct _GeditEncoding
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

  GEDIT_ENCODING_ISO_8859_1,
  GEDIT_ENCODING_ISO_8859_2,
  GEDIT_ENCODING_ISO_8859_3,
  GEDIT_ENCODING_ISO_8859_4,
  GEDIT_ENCODING_ISO_8859_5,
  GEDIT_ENCODING_ISO_8859_6,
  GEDIT_ENCODING_ISO_8859_7,
  GEDIT_ENCODING_ISO_8859_8,
  GEDIT_ENCODING_ISO_8859_9,
  GEDIT_ENCODING_ISO_8859_10,
  GEDIT_ENCODING_ISO_8859_13,
  GEDIT_ENCODING_ISO_8859_14,
  GEDIT_ENCODING_ISO_8859_15,
  GEDIT_ENCODING_ISO_8859_16,

  GEDIT_ENCODING_UTF_7,
  GEDIT_ENCODING_UTF_16,
  GEDIT_ENCODING_UTF_16_BE,
  GEDIT_ENCODING_UTF_16_LE,
  GEDIT_ENCODING_UTF_32,
  GEDIT_ENCODING_UCS_2,
  GEDIT_ENCODING_UCS_4,

  GEDIT_ENCODING_ARMSCII_8,
  GEDIT_ENCODING_BIG5,
  GEDIT_ENCODING_BIG5_HKSCS,
  GEDIT_ENCODING_CP_866,

  GEDIT_ENCODING_EUC_JP,
  GEDIT_ENCODING_EUC_JP_MS,
  GEDIT_ENCODING_CP932,
  GEDIT_ENCODING_EUC_KR,
  GEDIT_ENCODING_EUC_TW,

  GEDIT_ENCODING_GB18030,
  GEDIT_ENCODING_GB2312,
  GEDIT_ENCODING_GBK,
  GEDIT_ENCODING_GEOSTD8,

  GEDIT_ENCODING_IBM_850,
  GEDIT_ENCODING_IBM_852,
  GEDIT_ENCODING_IBM_855,
  GEDIT_ENCODING_IBM_857,
  GEDIT_ENCODING_IBM_862,
  GEDIT_ENCODING_IBM_864,

  GEDIT_ENCODING_ISO_2022_JP,
  GEDIT_ENCODING_ISO_2022_KR,
  GEDIT_ENCODING_ISO_IR_111,
  GEDIT_ENCODING_JOHAB,
  GEDIT_ENCODING_KOI8_R,
  GEDIT_ENCODING_KOI8__R,
  GEDIT_ENCODING_KOI8_U,
  
  GEDIT_ENCODING_SHIFT_JIS,
  GEDIT_ENCODING_TCVN,
  GEDIT_ENCODING_TIS_620,
  GEDIT_ENCODING_UHC,
  GEDIT_ENCODING_VISCII,

  GEDIT_ENCODING_WINDOWS_1250,
  GEDIT_ENCODING_WINDOWS_1251,
  GEDIT_ENCODING_WINDOWS_1252,
  GEDIT_ENCODING_WINDOWS_1253,
  GEDIT_ENCODING_WINDOWS_1254,
  GEDIT_ENCODING_WINDOWS_1255,
  GEDIT_ENCODING_WINDOWS_1256,
  GEDIT_ENCODING_WINDOWS_1257,
  GEDIT_ENCODING_WINDOWS_1258,

  GEDIT_ENCODING_LAST,

  GEDIT_ENCODING_UTF_8,
  GEDIT_ENCODING_UNKNOWN
  
} GeditEncodingIndex;

static const GeditEncoding utf8_encoding =  {
	GEDIT_ENCODING_UTF_8,
	"UTF-8",
	N_("Unicode")
};

/* initialized in gedit_encoding_lazy_init() */
static GeditEncoding unknown_encoding = {
	GEDIT_ENCODING_UNKNOWN,
	NULL, 
	NULL 
};

static const GeditEncoding encodings [] = {

  { GEDIT_ENCODING_ISO_8859_1,
    "ISO-8859-1", N_("Western") },
  { GEDIT_ENCODING_ISO_8859_2,
   "ISO-8859-2", N_("Central European") },
  { GEDIT_ENCODING_ISO_8859_3,
    "ISO-8859-3", N_("South European") },
  { GEDIT_ENCODING_ISO_8859_4,
    "ISO-8859-4", N_("Baltic") },
  { GEDIT_ENCODING_ISO_8859_5,
    "ISO-8859-5", N_("Cyrillic") },
  { GEDIT_ENCODING_ISO_8859_6,
    "ISO-8859-6", N_("Arabic") },
  { GEDIT_ENCODING_ISO_8859_7,
    "ISO-8859-7", N_("Greek") },
  { GEDIT_ENCODING_ISO_8859_8,
    "ISO-8859-8", N_("Hebrew Visual") },
  { GEDIT_ENCODING_ISO_8859_9,
    "ISO-8859-9", N_("Turkish") },
  { GEDIT_ENCODING_ISO_8859_10,
    "ISO-8859-10", N_("Nordic") },
  { GEDIT_ENCODING_ISO_8859_13,
    "ISO-8859-13", N_("Baltic") },
  { GEDIT_ENCODING_ISO_8859_14,
    "ISO-8859-14", N_("Celtic") },
  { GEDIT_ENCODING_ISO_8859_15,
    "ISO-8859-15", N_("Western") },
  { GEDIT_ENCODING_ISO_8859_16,
    "ISO-8859-16", N_("Romanian") },

  { GEDIT_ENCODING_UTF_7,
    "UTF-7", N_("Unicode") },
  { GEDIT_ENCODING_UTF_16,
    "UTF-16", N_("Unicode") },
  { GEDIT_ENCODING_UTF_16_BE,
    "UTF-16BE", N_("Unicode") },
  { GEDIT_ENCODING_UTF_16_LE,
    "UTF-16LE", N_("Unicode") },
  { GEDIT_ENCODING_UTF_32,
    "UTF-32", N_("Unicode") },
  { GEDIT_ENCODING_UCS_2,
    "UCS-2", N_("Unicode") },
  { GEDIT_ENCODING_UCS_4,
    "UCS-4", N_("Unicode") },

  { GEDIT_ENCODING_ARMSCII_8,
    "ARMSCII-8", N_("Armenian") },
  { GEDIT_ENCODING_BIG5,
    "BIG5", N_("Chinese Traditional") },
  { GEDIT_ENCODING_BIG5_HKSCS,
    "BIG5-HKSCS", N_("Chinese Traditional") },
  { GEDIT_ENCODING_CP_866,
    "CP866", N_("Cyrillic/Russian") },

  { GEDIT_ENCODING_EUC_JP,
    "EUC-JP", N_("Japanese") },
  { GEDIT_ENCODING_EUC_JP_MS,
    "EUC-JP-MS", N_("Japanese") },
  { GEDIT_ENCODING_CP932,
    "CP932", N_("Japanese") },

  { GEDIT_ENCODING_EUC_KR,
    "EUC-KR", N_("Korean") },
  { GEDIT_ENCODING_EUC_TW,
    "EUC-TW", N_("Chinese Traditional") },

  { GEDIT_ENCODING_GB18030,
    "GB18030", N_("Chinese Simplified") },
  { GEDIT_ENCODING_GB2312,
    "GB2312", N_("Chinese Simplified") },
  { GEDIT_ENCODING_GBK,
    "GBK", N_("Chinese Simplified") },
  { GEDIT_ENCODING_GEOSTD8,
    "GEORGIAN-ACADEMY", N_("Georgian") }, /* FIXME GEOSTD8 ? */

  { GEDIT_ENCODING_IBM_850,
    "IBM850", N_("Western") },
  { GEDIT_ENCODING_IBM_852,
    "IBM852", N_("Central European") },
  { GEDIT_ENCODING_IBM_855,
    "IBM855", N_("Cyrillic") },
  { GEDIT_ENCODING_IBM_857,
    "IBM857", N_("Turkish") },
  { GEDIT_ENCODING_IBM_862,
    "IBM862", N_("Hebrew") },
  { GEDIT_ENCODING_IBM_864,
    "IBM864", N_("Arabic") },

  { GEDIT_ENCODING_ISO_2022_JP,
    "ISO-2022-JP", N_("Japanese") },
  { GEDIT_ENCODING_ISO_2022_KR,
    "ISO-2022-KR", N_("Korean") },
  { GEDIT_ENCODING_ISO_IR_111,
    "ISO-IR-111", N_("Cyrillic") },
  { GEDIT_ENCODING_JOHAB,
    "JOHAB", N_("Korean") },
  { GEDIT_ENCODING_KOI8_R,
    "KOI8R", N_("Cyrillic") },
  { GEDIT_ENCODING_KOI8__R,
    "KOI8-R", N_("Cyrillic") },
  { GEDIT_ENCODING_KOI8_U,
    "KOI8U", N_("Cyrillic/Ukrainian") },
  
  { GEDIT_ENCODING_SHIFT_JIS,
    "SHIFT_JIS", N_("Japanese") },
  { GEDIT_ENCODING_TCVN,
    "TCVN", N_("Vietnamese") },
  { GEDIT_ENCODING_TIS_620,
    "TIS-620", N_("Thai") },
  { GEDIT_ENCODING_UHC,
    "UHC", N_("Korean") },
  { GEDIT_ENCODING_VISCII,
    "VISCII", N_("Vietnamese") },

  { GEDIT_ENCODING_WINDOWS_1250,
    "WINDOWS-1250", N_("Central European") },
  { GEDIT_ENCODING_WINDOWS_1251,
    "WINDOWS-1251", N_("Cyrillic") },
  { GEDIT_ENCODING_WINDOWS_1252,
    "WINDOWS-1252", N_("Western") },
  { GEDIT_ENCODING_WINDOWS_1253,
    "WINDOWS-1253", N_("Greek") },
  { GEDIT_ENCODING_WINDOWS_1254,
    "WINDOWS-1254", N_("Turkish") },
  { GEDIT_ENCODING_WINDOWS_1255,
    "WINDOWS-1255", N_("Hebrew") },
  { GEDIT_ENCODING_WINDOWS_1256,
    "WINDOWS-1256", N_("Arabic") },
  { GEDIT_ENCODING_WINDOWS_1257,
    "WINDOWS-1257", N_("Baltic") },
  { GEDIT_ENCODING_WINDOWS_1258,
    "WINDOWS-1258", N_("Vietnamese") }
};

static void
gedit_encoding_lazy_init (void)
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

const GeditEncoding *
gedit_encoding_get_from_charset (const gchar *charset)
{
	gint i;

	g_return_val_if_fail (charset != NULL, NULL);

	gedit_encoding_lazy_init ();

	if (charset == NULL)
		return NULL;

	if (g_ascii_strcasecmp (charset, "UTF-8") == 0)
		return gedit_encoding_get_utf8 ();

	i = 0; 
	while (i < GEDIT_ENCODING_LAST)
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

const GeditEncoding *
gedit_encoding_get_from_index (gint idx)
{
	g_return_val_if_fail (idx >= 0, NULL);

	if (idx >= GEDIT_ENCODING_LAST)
		return NULL;

	gedit_encoding_lazy_init ();

	return &encodings[idx];
}

const GeditEncoding *
gedit_encoding_get_utf8 (void)
{
	gedit_encoding_lazy_init ();

	return &utf8_encoding;
}

const GeditEncoding *
gedit_encoding_get_current (void)
{
	static gboolean initialized = FALSE;
	static const GeditEncoding *locale_encoding = NULL;

	const gchar *locale_charset;

	gedit_encoding_lazy_init ();

	if (initialized != FALSE)
		return locale_encoding;

	if (g_get_charset (&locale_charset) == FALSE) 
	{
		g_return_val_if_fail (locale_charset != NULL, &utf8_encoding);
		
		locale_encoding = gedit_encoding_get_from_charset (locale_charset);
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
gedit_encoding_to_string (const GeditEncoding* enc)
{
	g_return_val_if_fail (enc != NULL, NULL);
	
	gedit_encoding_lazy_init ();

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
gedit_encoding_get_charset (const GeditEncoding* enc)
{
	g_return_val_if_fail (enc != NULL, NULL);

	gedit_encoding_lazy_init ();

	g_return_val_if_fail (enc->charset != NULL, NULL);

	return enc->charset;
}

const gchar *
gedit_encoding_get_name (const GeditEncoding* enc)
{
	g_return_val_if_fail (enc != NULL, NULL);

	gedit_encoding_lazy_init ();

	return (enc->name == NULL) ? _("Unknown") : _(enc->name);
}

/* These are to make language bindings happy. Since Encodings are
 * const, copy() just returns the same pointer and fres() doesn't
 * do nothing */

GeditEncoding *
gedit_encoding_copy (const GeditEncoding *enc)
{
	g_return_val_if_fail (enc != NULL, NULL);

	return (GeditEncoding *) enc;
}

void 
gedit_encoding_free (GeditEncoding *enc)
{
	g_return_if_fail (enc != NULL);
}

/**
 * gedit_encoding_get_type:
 * 
 * Retrieves the GType object which is associated with the
 * #GeditEncoding class.
 * 
 * Return value: the GType associated with #GeditEncoding.
 **/
GType 
gedit_encoding_get_type (void)
{
	static GType our_type = 0;

	if (!our_type)
		our_type = g_boxed_type_register_static (
			"GeditEncoding",
			(GBoxedCopyFunc) gedit_encoding_copy,
			(GBoxedFreeFunc) gedit_encoding_free);

	return our_type;
} 

