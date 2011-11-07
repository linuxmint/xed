/*
 * modeline-parser.c
 * Emacs, Kate and Vim-style modelines support for gedit.
 *
 * Copyright (C) 2005-2007 - Steve Frécinaux <code@istique.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <gedit/gedit-language-manager.h>
#include <gedit/gedit-prefs-manager.h>
#include <gedit/gedit-debug.h>
#include "modeline-parser.h"

#define MODELINES_LANGUAGE_MAPPINGS_FILE "language-mappings"

/* base dir to lookup configuration files */
static gchar *modelines_data_dir;

/* Mappings: language name -> Gedit language ID */
static GHashTable *vim_languages;
static GHashTable *emacs_languages;
static GHashTable *kate_languages;

typedef enum
{
	MODELINE_SET_NONE = 0,
	MODELINE_SET_TAB_WIDTH = 1 << 0,
	MODELINE_SET_INDENT_WIDTH = 1 << 1,
	MODELINE_SET_WRAP_MODE = 1 << 2,
	MODELINE_SET_SHOW_RIGHT_MARGIN = 1 << 3,
	MODELINE_SET_RIGHT_MARGIN_POSITION = 1 << 4,
	MODELINE_SET_LANGUAGE = 1 << 5,
	MODELINE_SET_INSERT_SPACES = 1 << 6
} ModelineSet;

typedef struct _ModelineOptions
{
	gchar		*language_id;

	/* these options are similar to the GtkSourceView properties of the
	 * same names.
	 */
	gboolean	insert_spaces;
	guint		tab_width;
	guint		indent_width;
	GtkWrapMode	wrap_mode;
	gboolean	display_right_margin;
	guint		right_margin_position;
	
	ModelineSet	set;
} ModelineOptions;

#define MODELINE_OPTIONS_DATA_KEY "ModelineOptionsDataKey"

static gboolean
has_option (ModelineOptions *options,
            ModelineSet      set)
{
	return options->set & set;
}

void
modeline_parser_init (const gchar *data_dir)
{
	modelines_data_dir = g_strdup (data_dir);
}

void
modeline_parser_shutdown ()
{
	if (vim_languages != NULL)
		g_hash_table_destroy (vim_languages);

	if (emacs_languages != NULL)
		g_hash_table_destroy (emacs_languages);

	if (kate_languages != NULL)
		g_hash_table_destroy (kate_languages);
	
	vim_languages = NULL;
	emacs_languages = NULL;
	kate_languages = NULL;

	g_free (modelines_data_dir);
}

static GHashTable *
load_language_mappings_group (GKeyFile *key_file, const gchar *group)
{
	GHashTable *table;
	gchar **keys;
	gsize length = 0;
	int i;

	table = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

	keys = g_key_file_get_keys (key_file, group, &length, NULL);

	gedit_debug_message (DEBUG_PLUGINS,
			     "%" G_GSIZE_FORMAT " mappings in group %s",
			     length, group);

	for (i = 0; i < length; i++)
	{
		gchar *name = keys[i];
		gchar *id = g_key_file_get_string (key_file, group, name, NULL);
		g_hash_table_insert (table, name, id);
	}
	g_free (keys);

	return table;
}

/* lazy loading of language mappings */
static void
load_language_mappings (void)
{
	gchar *fname;
	GKeyFile *mappings;
	GError *error = NULL;

	fname = g_build_filename (modelines_data_dir,
				  MODELINES_LANGUAGE_MAPPINGS_FILE,
				  NULL);

	mappings = g_key_file_new ();

	if (g_key_file_load_from_file (mappings, fname, 0, &error))
	{
		gedit_debug_message (DEBUG_PLUGINS,
				     "Loaded language mappings from %s",
				     fname);

		vim_languages = load_language_mappings_group (mappings, "vim");
		emacs_languages	= load_language_mappings_group (mappings, "emacs");
		kate_languages = load_language_mappings_group (mappings, "kate");
	}
	else
	{
		gedit_debug_message (DEBUG_PLUGINS,
				     "Failed to loaded language mappings from %s: %s",
				     fname, error->message);

		g_error_free (error);
	}

	g_key_file_free (mappings);
	g_free (fname);
}

