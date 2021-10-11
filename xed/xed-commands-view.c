/*
 * xed-view-commands.c
 * This file is part of xed
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi
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
 * Modified by the xed Team, 1998-2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#include <config.h>
#include <gtk/gtk.h>

#include "xed-commands.h"
#include "xed-debug.h"
#include "xed-window.h"
#include "xed-window-private.h"
#include "xed-paned.h"
#include "xed-view-frame.h"
#include "xed-highlight-mode-dialog.h"
#include "xed-highlight-mode-selector.h"

void
_xed_cmd_view_show_toolbar (GtkAction *action,
                            XedWindow *window)
{
    gboolean visible;

    xed_debug (DEBUG_COMMANDS);

    visible = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

    if (visible)
    {
        gtk_widget_show (window->priv->toolbar);
    }
    else
    {
        gtk_widget_hide (window->priv->toolbar);
    }
}

void
_xed_cmd_view_show_menubar (GtkAction *action,
                            XedWindow *window)
{
    gboolean visible;

    xed_debug (DEBUG_COMMANDS);

    visible = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

    if (visible)
    {
        gtk_widget_show (window->priv->menubar);
        g_settings_set_boolean (window->priv->ui_settings, XED_SETTINGS_MENUBAR_VISIBLE, TRUE);
    }
    else
    {
        gtk_widget_hide (window->priv->menubar);
        g_settings_set_boolean (window->priv->ui_settings, XED_SETTINGS_MENUBAR_VISIBLE, FALSE);
    }
}

void
_xed_cmd_view_show_statusbar (GtkAction *action,
                              XedWindow *window)
{
    gboolean visible;

    xed_debug (DEBUG_COMMANDS);

    visible = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

    if (visible)
    {
        gtk_widget_show (window->priv->statusbar);
    }
    else
    {
        gtk_widget_hide (window->priv->statusbar);
    }
}

void
_xed_cmd_view_show_side_pane (GtkAction *action,
                              XedWindow *window)
{
    gboolean visible;
    XedPanel *panel;
    XedPaned *paned;

    xed_debug (DEBUG_COMMANDS);

    visible = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

    panel = xed_window_get_side_panel (window);
    paned = _xed_window_get_hpaned (window);

    if (visible)
    {
        gtk_widget_show (GTK_WIDGET (panel));
        xed_paned_open (paned, 1, _xed_window_get_side_panel_size (window));
        gtk_widget_grab_focus (GTK_WIDGET (panel));
    }
    else
    {
        xed_paned_close (paned, 1);
    }
}

void
_xed_cmd_view_show_bottom_pane (GtkAction *action,
                                XedWindow *window)
{
    gboolean visible;
    XedPanel *panel;
    XedPaned *paned;

    xed_debug (DEBUG_COMMANDS);

    visible = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

    panel = xed_window_get_bottom_panel (window);
    paned = _xed_window_get_vpaned (window);

    if (visible)
    {
        gint position;
        gint panel_size;
        gint paned_size;

        panel_size = _xed_window_get_bottom_panel_size (window);
        g_object_get (G_OBJECT (paned), "max-position", &paned_size, NULL);
        position = paned_size - panel_size;
        gtk_widget_show (GTK_WIDGET (panel));
        xed_paned_open (paned, 2, position);
        gtk_widget_grab_focus (GTK_WIDGET (panel));
    }
    else
    {
        xed_paned_close (paned, 2);
    }
}

void
_xed_cmd_view_toggle_overview_map (GtkAction *action,
                                   XedWindow *window)
{
    XedTab *tab;
    XedViewFrame *frame;
    GtkFrame *map_frame;
    gboolean visible;

    xed_debug (DEBUG_COMMANDS);

    tab = xed_window_get_active_tab (window);

    if (tab == NULL)
    {
        return;
    }

    frame = XED_VIEW_FRAME (_xed_tab_get_view_frame (tab));
    map_frame = xed_view_frame_get_map_frame (frame);
    visible = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

    gtk_widget_set_visible (GTK_WIDGET (map_frame), visible);
}

void
_xed_cmd_view_toggle_fullscreen_mode (GtkAction *action,
                                      XedWindow *window)
{
    xed_debug (DEBUG_COMMANDS);

    if (_xed_window_is_fullscreen (window))
    {
        _xed_window_unfullscreen (window);
    }
    else
    {
        _xed_window_fullscreen (window);
    }
}

void
_xed_cmd_view_toggle_word_wrap (GtkAction *action,
                                XedWindow *window)
{
    XedView *view;
    gboolean do_word_wrap;

    xed_debug (DEBUG_COMMANDS);

    do_word_wrap = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
    view = xed_window_get_active_view (window);

    if (do_word_wrap)
    {
        gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
    }
    else
    {
        gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_NONE);
    }
}

void
_xed_cmd_view_leave_fullscreen_mode (GtkAction *action,
                                     XedWindow *window)
{
    GtkAction *view_action;

    view_action = gtk_action_group_get_action (window->priv->always_sensitive_action_group, "ViewFullscreen");
    g_signal_handlers_block_by_func (view_action, G_CALLBACK (_xed_cmd_view_toggle_fullscreen_mode), window);
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (view_action), FALSE);
    _xed_window_unfullscreen (window);
    g_signal_handlers_unblock_by_func (view_action, G_CALLBACK (_xed_cmd_view_toggle_fullscreen_mode), window);
}

static void
on_language_selected (XedHighlightModeSelector *sel,
                      GtkSourceLanguage          *language,
                      XedWindow                *window)
{
    XedDocument *doc;

    doc = xed_window_get_active_document (window);
    if (doc)
    {
        xed_document_set_language (doc, language);
    }
}

void
_xed_cmd_view_change_highlight_mode (GtkAction *action,
                                     XedWindow *window)
{
    GtkWidget *dlg;
    XedHighlightModeSelector *sel;
    XedDocument *doc;

    dlg = xed_highlight_mode_dialog_new (GTK_WINDOW (window));
    sel = xed_highlight_mode_dialog_get_selector (XED_HIGHLIGHT_MODE_DIALOG (dlg));

    doc = xed_window_get_active_document (XED_WINDOW (window));
    if (doc)
    {
        xed_highlight_mode_selector_select_language (sel,
                                                       xed_document_get_language (doc));
    }

    g_signal_connect (sel, "language-selected",
                      G_CALLBACK (on_language_selected), window);

    gtk_widget_show (GTK_WIDGET (dlg));
}
