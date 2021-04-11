/*
 * xed-settings.h
 * This file is part of xed
 *
 * Copyright (C) 2009 - Ignacio Casal Quinteiro
 *               2002 - Paolo Maggi
 *
 * xed is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xed is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xed; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */


#ifndef __XED_SETTINGS_H__
#define __XED_SETTINGS_H__

#include <glib-object.h>
#include <glib.h>
#include "xed-app.h"

G_BEGIN_DECLS

#define XED_TYPE_SETTINGS               (xed_settings_get_type ())
#define XED_SETTINGS(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_SETTINGS, XedSettings))
#define XED_SETTINGS_CONST(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_SETTINGS, XedSettings const))
#define XED_SETTINGS_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_SETTINGS, XedSettingsClass))
#define XED_IS_SETTINGS(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_SETTINGS))
#define XED_IS_SETTINGS_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_SETTINGS))
#define XED_SETTINGS_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), XED_TYPE_SETTINGS, XedSettingsClass))

typedef struct _XedSettings         XedSettings;
typedef struct _XedSettingsClass    XedSettingsClass;
typedef struct _XedSettingsPrivate  XedSettingsPrivate;

struct _XedSettings
{
    GObject parent;

    XedSettingsPrivate *priv;
};

struct _XedSettingsClass
{
    GObjectClass parent_class;
};

GType xed_settings_get_type (void) G_GNUC_CONST;

GObject *xed_settings_new (void);

gchar *xed_settings_get_system_font (XedSettings *xs);

/* Utility functions */
GSList *xed_settings_get_list (GSettings   *settings,
                               const gchar *key);

void xed_settings_set_list (GSettings    *settings,
                            const gchar  *key,
                            const GSList *list);

/* key constants */
#define XED_SETTINGS_USE_DEFAULT_FONT           "use-default-font"
#define XED_SETTINGS_EDITOR_FONT                "editor-font"
#define XED_SETTINGS_PREFER_DARK_THEME          "prefer-dark-theme"
#define XED_SETTINGS_SCHEME                     "scheme"
#define XED_SETTINGS_CREATE_BACKUP_COPY         "create-backup-copy"
#define XED_SETTINGS_AUTO_SAVE                  "auto-save"
#define XED_SETTINGS_AUTO_SAVE_INTERVAL         "auto-save-interval"
#define XED_SETTINGS_UNDO_ACTIONS_LIMIT         "undo-actions-limit"
#define XED_SETTINGS_MAX_UNDO_ACTIONS           "max-undo-actions"
#define XED_SETTINGS_WRAP_MODE                  "wrap-mode"
#define XED_SETTINGS_TABS_SIZE                  "tabs-size"
#define XED_SETTINGS_INSERT_SPACES              "insert-spaces"
#define XED_SETTINGS_AUTO_INDENT                "auto-indent"
#define XED_SETTINGS_DISPLAY_LINE_NUMBERS       "display-line-numbers"
#define XED_SETTINGS_HIGHLIGHT_CURRENT_LINE     "highlight-current-line"
#define XED_SETTINGS_BRACKET_MATCHING           "bracket-matching"
#define XED_SETTINGS_DISPLAY_RIGHT_MARGIN       "display-right-margin"
#define XED_SETTINGS_RIGHT_MARGIN_POSITION      "right-margin-position"
#define XED_SETTINGS_DRAW_WHITESPACE            "draw-whitespace"
#define XED_SETTINGS_DRAW_WHITESPACE_LEADING    "draw-whitespace-leading"
#define XED_SETTINGS_DRAW_WHITESPACE_TRAILING   "draw-whitespace-trailing"
#define XED_SETTINGS_DRAW_WHITESPACE_INSIDE     "draw-whitespace-inside"
#define XED_SETTINGS_DRAW_WHITESPACE_NEWLINE    "draw-whitespace-newline"
#define XED_SETTINGS_SMART_HOME_END             "smart-home-end"
#define XED_SETTINGS_WRITABLE_VFS_SCHEMES       "writable-vfs-schemes"
#define XED_SETTINGS_RESTORE_CURSOR_POSITION    "restore-cursor-position"
#define XED_SETTINGS_SYNTAX_HIGHLIGHTING        "syntax-highlighting"
#define XED_SETTINGS_SEARCH_HIGHLIGHTING        "search-highlighting"
#define XED_SETTINGS_ENABLE_TAB_SCROLLING       "enable-tab-scrolling"
#define XED_SETTINGS_TOOLBAR_VISIBLE            "toolbar-visible"
#define XED_SETTINGS_STATUSBAR_VISIBLE          "statusbar-visible"
#define XED_SETTINGS_SIDE_PANEL_VISIBLE         "side-panel-visible"
#define XED_SETTINGS_BOTTOM_PANEL_VISIBLE       "bottom-panel-visible"
#define XED_SETTINGS_MINIMAP_VISIBLE            "minimap-visible"
#define XED_SETTINGS_MAX_RECENTS                "max-recents"
#define XED_SETTINGS_PRINT_SYNTAX_HIGHLIGHTING  "print-syntax-highlighting"
#define XED_SETTINGS_PRINT_HEADER               "print-header"
#define XED_SETTINGS_PRINT_WRAP_MODE            "print-wrap-mode"
#define XED_SETTINGS_PRINT_LINE_NUMBERS         "print-line-numbers"
#define XED_SETTINGS_PRINT_FONT_BODY_PANGO      "print-font-body-pango"
#define XED_SETTINGS_PRINT_FONT_HEADER_PANGO    "print-font-header-pango"
#define XED_SETTINGS_PRINT_FONT_NUMBERS_PANGO   "print-font-numbers-pango"
#define XED_SETTINGS_ENCODING_AUTO_DETECTED     "auto-detected"
#define XED_SETTINGS_ENCODING_SHOWN_IN_MENU     "shown-in-menu"
#define XED_SETTINGS_ACTIVE_PLUGINS             "active-plugins"
#define XED_SETTINGS_ENSURE_TRAILING_NEWLINE    "ensure-trailing-newline"

/* window state keys */
#define XED_SETTINGS_WINDOW_STATE               "state"
#define XED_SETTINGS_WINDOW_SIZE                "size"
#define XED_SETTINGS_SIDE_PANEL_SIZE            "side-panel-size"
#define XED_SETTINGS_SIDE_PANEL_ACTIVE_PAGE     "side-panel-active-page"
#define XED_SETTINGS_BOTTOM_PANEL_SIZE          "bottom-panel-size"
#define XED_SETTINGS_BOTTOM_PANEL_ACTIVE_PAGE   "bottom-panel-active-page"
#define XED_SETTINGS_ACTIVE_FILE_FILTER         "filter-id"

G_END_DECLS

#endif /* __XED_SETTINGS_H__ */
