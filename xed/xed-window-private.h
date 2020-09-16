/*
 * xed-window-private.h
 * This file is part of xed
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
 * Modified by the xed Team, 2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __XED_WINDOW_PRIVATE_H__
#define __XED_WINDOW_PRIVATE_H__

#include <libpeas/peas-extension-set.h>
#include <libxapp/xapp-favorites.h>

#include "xed/xed-window.h"
#include "xed-message-bus.h"
#include "xed-settings.h"

G_BEGIN_DECLS

/* WindowPrivate is in a separate .h so that we can access it from xed-commands */

struct _XedWindowPrivate
{
    GSettings *editor_settings;
    GSettings *ui_settings;
    GSettings *window_settings;

    GtkWidget *notebook;

    GtkWidget *side_panel;
    GtkWidget *bottom_panel;

    GtkWidget *hpaned;
    GtkWidget *vpaned;

    GtkWidget *tab_width_button;
    GtkWidget *tab_width_menu;
    GtkWidget *tab_width_item;
    GtkWidget *language_button;
    GtkWidget *language_popover;
    GtkWidget *show_side_pane_button;
    GtkWidget *show_bottom_pane_button;
    GtkWidget *bottom_pane_button_revealer;

    XedMessageBus    *message_bus;
    PeasExtensionSet *extensions;

    /* Widgets for fullscreen mode */
    GtkWidget *fullscreen_controls;
    GtkWidget *fullscreen_overlay;
    GtkWidget *fullscreen_eventbox;
    GtkWidget *fullscreen_revealer;

    /* statusbar and context ids for statusbar messages */
    GtkWidget *statusbar;
    GtkWidget *searchbar;
    guint      generic_message_cid;
    guint      tip_message_cid;
    guint      tab_width_id;
    guint      spaces_instead_of_tabs_id;
    guint      language_changed_id;
    guint      use_word_wrap_id;
    guint      show_overview_map_id;

    /* Menus & Toolbars */
    GtkUIManager   *manager;
    GtkActionGroup *action_group;
    GtkActionGroup *always_sensitive_action_group;
    GtkActionGroup *close_action_group;
    GtkActionGroup *quit_action_group;
    GtkActionGroup *panes_action_group;
    GtkActionGroup *documents_list_action_group;
    guint           documents_list_menu_ui_id;
    GtkWidget      *toolbar;
    GtkWidget      *menubar;

    /* recent files */
    GtkActionGroup *recents_action_group;
    guint           recents_menu_ui_id;
    gulong          recents_handler_id;

    GtkActionGroup *favorites_action_group;
    guint           favorites_menu_ui_id;
    gulong          favorites_handler_id;

    XedTab *active_tab;
    gint    num_tabs;

    gint num_tabs_with_error;

    gint            width;
    gint            height;
    GdkWindowState  window_state;

    guint inhibition_cookie;

    gint side_panel_size;
    gint bottom_panel_size;

    XedWindowState state;

    gint bottom_panel_item_removed_handler_id;

    GtkWindowGroup *window_group;

    GFile *default_location;

    gboolean removing_tabs : 1;
    gboolean dispose_has_run : 1;
};

G_END_DECLS

#endif  /* __XED_WINDOW_PRIVATE_H__  */
