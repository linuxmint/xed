/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xed-prefs-manager.c
 * This file is part of xed
 *
 * Copyright (C) 2002  Paolo Maggi 
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
 * Modified by the xed Team, 2002. See the AUTHORS file for a 
 * list of people on the xed Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <glib/gi18n.h>
#include <gio/gio.h>

#include "xed-prefs-manager.h"
#include "xed-prefs-manager-private.h"
#include "xed-debug.h"
#include "xed-encodings.h"
#include "xed-utils.h"

#define DEFINE_BOOL_PREF(name, key) gboolean 				\
xed_prefs_manager_get_ ## name (void)					\
{									\
	xed_debug (DEBUG_PREFS);					\
									\
	return xed_prefs_manager_get_bool (key);			\
}									\
									\
void 									\
xed_prefs_manager_set_ ## name (gboolean v)				\
{									\
	xed_debug (DEBUG_PREFS);					\
									\
	xed_prefs_manager_set_bool (key,				\
				      v);				\
}									\
				      					\
gboolean								\
xed_prefs_manager_ ## name ## _can_set (void)				\
{									\
	xed_debug (DEBUG_PREFS);					\
									\
	return xed_prefs_manager_key_is_writable (key);		\
}	



#define DEFINE_INT_PREF(name, key) gint					\
xed_prefs_manager_get_ ## name (void)			 		\
{									\
	xed_debug (DEBUG_PREFS);					\
									\
	return xed_prefs_manager_get_int (key);			\
}									\
									\
void 									\
xed_prefs_manager_set_ ## name (gint v)				\
{									\
	xed_debug (DEBUG_PREFS);					\
									\
	xed_prefs_manager_set_int (key,				\
				     v);				\
}									\
				      					\
gboolean								\
xed_prefs_manager_ ## name ## _can_set (void)				\
{									\
	xed_debug (DEBUG_PREFS);					\
									\
	return xed_prefs_manager_key_is_writable (key);		\
}		



#define DEFINE_STRING_PREF(name, key) gchar*				\
xed_prefs_manager_get_ ## name (void)			 		\
{									\
	xed_debug (DEBUG_PREFS);					\
									\
	return xed_prefs_manager_get_string (key);			\
}									\
									\
void 									\
xed_prefs_manager_set_ ## name (const gchar* v)			\
{									\
	xed_debug (DEBUG_PREFS);					\
									\
	xed_prefs_manager_set_string (key,				\
				        v);				\
}									\
				      					\
gboolean								\
xed_prefs_manager_ ## name ## _can_set (void)				\
{									\
	xed_debug (DEBUG_PREFS);					\
									\
	return xed_prefs_manager_key_is_writable (key);		\
}		


XedPrefsManager *xed_prefs_manager = NULL;


static GtkWrapMode 	 get_wrap_mode_from_string 		(const gchar* str);

static gboolean		 xed_prefs_manager_get_bool		(const gchar* key);

static gint		 xed_prefs_manager_get_int		(const gchar* key);

static gchar		*xed_prefs_manager_get_string		(const gchar* key);


gboolean
xed_prefs_manager_init (void)
{
	xed_debug (DEBUG_PREFS);

	if (xed_prefs_manager == NULL)
	{
		xed_prefs_manager = g_new0 (XedPrefsManager, 1);
		xed_prefs_manager->settings = g_settings_new (XED_SCHEMA);
		xed_prefs_manager->interface_settings = g_settings_new (GPM_INTERFACE_SCHEMA);
	}

	return xed_prefs_manager != NULL;
}

void
xed_prefs_manager_shutdown (void)
{
	xed_debug (DEBUG_PREFS);

	g_return_if_fail (xed_prefs_manager != NULL);

	g_object_unref (xed_prefs_manager->settings);
	xed_prefs_manager->settings = NULL;
	g_object_unref (xed_prefs_manager->interface_settings);
	xed_prefs_manager->interface_settings = NULL;
}

