/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gedit-prefs-manager.c
 * This file is part of gedit
 *
 * Copyright (C) 2002-2005  Paolo Maggi 
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
 * Modified by the gedit Team, 2002-2003. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "gedit-prefs-manager.h"
#include "gedit-prefs-manager-private.h"
#include "gedit-prefs-manager-app.h"
#include "gedit-app.h"
#include "gedit-debug.h"
#include "gedit-view.h"
#include "gedit-window.h"
#include "gedit-window-private.h"
#include "gedit-plugins-engine.h"
#include "gedit-style-scheme-manager.h"
#include "gedit-dirs.h"

static void gedit_prefs_manager_editor_font_changed	(MateConfClient *client,
							 guint        cnxn_id,
							 MateConfEntry  *entry,
							 gpointer     user_data);

static void gedit_prefs_manager_system_font_changed	(MateConfClient *client,
							 guint        cnxn_id,
							 MateConfEntry  *entry,
							 gpointer     user_data);

static void gedit_prefs_manager_tabs_size_changed	(MateConfClient *client,
							 guint        cnxn_id, 
							 MateConfEntry  *entry, 
							 gpointer     user_data);

static void gedit_prefs_manager_wrap_mode_changed	(MateConfClient *client,
							 guint        cnxn_id, 
							 MateConfEntry  *entry, 
							 gpointer     user_data);

static void gedit_prefs_manager_line_numbers_changed	(MateConfClient *client,
							 guint        cnxn_id, 
							 MateConfEntry  *entry, 
							 gpointer     user_data);

static void gedit_prefs_manager_auto_indent_changed	(MateConfClient *client,
							 guint        cnxn_id, 
							 MateConfEntry  *entry, 
							 gpointer     user_data);

static void gedit_prefs_manager_undo_changed		(MateConfClient *client,
							 guint        cnxn_id, 
							 MateConfEntry  *entry, 
							 gpointer     user_data);

static void gedit_prefs_manager_right_margin_changed	(MateConfClient *client,
							 guint        cnxn_id, 
							 MateConfEntry  *entry, 
							 gpointer     user_data);

static void gedit_prefs_manager_smart_home_end_changed	(MateConfClient *client,
							 guint        cnxn_id, 
							 MateConfEntry  *entry, 
							 gpointer     user_data);

static void gedit_prefs_manager_hl_current_line_changed	(MateConfClient *client,
							 guint        cnxn_id, 
							 MateConfEntry  *entry, 
							 gpointer     user_data);
							 
static void gedit_prefs_manager_bracket_matching_changed(MateConfClient *client,
							 guint        cnxn_id, 
							 MateConfEntry  *entry, 
							 gpointer     user_data);

static void gedit_prefs_manager_syntax_hl_enable_changed(MateConfClient *client,
							 guint        cnxn_id, 
							 MateConfEntry  *entry, 
							 gpointer     user_data);

static void gedit_prefs_manager_search_hl_enable_changed(MateConfClient *client,
							 guint        cnxn_id, 
							 MateConfEntry  *entry, 
							 gpointer     user_data);

static void gedit_prefs_manager_source_style_scheme_changed
							(MateConfClient *client,
							 guint        cnxn_id, 
							 MateConfEntry  *entry, 
							 gpointer     user_data);

static void gedit_prefs_manager_max_recents_changed	(MateConfClient *client,
							 guint        cnxn_id, 
							 MateConfEntry  *entry, 
							 gpointer     user_data);

static void gedit_prefs_manager_auto_save_changed	(MateConfClient *client,
							 guint        cnxn_id,
							 MateConfEntry  *entry,
							 gpointer     user_data);

static void gedit_prefs_manager_active_plugins_changed	(MateConfClient *client,
							 guint        cnxn_id, 
							 MateConfEntry  *entry, 
							 gpointer     user_data);

static void gedit_prefs_manager_lockdown_changed	(MateConfClient *client,
							 guint        cnxn_id,
							 MateConfEntry  *entry,
							 gpointer     user_data);

/* GUI state is serialized to a .desktop file, not in mateconf */

#define GEDIT_STATE_DEFAULT_WINDOW_STATE	0
#define GEDIT_STATE_DEFAULT_WINDOW_WIDTH	650
#define GEDIT_STATE_DEFAULT_WINDOW_HEIGHT	500
#define GEDIT_STATE_DEFAULT_SIDE_PANEL_SIZE	200
#define GEDIT_STATE_DEFAULT_BOTTOM_PANEL_SIZE	140

#define GEDIT_STATE_FILE_LOCATION "gedit-2"

#define GEDIT_STATE_WINDOW_GROUP "window"
#define GEDIT_STATE_WINDOW_STATE "state"
#define GEDIT_STATE_WINDOW_HEIGHT "height"
#define GEDIT_STATE_WINDOW_WIDTH "width"
#define GEDIT_STATE_SIDE_PANEL_SIZE "side_panel_size"
#define GEDIT_STATE_BOTTOM_PANEL_SIZE "bottom_panel_size"
#define GEDIT_STATE_SIDE_PANEL_ACTIVE_PAGE "side_panel_active_page"
#define GEDIT_STATE_BOTTOM_PANEL_ACTIVE_PAGE "bottom_panel_active_page"

#define GEDIT_STATE_FILEFILTER_GROUP "filefilter"
#define GEDIT_STATE_FILEFILTER_ID "id"

static gint window_state = -1;
static gint window_height = -1;
static gint window_width = -1;
static gint side_panel_size = -1;
static gint bottom_panel_size = -1;
static gint side_panel_active_page = -1;
static gint bottom_panel_active_page = -1;
static gint active_file_filter = -1;