static gchar *
get_language_id (const gchar *language_name, GHashTable *mapping)
{
	gchar *name;
	gchar *language_id;

	name = g_ascii_strdown (language_name, -1);

	language_id = g_hash_table_lookup (mapping, name);

	if (language_id != NULL)
	{
		g_free (name);
		return g_strdup (language_id);
	}
	else
	{
		/* by default assume that the gtksourcevuew id is the same */
		return name;
	}
}

static gchar *
get_language_id_vim (const gchar *language_name)
{
	if (vim_languages == NULL)
		load_language_mappings ();

	return get_language_id (language_name, vim_languages);
}

static gchar *
get_language_id_emacs (const gchar *language_name)
{
	if (emacs_languages == NULL)
		load_language_mappings ();

	return get_language_id (language_name, emacs_languages);
}

static gchar *
get_language_id_kate (const gchar *language_name)
{
	if (kate_languages == NULL)
		load_language_mappings ();

	return get_language_id (language_name, kate_languages);
}

static gboolean
skip_whitespaces (gchar **s)
{
	while (**s != '\0' && g_ascii_isspace (**s))
		(*s)++;
	return **s != '\0';
}

/* Parse vi(m) modelines.
 * Vi(m) modelines looks like this: 
 *   - first form:   [text]{white}{vi:|vim:|ex:}[white]{options}
 *   - second form:  [text]{white}{vi:|vim:|ex:}[white]se[t] {options}:[text]
 * They can happen on the three first or last lines.
 */
static gchar *
parse_vim_modeline (gchar           *s,
		    ModelineOptions *options)
{
	gboolean in_set = FALSE;
	gboolean neg;
	guint intval;
	GString *key, *value;

	key = g_string_sized_new (8);
	value = g_string_sized_new (8);

	while (*s != '\0' && !(in_set && *s == ':'))
	{
		while (*s != '\0' && (*s == ':' || g_ascii_isspace (*s)))
			s++;

		if (*s == '\0')
			break;

		if (strncmp (s, "set ", 4) == 0 ||
		    strncmp (s, "se ", 3) == 0)
		{
			s = strchr(s, ' ') + 1;
			in_set = TRUE;
		}

		neg = FALSE;
		if (strncmp (s, "no", 2) == 0)
		{
			neg = TRUE;
			s += 2;
		}

		g_string_assign (key, "");
		g_string_assign (value, "");

		while (*s != '\0' && *s != ':' && *s != '=' &&
		       !g_ascii_isspace (*s))
		{
			g_string_append_c (key, *s);
			s++;
		}

		if (*s == '=')
		{
			s++;
			while (*s != '\0' && *s != ':' &&
			       !g_ascii_isspace (*s))
			{
				g_string_append_c (value, *s);
				s++;
			}
		}

		if (strcmp (key->str, "ft") == 0 ||
		    strcmp (key->str, "filetype") == 0)
		{
			g_free (options->language_id);
			options->language_id = get_language_id_vim (value->str);
			
			options->set |= MODELINE_SET_LANGUAGE;
		}
		else if (strcmp (key->str, "et") == 0 ||
		    strcmp (key->str, "expandtab") == 0)
		{
			options->insert_spaces = !neg;
			options->set |= MODELINE_SET_INSERT_SPACES;
		}
		else if (strcmp (key->str, "ts") == 0 ||
			 strcmp (key->str, "tabstop") == 0)
		{
			intval = atoi (value->str);
			
			if (intval)
			{
				options->tab_width = intval;
				options->set |= MODELINE_SET_TAB_WIDTH;
			}
		}
		else if (strcmp (key->str, "sw") == 0 ||
			 strcmp (key->str, "shiftwidth") == 0)
		{
			intval = atoi (value->str);
			
			if (intval)
			{
				options->indent_width = intval;
				options->set |= MODELINE_SET_INDENT_WIDTH;
			}
		}
		else if (strcmp (key->str, "wrap") == 0)
		{
			options->wrap_mode = neg ? GTK_WRAP_NONE : GTK_WRAP_WORD;

			options->set |= MODELINE_SET_WRAP_MODE;
		}
		else if (strcmp (key->str, "textwidth") == 0)
		{
			intval = atoi (value->str);
			
			if (intval)
			{
				options->right_margin_position = intval;
				options->display_right_margin = TRUE;
				
				options->set |= MODELINE_SET_SHOW_RIGHT_MARGIN |
				                MODELINE_SET_RIGHT_MARGIN_POSITION;
				
			}
		}
	}

	g_string_free (key, TRUE);
	g_string_free (value, TRUE);

	return s;
}

