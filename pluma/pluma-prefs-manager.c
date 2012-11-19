/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pluma-prefs-manager.c
 * This file is part of pluma
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
 * Modified by the pluma Team, 2002. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <glib/gi18n.h>
#include <mateconf/mateconf-value.h>

#include "pluma-prefs-manager.h"
#include "pluma-prefs-manager-private.h"
#include "pluma-debug.h"
#include "pluma-encodings.h"
#include "pluma-utils.h"

#define DEFINE_BOOL_PREF(name, key, def) gboolean 			\
pluma_prefs_manager_get_ ## name (void)					\
{									\
	pluma_debug (DEBUG_PREFS);					\
									\
	return pluma_prefs_manager_get_bool (key,			\
					     (def));			\
}									\
									\
void 									\
pluma_prefs_manager_set_ ## name (gboolean v)				\
{									\
	pluma_debug (DEBUG_PREFS);					\
									\
	pluma_prefs_manager_set_bool (key,				\
				      v);				\
}									\
				      					\
gboolean								\
pluma_prefs_manager_ ## name ## _can_set (void)				\
{									\
	pluma_debug (DEBUG_PREFS);					\
									\
	return pluma_prefs_manager_key_is_writable (key);		\
}	



#define DEFINE_INT_PREF(name, key, def) gint	 			\
pluma_prefs_manager_get_ ## name (void)			 		\
{									\
	pluma_debug (DEBUG_PREFS);					\
									\
	return pluma_prefs_manager_get_int (key,			\
					    (def));			\
}									\
									\
void 									\
pluma_prefs_manager_set_ ## name (gint v)				\
{									\
	pluma_debug (DEBUG_PREFS);					\
									\
	pluma_prefs_manager_set_int (key,				\
				     v);				\
}									\
				      					\
gboolean								\
pluma_prefs_manager_ ## name ## _can_set (void)				\
{									\
	pluma_debug (DEBUG_PREFS);					\
									\
	return pluma_prefs_manager_key_is_writable (key);		\
}		



#define DEFINE_STRING_PREF(name, key, def) gchar*	 		\
pluma_prefs_manager_get_ ## name (void)			 		\
{									\
	pluma_debug (DEBUG_PREFS);					\
									\
	return pluma_prefs_manager_get_string (key,			\
					       def);			\
}									\
									\
void 									\
pluma_prefs_manager_set_ ## name (const gchar* v)			\
{									\
	pluma_debug (DEBUG_PREFS);					\
									\
	pluma_prefs_manager_set_string (key,				\
				        v);				\
}									\
				      					\
gboolean								\
pluma_prefs_manager_ ## name ## _can_set (void)				\
{									\
	pluma_debug (DEBUG_PREFS);					\
									\
	return pluma_prefs_manager_key_is_writable (key);		\
}		


PlumaPrefsManager *pluma_prefs_manager = NULL;


static GtkWrapMode 	 get_wrap_mode_from_string 		(const gchar* str);

static gboolean 	 mateconf_client_get_bool_with_default 	(MateConfClient* client, 
								 const gchar* key, 
								 gboolean def, 
								 GError** err);

static gchar		*mateconf_client_get_string_with_default 	(MateConfClient* client, 
								 const gchar* key,
								 const gchar* def, 
								 GError** err);

static gint		 mateconf_client_get_int_with_default 	(MateConfClient* client, 
								 const gchar* key,
								 gint def, 
								 GError** err);

static gboolean		 pluma_prefs_manager_get_bool		(const gchar* key, 
								 gboolean def);

static gint		 pluma_prefs_manager_get_int		(const gchar* key, 
								 gint def);

static gchar		*pluma_prefs_manager_get_string		(const gchar* key, 
								 const gchar* def);