static gchar *
get_state_filename (void)
{
	gchar *config_dir;
	gchar *filename = NULL;

	config_dir = gedit_dirs_get_user_config_dir ();

	if (config_dir != NULL)
	{
		filename = g_build_filename (config_dir,
					     GEDIT_STATE_FILE_LOCATION,
					     NULL);
		g_free (config_dir);
	}

	return filename;
}

static GKeyFile *
get_gedit_state_file (void)
{
	static GKeyFile *state_file = NULL;

	if (state_file == NULL)
	{
		gchar *filename;
		GError *err = NULL;

		state_file = g_key_file_new ();

		filename = get_state_filename ();

		if (!g_key_file_load_from_file (state_file,
						filename,
						G_KEY_FILE_NONE,
						&err))
		{
			if (err->domain != G_FILE_ERROR ||
			    err->code != G_FILE_ERROR_NOENT)
			{
				g_warning ("Could not load gedit state file: %s\n",
					   err->message);
			}

			g_error_free (err);
		}

		g_free (filename);
	}

	return state_file;
}

static void
gedit_state_get_int (const gchar *group,
		     const gchar *key,
		     gint         defval,
		     gint        *result)
{
	GKeyFile *state_file;
	gint res;
	GError *err = NULL;

	state_file = get_gedit_state_file ();
	res = g_key_file_get_integer (state_file,
				      group,
				      key,
				      &err);

	if (err != NULL)
	{
		if ((err->domain != G_KEY_FILE_ERROR) ||
		    ((err->code != G_KEY_FILE_ERROR_GROUP_NOT_FOUND &&
		      err->code != G_KEY_FILE_ERROR_KEY_NOT_FOUND)))
		{
			g_warning ("Could not get state value %s::%s : %s\n",
				   group,
				   key,
				   err->message);
		}

		*result = defval;
		g_error_free (err);
	}
	else
	{
		*result = res;
	}
}

static void
gedit_state_set_int (const gchar *group,
		     const gchar *key,
		     gint         value)
{
	GKeyFile *state_file;

	state_file = get_gedit_state_file ();
	g_key_file_set_integer (state_file,
				group,
				key,
				value);
}

static gboolean
gedit_state_file_sync (void)
{
	GKeyFile *state_file;
	gchar *config_dir;
	gchar *filename = NULL;
	gchar *content = NULL;
	gsize length;
	gint res;
	GError *err = NULL;
	gboolean ret = FALSE;

	state_file = get_gedit_state_file ();
	g_return_val_if_fail (state_file != NULL, FALSE);

	config_dir = gedit_dirs_get_user_config_dir ();
	if (config_dir == NULL)
	{
		g_warning ("Could not get config directory\n");
		return ret;
	}

	res = g_mkdir_with_parents (config_dir, 0755);
	if (res < 0)
	{
		g_warning ("Could not create config directory\n");
		goto out;
	}

	content = g_key_file_to_data (state_file,
				      &length,
				      &err);

	if (err != NULL)
	{
		g_warning ("Could not get data from state file: %s\n",
			   err->message);
		goto out;
	}

	if (content != NULL)
	{
		filename = get_state_filename ();
		if (!g_file_set_contents (filename,
					  content,
					  length,
					  &err))
		{
			g_warning ("Could not write gedit state file: %s\n",
				   err->message);
			goto out;
		}
	}

	ret = TRUE;

 out:
	if (err != NULL)
		g_error_free (err);

	g_free (config_dir);
	g_free (filename);
	g_free (content);

	return ret;
}

/* Window state */
gint
gedit_prefs_manager_get_window_state (void)
{
	if (window_state == -1)
	{
		gedit_state_get_int (GEDIT_STATE_WINDOW_GROUP,
				     GEDIT_STATE_WINDOW_STATE,
				     GEDIT_STATE_DEFAULT_WINDOW_STATE,
				     &window_state);
	}

	return window_state;
}
			
void
gedit_prefs_manager_set_window_state (gint ws)
{
	g_return_if_fail (ws > -1);
	
	window_state = ws;

	gedit_state_set_int (GEDIT_STATE_WINDOW_GROUP,
			     GEDIT_STATE_WINDOW_STATE,
			     ws);
}

gboolean
gedit_prefs_manager_window_state_can_set (void)
{
	return TRUE;
}

/* Window size */
void
gedit_prefs_manager_get_window_size (gint *width, gint *height)
{
	g_return_if_fail (width != NULL && height != NULL);

	if (window_width == -1)
	{
		gedit_state_get_int (GEDIT_STATE_WINDOW_GROUP,
				     GEDIT_STATE_WINDOW_WIDTH,
				     GEDIT_STATE_DEFAULT_WINDOW_WIDTH,
				     &window_width);
	}

	if (window_height == -1)
	{
		gedit_state_get_int (GEDIT_STATE_WINDOW_GROUP,
				     GEDIT_STATE_WINDOW_HEIGHT,
				     GEDIT_STATE_DEFAULT_WINDOW_HEIGHT,
				     &window_height);
	}

	*width = window_width;
	*height = window_height;
}

void
gedit_prefs_manager_get_default_window_size (gint *width, gint *height)
{
	g_return_if_fail (width != NULL && height != NULL);

	*width = GEDIT_STATE_DEFAULT_WINDOW_WIDTH;
	*height = GEDIT_STATE_DEFAULT_WINDOW_HEIGHT;
}