/* Parse emacs modelines.
 * Emacs modelines looks like this: "-*- key1: value1; key2: value2 -*-"
 * They can happen on the first line, or on the second one if the first line is
 * a shebang (#!)
 * See http://www.delorie.com/gnu/docs/emacs/emacs_486.html
 */
static gchar *
parse_emacs_modeline (gchar           *s,
		      ModelineOptions *options)
{
	guint intval;
	GString *key, *value;

	key = g_string_sized_new (8);
	value = g_string_sized_new (8);

	while (*s != '\0')
	{
		while (*s != '\0' && (*s == ';' || g_ascii_isspace (*s)))
			s++;
		if (*s == '\0' || strncmp (s, "-*-", 3) == 0)
			break;

		g_string_assign (key, "");
		g_string_assign (value, "");

		while (*s != '\0' && *s != ':' && *s != ';' &&
		       !g_ascii_isspace (*s))
		{
			g_string_append_c (key, *s);
			s++;
		}

		if (!skip_whitespaces (&s))
			break;

		if (*s != ':')
			continue;
		s++;

		if (!skip_whitespaces (&s))
			break;

		while (*s != '\0' && *s != ';' && !g_ascii_isspace (*s))
		{
			g_string_append_c (value, *s);
			s++;
		}

		gedit_debug_message (DEBUG_PLUGINS,
				     "Emacs modeline bit: %s = %s",
				     key->str, value->str);

		/* "Mode" key is case insenstive */
		if (g_ascii_strcasecmp (key->str, "Mode") == 0)
		{
			g_free (options->language_id);
			options->language_id = get_language_id_emacs (value->str);
			
			options->set |= MODELINE_SET_LANGUAGE;
		}
		else if (strcmp (key->str, "tab-width") == 0)
		{
			intval = atoi (value->str);
			
			if (intval)
			{
				options->tab_width = intval;
				options->set |= MODELINE_SET_TAB_WIDTH;
			}
		}
		else if (strcmp (key->str, "indent-offset") == 0)
		{
			intval = atoi (value->str);
			
			if (intval)
			{
				options->indent_width = intval;
				options->set |= MODELINE_SET_INDENT_WIDTH;
			}
		}
		else if (strcmp (key->str, "indent-tabs-mode") == 0)
		{
			intval = strcmp (value->str, "nil") == 0;
			options->insert_spaces = intval;
			
			options->set |= MODELINE_SET_INSERT_SPACES;
		}
		else if (strcmp (key->str, "autowrap") == 0)
		{
			intval = strcmp (value->str, "nil") != 0;
			options->wrap_mode = intval ? GTK_WRAP_WORD : GTK_WRAP_NONE;
			
			options->set |= MODELINE_SET_WRAP_MODE;
		}
	}

	g_string_free (key, TRUE);
	g_string_free (value, TRUE);

	return *s == '\0' ? s : s + 2;
}

/*
 * Parse kate modelines.
 * Kate modelines are of the form "kate: key1 value1; key2 value2;"
 * These can happen on the 10 first or 10 last lines of the buffer.
 * See http://wiki.kate-editor.org/index.php/Modelines
 */