static gboolean		 
xed_prefs_manager_get_bool (const gchar* key)
{
	xed_debug (DEBUG_PREFS);

	return g_settings_get_boolean (xed_prefs_manager->settings, key);
}

static gint 
xed_prefs_manager_get_int (const gchar* key)
{
	xed_debug (DEBUG_PREFS);

	return g_settings_get_int (xed_prefs_manager->settings, key);
}

static gchar *
xed_prefs_manager_get_string (const gchar* key)
{
	xed_debug (DEBUG_PREFS);

	return g_settings_get_string (xed_prefs_manager->settings, key);
}

static void		 
xed_prefs_manager_set_bool (const gchar* key, gboolean value)
{
	xed_debug (DEBUG_PREFS);

	g_return_if_fail (g_settings_is_writable (
				xed_prefs_manager->settings, key));

	g_settings_set_boolean (xed_prefs_manager->settings, key, value);
}

static void		 
xed_prefs_manager_set_int (const gchar* key, gint value)
{
	xed_debug (DEBUG_PREFS);

	g_return_if_fail (g_settings_is_writable (
				xed_prefs_manager->settings, key));

	g_settings_set_int (xed_prefs_manager->settings, key, value);
}

static void		 
xed_prefs_manager_set_string (const gchar* key, const gchar* value)
{
	xed_debug (DEBUG_PREFS);

	g_return_if_fail (value != NULL);
	
	g_return_if_fail (g_settings_is_writable (
				xed_prefs_manager->settings, key));

	g_settings_set_string (xed_prefs_manager->settings, key, value);
}

static gboolean 
xed_prefs_manager_key_is_writable (const gchar* key)
{
	xed_debug (DEBUG_PREFS);

	g_return_val_if_fail (xed_prefs_manager != NULL, FALSE);
	g_return_val_if_fail (xed_prefs_manager->settings != NULL, FALSE);

	return g_settings_is_writable (xed_prefs_manager->settings, key);
}

/* Use default font */
DEFINE_BOOL_PREF (use_default_font,
		  GPM_USE_DEFAULT_FONT)

/* Editor font */
DEFINE_STRING_PREF (editor_font,
		    GPM_EDITOR_FONT)

/* System font */
gchar *
xed_prefs_manager_get_system_font (void)
{
	xed_debug (DEBUG_PREFS);

	return g_settings_get_string (xed_prefs_manager->interface_settings,
				      GPM_SYSTEM_FONT);
}

/* Create backup copy */
DEFINE_BOOL_PREF (create_backup_copy,
		  GPM_CREATE_BACKUP_COPY)

/* Auto save */
DEFINE_BOOL_PREF (auto_save,
		  GPM_AUTO_SAVE)

/* Auto save interval */
DEFINE_INT_PREF (auto_save_interval,
		 GPM_AUTO_SAVE_INTERVAL)


/* Undo actions limit: if < 1 then no limits */
DEFINE_INT_PREF (undo_actions_limit,
		 GPM_UNDO_ACTIONS_LIMIT)

static GtkWrapMode 
get_wrap_mode_from_string (const gchar* str)
{
	GtkWrapMode res;

	g_return_val_if_fail (str != NULL, GTK_WRAP_WORD);
	
	if (strcmp (str, "GTK_WRAP_NONE") == 0)
		res = GTK_WRAP_NONE;
	else
	{
		if (strcmp (str, "GTK_WRAP_CHAR") == 0)
			res = GTK_WRAP_CHAR;
		else
			res = GTK_WRAP_WORD;
	}

	return res;
}

/* Wrap mode */
GtkWrapMode
xed_prefs_manager_get_wrap_mode (void)
{
	gchar *str;
	GtkWrapMode res;
	
	xed_debug (DEBUG_PREFS);
	
	str = xed_prefs_manager_get_string (GPM_WRAP_MODE);

	res = get_wrap_mode_from_string (str);

	g_free (str);

	return res;
}
	