void
gedit_prefs_manager_set_window_size (gint width, gint height)
{
	g_return_if_fail (width > -1 && height > -1);

	window_width = width;
	window_height = height;

	gedit_state_set_int (GEDIT_STATE_WINDOW_GROUP,
			     GEDIT_STATE_WINDOW_WIDTH,
			     width);
	gedit_state_set_int (GEDIT_STATE_WINDOW_GROUP,
			     GEDIT_STATE_WINDOW_HEIGHT,
			     height);
}

gboolean 
gedit_prefs_manager_window_size_can_set (void)
{
	return TRUE;
}

/* Side panel */
gint
gedit_prefs_manager_get_side_panel_size (void)
{
	if (side_panel_size == -1)
	{
		gedit_state_get_int (GEDIT_STATE_WINDOW_GROUP,
				     GEDIT_STATE_SIDE_PANEL_SIZE,
				     GEDIT_STATE_DEFAULT_SIDE_PANEL_SIZE,
				     &side_panel_size);
	}

	return side_panel_size;
}

gint 
gedit_prefs_manager_get_default_side_panel_size (void)
{
	return GEDIT_STATE_DEFAULT_SIDE_PANEL_SIZE;
}

void 
gedit_prefs_manager_set_side_panel_size (gint ps)
{
	g_return_if_fail (ps > -1);
	
	if (side_panel_size == ps)
		return;
		
	side_panel_size = ps;
	gedit_state_set_int (GEDIT_STATE_WINDOW_GROUP,
			     GEDIT_STATE_SIDE_PANEL_SIZE,
			     ps);
}

gboolean 
gedit_prefs_manager_side_panel_size_can_set (void)
{
	return TRUE;
}

gint
gedit_prefs_manager_get_side_panel_active_page (void)
{
	if (side_panel_active_page == -1)
	{
		gedit_state_get_int (GEDIT_STATE_WINDOW_GROUP,
				     GEDIT_STATE_SIDE_PANEL_ACTIVE_PAGE,
				     0,
				     &side_panel_active_page);
	}

	return side_panel_active_page;
}

void
gedit_prefs_manager_set_side_panel_active_page (gint id)
{
	if (side_panel_active_page == id)
		return;

	side_panel_active_page = id;
	gedit_state_set_int (GEDIT_STATE_WINDOW_GROUP,
			     GEDIT_STATE_SIDE_PANEL_ACTIVE_PAGE,
			     id);
}

gboolean 
gedit_prefs_manager_side_panel_active_page_can_set (void)
{
	return TRUE;
}

/* Bottom panel */
gint
gedit_prefs_manager_get_bottom_panel_size (void)
{
	if (bottom_panel_size == -1)
	{
		gedit_state_get_int (GEDIT_STATE_WINDOW_GROUP,
				     GEDIT_STATE_BOTTOM_PANEL_SIZE,
				     GEDIT_STATE_DEFAULT_BOTTOM_PANEL_SIZE,
				     &bottom_panel_size);
	}

	return bottom_panel_size;
}

gint 
gedit_prefs_manager_get_default_bottom_panel_size (void)
{
	return GEDIT_STATE_DEFAULT_BOTTOM_PANEL_SIZE;
}

void 
gedit_prefs_manager_set_bottom_panel_size (gint ps)
{
	g_return_if_fail (ps > -1);

	if (bottom_panel_size == ps)
		return;
	
	bottom_panel_size = ps;
	gedit_state_set_int (GEDIT_STATE_WINDOW_GROUP,
			     GEDIT_STATE_BOTTOM_PANEL_SIZE,
			     ps);
}

gboolean 
gedit_prefs_manager_bottom_panel_size_can_set (void)
{
	return TRUE;
}

gint
gedit_prefs_manager_get_bottom_panel_active_page (void)
{
	if (bottom_panel_active_page == -1)
	{
		gedit_state_get_int (GEDIT_STATE_WINDOW_GROUP,
				     GEDIT_STATE_BOTTOM_PANEL_ACTIVE_PAGE,
				     0,
				     &bottom_panel_active_page);
	}

	return bottom_panel_active_page;
}

void
gedit_prefs_manager_set_bottom_panel_active_page (gint id)
{
	if (bottom_panel_active_page == id)
		return;

	bottom_panel_active_page = id;
	gedit_state_set_int (GEDIT_STATE_WINDOW_GROUP,
			     GEDIT_STATE_BOTTOM_PANEL_ACTIVE_PAGE,
			     id);
}

gboolean 
gedit_prefs_manager_bottom_panel_active_page_can_set (void)
{
	return TRUE;
}

/* File filter */
gint
gedit_prefs_manager_get_active_file_filter (void)
{
	if (active_file_filter == -1)
	{
		gedit_state_get_int (GEDIT_STATE_FILEFILTER_GROUP,
				     GEDIT_STATE_FILEFILTER_ID,
				     0,
				     &active_file_filter);
	}

	return active_file_filter;
}

void
gedit_prefs_manager_set_active_file_filter (gint id)
{
	g_return_if_fail (id >= 0);
	
	if (active_file_filter == id)
		return;

	active_file_filter = id;
	gedit_state_set_int (GEDIT_STATE_FILEFILTER_GROUP,
			     GEDIT_STATE_FILEFILTER_ID,
			     id);
}

gboolean 
gedit_prefs_manager_active_file_filter_can_set (void)
{
	return TRUE;
}

/* Normal prefs are stored in MateConf */