gboolean
pluma_prefs_manager_init (void)
{
	pluma_debug (DEBUG_PREFS);

	if (pluma_prefs_manager == NULL)
	{
		MateConfClient *mateconf_client;

		mateconf_client = mateconf_client_get_default ();
		if (mateconf_client == NULL)
		{
			g_warning (_("Cannot initialize preferences manager."));
			return FALSE;
		}

		pluma_prefs_manager = g_new0 (PlumaPrefsManager, 1);

		pluma_prefs_manager->mateconf_client = mateconf_client;
	}

	if (pluma_prefs_manager->mateconf_client == NULL)
	{
		g_free (pluma_prefs_manager);
		pluma_prefs_manager = NULL;
	}

	return pluma_prefs_manager != NULL;
	
}

void
pluma_prefs_manager_shutdown (void)
{
	pluma_debug (DEBUG_PREFS);

	g_return_if_fail (pluma_prefs_manager != NULL);

	g_object_unref (pluma_prefs_manager->mateconf_client);
	pluma_prefs_manager->mateconf_client = NULL;
}

static gboolean		 
pluma_prefs_manager_get_bool (const gchar* key, gboolean def)
{
	pluma_debug (DEBUG_PREFS);

	g_return_val_if_fail (pluma_prefs_manager != NULL, def);
	g_return_val_if_fail (pluma_prefs_manager->mateconf_client != NULL, def);

	return mateconf_client_get_bool_with_default (pluma_prefs_manager->mateconf_client,
						   key,
						   def,
						   NULL);
}

static gint 
pluma_prefs_manager_get_int (const gchar* key, gint def)
{
	pluma_debug (DEBUG_PREFS);

	g_return_val_if_fail (pluma_prefs_manager != NULL, def);
	g_return_val_if_fail (pluma_prefs_manager->mateconf_client != NULL, def);

	return mateconf_client_get_int_with_default (pluma_prefs_manager->mateconf_client,
						  key,
						  def,
						  NULL);
}	

static gchar *
pluma_prefs_manager_get_string (const gchar* key, const gchar* def)
{
	pluma_debug (DEBUG_PREFS);

	g_return_val_if_fail (pluma_prefs_manager != NULL, 
			      def ? g_strdup (def) : NULL);
	g_return_val_if_fail (pluma_prefs_manager->mateconf_client != NULL, 
			      def ? g_strdup (def) : NULL);

	return mateconf_client_get_string_with_default (pluma_prefs_manager->mateconf_client,
						     key,
						     def,
						     NULL);
}	

static void		 
pluma_prefs_manager_set_bool (const gchar* key, gboolean value)
{
	pluma_debug (DEBUG_PREFS);

	g_return_if_fail (pluma_prefs_manager != NULL);
	g_return_if_fail (pluma_prefs_manager->mateconf_client != NULL);
	g_return_if_fail (mateconf_client_key_is_writable (
				pluma_prefs_manager->mateconf_client, key, NULL));
			
	mateconf_client_set_bool (pluma_prefs_manager->mateconf_client, key, value, NULL);
}

static void		 
pluma_prefs_manager_set_int (const gchar* key, gint value)
{
	pluma_debug (DEBUG_PREFS);

	g_return_if_fail (pluma_prefs_manager != NULL);
	g_return_if_fail (pluma_prefs_manager->mateconf_client != NULL);
	g_return_if_fail (mateconf_client_key_is_writable (
				pluma_prefs_manager->mateconf_client, key, NULL));
			
	mateconf_client_set_int (pluma_prefs_manager->mateconf_client, key, value, NULL);
}

static void		 
pluma_prefs_manager_set_string (const gchar* key, const gchar* value)
{
	pluma_debug (DEBUG_PREFS);

	g_return_if_fail (value != NULL);
	
	g_return_if_fail (pluma_prefs_manager != NULL);
	g_return_if_fail (pluma_prefs_manager->mateconf_client != NULL);
	g_return_if_fail (mateconf_client_key_is_writable (
				pluma_prefs_manager->mateconf_client, key, NULL));
			
	mateconf_client_set_string (pluma_prefs_manager->mateconf_client, key, value, NULL);
}