void
xed_prefs_manager_set_wrap_mode (GtkWrapMode wp)
{
	const gchar * str;
	
	xed_debug (DEBUG_PREFS);

	switch (wp)
	{
		case GTK_WRAP_NONE:
			str = "GTK_WRAP_NONE";
			break;

		case GTK_WRAP_CHAR:
			str = "GTK_WRAP_CHAR";
			break;

		default: /* GTK_WRAP_WORD */
			str = "GTK_WRAP_WORD";
	}

	xed_prefs_manager_set_string (GPM_WRAP_MODE,
					str);
}
	
gboolean
xed_prefs_manager_wrap_mode_can_set (void)
{
	xed_debug (DEBUG_PREFS);
	
	return xed_prefs_manager_key_is_writable (GPM_WRAP_MODE);
}


/* Tabs size */
DEFINE_INT_PREF (tabs_size, 
		 GPM_TABS_SIZE)

/* Insert spaces */
DEFINE_BOOL_PREF (insert_spaces, 
		  GPM_INSERT_SPACES)

/* Auto indent */
DEFINE_BOOL_PREF (auto_indent, 
		  GPM_AUTO_INDENT)

/* Display line numbers */
DEFINE_BOOL_PREF (display_line_numbers, 
		  GPM_DISPLAY_LINE_NUMBERS)

/* Toolbar visibility */
DEFINE_BOOL_PREF (toolbar_visible,
		  GPM_TOOLBAR_VISIBLE)

/* Statusbar visiblity */
DEFINE_BOOL_PREF (statusbar_visible,
		  GPM_STATUSBAR_VISIBLE)
		  
/* Side Pane visiblity */
DEFINE_BOOL_PREF (side_pane_visible,
		  GPM_SIDE_PANE_VISIBLE)
		  
/* Bottom Panel visiblity */
DEFINE_BOOL_PREF (bottom_panel_visible,
		  GPM_BOTTOM_PANEL_VISIBLE)

/* Print syntax highlighting */
DEFINE_BOOL_PREF (print_syntax_hl,
		  GPM_PRINT_SYNTAX)

/* Print header */
DEFINE_BOOL_PREF (print_header,
		  GPM_PRINT_HEADER)


/* Print Wrap mode */
GtkWrapMode
xed_prefs_manager_get_print_wrap_mode (void)
{
	gchar *str;
	GtkWrapMode res;
	
	xed_debug (DEBUG_PREFS);
	
	str = xed_prefs_manager_get_string (GPM_PRINT_WRAP_MODE);

	if (strcmp (str, "GTK_WRAP_NONE") == 0)
		res = GTK_WRAP_NONE;
	else
	{
		if (strcmp (str, "GTK_WRAP_WORD") == 0)
			res = GTK_WRAP_WORD;
		else
			res = GTK_WRAP_CHAR;
	}

	g_free (str);

	return res;
}
	
void
xed_prefs_manager_set_print_wrap_mode (GtkWrapMode pwp)
{
	const gchar *str;

	xed_debug (DEBUG_PREFS);

	switch (pwp)
	{
		case GTK_WRAP_NONE:
			str = "GTK_WRAP_NONE";
			break;

		case GTK_WRAP_WORD:
			str = "GTK_WRAP_WORD";
			break;

		default: /* GTK_WRAP_CHAR */
			str = "GTK_WRAP_CHAR";
	}

	xed_prefs_manager_set_string (GPM_PRINT_WRAP_MODE, str);
}

gboolean
xed_prefs_manager_print_wrap_mode_can_set (void)
{
	xed_debug (DEBUG_PREFS);
	
	return xed_prefs_manager_key_is_writable (GPM_PRINT_WRAP_MODE);
}

/* Print line numbers */	
DEFINE_INT_PREF (print_line_numbers,
		 GPM_PRINT_LINE_NUMBERS)

/* Printing fonts */
DEFINE_STRING_PREF (print_font_body,
		    GPM_PRINT_FONT_BODY)

