/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gedit-spell-checker-language.c
 * This file is part of gedit
 *
 * Copyright (C) 2006 Paolo Maggi 
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
 * Modified by the gedit Team, 2006. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 */

/* Part of the code taked from Epiphany.
 *
 * Copyright (C) 2003, 2004 Christian Persch
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <enchant.h>

#include <glib/gi18n.h>
#include <libxml/xmlreader.h>

#include "gedit-spell-checker-language.h"

#include <gedit/gedit-debug.h>

#define ISO_639_DOMAIN	"iso_639"
#define ISO_3166_DOMAIN	"iso_3166"

#define ISOCODESLOCALEDIR	ISO_CODES_PREFIX "/share/locale"

struct _GeditSpellCheckerLanguage 
{
	gchar *abrev;
	gchar *name;	
};

static gboolean available_languages_initialized = FALSE;
static GSList *available_languages = NULL;

static GHashTable *iso_639_table = NULL;
static GHashTable *iso_3166_table = NULL;

static void
bind_iso_domains (void)
{
	static gboolean bound = FALSE;

	if (bound == FALSE)
	{
	        bindtextdomain (ISO_639_DOMAIN, ISOCODESLOCALEDIR);
	        bind_textdomain_codeset (ISO_639_DOMAIN, "UTF-8");

	        bindtextdomain(ISO_3166_DOMAIN, ISOCODESLOCALEDIR);
	        bind_textdomain_codeset (ISO_3166_DOMAIN, "UTF-8");

		bound = TRUE;
	}
}

static void
read_iso_639_entry (xmlTextReaderPtr reader,
		    GHashTable *table)
{
	xmlChar *code, *name;

	code = xmlTextReaderGetAttribute (reader, (const xmlChar *) "iso_639_1_code");
	name = xmlTextReaderGetAttribute (reader, (const xmlChar *) "name");

	/* Get iso-639-2 code */
	if (code == NULL || code[0] == '\0')
	{
		xmlFree (code);
		/* FIXME: use the 2T or 2B code? */
		code = xmlTextReaderGetAttribute (reader, (const xmlChar *) "iso_639_2T_code");
	}

	if (code != NULL && code[0] != '\0' && name != NULL && name[0] != '\0')
	{
		g_hash_table_insert (table, code, name);
	}
	else
	{
		xmlFree (code);
		xmlFree (name);
	}
}

static void
read_iso_3166_entry (xmlTextReaderPtr reader,
		     GHashTable *table)
{
	xmlChar *code, *name;

	code = xmlTextReaderGetAttribute (reader, (const xmlChar *) "alpha_2_code");
	name = xmlTextReaderGetAttribute (reader, (const xmlChar *) "name");

	if (code != NULL && code[0] != '\0' && name != NULL && name[0] != '\0')
	{
		char *lcode;

		lcode = g_ascii_strdown ((char *) code, -1);
		xmlFree (code);

		/* g_print ("%s -> %s\n", lcode, name); */
		
		g_hash_table_insert (table, lcode, name);
	}
	else
	{
		xmlFree (code);
		xmlFree (name);
	}
}

typedef enum
{
	STATE_START,
	STATE_STOP,
	STATE_ENTRIES,
} ParserState;

static void
load_iso_entries (int iso,
		  GFunc read_entry_func,
		  gpointer user_data)
{
	xmlTextReaderPtr reader;
	ParserState state = STATE_START;
	xmlChar iso_entries[32], iso_entry[32];
	char *filename;
	int ret = -1;

	gedit_debug_message (DEBUG_PLUGINS, "Loading ISO-%d codes", iso);

	filename = g_strdup_printf (ISO_CODES_PREFIX "/share/xml/iso-codes/iso_%d.xml", iso);
	reader = xmlNewTextReaderFilename (filename);
	if (reader == NULL) goto out;

	xmlStrPrintf (iso_entries, sizeof (iso_entries), (const xmlChar *)"iso_%d_entries", iso);
	xmlStrPrintf (iso_entry, sizeof (iso_entry), (const xmlChar *)"iso_%d_entry", iso);

	ret = xmlTextReaderRead (reader);

	while (ret == 1)
	{
		const xmlChar *tag;
		xmlReaderTypes type;

		tag = xmlTextReaderConstName (reader);
		type = xmlTextReaderNodeType (reader);

		if (state == STATE_ENTRIES &&
		    type == XML_READER_TYPE_ELEMENT &&
		    xmlStrEqual (tag, iso_entry))
		{
			read_entry_func (reader, user_data);
		}
		else if (state == STATE_START &&
			 type == XML_READER_TYPE_ELEMENT &&
			 xmlStrEqual (tag, iso_entries))
		{
			state = STATE_ENTRIES;
		}
		else if (state == STATE_ENTRIES &&
			 type == XML_READER_TYPE_END_ELEMENT &&
			 xmlStrEqual (tag, iso_entries))
		{
			state = STATE_STOP;
		}
		else if (type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE ||
			 type == XML_READER_TYPE_WHITESPACE ||
			 type == XML_READER_TYPE_TEXT ||
			 type == XML_READER_TYPE_COMMENT)
		{
			/* eat it */
		}
		else
		{
			/* ignore it */
		}

		ret = xmlTextReaderRead (reader);
	}

	xmlFreeTextReader (reader);

out:
	if (ret < 0 || state != STATE_STOP)
	{
		g_warning ("Failed to load ISO-%d codes from %s!\n",
			   iso, filename);
	}

	g_free (filename);
}

static GHashTable *
create_iso_639_table (void)
{
	GHashTable *table;

	bind_iso_domains ();
	table = g_hash_table_new_full (g_str_hash, g_str_equal,
				       (GDestroyNotify) xmlFree,
				       (GDestroyNotify) xmlFree);

	load_iso_entries (639, (GFunc) read_iso_639_entry, table);

	return table;
}