static gboolean 
pluma_prefs_manager_key_is_writable (const gchar* key)
{
	pluma_debug (DEBUG_PREFS);

	g_return_val_if_fail (pluma_prefs_manager != NULL, FALSE);
	g_return_val_if_fail (pluma_prefs_manager->mateconf_client != NULL, FALSE);

	return mateconf_client_key_is_writable (pluma_prefs_manager->mateconf_client, key, NULL);
}

/* Use default font */
DEFINE_BOOL_PREF (use_default_font,
		  GPM_USE_DEFAULT_FONT,
		  GPM_DEFAULT_USE_DEFAULT_FONT)

/* Editor font */
DEFINE_STRING_PREF (editor_font,
		    GPM_EDITOR_FONT,
		    GPM_DEFAULT_EDITOR_FONT)

/* System font */
gchar *
pluma_prefs_manager_get_system_font (void)
{
	pluma_debug (DEBUG_PREFS);

	return pluma_prefs_manager_get_string (GPM_SYSTEM_FONT,
					       GPM_DEFAULT_SYSTEM_FONT);
}

/* Create backup copy */
DEFINE_BOOL_PREF (create_backup_copy,
		  GPM_CREATE_BACKUP_COPY,
		  GPM_DEFAULT_CREATE_BACKUP_COPY)

/* Auto save */
DEFINE_BOOL_PREF (auto_save,
		  GPM_AUTO_SAVE,
		  GPM_DEFAULT_AUTO_SAVE)

/* Auto save interval */
DEFINE_INT_PREF (auto_save_interval,
		 GPM_AUTO_SAVE_INTERVAL,
		 GPM_DEFAULT_AUTO_SAVE_INTERVAL)


/* Undo actions limit: if < 1 then no limits */
DEFINE_INT_PREF (undo_actions_limit,
		 GPM_UNDO_ACTIONS_LIMIT,
		 GPM_DEFAULT_UNDO_ACTIONS_LIMIT)

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
pluma_prefs_manager_get_wrap_mode (void)
{
	gchar *str;
	GtkWrapMode res;
	
	pluma_debug (DEBUG_PREFS);
	
	str = pluma_prefs_manager_get_string (GPM_WRAP_MODE,
					      GPM_DEFAULT_WRAP_MODE);

	res = get_wrap_mode_from_string (str);

	g_free (str);

	return res;
}
	
void
pluma_prefs_manager_set_wrap_mode (GtkWrapMode wp)
{
	const gchar * str;
	
	pluma_debug (DEBUG_PREFS);

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

	pluma_prefs_manager_set_string (GPM_WRAP_MODE,
					str);
}
	
gboolean
pluma_prefs_manager_wrap_mode_can_set (void)
{
	pluma_debug (DEBUG_PREFS);
	
	return pluma_prefs_manager_key_is_writable (GPM_WRAP_MODE);
}


/* Tabs size */
DEFINE_INT_PREF (tabs_size, 
		 GPM_TABS_SIZE, 
		 GPM_DEFAULT_TABS_SIZE)

/* Insert spaces */
DEFINE_BOOL_PREF (insert_spaces, 
		  GPM_INSERT_SPACES, 
		  GPM_DEFAULT_INSERT_SPACES)

/* Auto indent */
DEFINE_BOOL_PREF (auto_indent, 
		  GPM_AUTO_INDENT, 
		  GPM_DEFAULT_AUTO_INDENT)

/* Display line numbers */
DEFINE_BOOL_PREF (display_line_numbers, 
		  GPM_DISPLAY_LINE_NUMBERS, 
		  GPM_DEFAULT_DISPLAY_LINE_NUMBERS)

/* Toolbar visibility */
DEFINE_BOOL_PREF (toolbar_visible,
		  GPM_TOOLBAR_VISIBLE,
		  GPM_DEFAULT_TOOLBAR_VISIBLE)