gboolean
gedit_prefs_manager_app_init (void)
{
	gedit_debug (DEBUG_PREFS);

	g_return_val_if_fail (gedit_prefs_manager == NULL, FALSE);

	gedit_prefs_manager_init ();

	if (gedit_prefs_manager != NULL)
	{
		/* TODO: notify, add dirs */
		mateconf_client_add_dir (gedit_prefs_manager->mateconf_client,
				GPM_PREFS_DIR,
				MATECONF_CLIENT_PRELOAD_RECURSIVE,
				NULL);

		mateconf_client_add_dir (gedit_prefs_manager->mateconf_client,
				GPM_PLUGINS_DIR,
				MATECONF_CLIENT_PRELOAD_RECURSIVE,
				NULL);
		
		mateconf_client_add_dir (gedit_prefs_manager->mateconf_client,
				GPM_LOCKDOWN_DIR,
				MATECONF_CLIENT_PRELOAD_RECURSIVE,
				NULL);
		
		mateconf_client_notify_add (gedit_prefs_manager->mateconf_client,
				GPM_FONT_DIR,
				gedit_prefs_manager_editor_font_changed,
				NULL, NULL, NULL);

		mateconf_client_notify_add (gedit_prefs_manager->mateconf_client,
				GPM_SYSTEM_FONT,
				gedit_prefs_manager_system_font_changed,
				NULL, NULL, NULL);

		mateconf_client_notify_add (gedit_prefs_manager->mateconf_client,
				GPM_TABS_DIR,
				gedit_prefs_manager_tabs_size_changed,
				NULL, NULL, NULL);

		mateconf_client_notify_add (gedit_prefs_manager->mateconf_client,
				GPM_WRAP_MODE_DIR,
				gedit_prefs_manager_wrap_mode_changed,
				NULL, NULL, NULL);
		
		mateconf_client_notify_add (gedit_prefs_manager->mateconf_client,
				GPM_LINE_NUMBERS_DIR,
				gedit_prefs_manager_line_numbers_changed,
				NULL, NULL, NULL);

		mateconf_client_notify_add (gedit_prefs_manager->mateconf_client,
				GPM_AUTO_INDENT_DIR,
				gedit_prefs_manager_auto_indent_changed,
				NULL, NULL, NULL);

		mateconf_client_notify_add (gedit_prefs_manager->mateconf_client,
				GPM_UNDO_DIR,
				gedit_prefs_manager_undo_changed,
				NULL, NULL, NULL);

		mateconf_client_notify_add (gedit_prefs_manager->mateconf_client,
				GPM_RIGHT_MARGIN_DIR,
				gedit_prefs_manager_right_margin_changed,
				NULL, NULL, NULL);

		mateconf_client_notify_add (gedit_prefs_manager->mateconf_client,
				GPM_SMART_HOME_END_DIR,
				gedit_prefs_manager_smart_home_end_changed,
				NULL, NULL, NULL);

		mateconf_client_notify_add (gedit_prefs_manager->mateconf_client,
				GPM_CURRENT_LINE_DIR,
				gedit_prefs_manager_hl_current_line_changed,
				NULL, NULL, NULL);

		mateconf_client_notify_add (gedit_prefs_manager->mateconf_client,
				GPM_BRACKET_MATCHING_DIR,
				gedit_prefs_manager_bracket_matching_changed,
				NULL, NULL, NULL);

		mateconf_client_notify_add (gedit_prefs_manager->mateconf_client,
				GPM_SYNTAX_HL_ENABLE,
				gedit_prefs_manager_syntax_hl_enable_changed,
				NULL, NULL, NULL);

		mateconf_client_notify_add (gedit_prefs_manager->mateconf_client,
				GPM_SEARCH_HIGHLIGHTING_ENABLE,
				gedit_prefs_manager_search_hl_enable_changed,
				NULL, NULL, NULL);

		mateconf_client_notify_add (gedit_prefs_manager->mateconf_client,
				GPM_SOURCE_STYLE_DIR,
				gedit_prefs_manager_source_style_scheme_changed,
				NULL, NULL, NULL);

		mateconf_client_notify_add (gedit_prefs_manager->mateconf_client,
				GPM_MAX_RECENTS,
				gedit_prefs_manager_max_recents_changed,
				NULL, NULL, NULL);

		mateconf_client_notify_add (gedit_prefs_manager->mateconf_client,
				GPM_SAVE_DIR,
				gedit_prefs_manager_auto_save_changed,
				NULL, NULL, NULL);

		mateconf_client_notify_add (gedit_prefs_manager->mateconf_client,
				GPM_ACTIVE_PLUGINS,
				gedit_prefs_manager_active_plugins_changed,
				NULL, NULL, NULL);

		mateconf_client_notify_add (gedit_prefs_manager->mateconf_client,
				GPM_LOCKDOWN_DIR,
				gedit_prefs_manager_lockdown_changed,
				NULL, NULL, NULL);
	}

	return gedit_prefs_manager != NULL;	
}

/* This function must be called before exiting gedit */
void
gedit_prefs_manager_app_shutdown (void)
{
	gedit_debug (DEBUG_PREFS);

	gedit_prefs_manager_shutdown ();

	gedit_state_file_sync ();
}