static gchar *
xed_prefs_manager_get_default_string_value (const gchar *key)
{
	gchar *font = NULL;
	g_settings_delay (xed_prefs_manager->settings);
	g_settings_reset (xed_prefs_manager->settings, key);
	font = g_settings_get_string (xed_prefs_manager->settings, key);
	g_settings_revert (xed_prefs_manager->settings);
	return font;
}

gchar *
xed_prefs_manager_get_default_print_font_body (void)
{
	return xed_prefs_manager_get_default_string_value (GPM_PRINT_FONT_BODY);
}

DEFINE_STRING_PREF (print_font_header,
		    GPM_PRINT_FONT_HEADER)

gchar *
xed_prefs_manager_get_default_print_font_header (void)
{
	return xed_prefs_manager_get_default_string_value (GPM_PRINT_FONT_HEADER);
}

DEFINE_STRING_PREF (print_font_numbers,
		    GPM_PRINT_FONT_NUMBERS)

gchar *
xed_prefs_manager_get_default_print_font_numbers (void)
{
	return xed_prefs_manager_get_default_string_value (GPM_PRINT_FONT_NUMBERS);
}

/* Max number of files in "Recent Files" menu. 
 * This is configurable only using gsettings, dconf or dconf-editor 
 */
gint
xed_prefs_manager_get_max_recents (void)
{
	xed_debug (DEBUG_PREFS);

	return xed_prefs_manager_get_int (GPM_MAX_RECENTS);

}

/* GSettings/GSList utility functions from mate-panel */

GSList*
xed_prefs_manager_get_gslist (GSettings *settings, const gchar *key)
{
    gchar **array;
    GSList *list = NULL;
    gint i;
    array = g_settings_get_strv (settings, key);
    if (array != NULL) {
        for (i = 0; array[i]; i++) {
            list = g_slist_append (list, g_strdup (array[i]));
        }
    }
    g_strfreev (array);
    return list;
}

void
xed_prefs_manager_set_gslist (GSettings *settings, const gchar *key, GSList *list)
{
    GArray *array;
    GSList *l;
    array = g_array_new (TRUE, TRUE, sizeof (gchar *));
    for (l = list; l; l = l->next) {
        array = g_array_append_val (array, l->data);
    }
    g_settings_set_strv (settings, key, (const gchar **) array->data);
    g_array_free (array, TRUE);
}


/* Encodings */

static gboolean
data_exists (GSList         *list,
	    const gpointer  data)
{
	while (list != NULL)
	{
      		if (list->data == data)
			return TRUE;

		list = g_slist_next (list);
    	}

  	return FALSE;
}

GSList *
xed_prefs_manager_get_auto_detected_encodings (void)
{
	GSList *strings;
	GSList *res = NULL;

	xed_debug (DEBUG_PREFS);

	g_return_val_if_fail (xed_prefs_manager != NULL, NULL);
	g_return_val_if_fail (xed_prefs_manager->settings != NULL, NULL);

	strings = xed_prefs_manager_get_gslist (xed_prefs_manager->settings, GPM_AUTO_DETECTED_ENCODINGS);

	if (strings != NULL)
	{	
		GSList *tmp;
		const XedEncoding *enc;

		tmp = strings;
		
		while (tmp)
		{
		      const char *charset = tmp->data;
      
		      if (strcmp (charset, "CURRENT") == 0)
			      g_get_charset (&charset);

		      g_return_val_if_fail (charset != NULL, NULL);
		      enc = xed_encoding_get_from_charset (charset);

		      if (enc != NULL)
		      {
			      if (!data_exists (res, (gpointer)enc))
				      res = g_slist_prepend (res, (gpointer)enc);

		      }

		      tmp = g_slist_next (tmp);
		}

		g_slist_foreach (strings, (GFunc) g_free, NULL);
		g_slist_free (strings);    

	 	res = g_slist_reverse (res);
	}

	xed_debug_message (DEBUG_PREFS, "Done");

	return res;
}

