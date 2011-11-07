/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gedit-languages-manager.c
 * This file is part of gedit
 *
 * Copyright (C) 2003-2006 - Paolo Maggi 
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
 * Modified by the gedit Team, 2003-2006. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#include <string.h>

#include "gedit-language-manager.h"
#include "gedit-prefs-manager.h"
#include "gedit-utils.h"
#include "gedit-debug.h"

static GtkSourceLanguageManager *language_manager = NULL;

GtkSourceLanguageManager *
gedit_get_language_manager (void)
{
	if (language_manager == NULL)
	{
		language_manager = gtk_source_language_manager_new ();
	}

	return language_manager;
}

static gint
language_compare (gconstpointer a, gconstpointer b)
{
	GtkSourceLanguage *lang_a = (GtkSourceLanguage *)a;
	GtkSourceLanguage *lang_b = (GtkSourceLanguage *)b;
	const gchar *name_a = gtk_source_language_get_name (lang_a);
	const gchar *name_b = gtk_source_language_get_name (lang_b);

	return g_utf8_collate (name_a, name_b);
}

GSList *
gedit_language_manager_list_languages_sorted (GtkSourceLanguageManager *lm,
					      gboolean                  include_hidden)
{
	GSList *languages = NULL;
	const gchar * const *ids;

	ids = gtk_source_language_manager_get_language_ids (lm);
	if (ids == NULL)
		return NULL;

	while (*ids != NULL)
	{
		GtkSourceLanguage *lang;

		lang = gtk_source_language_manager_get_language (lm, *ids);
		g_return_val_if_fail (GTK_IS_SOURCE_LANGUAGE (lang), NULL);
		++ids;

		if (include_hidden || !gtk_source_language_get_hidden (lang))
		{
			languages = g_slist_prepend (languages, lang);
		}
	}

	return g_slist_sort (languages, (GCompareFunc)language_compare);
}