static gchar *
parse_kate_modeline (gchar           *s,
		     ModelineOptions *options)
{
	guint intval;
	GString *key, *value;

	key = g_string_sized_new (8);
	value = g_string_sized_new (8);

	while (*s != '\0')
	{
		while (*s != '\0' && (*s == ';' || g_ascii_isspace (*s)))
			s++;
		if (*s == '\0')
			break;

		g_string_assign (key, "");
		g_string_assign (value, "");

		while (*s != '\0' && *s != ';' && !g_ascii_isspace (*s))
		{
			g_string_append_c (key, *s);
			s++;
		}

		if (!skip_whitespaces (&s))
			break;
		if (*s == ';')
			continue;

		while (*s != '\0' && *s != ';' &&
		       !g_ascii_isspace (*s))
		{
			g_string_append_c (value, *s);
			s++;
		}

		gedit_debug_message (DEBUG_PLUGINS,
				     "Kate modeline bit: %s = %s",
				     key->str, value->str);

		if (strcmp (key->str, "hl") == 0 ||
		    strcmp (key->str, "syntax") == 0)
		{
			g_free (options->language_id);
			options->language_id = get_language_id_kate (value->str);
			
			options->set |= MODELINE_SET_LANGUAGE;
		}
		else if (strcmp (key->str, "tab-width") == 0)
		{
			intval = atoi (value->str);
			
			if (intval)
			{
				options->tab_width = intval;
				options->set |= MODELINE_SET_TAB_WIDTH;
			}
		}
		else if (strcmp (key->str, "indent-width") == 0)
		{
			intval = atoi (value->str);
			if (intval) options->indent_width = intval;
		}
		else if (strcmp (key->str, "space-indent") == 0)
		{
			intval = strcmp (value->str, "on") == 0 ||
			         strcmp (value->str, "true") == 0 ||
			         strcmp (value->str, "1") == 0;

			options->insert_spaces = intval;
			options->set |= MODELINE_SET_INSERT_SPACES;
		}
		else if (strcmp (key->str, "word-wrap") == 0)
		{
			intval = strcmp (value->str, "on") == 0 ||
			         strcmp (value->str, "true") == 0 ||
			         strcmp (value->str, "1") == 0;

			options->wrap_mode = intval ? GTK_WRAP_WORD : GTK_WRAP_NONE;

			options->set |= MODELINE_SET_WRAP_MODE;			
		}
		else if (strcmp (key->str, "word-wrap-column") == 0)
		{
			intval = atoi (value->str);
			
			if (intval)
			{
				options->right_margin_position = intval;
				options->display_right_margin = TRUE;
				
				options->set |= MODELINE_SET_RIGHT_MARGIN_POSITION |
				                MODELINE_SET_SHOW_RIGHT_MARGIN;
			}
		}
	}

	g_string_free (key, TRUE);
	g_string_free (value, TRUE);

	return s;
}

/* Scan a line for vi(m)/emacs/kate modelines.
 * Line numbers are counted starting at one.
 */
static void
parse_modeline (gchar           *s,
		gint             line_number,
		gint             line_count,
		ModelineOptions *options)
{
	gchar prev;

	/* look for the beginning of a modeline */
	for (prev = ' '; (s != NULL) && (*s != '\0'); prev = *(s++))
	{
		if (!g_ascii_isspace (prev))
			continue;

		if ((line_number <= 3 || line_number > line_count - 3) &&
		    (strncmp (s, "ex:", 3) == 0 ||
		     strncmp (s, "vi:", 3) == 0 ||
		     strncmp (s, "vim:", 4) == 0))
		{
			gedit_debug_message (DEBUG_PLUGINS, "Vim modeline on line %d", line_number);

		    	while (*s != ':') s++;
		    	s = parse_vim_modeline (s + 1, options);
		}
		else if (line_number <= 2 && strncmp (s, "-*-", 3) == 0)
		{
			gedit_debug_message (DEBUG_PLUGINS, "Emacs modeline on line %d", line_number);

			s = parse_emacs_modeline (s + 3, options);
		}
		else if ((line_number <= 10 || line_number > line_count - 10) &&
			 strncmp (s, "kate:", 5) == 0)
		{
			gedit_debug_message (DEBUG_PLUGINS, "Kate modeline on line %d", line_number);

			s = parse_kate_modeline (s + 5, options);
		}
	}
}