static void 
gedit_prefs_manager_editor_font_changed (MateConfClient *client,
					 guint        cnxn_id, 
					 MateConfEntry  *entry, 
					 gpointer     user_data)
{
	GList *views;
	GList *l;
	gchar *font = NULL;
	gboolean def = TRUE;
	gint ts;

	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (entry->key != NULL);
	g_return_if_fail (entry->value != NULL);

	if (strcmp (entry->key, GPM_USE_DEFAULT_FONT) == 0)
	{
		if (entry->value->type == MATECONF_VALUE_BOOL)
			def = mateconf_value_get_bool (entry->value);
		else
			def = GPM_DEFAULT_USE_DEFAULT_FONT;
		
		if (def)
			font = gedit_prefs_manager_get_system_font ();
		else
			font = gedit_prefs_manager_get_editor_font ();
	}
	else if (strcmp (entry->key, GPM_EDITOR_FONT) == 0)
	{
		if (entry->value->type == MATECONF_VALUE_STRING)
			font = g_strdup (mateconf_value_get_string (entry->value));
		else
			font = g_strdup (GPM_DEFAULT_EDITOR_FONT);
				
		def = gedit_prefs_manager_get_use_default_font ();
	}
	else
		return;

	g_return_if_fail (font != NULL);
	
	ts = gedit_prefs_manager_get_tabs_size ();

	views = gedit_app_get_views (gedit_app_get_default ());
	l = views;

	while (l != NULL)
	{
		/* Note: we use def=FALSE to avoid GeditView to query mateconf */
		gedit_view_set_font (GEDIT_VIEW (l->data), FALSE,  font);
		gtk_source_view_set_tab_width (GTK_SOURCE_VIEW (l->data), ts);

		l = l->next;
	}

	g_list_free (views);
	g_free (font);
}

static void 
gedit_prefs_manager_system_font_changed (MateConfClient *client,
					 guint        cnxn_id, 
					 MateConfEntry  *entry, 
					 gpointer     user_data)
{
	GList *views;
	GList *l;
	gchar *font;
	gint ts;

	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (entry->key != NULL);
	g_return_if_fail (entry->value != NULL);

	if (strcmp (entry->key, GPM_SYSTEM_FONT) != 0)
		return;

	if (!gedit_prefs_manager_get_use_default_font ())
		return;

	if (entry->value->type == MATECONF_VALUE_STRING)
		font = g_strdup (mateconf_value_get_string (entry->value));
	else
		font = g_strdup (GPM_DEFAULT_SYSTEM_FONT);

	ts = gedit_prefs_manager_get_tabs_size ();

	views = gedit_app_get_views (gedit_app_get_default ());
	l = views;

	while (l != NULL)
	{
		/* Note: we use def=FALSE to avoid GeditView to query mateconf */
		gedit_view_set_font (GEDIT_VIEW (l->data), FALSE, font);

		gtk_source_view_set_tab_width (GTK_SOURCE_VIEW (l->data), ts);
		l = l->next;
	}

	g_list_free (views);
	g_free (font);
}

static void 
gedit_prefs_manager_tabs_size_changed (MateConfClient *client,
				       guint        cnxn_id,
				       MateConfEntry  *entry, 
				       gpointer     user_data)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (entry->key != NULL);
	g_return_if_fail (entry->value != NULL);

	if (strcmp (entry->key, GPM_TABS_SIZE) == 0)
	{
		gint tab_width;
		GList *views;
		GList *l;

		if (entry->value->type == MATECONF_VALUE_INT)
			tab_width = mateconf_value_get_int (entry->value);
		else
			tab_width = GPM_DEFAULT_TABS_SIZE;
	
		tab_width = CLAMP (tab_width, 1, 24);

		views = gedit_app_get_views (gedit_app_get_default ());
		l = views;

		while (l != NULL)
		{
			gtk_source_view_set_tab_width (GTK_SOURCE_VIEW (l->data), 
						       tab_width);

			l = l->next;
		}

		g_list_free (views);
	}
	else if (strcmp (entry->key, GPM_INSERT_SPACES) == 0)
	{
		gboolean enable;
		GList *views;
		GList *l;

		if (entry->value->type == MATECONF_VALUE_BOOL)
			enable = mateconf_value_get_bool (entry->value);	
		else
			enable = GPM_DEFAULT_INSERT_SPACES;

		views = gedit_app_get_views (gedit_app_get_default ());
		l = views;

		while (l != NULL)
		{
			gtk_source_view_set_insert_spaces_instead_of_tabs (
					GTK_SOURCE_VIEW (l->data), 
					enable);

			l = l->next;
		}

		g_list_free (views);
	}
}

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

static void 
gedit_prefs_manager_wrap_mode_changed (MateConfClient *client,
	                               guint        cnxn_id, 
	                               MateConfEntry  *entry, 
	                               gpointer     user_data)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (entry->key != NULL);
	g_return_if_fail (entry->value != NULL);

	if (strcmp (entry->key, GPM_WRAP_MODE) == 0)
	{
		GtkWrapMode wrap_mode;
		GList *views;
		GList *l;

		if (entry->value->type == MATECONF_VALUE_STRING)
			wrap_mode = 
				get_wrap_mode_from_string (mateconf_value_get_string (entry->value));	
		else
			wrap_mode = get_wrap_mode_from_string (GPM_DEFAULT_WRAP_MODE);

		views = gedit_app_get_views (gedit_app_get_default ());
		l = views;

		while (l != NULL)
		{
			gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (l->data),
						     wrap_mode);

			l = l->next;
		}

		g_list_free (views);
	}
}