GSList *
xed_prefs_manager_get_shown_in_menu_encodings (void)
{
	GSList *strings;
	GSList *res = NULL;

	xed_debug (DEBUG_PREFS);

	g_return_val_if_fail (xed_prefs_manager != NULL, NULL);
	g_return_val_if_fail (xed_prefs_manager->settings != NULL, NULL);

	strings = xed_prefs_manager_get_gslist (xed_prefs_manager->settings, GPM_SHOWN_IN_MENU_ENCODINGS);

	if (strings != NULL)
	{	
		GSList *tmp;
		const XedEncoding *enc;

		tmp = strings;
		
		while (tmp)
		{
		      const char *charset = tmp->data;

		      if (strcmp (charset, "CURRENT") == 0)
			      g_get_charset (&charset);

		      g_return_val_if_fail (charset != NULL, NULL);
		      enc = xed_encoding_get_from_charset (charset);

		      if (enc != NULL)
		      {
			      if (!data_exists (res, (gpointer)enc))
				      res = g_slist_prepend (res, (gpointer)enc);
		      }

		      tmp = g_slist_next (tmp);
		}

		g_slist_foreach (strings, (GFunc) g_free, NULL);
		g_slist_free (strings);    

	 	res = g_slist_reverse (res);
	}

	return res;
}

void
xed_prefs_manager_set_shown_in_menu_encodings (const GSList *encs)
{	
	GSList *list = NULL;
	
	g_return_if_fail (xed_prefs_manager != NULL);
	g_return_if_fail (xed_prefs_manager->settings != NULL);
	g_return_if_fail (xed_prefs_manager_shown_in_menu_encodings_can_set ());

	while (encs != NULL)
	{
		const XedEncoding *enc;
		const gchar *charset;
		
		enc = (const XedEncoding *)encs->data;

		charset = xed_encoding_get_charset (enc);
		g_return_if_fail (charset != NULL);

		list = g_slist_prepend (list, (gpointer)charset);

		encs = g_slist_next (encs);
	}

	list = g_slist_reverse (list);

	xed_prefs_manager_set_gslist (xed_prefs_manager->settings, GPM_SHOWN_IN_MENU_ENCODINGS, list);

	g_slist_free (list);
}

gboolean
xed_prefs_manager_shown_in_menu_encodings_can_set (void)
{
	xed_debug (DEBUG_PREFS);
	
	return xed_prefs_manager_key_is_writable (GPM_SHOWN_IN_MENU_ENCODINGS);

}

/* Highlight current line */
DEFINE_BOOL_PREF (highlight_current_line,
		  GPM_HIGHLIGHT_CURRENT_LINE)

/* Highlight matching bracket */
DEFINE_BOOL_PREF (bracket_matching,
		  GPM_BRACKET_MATCHING)
	
/* Display Right Margin */
DEFINE_BOOL_PREF (display_right_margin,
		  GPM_DISPLAY_RIGHT_MARGIN)

/* Right Margin Position */	
DEFINE_INT_PREF (right_margin_position,
		 GPM_RIGHT_MARGIN_POSITION)

static GtkSourceSmartHomeEndType
get_smart_home_end_from_string (const gchar *str)
{
	GtkSourceSmartHomeEndType res;

	g_return_val_if_fail (str != NULL, GTK_SOURCE_SMART_HOME_END_AFTER);

	if (strcmp (str, "DISABLED") == 0)
		res = GTK_SOURCE_SMART_HOME_END_DISABLED;
	else if (strcmp (str, "BEFORE") == 0)
		res = GTK_SOURCE_SMART_HOME_END_BEFORE;
	else if (strcmp (str, "ALWAYS") == 0)
		res = GTK_SOURCE_SMART_HOME_END_ALWAYS;
	else
		res = GTK_SOURCE_SMART_HOME_END_AFTER;

	return res;
}