static gboolean
check_previous (GtkSourceView   *view,
                ModelineOptions *previous,
                ModelineSet      set)
{
	GtkSourceBuffer *buffer = GTK_SOURCE_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)));
	   
	/* Do not restore default when this is the first time */
	if (!previous)
		return FALSE;

	/* Do not restore default when previous was not set */
	if (!(previous->set & set))
		return FALSE;

	/* Only restore default when setting has not changed */
	switch (set)
	{
		case MODELINE_SET_INSERT_SPACES:
			return gtk_source_view_get_insert_spaces_instead_of_tabs (view) ==
			       previous->insert_spaces;
		break;
		case MODELINE_SET_TAB_WIDTH:
			return gtk_source_view_get_tab_width (view) == previous->tab_width;
		break;
		case MODELINE_SET_INDENT_WIDTH:
			return gtk_source_view_get_indent_width (view) == previous->indent_width;
		break;
		case MODELINE_SET_WRAP_MODE:
			return gtk_text_view_get_wrap_mode (GTK_TEXT_VIEW (view)) ==
			       previous->wrap_mode;
		break;
		case MODELINE_SET_RIGHT_MARGIN_POSITION:
			return gtk_source_view_get_right_margin_position (view) ==
			       previous->right_margin_position;
		break;
		case MODELINE_SET_SHOW_RIGHT_MARGIN:
			return gtk_source_view_get_show_right_margin (view) ==
			       previous->display_right_margin;
		break;
		case MODELINE_SET_LANGUAGE:
		{
			GtkSourceLanguage *language = gtk_source_buffer_get_language (buffer);
			
			return (language == NULL && previous->language_id == NULL) ||
			       (language != NULL && g_strcmp0 (gtk_source_language_get_id (language),
			                                       previous->language_id) == 0);
		}
		break;
		default:
			return FALSE;
		break;
	}
}

static void
free_modeline_options (ModelineOptions *options)
{
	g_free (options->language_id);
	g_slice_free (ModelineOptions, options);
}