/* Toolbar suttons style */
PlumaToolbarSetting 
pluma_prefs_manager_get_toolbar_buttons_style (void) 
{
	gchar *str;
	PlumaToolbarSetting res;
	
	pluma_debug (DEBUG_PREFS);
	
	str = pluma_prefs_manager_get_string (GPM_TOOLBAR_BUTTONS_STYLE,
					      GPM_DEFAULT_TOOLBAR_BUTTONS_STYLE);

	if (strcmp (str, "PLUMA_TOOLBAR_ICONS") == 0)
		res = PLUMA_TOOLBAR_ICONS;
	else
	{
		if (strcmp (str, "PLUMA_TOOLBAR_ICONS_AND_TEXT") == 0)
			res = PLUMA_TOOLBAR_ICONS_AND_TEXT;
		else 
		{
			if (strcmp (str, "PLUMA_TOOLBAR_ICONS_BOTH_HORIZ") == 0)
				res = PLUMA_TOOLBAR_ICONS_BOTH_HORIZ;
			else
				res = PLUMA_TOOLBAR_SYSTEM;
		}
	}

	g_free (str);

	return res;
}

void
pluma_prefs_manager_set_toolbar_buttons_style (PlumaToolbarSetting tbs)
{
	const gchar * str;
	
	pluma_debug (DEBUG_PREFS);

	switch (tbs)
	{
		case PLUMA_TOOLBAR_ICONS:
			str = "PLUMA_TOOLBAR_ICONS";
			break;

		case PLUMA_TOOLBAR_ICONS_AND_TEXT:
			str = "PLUMA_TOOLBAR_ICONS_AND_TEXT";
			break;

	        case PLUMA_TOOLBAR_ICONS_BOTH_HORIZ:
			str = "PLUMA_TOOLBAR_ICONS_BOTH_HORIZ";
			break;
		default: /* PLUMA_TOOLBAR_SYSTEM */
			str = "PLUMA_TOOLBAR_SYSTEM";
	}

	pluma_prefs_manager_set_string (GPM_TOOLBAR_BUTTONS_STYLE,
					str);

}

gboolean
pluma_prefs_manager_toolbar_buttons_style_can_set (void)
{
	pluma_debug (DEBUG_PREFS);
	
	return pluma_prefs_manager_key_is_writable (GPM_TOOLBAR_BUTTONS_STYLE);

}

/* Statusbar visiblity */
DEFINE_BOOL_PREF (statusbar_visible,
		  GPM_STATUSBAR_VISIBLE,
		  GPM_DEFAULT_STATUSBAR_VISIBLE)
		  
/* Side Pane visiblity */
DEFINE_BOOL_PREF (side_pane_visible,
		  GPM_SIDE_PANE_VISIBLE,
		  GPM_DEFAULT_SIDE_PANE_VISIBLE)
		  
/* Bottom Panel visiblity */
DEFINE_BOOL_PREF (bottom_panel_visible,
		  GPM_BOTTOM_PANEL_VISIBLE,
		  GPM_DEFAULT_BOTTOM_PANEL_VISIBLE)		  		  

/* Print syntax highlighting */
DEFINE_BOOL_PREF (print_syntax_hl,
		  GPM_PRINT_SYNTAX,
		  GPM_DEFAULT_PRINT_SYNTAX)

/* Print header */
DEFINE_BOOL_PREF (print_header,
		  GPM_PRINT_HEADER,
		  GPM_DEFAULT_PRINT_HEADER)