GtkSourceSmartHomeEndType
xed_prefs_manager_get_smart_home_end (void)
{
	gchar *str;
	GtkSourceSmartHomeEndType res;

	xed_debug (DEBUG_PREFS);

	str = xed_prefs_manager_get_string (GPM_SMART_HOME_END);

	res = get_smart_home_end_from_string (str);

	g_free (str);

	return res;
}
	
void
xed_prefs_manager_set_smart_home_end (GtkSourceSmartHomeEndType smart_he)
{
	const gchar *str;

	xed_debug (DEBUG_PREFS);

	switch (smart_he)
	{
		case GTK_SOURCE_SMART_HOME_END_DISABLED:
			str = "DISABLED";
			break;

		case GTK_SOURCE_SMART_HOME_END_BEFORE:
			str = "BEFORE";
			break;

		case GTK_SOURCE_SMART_HOME_END_ALWAYS:
			str = "ALWAYS";
			break;

		default: /* GTK_SOURCE_SMART_HOME_END_AFTER */
			str = "AFTER";
	}

	xed_prefs_manager_set_string (GPM_WRAP_MODE, str);
}

gboolean
xed_prefs_manager_smart_home_end_can_set (void)
{
	xed_debug (DEBUG_PREFS);
	
	return xed_prefs_manager_key_is_writable (GPM_SMART_HOME_END);
}

/* Enable syntax highlighting */
DEFINE_BOOL_PREF (enable_syntax_highlighting,
		  GPM_SYNTAX_HL_ENABLE)

/* Enable search highlighting */
DEFINE_BOOL_PREF (enable_search_highlighting,
		  GPM_SEARCH_HIGHLIGHTING_ENABLE)

/* Source style scheme */
DEFINE_STRING_PREF (source_style_scheme,
		    GPM_SOURCE_STYLE_SCHEME)

GSList *
xed_prefs_manager_get_writable_vfs_schemes (void)
{
	GSList *strings;
	
	xed_debug (DEBUG_PREFS);

	g_return_val_if_fail (xed_prefs_manager != NULL, NULL);
	g_return_val_if_fail (xed_prefs_manager->settings != NULL, NULL);

	strings = xed_prefs_manager_get_gslist (xed_prefs_manager->settings, GPM_WRITABLE_VFS_SCHEMES);

	/* The 'file' scheme is writable by default. */
	strings = g_slist_prepend (strings, g_strdup ("file")); 
	
	xed_debug_message (DEBUG_PREFS, "Done");

	return strings;
}

gboolean
xed_prefs_manager_get_restore_cursor_position (void)
{
	xed_debug (DEBUG_PREFS);

	return xed_prefs_manager_get_bool (GPM_RESTORE_CURSOR_POSITION);
}

/* Plugins: we just store/return a list of strings, all the magic has to
 * happen in the plugin engine */

GSList *
xed_prefs_manager_get_active_plugins (void)
{
	GSList *plugins;

	xed_debug (DEBUG_PREFS);

	g_return_val_if_fail (xed_prefs_manager != NULL, NULL);
	g_return_val_if_fail (xed_prefs_manager->settings != NULL, NULL);

	plugins = xed_prefs_manager_get_gslist (xed_prefs_manager->settings, GPM_ACTIVE_PLUGINS);

	return plugins;
}

void
xed_prefs_manager_set_active_plugins (const GSList *plugins)
{	
	g_return_if_fail (xed_prefs_manager != NULL);
	g_return_if_fail (xed_prefs_manager->settings != NULL);
	g_return_if_fail (xed_prefs_manager_active_plugins_can_set ());

	xed_prefs_manager_set_gslist (xed_prefs_manager->settings, GPM_ACTIVE_PLUGINS, (GSList *) plugins);
}

gboolean
xed_prefs_manager_active_plugins_can_set (void)
{
	xed_debug (DEBUG_PREFS);

	return xed_prefs_manager_key_is_writable (GPM_ACTIVE_PLUGINS);
}