static void 
gedit_prefs_manager_line_numbers_changed (MateConfClient *client,
					  guint        cnxn_id, 
					  MateConfEntry  *entry, 
					  gpointer     user_data)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (entry->key != NULL);
	g_return_if_fail (entry->value != NULL);

	if (strcmp (entry->key, GPM_DISPLAY_LINE_NUMBERS) == 0)
	{
		gboolean dln;
		GList *views;
		GList *l;

		if (entry->value->type == MATECONF_VALUE_BOOL)
			dln = mateconf_value_get_bool (entry->value);	
		else
			dln = GPM_DEFAULT_DISPLAY_LINE_NUMBERS;

		views = gedit_app_get_views (gedit_app_get_default ());
		l = views;

		while (l != NULL)
		{
			gtk_source_view_set_show_line_numbers (GTK_SOURCE_VIEW (l->data), 
							       dln);

			l = l->next;
		}

		g_list_free (views);
	}
}

static void 
gedit_prefs_manager_hl_current_line_changed (MateConfClient *client,
					     guint        cnxn_id, 
					     MateConfEntry  *entry, 
					     gpointer     user_data)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (entry->key != NULL);
	g_return_if_fail (entry->value != NULL);

	if (strcmp (entry->key, GPM_HIGHLIGHT_CURRENT_LINE) == 0)
	{
		gboolean hl;
		GList *views;
		GList *l;

		if (entry->value->type == MATECONF_VALUE_BOOL)
			hl = mateconf_value_get_bool (entry->value);	
		else
			hl = GPM_DEFAULT_HIGHLIGHT_CURRENT_LINE;

		views = gedit_app_get_views (gedit_app_get_default ());
		l = views;

		while (l != NULL)
		{
			gtk_source_view_set_highlight_current_line (GTK_SOURCE_VIEW (l->data), 
								    hl);

			l = l->next;
		}

		g_list_free (views);
	}
}

static void 
gedit_prefs_manager_bracket_matching_changed (MateConfClient *client,
					      guint        cnxn_id, 
					      MateConfEntry  *entry, 
					      gpointer     user_data)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (entry->key != NULL);
	g_return_if_fail (entry->value != NULL);

	if (strcmp (entry->key, GPM_BRACKET_MATCHING) == 0)
	{
		gboolean enable;
		GList *docs;
		GList *l;

		if (entry->value->type == MATECONF_VALUE_BOOL)
			enable = mateconf_value_get_bool (entry->value);
		else
			enable = GPM_DEFAULT_BRACKET_MATCHING;

		docs = gedit_app_get_documents (gedit_app_get_default ());
		l = docs;

		while (l != NULL)
		{
			gtk_source_buffer_set_highlight_matching_brackets (GTK_SOURCE_BUFFER (l->data),
									   enable);

			l = l->next;
		}

		g_list_free (docs);
	}
}

static void 
gedit_prefs_manager_auto_indent_changed (MateConfClient *client,
					 guint        cnxn_id, 
					 MateConfEntry  *entry, 
					 gpointer     user_data)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (entry->key != NULL);
	g_return_if_fail (entry->value != NULL);

	if (strcmp (entry->key, GPM_AUTO_INDENT) == 0)
	{
		gboolean enable;
		GList *views;
		GList *l;

		if (entry->value->type == MATECONF_VALUE_BOOL)
			enable = mateconf_value_get_bool (entry->value);	
		else
			enable = GPM_DEFAULT_AUTO_INDENT;
	
		views = gedit_app_get_views (gedit_app_get_default ());
		l = views;

		while (l != NULL)
		{		
			gtk_source_view_set_auto_indent (GTK_SOURCE_VIEW (l->data), 
							 enable);
			
			l = l->next;
		}

		g_list_free (views);
	}
}

static void 
gedit_prefs_manager_undo_changed (MateConfClient *client,
				  guint        cnxn_id, 
				  MateConfEntry  *entry, 
				  gpointer     user_data)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (entry->key != NULL);
	g_return_if_fail (entry->value != NULL);

	if (strcmp (entry->key, GPM_UNDO_ACTIONS_LIMIT) == 0)
	{
		gint ul;
		GList *docs;
		GList *l;

		if (entry->value->type == MATECONF_VALUE_INT)
			ul = mateconf_value_get_int (entry->value);
		else
			ul = GPM_DEFAULT_UNDO_ACTIONS_LIMIT;
	
		ul = CLAMP (ul, -1, 250);

		docs = gedit_app_get_documents (gedit_app_get_default ());
		l = docs;
		
		while (l != NULL)
		{
			gtk_source_buffer_set_max_undo_levels (GTK_SOURCE_BUFFER (l->data), 
							       ul);

			l = l->next;
		}

		g_list_free (docs);
	}
}

static void 
gedit_prefs_manager_right_margin_changed (MateConfClient *client,
					  guint cnxn_id,
					  MateConfEntry *entry,
					  gpointer user_data)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (entry->key != NULL);
	g_return_if_fail (entry->value != NULL);

	if (strcmp (entry->key, GPM_RIGHT_MARGIN_POSITION) == 0)
	{
		gint pos;
		GList *views;
		GList *l;

		if (entry->value->type == MATECONF_VALUE_INT)
			pos = mateconf_value_get_int (entry->value);
		else
			pos = GPM_DEFAULT_RIGHT_MARGIN_POSITION;

		pos = CLAMP (pos, 1, 160);

		views = gedit_app_get_views (gedit_app_get_default ());
		l = views;

		while (l != NULL)
		{
			gtk_source_view_set_right_margin_position (GTK_SOURCE_VIEW (l->data),
								   pos);

			l = l->next;
		}

		g_list_free (views);
	}
	else if (strcmp (entry->key, GPM_DISPLAY_RIGHT_MARGIN) == 0)
	{
		gboolean display;
		GList *views;
		GList *l;

		if (entry->value->type == MATECONF_VALUE_BOOL)
			display = mateconf_value_get_bool (entry->value);	
		else
			display = GPM_DEFAULT_DISPLAY_RIGHT_MARGIN;

		views = gedit_app_get_views (gedit_app_get_default ());
		l = views;

		while (l != NULL)
		{
			gtk_source_view_set_show_right_margin (GTK_SOURCE_VIEW (l->data),
							       display);

			l = l->next;
		}

		g_list_free (views);
	}
}

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