/* Print Wrap mode */
GtkWrapMode
pluma_prefs_manager_get_print_wrap_mode (void)
{
	gchar *str;
	GtkWrapMode res;
	
	pluma_debug (DEBUG_PREFS);
	
	str = pluma_prefs_manager_get_string (GPM_PRINT_WRAP_MODE,
					      GPM_DEFAULT_PRINT_WRAP_MODE);

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
pluma_prefs_manager_set_print_wrap_mode (GtkWrapMode pwp)
{
	const gchar *str;

	pluma_debug (DEBUG_PREFS);

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

	pluma_prefs_manager_set_string (GPM_PRINT_WRAP_MODE, str);
}

gboolean
pluma_prefs_manager_print_wrap_mode_can_set (void)
{
	pluma_debug (DEBUG_PREFS);
	
	return pluma_prefs_manager_key_is_writable (GPM_PRINT_WRAP_MODE);
}

/* Print line numbers */	
DEFINE_INT_PREF (print_line_numbers,
		 GPM_PRINT_LINE_NUMBERS,
		 GPM_DEFAULT_PRINT_LINE_NUMBERS)

/* Printing fonts */
DEFINE_STRING_PREF (print_font_body,
		    GPM_PRINT_FONT_BODY,
		    GPM_DEFAULT_PRINT_FONT_BODY)

const gchar *
pluma_prefs_manager_get_default_print_font_body (void)
{
	return GPM_DEFAULT_PRINT_FONT_BODY;
}

DEFINE_STRING_PREF (print_font_header,
		    GPM_PRINT_FONT_HEADER,
		    GPM_DEFAULT_PRINT_FONT_HEADER)

const gchar *
pluma_prefs_manager_get_default_print_font_header (void)
{
	return GPM_DEFAULT_PRINT_FONT_HEADER;
}

DEFINE_STRING_PREF (print_font_numbers,
		    GPM_PRINT_FONT_NUMBERS,
		    GPM_DEFAULT_PRINT_FONT_NUMBERS)

const gchar *
pluma_prefs_manager_get_default_print_font_numbers (void)
{
	return GPM_DEFAULT_PRINT_FONT_NUMBERS;
}

/* Max number of files in "Recent Files" menu. 
 * This is configurable only using mateconftool or mateconf-editor 
 */
gint
pluma_prefs_manager_get_max_recents (void)
{
	pluma_debug (DEBUG_PREFS);

	return pluma_prefs_manager_get_int (GPM_MAX_RECENTS,	
					    GPM_DEFAULT_MAX_RECENTS);

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
pluma_prefs_manager_get_auto_detected_encodings (void)
{
	GSList *strings;
	GSList *res = NULL;

	pluma_debug (DEBUG_PREFS);

	g_return_val_if_fail (pluma_prefs_manager != NULL, NULL);
	g_return_val_if_fail (pluma_prefs_manager->mateconf_client != NULL, NULL);

	strings = mateconf_client_get_list (pluma_prefs_manager->mateconf_client,
				GPM_AUTO_DETECTED_ENCODINGS,
				MATECONF_VALUE_STRING, 
				NULL);

	if (strings == NULL)
	{
		gint i = 0;
		const gchar* s[] = GPM_DEFAULT_AUTO_DETECTED_ENCODINGS;

		while (s[i] != NULL)
		{
			strings = g_slist_prepend (strings, g_strdup (s[i]));

			++i;
		}


		strings = g_slist_reverse (strings);
	}

	if (strings != NULL)
	{	
		GSList *tmp;
		const PlumaEncoding *enc;

		tmp = strings;
		
		while (tmp)
		{
		      const char *charset = tmp->data;
      
		      if (strcmp (charset, "CURRENT") == 0)
			      g_get_charset (&charset);

		      g_return_val_if_fail (charset != NULL, NULL);
		      enc = pluma_encoding_get_from_charset (charset);

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

	pluma_debug_message (DEBUG_PREFS, "Done");

	return res;
}

GSList *
pluma_prefs_manager_get_shown_in_menu_encodings (void)
{
	GSList *strings;
	GSList *res = NULL;

	pluma_debug (DEBUG_PREFS);

	g_return_val_if_fail (pluma_prefs_manager != NULL, NULL);
	g_return_val_if_fail (pluma_prefs_manager->mateconf_client != NULL, NULL);

	strings = mateconf_client_get_list (pluma_prefs_manager->mateconf_client,
				GPM_SHOWN_IN_MENU_ENCODINGS,
				MATECONF_VALUE_STRING, 
				NULL);

	if (strings != NULL)
	{	
		GSList *tmp;
		const PlumaEncoding *enc;

		tmp = strings;
		
		while (tmp)
		{
		      const char *charset = tmp->data;

		      if (strcmp (charset, "CURRENT") == 0)
			      g_get_charset (&charset);

		      g_return_val_if_fail (charset != NULL, NULL);
		      enc = pluma_encoding_get_from_charset (charset);

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
pluma_prefs_manager_set_shown_in_menu_encodings (const GSList *encs)
{	
	GSList *list = NULL;
	
	g_return_if_fail (pluma_prefs_manager != NULL);
	g_return_if_fail (pluma_prefs_manager->mateconf_client != NULL);
	g_return_if_fail (pluma_prefs_manager_shown_in_menu_encodings_can_set ());

	while (encs != NULL)
	{
		const PlumaEncoding *enc;
		const gchar *charset;
		
		enc = (const PlumaEncoding *)encs->data;

		charset = pluma_encoding_get_charset (enc);
		g_return_if_fail (charset != NULL);

		list = g_slist_prepend (list, (gpointer)charset);

		encs = g_slist_next (encs);
	}

	list = g_slist_reverse (list);
		
	mateconf_client_set_list (pluma_prefs_manager->mateconf_client,
			GPM_SHOWN_IN_MENU_ENCODINGS,
			MATECONF_VALUE_STRING,
		       	list,
			NULL);

	g_slist_free (list);
}

gboolean
pluma_prefs_manager_shown_in_menu_encodings_can_set (void)
{
	pluma_debug (DEBUG_PREFS);
	
	return pluma_prefs_manager_key_is_writable (GPM_SHOWN_IN_MENU_ENCODINGS);

}

/* Highlight current line */
DEFINE_BOOL_PREF (highlight_current_line,
		  GPM_HIGHLIGHT_CURRENT_LINE,
		  GPM_DEFAULT_HIGHLIGHT_CURRENT_LINE)

/* Highlight matching bracket */
DEFINE_BOOL_PREF (bracket_matching,
		  GPM_BRACKET_MATCHING,
		  GPM_DEFAULT_BRACKET_MATCHING)
	
/* Display Right Margin */
DEFINE_BOOL_PREF (display_right_margin,
		  GPM_DISPLAY_RIGHT_MARGIN,
		  GPM_DEFAULT_DISPLAY_RIGHT_MARGIN)

/* Right Margin Position */	
DEFINE_INT_PREF (right_margin_position,
		 GPM_RIGHT_MARGIN_POSITION,
		 GPM_DEFAULT_RIGHT_MARGIN_POSITION)

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
pluma_prefs_manager_get_smart_home_end (void)
{
	gchar *str;
	GtkSourceSmartHomeEndType res;

	pluma_debug (DEBUG_PREFS);

	str = pluma_prefs_manager_get_string (GPM_SMART_HOME_END,
					      GPM_DEFAULT_SMART_HOME_END);

	res = get_smart_home_end_from_string (str);

	g_free (str);

	return res;
}
	
void
pluma_prefs_manager_set_smart_home_end (GtkSourceSmartHomeEndType smart_he)
{
	const gchar *str;

	pluma_debug (DEBUG_PREFS);

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

	pluma_prefs_manager_set_string (GPM_WRAP_MODE, str);
}

gboolean
pluma_prefs_manager_smart_home_end_can_set (void)
{
	pluma_debug (DEBUG_PREFS);
	
	return pluma_prefs_manager_key_is_writable (GPM_SMART_HOME_END);
}

/* Enable syntax highlighting */
DEFINE_BOOL_PREF (enable_syntax_highlighting,
		  GPM_SYNTAX_HL_ENABLE,
		  GPM_DEFAULT_SYNTAX_HL_ENABLE)

/* Enable search highlighting */
DEFINE_BOOL_PREF (enable_search_highlighting,
		  GPM_SEARCH_HIGHLIGHTING_ENABLE,
		  GPM_DEFAULT_SEARCH_HIGHLIGHTING_ENABLE)

/* Source style scheme */
DEFINE_STRING_PREF (source_style_scheme,
		    GPM_SOURCE_STYLE_SCHEME,
		    GPM_DEFAULT_SOURCE_STYLE_SCHEME)

GSList *
pluma_prefs_manager_get_writable_vfs_schemes (void)
{
	GSList *strings;
	
	pluma_debug (DEBUG_PREFS);

	g_return_val_if_fail (pluma_prefs_manager != NULL, NULL);
	g_return_val_if_fail (pluma_prefs_manager->mateconf_client != NULL, NULL);

	strings = mateconf_client_get_list (pluma_prefs_manager->mateconf_client,
				GPM_WRITABLE_VFS_SCHEMES,
				MATECONF_VALUE_STRING, 
				NULL);

	if (strings == NULL)
	{
		gint i = 0;
		const gchar* s[] = GPM_DEFAULT_WRITABLE_VFS_SCHEMES;

		while (s[i] != NULL)
		{
			strings = g_slist_prepend (strings, g_strdup (s[i]));

			++i;
		}

		strings = g_slist_reverse (strings);
	}

	/* The 'file' scheme is writable by default. */
	strings = g_slist_prepend (strings, g_strdup ("file")); 
	
	pluma_debug_message (DEBUG_PREFS, "Done");

	return strings;
}

gboolean
pluma_prefs_manager_get_restore_cursor_position (void)
{
	pluma_debug (DEBUG_PREFS);

	return pluma_prefs_manager_get_bool (GPM_RESTORE_CURSOR_POSITION,
					     GPM_DEFAULT_RESTORE_CURSOR_POSITION);
}

/* Plugins: we just store/return a list of strings, all the magic has to
 * happen in the plugin engine */

GSList *
pluma_prefs_manager_get_active_plugins (void)
{
	GSList *plugins;

	pluma_debug (DEBUG_PREFS);

	g_return_val_if_fail (pluma_prefs_manager != NULL, NULL);
	g_return_val_if_fail (pluma_prefs_manager->mateconf_client != NULL, NULL);

	plugins = mateconf_client_get_list (pluma_prefs_manager->mateconf_client,
					 GPM_ACTIVE_PLUGINS,
					 MATECONF_VALUE_STRING, 
					 NULL);

	return plugins;
}

void
pluma_prefs_manager_set_active_plugins (const GSList *plugins)
{	
	g_return_if_fail (pluma_prefs_manager != NULL);
	g_return_if_fail (pluma_prefs_manager->mateconf_client != NULL);
	g_return_if_fail (pluma_prefs_manager_active_plugins_can_set ());

	mateconf_client_set_list (pluma_prefs_manager->mateconf_client,
			       GPM_ACTIVE_PLUGINS,
			       MATECONF_VALUE_STRING,
		       	       (GSList *) plugins,
			       NULL);
}

gboolean
pluma_prefs_manager_active_plugins_can_set (void)
{
	pluma_debug (DEBUG_PREFS);

	return pluma_prefs_manager_key_is_writable (GPM_ACTIVE_PLUGINS);
}

/* Global Lockdown */

PlumaLockdownMask
pluma_prefs_manager_get_lockdown (void)
{
	guint lockdown = 0;

	if (pluma_prefs_manager_get_bool (GPM_LOCKDOWN_COMMAND_LINE, FALSE))
		lockdown |= PLUMA_LOCKDOWN_COMMAND_LINE;

	if (pluma_prefs_manager_get_bool (GPM_LOCKDOWN_PRINTING, FALSE))
		lockdown |= PLUMA_LOCKDOWN_PRINTING;

	if (pluma_prefs_manager_get_bool (GPM_LOCKDOWN_PRINT_SETUP, FALSE))
		lockdown |= PLUMA_LOCKDOWN_PRINT_SETUP;

	if (pluma_prefs_manager_get_bool (GPM_LOCKDOWN_SAVE_TO_DISK, FALSE))
		lockdown |= PLUMA_LOCKDOWN_SAVE_TO_DISK;

	return lockdown;
}

/* The following functions are taken from mateconf-client.c 
 * and partially modified. 
 * The licensing terms on these is: 
 *
 * 
 * MateConf
 * Copyright (C) 1999, 2000, 2000 Red Hat Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


static const gchar* 
mateconf_value_type_to_string(MateConfValueType type)
{
  switch (type)
    {
    case MATECONF_VALUE_INT:
      return "int";
      break;
    case MATECONF_VALUE_STRING:
      return "string";
      break;
    case MATECONF_VALUE_FLOAT:
      return "float";
      break;
    case MATECONF_VALUE_BOOL:
      return "bool";
      break;
    case MATECONF_VALUE_SCHEMA:
      return "schema";
      break;
    case MATECONF_VALUE_LIST:
      return "list";
      break;
    case MATECONF_VALUE_PAIR:
      return "pair";
      break;
    case MATECONF_VALUE_INVALID:
      return "*invalid*";
      break;
    default:
      g_return_val_if_reached (NULL);
      break;
    }
}

/* Emit the proper signals for the error, and fill in err */
static gboolean
handle_error (MateConfClient* client, GError* error, GError** err)
{
  if (error != NULL)
    {
      mateconf_client_error(client, error);
      
      if (err == NULL)
        {
          mateconf_client_unreturned_error(client, error);

          g_error_free(error);
        }
      else
        *err = error;

      return TRUE;
    }
  else
    return FALSE;
}

static gboolean
check_type (const gchar* key, MateConfValue* val, MateConfValueType t, GError** err)
{
  if (val->type != t)
    {
      /*
      mateconf_set_error(err, MATECONF_ERROR_TYPE_MISMATCH,
                      _("Expected `%s' got, `%s' for key %s"),
                      mateconf_value_type_to_string(t),
                      mateconf_value_type_to_string(val->type),
                      key);
      */
      g_set_error (err, MATECONF_ERROR, MATECONF_ERROR_TYPE_MISMATCH,
	  	   _("Expected `%s', got `%s' for key %s"),
                   mateconf_value_type_to_string(t),
                   mateconf_value_type_to_string(val->type),
                   key);
	      
      return FALSE;
    }
  else
    return TRUE;
}

static gboolean
mateconf_client_get_bool_with_default (MateConfClient* client, const gchar* key,
                        	    gboolean def, GError** err)
{
  GError* error = NULL;
  MateConfValue* val;

  g_return_val_if_fail (err == NULL || *err == NULL, def);

  val = mateconf_client_get (client, key, &error);

  if (val != NULL)
    {
      gboolean retval = def;

      g_return_val_if_fail (error == NULL, retval);
      
      if (check_type (key, val, MATECONF_VALUE_BOOL, &error))
        retval = mateconf_value_get_bool (val);
      else
        handle_error (client, error, err);

      mateconf_value_free (val);

      return retval;
    }
  else
    {
      if (error != NULL)
        handle_error (client, error, err);
      return def;
    }
}

static gchar*
mateconf_client_get_string_with_default (MateConfClient* client, const gchar* key,
                        	      const gchar* def, GError** err)
{
  GError* error = NULL;
  gchar* val;

  g_return_val_if_fail (err == NULL || *err == NULL, def ? g_strdup (def) : NULL);

  val = mateconf_client_get_string (client, key, &error);

  if (val != NULL)
    {
      g_return_val_if_fail (error == NULL, def ? g_strdup (def) : NULL);
      
      return val;
    }
  else
    {
      if (error != NULL)
        handle_error (client, error, err);
      return def ? g_strdup (def) : NULL;
    }
}

static gint
mateconf_client_get_int_with_default (MateConfClient* client, const gchar* key,
                        	   gint def, GError** err)
{
  GError* error = NULL;
  MateConfValue* val;

  g_return_val_if_fail (err == NULL || *err == NULL, def);

  val = mateconf_client_get (client, key, &error);

  if (val != NULL)
    {
      gint retval = def;

      g_return_val_if_fail (error == NULL, def);
      
      if (check_type (key, val, MATECONF_VALUE_INT, &error))
        retval = mateconf_value_get_int(val);
      else
        handle_error (client, error, err);

      mateconf_value_free (val);

      return retval;
    }
  else
    {
      if (error != NULL)
        handle_error (client, error, err);
      return def;
    }
}