static GHashTable *
create_iso_3166_table (void)
{
	GHashTable *table;

	bind_iso_domains ();
	table = g_hash_table_new_full (g_str_hash, g_str_equal,
				       (GDestroyNotify) g_free,
				       (GDestroyNotify) xmlFree);
	
	load_iso_entries (3166, (GFunc) read_iso_3166_entry, table);

	return table;
}

static char *
create_name_for_language (const char *code)
{
	char **str;
	char *name = NULL;
	const char *langname, *localename;
	int len;

	g_return_val_if_fail (iso_639_table != NULL, NULL);
	g_return_val_if_fail (iso_3166_table != NULL, NULL);
		
	str = g_strsplit (code, "_", -1);
	len = g_strv_length (str);
	g_return_val_if_fail (len != 0, NULL);

	langname = (const char *) g_hash_table_lookup (iso_639_table, str[0]);

	if (len == 1 && langname != NULL)
	{
		name = g_strdup (dgettext (ISO_639_DOMAIN, langname));
	}
	else if (len == 2 && langname != NULL)
	{
		gchar *locale_code = g_ascii_strdown (str[1], -1);
		
		localename = (const char *) g_hash_table_lookup (iso_3166_table, locale_code);
		g_free (locale_code);
		
		if (localename != NULL)
		{
			/* Translators: the first %s is the language name, and
			 * the second %s is the locale name. Example:
			 * "French (France)"
			 */
			name = g_strdup_printf (C_("language", "%s (%s)"),
						dgettext (ISO_639_DOMAIN, langname),
						dgettext (ISO_3166_DOMAIN, localename));
		}
		else
		{
			name = g_strdup_printf (C_("language", "%s (%s)"),
						dgettext (ISO_639_DOMAIN, langname), str[1]);
		}
	}
	else
	{
		/* Translators: this refers to an unknown language code
		 * (one which isn't in our built-in list).
		 */
		name = g_strdup_printf (C_("language", "Unknown (%s)"), code);
	}

	g_strfreev (str);

	return name;
}

static void
enumerate_dicts (const char * const lang_tag,
		 const char * const provider_name,
		 const char * const provider_desc,
		 const char * const provider_file,
		 void * user_data)
{
	gchar *lang_name;
	
	GTree *dicts = (GTree *)user_data;
	
	lang_name = create_name_for_language (lang_tag);
	g_return_if_fail (lang_name != NULL);
	
	/* g_print ("%s - %s\n", lang_tag, lang_name); */
	
	g_tree_replace (dicts, g_strdup (lang_tag), lang_name);
}

static gint
key_cmp (gconstpointer a, gconstpointer b, gpointer user_data)
{
	return strcmp (a, b);
}

static gint
lang_cmp (const GeditSpellCheckerLanguage *a,
          const GeditSpellCheckerLanguage *b)
{
	return g_utf8_collate (a->name, b->name);
}

static gboolean
build_langs_list (const gchar *key, 
		  const gchar *value, 
		  gpointer     data)
{
	GeditSpellCheckerLanguage *lang = g_new (GeditSpellCheckerLanguage, 1);
	
	lang->abrev = g_strdup (key);
	lang->name = g_strdup (value);
	
	available_languages = g_slist_insert_sorted (available_languages,
						     lang,
						     (GCompareFunc)lang_cmp);

	return FALSE;
}

const GSList *
gedit_spell_checker_get_available_languages (void)
{
	EnchantBroker *broker;
	GTree *dicts;

	if (available_languages_initialized)
		return available_languages;

	g_return_val_if_fail (available_languages == NULL, NULL);
			
	available_languages_initialized = TRUE;
	
	broker = enchant_broker_init ();
	g_return_val_if_fail (broker != NULL, NULL);
	
	/* Use a GTree to efficiently remove duplicates while building the list */
	dicts = g_tree_new_full (key_cmp,
				 NULL,
				 (GDestroyNotify)g_free,
				 (GDestroyNotify)g_free);

	iso_639_table = create_iso_639_table ();
	iso_3166_table = create_iso_3166_table ();
	
	enchant_broker_list_dicts (broker, enumerate_dicts, dicts);

	enchant_broker_free (broker);
	
	g_hash_table_destroy (iso_639_table);
	g_hash_table_destroy (iso_3166_table);
	
	iso_639_table = NULL;
	iso_3166_table = NULL;
	
	g_tree_foreach (dicts, (GTraverseFunc)build_langs_list, NULL);
	
	g_tree_destroy (dicts);
	
	return available_languages;
}

const gchar *
gedit_spell_checker_language_to_string (const GeditSpellCheckerLanguage *lang)
{
	if (lang == NULL)
		/* Translators: this refers the Default language used by the
		 * spell checker
		 */
		return C_("language", "Default");

	return lang->name;
}

const gchar *
gedit_spell_checker_language_to_key (const GeditSpellCheckerLanguage *lang)
{
	g_return_val_if_fail (lang != NULL, NULL);
	
	return lang->abrev;
}

const GeditSpellCheckerLanguage *
gedit_spell_checker_language_from_key (const gchar *key)
{
	const GSList *langs;

	g_return_val_if_fail (key != NULL, NULL);

	langs = gedit_spell_checker_get_available_languages ();

	while (langs != NULL)
	{
		const GeditSpellCheckerLanguage *l = (const GeditSpellCheckerLanguage *)langs->data;

		if (g_ascii_strcasecmp (key, l->abrev) == 0)
			return l;

		langs = g_slist_next (langs);
	}

	return NULL;
}