static void
gedit_prefs_manager_smart_home_end_changed (MateConfClient *client,
					    guint        cnxn_id,
					    MateConfEntry  *entry,
					    gpointer     user_data)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (entry->key != NULL);
	g_return_if_fail (entry->value != NULL);

	if (strcmp (entry->key, GPM_SMART_HOME_END) == 0)
	{
		GtkSourceSmartHomeEndType smart_he;
		GList *views;
		GList *l;

		if (entry->value->type == MATECONF_VALUE_STRING)
			smart_he = 
				get_smart_home_end_from_string (mateconf_value_get_string (entry->value));	
		else
			smart_he = get_smart_home_end_from_string (GPM_DEFAULT_SMART_HOME_END);

		views = gedit_app_get_views (gedit_app_get_default ());
		l = views;

		while (l != NULL)
		{
			gtk_source_view_set_smart_home_end (GTK_SOURCE_VIEW (l->data),
							    smart_he);

			l = l->next;
		}

		g_list_free (views);
	}
}

static void
gedit_prefs_manager_syntax_hl_enable_changed (MateConfClient *client,
					      guint        cnxn_id,
					      MateConfEntry  *entry,
					      gpointer     user_data)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (entry->key != NULL);
	g_return_if_fail (entry->value != NULL);

	if (strcmp (entry->key, GPM_SYNTAX_HL_ENABLE) == 0)
	{
		gboolean enable;
		GList *docs;
		GList *l;
		const GList *windows;

		if (entry->value->type == MATECONF_VALUE_BOOL)
			enable = mateconf_value_get_bool (entry->value);
		else
			enable = GPM_DEFAULT_SYNTAX_HL_ENABLE;

		docs = gedit_app_get_documents (gedit_app_get_default ());
		l = docs;

		while (l != NULL)
		{
			g_return_if_fail (GTK_IS_SOURCE_BUFFER (l->data));

			gtk_source_buffer_set_highlight_syntax (GTK_SOURCE_BUFFER (l->data),
								enable);

			l = l->next;
		}

		g_list_free (docs);

		/* update the sensitivity of the Higlight Mode menu item */
		windows = gedit_app_get_windows (gedit_app_get_default ());
		while (windows != NULL)
		{
			GtkUIManager *ui;
			GtkAction *a;

			ui = gedit_window_get_ui_manager (GEDIT_WINDOW (windows->data));

			a = gtk_ui_manager_get_action (ui,
						       "/MenuBar/ViewMenu/ViewHighlightModeMenu");

			gtk_action_set_sensitive (a, enable);

			windows = g_list_next (windows);
		}
	}
}

static void
gedit_prefs_manager_search_hl_enable_changed (MateConfClient *client,
					      guint        cnxn_id,
					      MateConfEntry  *entry,
					      gpointer     user_data)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (entry->key != NULL);
	g_return_if_fail (entry->value != NULL);

	if (strcmp (entry->key, GPM_SEARCH_HIGHLIGHTING_ENABLE) == 0)
	{
		gboolean enable;
		GList *docs;
		GList *l;

		if (entry->value->type == MATECONF_VALUE_BOOL)
			enable = mateconf_value_get_bool (entry->value);
		else
			enable = GPM_DEFAULT_SEARCH_HIGHLIGHTING_ENABLE;

		docs = gedit_app_get_documents (gedit_app_get_default ());
		l = docs;

		while (l != NULL)
		{
			g_return_if_fail (GEDIT_IS_DOCUMENT (l->data));

			gedit_document_set_enable_search_highlighting  (GEDIT_DOCUMENT (l->data),
									enable);

			l = l->next;
		}

		g_list_free (docs);
	}
}

static void
gedit_prefs_manager_source_style_scheme_changed (MateConfClient *client,
						 guint        cnxn_id,
						 MateConfEntry  *entry,
						 gpointer     user_data)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (entry->key != NULL);
	g_return_if_fail (entry->value != NULL);

	if (strcmp (entry->key, GPM_SOURCE_STYLE_SCHEME) == 0)
	{
		static gchar *old_scheme = NULL;
		const gchar *scheme;
		GtkSourceStyleScheme *style;
		GList *docs;
		GList *l;

		if (entry->value->type == MATECONF_VALUE_STRING)
			scheme = mateconf_value_get_string (entry->value);
		else
			scheme = GPM_DEFAULT_SOURCE_STYLE_SCHEME;

		if (old_scheme != NULL && (strcmp (scheme, old_scheme) == 0))
		    	return;

		g_free (old_scheme);
		old_scheme = g_strdup (scheme);

		style = gtk_source_style_scheme_manager_get_scheme (
				gedit_get_style_scheme_manager (),
				scheme);

		if (style == NULL)
		{
			g_warning ("Default style scheme '%s' not found, falling back to 'classic'", scheme);
			
			style = gtk_source_style_scheme_manager_get_scheme (
				gedit_get_style_scheme_manager (),
				"classic");

			if (style == NULL) 
			{
				g_warning ("Style scheme 'classic' cannot be found, check your GtkSourceView installation.");
				return;
			}
		}

		docs = gedit_app_get_documents (gedit_app_get_default ());
		for (l = docs; l != NULL; l = l->next)
		{
			g_return_if_fail (GTK_IS_SOURCE_BUFFER (l->data));

			gtk_source_buffer_set_style_scheme (GTK_SOURCE_BUFFER (l->data),
							    style);
		}

		g_list_free (docs);
	}
}

