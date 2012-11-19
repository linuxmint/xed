/*
 * pluma-window-private.h
 * This file is part of pluma
 *
 * Copyright (C) 2005 - Paolo Maggi 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANWINDOWILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA 02110-1301, USA.
 */
 
/*
 * Modified by the pluma Team, 2005. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __PLUMA_WINDOW_PRIVATE_H__
#define __PLUMA_WINDOW_PRIVATE_H__

#include "pluma/pluma-window.h"
#include "pluma-prefs-manager.h"
#include "pluma-message-bus.h"

#ifdef OS_OSX
#include <ige-mac-integration.h>
#endif

G_BEGIN_DECLS

/* WindowPrivate is in a separate .h so that we can access it from pluma-commands */

struct _PlumaWindowPrivate
{
	GtkWidget      *notebook;

	GtkWidget      *side_panel;
	GtkWidget      *bottom_panel;

	GtkWidget      *hpaned;
	GtkWidget      *vpaned;
	
	GtkWidget      *tab_width_combo;
	GtkWidget      *language_combo;
	
	PlumaMessageBus *message_bus;	

	/* Widgets for fullscreen mode */
	GtkWidget      *fullscreen_controls;
	guint           fullscreen_animation_timeout_id;
	gboolean        fullscreen_animation_enter;

	/* statusbar and context ids for statusbar messages */
	GtkWidget      *statusbar;	
	guint           generic_message_cid;
	guint           tip_message_cid;
	guint 		tab_width_id;
	guint 		spaces_instead_of_tabs_id;
	guint 		language_changed_id;

	/* Menus & Toolbars */
	GtkUIManager   *manager;
	GtkActionGroup *action_group;
	GtkActionGroup *always_sensitive_action_group;
	GtkActionGroup *close_action_group;
	GtkActionGroup *quit_action_group;
	GtkActionGroup *panes_action_group;
	GtkActionGroup *languages_action_group;
	GtkActionGroup *documents_list_action_group;
	guint           documents_list_menu_ui_id;
	GtkWidget      *toolbar;
	GtkWidget      *toolbar_recent_menu;
	GtkWidget      *menubar;
	PlumaToolbarSetting toolbar_style;

	/* recent files */
	GtkActionGroup *recents_action_group;
	guint           recents_menu_ui_id;
	gulong          recents_handler_id;

	PlumaTab       *active_tab;
	gint            num_tabs;

	gint            num_tabs_with_error;

	gint            width;
	gint            height;	
	GdkWindowState  window_state;

	gint            side_panel_size;
	gint            bottom_panel_size;

	PlumaWindowState state;

	gint            bottom_panel_item_removed_handler_id;

	GtkWindowGroup *window_group;

	GFile          *default_location;

#ifdef OS_OSX
	IgeMacMenuGroup *mac_menu_group;
#endif

	gboolean        removing_tabs : 1;
	gboolean        dispose_has_run : 1;
};

G_END_DECLS

#endif  /* __PLUMA_WINDOW_PRIVATE_H__  */