void
modeline_parser_apply_modeline (GtkSourceView *view)
{
	ModelineOptions options;
	GtkTextBuffer *buffer;
	GtkTextIter iter, liter;
	gint line_count;
	
	options.language_id = NULL;
	options.set = MODELINE_SET_NONE;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gtk_text_buffer_get_start_iter (buffer, &iter);

	line_count = gtk_text_buffer_get_line_count (buffer);

	/* Parse the modelines on the 10 first lines... */
	while ((gtk_text_iter_get_line (&iter) < 10) &&
	       !gtk_text_iter_is_end (&iter))
	{
		gchar *line;

		liter = iter;
		gtk_text_iter_forward_to_line_end (&iter);
		line = gtk_text_buffer_get_text (buffer, &liter, &iter, TRUE);

		parse_modeline (line,
				1 + gtk_text_iter_get_line (&iter),
				line_count,
				&options);

		gtk_text_iter_forward_line (&iter);

		g_free (line);
	}

	/* ...and on the 10 last ones (modelines are not allowed in between) */
	if (!gtk_text_iter_is_end (&iter))
	{
		gint cur_line;
		guint remaining_lines;

		/* we are on the 11th line (count from 0) */
		cur_line = gtk_text_iter_get_line (&iter);
		/* g_assert (10 == cur_line); */

		remaining_lines = line_count - cur_line - 1;

		if (remaining_lines > 10)
		{
			gtk_text_buffer_get_end_iter (buffer, &iter);
			gtk_text_iter_backward_lines (&iter, 9);
		}
	}

	while (!gtk_text_iter_is_end (&iter))
	{
		gchar *line;

		liter = iter;
		gtk_text_iter_forward_to_line_end (&iter);
		line = gtk_text_buffer_get_text (buffer, &liter, &iter, TRUE);

		parse_modeline (line,
				1 + gtk_text_iter_get_line (&iter),
				line_count,
				&options);

		gtk_text_iter_forward_line (&iter);

		g_free (line);
	}

	/* Try to set language */
	if (has_option (&options, MODELINE_SET_LANGUAGE) && options.language_id)
	{
		GtkSourceLanguageManager *manager;
		GtkSourceLanguage *language;

		manager = gedit_get_language_manager ();
		language = gtk_source_language_manager_get_language
				(manager, options.language_id);

		if (language != NULL)
		{
			gtk_source_buffer_set_language (GTK_SOURCE_BUFFER (buffer),
							language);
		}
	}

	ModelineOptions *previous = g_object_get_data (G_OBJECT (buffer), 
	                                               MODELINE_OPTIONS_DATA_KEY);

	/* Apply the options we got from modelines and restore defaults if
	   we set them before */
	if (has_option (&options, MODELINE_SET_INSERT_SPACES))
	{
		gtk_source_view_set_insert_spaces_instead_of_tabs
							(view, options.insert_spaces);
	}
	else if (check_previous (view, previous, MODELINE_SET_INSERT_SPACES))
	{
		gtk_source_view_set_insert_spaces_instead_of_tabs
							(view,
							 gedit_prefs_manager_get_insert_spaces ());
	}
	
	if (has_option (&options, MODELINE_SET_TAB_WIDTH))
	{
		gtk_source_view_set_tab_width (view, options.tab_width);
	}
	else if (check_previous (view, previous, MODELINE_SET_TAB_WIDTH))
	{
		gtk_source_view_set_tab_width (view, 
		                               gedit_prefs_manager_get_tabs_size ());
	}
	
	if (has_option (&options, MODELINE_SET_INDENT_WIDTH))
	{
		gtk_source_view_set_indent_width (view, options.indent_width);
	}
	else if (check_previous (view, previous, MODELINE_SET_INDENT_WIDTH))
	{
		gtk_source_view_set_indent_width (view, -1);
	}
	
	if (has_option (&options, MODELINE_SET_WRAP_MODE))
	{
		gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), options.wrap_mode);
	}
	else if (check_previous (view, previous, MODELINE_SET_WRAP_MODE))
	{
		gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), 
		                             gedit_prefs_manager_get_wrap_mode ());
	}
	
	if (has_option (&options, MODELINE_SET_RIGHT_MARGIN_POSITION))
	{
		gtk_source_view_set_right_margin_position (view, options.right_margin_position);
	}
	else if (check_previous (view, previous, MODELINE_SET_RIGHT_MARGIN_POSITION))
	{
		gtk_source_view_set_right_margin_position (view, 
		                                           gedit_prefs_manager_get_right_margin_position ());
	}
	
	if (has_option (&options, MODELINE_SET_SHOW_RIGHT_MARGIN))
	{
		gtk_source_view_set_show_right_margin (view, options.display_right_margin);
	}
	else if (check_previous (view, previous, MODELINE_SET_SHOW_RIGHT_MARGIN))
	{
		gtk_source_view_set_show_right_margin (view, 
		                                       gedit_prefs_manager_get_display_right_margin ());
	}
	
	if (previous)
	{
		*previous = options;
		previous->language_id = g_strdup (options.language_id);
	}
	else
	{
		previous = g_slice_new (ModelineOptions);
		*previous = options;
		previous->language_id = g_strdup (options.language_id);
		
		g_object_set_data_full (G_OBJECT (buffer), 
		                        MODELINE_OPTIONS_DATA_KEY, 
		                        previous,
		                        (GDestroyNotify)free_modeline_options);
	}
	
	g_free (options.language_id);
}

void
modeline_parser_deactivate (GtkSourceView *view)
{
	g_object_set_data (G_OBJECT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view))),
	                   MODELINE_OPTIONS_DATA_KEY,
	                   NULL);
}

/* vi:ts=8 */