static void
gedit_prefs_manager_max_recents_changed (MateConfClient *client,
					 guint        cnxn_id,
					 MateConfEntry  *entry,
					 gpointer     user_data)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (entry->key != NULL);
	g_return_if_fail (entry->value != NULL);

	if (strcmp (entry->key, GPM_MAX_RECENTS) == 0)
	{
		const GList *windows;
		gint max;

		if (entry->value->type == MATECONF_VALUE_INT)
		{
			max = mateconf_value_get_int (entry->value);

			if (max < 0)
				max = GPM_DEFAULT_MAX_RECENTS;
		}
		else
			max = GPM_DEFAULT_MAX_RECENTS;

		windows = gedit_app_get_windows (gedit_app_get_default ());
		while (windows != NULL)
		{
			GeditWindow *w = windows->data;

			gtk_recent_chooser_set_limit (GTK_RECENT_CHOOSER (w->priv->toolbar_recent_menu),
						      max);

			windows = g_list_next (windows);
		}

		/* FIXME: we have no way at the moment to trigger the
		 * update of the inline recents in the File menu */
	}
}

static void
gedit_prefs_manager_auto_save_changed (MateConfClient *client,
				       guint        cnxn_id,
				       MateConfEntry  *entry,
				       gpointer     user_data)
{
	GList *docs;
	GList *l;

	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (entry->key != NULL);
	g_return_if_fail (entry->value != NULL);

	if (strcmp (entry->key, GPM_AUTO_SAVE) == 0)
	{
		gboolean auto_save;

		if (entry->value->type == MATECONF_VALUE_BOOL)
			auto_save = mateconf_value_get_bool (entry->value);
		else
			auto_save = GPM_DEFAULT_AUTO_SAVE;

		docs = gedit_app_get_documents (gedit_app_get_default ());
		l = docs;

		while (l != NULL)
		{
			GeditDocument *doc = GEDIT_DOCUMENT (l->data);
			GeditTab *tab = gedit_tab_get_from_document (doc);

			gedit_tab_set_auto_save_enabled (tab, auto_save);

			l = l->next;
		}

		g_list_free (docs);
	}
	else if (strcmp (entry->key,  GPM_AUTO_SAVE_INTERVAL) == 0)
	{
		gint auto_save_interval;

		if (entry->value->type == MATECONF_VALUE_INT)
		{
			auto_save_interval = mateconf_value_get_int (entry->value);

			if (auto_save_interval <= 0)
				auto_save_interval = GPM_DEFAULT_AUTO_SAVE_INTERVAL;
		}
		else
			auto_save_interval = GPM_DEFAULT_AUTO_SAVE_INTERVAL;

		docs = gedit_app_get_documents (gedit_app_get_default ());
		l = docs;

		while (l != NULL)
		{
			GeditDocument *doc = GEDIT_DOCUMENT (l->data);
			GeditTab *tab = gedit_tab_get_from_document (doc);

			gedit_tab_set_auto_save_interval (tab, auto_save_interval);

			l = l->next;
		}

		g_list_free (docs);
	}
}

static void 
gedit_prefs_manager_active_plugins_changed (MateConfClient *client,
					    guint        cnxn_id,
					    MateConfEntry  *entry,
					    gpointer     user_data)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (entry->key != NULL);
	g_return_if_fail (entry->value != NULL);

	if (strcmp (entry->key, GPM_ACTIVE_PLUGINS) == 0)
	{
		if ((entry->value->type == MATECONF_VALUE_LIST) && 
		    (mateconf_value_get_list_type (entry->value) == MATECONF_VALUE_STRING))
		{
			GeditPluginsEngine *engine;

			engine = gedit_plugins_engine_get_default ();

			gedit_plugins_engine_active_plugins_changed (engine);
		}
	}
}

static void
gedit_prefs_manager_lockdown_changed (MateConfClient *client,
				      guint        cnxn_id,
				      MateConfEntry  *entry,
				      gpointer     user_data)
{
	GeditApp *app;
	gboolean locked;

	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (entry->key != NULL);
	g_return_if_fail (entry->value != NULL);

	if (entry->value->type == MATECONF_VALUE_BOOL)
		locked = mateconf_value_get_bool (entry->value);
	else
		locked = FALSE;

	app = gedit_app_get_default ();

	if (strcmp (entry->key, GPM_LOCKDOWN_COMMAND_LINE) == 0)
		_gedit_app_set_lockdown_bit (app, 
					     GEDIT_LOCKDOWN_COMMAND_LINE,
					     locked);

	else if (strcmp (entry->key, GPM_LOCKDOWN_PRINTING) == 0)
		_gedit_app_set_lockdown_bit (app, 
					     GEDIT_LOCKDOWN_PRINTING,
					     locked);

	else if (strcmp (entry->key, GPM_LOCKDOWN_PRINT_SETUP) == 0)
		_gedit_app_set_lockdown_bit (app, 
					     GEDIT_LOCKDOWN_PRINT_SETUP,
					     locked);

	else if (strcmp (entry->key, GPM_LOCKDOWN_SAVE_TO_DISK) == 0)
		_gedit_app_set_lockdown_bit (app, 
					     GEDIT_LOCKDOWN_SAVE_TO_DISK,
					     locked);
}
