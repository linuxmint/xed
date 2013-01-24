/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pluma-prefs-manager.h
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

#ifndef __PLUMA_PREFS_MANAGER_H__
#define __PLUMA_PREFS_MANAGER_H__

#include <gtk/gtk.h>
#include <glib.h>
#include <gtksourceview/gtksourceview.h>
#include "pluma-app.h"

#define PLUMA_SCHEMA	"org.mate.pluma"

/* Editor */
#define GPM_USE_DEFAULT_FONT	"use-default-font"
#define GPM_EDITOR_FONT			"editor-font"

#define GPM_CREATE_BACKUP_COPY	"create-backup-copy"

#define GPM_AUTO_SAVE			"auto-save"
#define GPM_AUTO_SAVE_INTERVAL	"auto-save-interval"

#define GPM_UNDO_ACTIONS_LIMIT	"max-undo-actions"

#define GPM_WRAP_MODE			"wrap-mode"

#define GPM_TABS_SIZE			"tabs-size"
#define GPM_INSERT_SPACES		"insert-spaces"

#define GPM_AUTO_INDENT			"auto-indent"

#define GPM_DISPLAY_LINE_NUMBERS	"display-line-numbers"

#define GPM_HIGHLIGHT_CURRENT_LINE	"highlight-current-line"

#define GPM_BRACKET_MATCHING		"bracket-matching"

#define GPM_DISPLAY_RIGHT_MARGIN	"display-right-margin"
#define GPM_RIGHT_MARGIN_POSITION	"right-margin-position"

#define GPM_SMART_HOME_END			"smart-home-end"

#define GPM_RESTORE_CURSOR_POSITION	"restore-cursor-position"

#define GPM_SEARCH_HIGHLIGHTING_ENABLE	"enable-search-highlighting"

#define GPM_SOURCE_STYLE_SCHEME		"color-scheme"

/* UI */
#define GPM_TOOLBAR_VISIBLE			"toolbar-visible"
#define GPM_TOOLBAR_BUTTONS_STYLE	"toolbar-buttons-style"

#define GPM_STATUSBAR_VISIBLE		"statusbar-visible"

#define GPM_SIDE_PANE_VISIBLE		"side-pane-visible"

#define GPM_BOTTOM_PANEL_VISIBLE	"bottom-panel-visible"

#define GPM_MAX_RECENTS			"max-recents"

/* Print */
#define GPM_PRINT_SYNTAX		"print-syntax-highlighting"
#define GPM_PRINT_HEADER		"print-header"
#define GPM_PRINT_WRAP_MODE		"print-wrap-mode"
#define GPM_PRINT_LINE_NUMBERS	"print-line-numbers"

#define GPM_PRINT_FONT_BODY			"print-font-body-pango"
#define GPM_PRINT_FONT_HEADER		"print-font-header-pango"
#define GPM_PRINT_FONT_NUMBERS		"print-font-numbers-pango"

/* Encodings */
#define GPM_AUTO_DETECTED_ENCODINGS		"auto-detected-encodings"
#define GPM_SHOWN_IN_MENU_ENCODINGS		"shown-in-menu-encodings"

/* Syntax highlighting */
#define GPM_SYNTAX_HL_ENABLE		"enable-syntax-highlighting"

/* White list of writable mate-vfs methods */
#define GPM_WRITABLE_VFS_SCHEMES 	"writable-vfs-schemes"

/* Plugins */
#define GPM_ACTIVE_PLUGINS			"active-plugins"

/* Global Interface keys */
#define GPM_INTERFACE_SCHEMA		"org.mate.interface"
#define GPM_SYSTEM_FONT				"monospace-font-name"

/* Global Lockdown keys */
#define GPM_LOCKDOWN_SCHEMA			"org.mate.lockdown"
#define GPM_LOCKDOWN_COMMAND_LINE	"disable-command-line"
#define GPM_LOCKDOWN_PRINTING		"disable-printing"
#define GPM_LOCKDOWN_PRINT_SETUP	"disable-print-setup"
#define GPM_LOCKDOWN_SAVE_TO_DISK	"disable-save-to-disk"

/* Fallback default values. Keep in sync with org.mate.pluma.gschema.xml */
#define GPM_DEFAULT_AUTO_SAVE_INTERVAL	10 /* minutes */
#define GPM_DEFAULT_MAX_RECENTS			5

typedef enum {
	PLUMA_TOOLBAR_SYSTEM = 0,
	PLUMA_TOOLBAR_ICONS,
	PLUMA_TOOLBAR_ICONS_AND_TEXT,
	PLUMA_TOOLBAR_ICONS_BOTH_HORIZ
} PlumaToolbarSetting;

/** LIFE CYCLE MANAGEMENT FUNCTIONS **/

gboolean		 pluma_prefs_manager_init (void);

/* This function must be called before exiting pluma */
void			 pluma_prefs_manager_shutdown (void);


/** PREFS MANAGEMENT FUNCTIONS **/

/* Use default font */
gboolean 		 pluma_prefs_manager_get_use_default_font 	(void);
void			 pluma_prefs_manager_set_use_default_font 	(gboolean udf);
gboolean		 pluma_prefs_manager_use_default_font_can_set	(void);

/* Editor font */
gchar 			*pluma_prefs_manager_get_editor_font		(void);
void			 pluma_prefs_manager_set_editor_font 		(const gchar *font);
gboolean		 pluma_prefs_manager_editor_font_can_set	(void);

/* System font */
gchar 			*pluma_prefs_manager_get_system_font		(void);

/* Create backup copy */
gboolean		 pluma_prefs_manager_get_create_backup_copy	(void);
void			 pluma_prefs_manager_set_create_backup_copy	(gboolean cbc);
gboolean		 pluma_prefs_manager_create_backup_copy_can_set	(void);

/* Auto save */
gboolean		 pluma_prefs_manager_get_auto_save		(void);
void			 pluma_prefs_manager_set_auto_save		(gboolean as);
gboolean		 pluma_prefs_manager_auto_save_can_set		(void);

/* Auto save interval */
gint			 pluma_prefs_manager_get_auto_save_interval	(void);
void			 pluma_prefs_manager_set_auto_save_interval	(gint asi);
gboolean		 pluma_prefs_manager_auto_save_interval_can_set	(void);

/* Undo actions limit: if < 1 then no limits */
gint 			 pluma_prefs_manager_get_undo_actions_limit	(void);
void			 pluma_prefs_manager_set_undo_actions_limit	(gint ual);
gboolean		 pluma_prefs_manager_undo_actions_limit_can_set	(void);

/* Wrap mode */
GtkWrapMode		 pluma_prefs_manager_get_wrap_mode		(void);
void			 pluma_prefs_manager_set_wrap_mode		(GtkWrapMode wp);
gboolean		 pluma_prefs_manager_wrap_mode_can_set		(void);

/* Tabs size */
gint			 pluma_prefs_manager_get_tabs_size		(void);
void			 pluma_prefs_manager_set_tabs_size		(gint ts);
gboolean		 pluma_prefs_manager_tabs_size_can_set		(void);

/* Insert spaces */
gboolean		 pluma_prefs_manager_get_insert_spaces	 	(void);
void			 pluma_prefs_manager_set_insert_spaces	 	(gboolean ai);
gboolean		 pluma_prefs_manager_insert_spaces_can_set 	(void);

/* Auto indent */
gboolean		 pluma_prefs_manager_get_auto_indent	 	(void);
void			 pluma_prefs_manager_set_auto_indent	 	(gboolean ai);
gboolean		 pluma_prefs_manager_auto_indent_can_set 	(void);

/* Display line numbers */
gboolean		 pluma_prefs_manager_get_display_line_numbers 	(void);
void			 pluma_prefs_manager_set_display_line_numbers 	(gboolean dln);
gboolean		 pluma_prefs_manager_display_line_numbers_can_set (void);

/* Toolbar visible */
gboolean		 pluma_prefs_manager_get_toolbar_visible	(void);
void			 pluma_prefs_manager_set_toolbar_visible	(gboolean tv);
gboolean		 pluma_prefs_manager_toolbar_visible_can_set	(void);

/* Toolbar buttons style */
PlumaToolbarSetting 	 pluma_prefs_manager_get_toolbar_buttons_style	(void);
void 			 pluma_prefs_manager_set_toolbar_buttons_style	(PlumaToolbarSetting tbs);
gboolean		 pluma_prefs_manager_toolbar_buttons_style_can_set (void);

/* Statusbar visible */
gboolean		 pluma_prefs_manager_get_statusbar_visible	(void);
void			 pluma_prefs_manager_set_statusbar_visible	(gboolean sv);
gboolean		 pluma_prefs_manager_statusbar_visible_can_set	(void);

/* Side pane visible */
gboolean		 pluma_prefs_manager_get_side_pane_visible	(void);
void			 pluma_prefs_manager_set_side_pane_visible	(gboolean tv);
gboolean		 pluma_prefs_manager_side_pane_visible_can_set	(void);

/* Bottom panel visible */
gboolean		 pluma_prefs_manager_get_bottom_panel_visible	(void);
void			 pluma_prefs_manager_set_bottom_panel_visible	(gboolean tv);
gboolean		 pluma_prefs_manager_bottom_panel_visible_can_set(void);
/* Print syntax highlighting */
gboolean		 pluma_prefs_manager_get_print_syntax_hl	(void);
void			 pluma_prefs_manager_set_print_syntax_hl	(gboolean ps);
gboolean		 pluma_prefs_manager_print_syntax_hl_can_set	(void);

/* Print header */
gboolean		 pluma_prefs_manager_get_print_header		(void);
void			 pluma_prefs_manager_set_print_header		(gboolean ph);
gboolean		 pluma_prefs_manager_print_header_can_set	(void);

/* Wrap mode while printing */
GtkWrapMode		 pluma_prefs_manager_get_print_wrap_mode	(void);
void			 pluma_prefs_manager_set_print_wrap_mode	(GtkWrapMode pwm);
gboolean		 pluma_prefs_manager_print_wrap_mode_can_set	(void);

/* Print line numbers */
gint		 	 pluma_prefs_manager_get_print_line_numbers	(void);
void 			 pluma_prefs_manager_set_print_line_numbers	(gint pln);
gboolean		 pluma_prefs_manager_print_line_numbers_can_set	(void);

/* Font used to print the body of documents */
gchar			*pluma_prefs_manager_get_print_font_body	(void);
void			 pluma_prefs_manager_set_print_font_body	(const gchar *font);
gboolean		 pluma_prefs_manager_print_font_body_can_set	(void);
gchar			*pluma_prefs_manager_get_default_print_font_body (void);

/* Font used to print headers */
gchar			*pluma_prefs_manager_get_print_font_header	(void);
void			 pluma_prefs_manager_set_print_font_header	(const gchar *font);
gboolean		 pluma_prefs_manager_print_font_header_can_set	(void);
gchar			*pluma_prefs_manager_get_default_print_font_header (void);

/* Font used to print line numbers */
gchar			*pluma_prefs_manager_get_print_font_numbers	(void);
void			 pluma_prefs_manager_set_print_font_numbers	(const gchar *font);
gboolean		 pluma_prefs_manager_print_font_numbers_can_set	(void);
gchar			*pluma_prefs_manager_get_default_print_font_numbers (void);

/* Max number of files in "Recent Files" menu.
 * This is configurable only using gsettings, dconf or dconf-editor
 */
gint		 	 pluma_prefs_manager_get_max_recents		(void);

/* Encodings */
GSList 			*pluma_prefs_manager_get_auto_detected_encodings (void);

GSList			*pluma_prefs_manager_get_shown_in_menu_encodings (void);
void			 pluma_prefs_manager_set_shown_in_menu_encodings (const GSList *encs);
gboolean 		 pluma_prefs_manager_shown_in_menu_encodings_can_set (void);

/* Highlight current line */
gboolean		 pluma_prefs_manager_get_highlight_current_line	(void);
void			 pluma_prefs_manager_set_highlight_current_line	(gboolean hl);
gboolean		 pluma_prefs_manager_highlight_current_line_can_set (void);

/* Highlight matching bracket */
gboolean		 pluma_prefs_manager_get_bracket_matching	(void);
void			 pluma_prefs_manager_set_bracket_matching	(gboolean bm);
gboolean		 pluma_prefs_manager_bracket_matching_can_set (void);

/* Display right margin */
gboolean		 pluma_prefs_manager_get_display_right_margin	(void);
void			 pluma_prefs_manager_set_display_right_margin	(gboolean drm);
gboolean		 pluma_prefs_manager_display_right_margin_can_set (void);

/* Right margin position */
gint		 	 pluma_prefs_manager_get_right_margin_position	(void);
void 			 pluma_prefs_manager_set_right_margin_position	(gint rmp);
gboolean		 pluma_prefs_manager_right_margin_position_can_set (void);

/* Smart home end */
GtkSourceSmartHomeEndType
		 	 pluma_prefs_manager_get_smart_home_end		(void);
void 			 pluma_prefs_manager_set_smart_home_end		(GtkSourceSmartHomeEndType  smart_he);
gboolean		 pluma_prefs_manager_smart_home_end_can_set	(void);

/* Enable syntax highlighting */
gboolean 		 pluma_prefs_manager_get_enable_syntax_highlighting (void);
void			 pluma_prefs_manager_set_enable_syntax_highlighting (gboolean esh);
gboolean		 pluma_prefs_manager_enable_syntax_highlighting_can_set (void);

/* Writable VFS schemes */
GSList			*pluma_prefs_manager_get_writable_vfs_schemes	(void);

/* Restore cursor position */
gboolean 		 pluma_prefs_manager_get_restore_cursor_position (void);

/* Enable search highlighting */
gboolean 		 pluma_prefs_manager_get_enable_search_highlighting (void);
void			 pluma_prefs_manager_set_enable_search_highlighting (gboolean esh);
gboolean		 pluma_prefs_manager_enable_search_highlighting_can_set (void);

/* Style scheme */
gchar			*pluma_prefs_manager_get_source_style_scheme	(void);
void			 pluma_prefs_manager_set_source_style_scheme	(const gchar *scheme);
gboolean		 pluma_prefs_manager_source_style_scheme_can_set(void);

/* Plugins */
GSList			*pluma_prefs_manager_get_active_plugins		(void);
void			 pluma_prefs_manager_set_active_plugins		(const GSList *plugins);
gboolean 		 pluma_prefs_manager_active_plugins_can_set	(void);

/* Global lockdown */
PlumaLockdownMask	 pluma_prefs_manager_get_lockdown			(void);

/* GSettings utilities */
GSList*				 pluma_prefs_manager_get_gslist (GSettings *settings, const gchar *key);
void				 pluma_prefs_manager_set_gslist (GSettings *settings, const gchar *key, GSList *list);


#endif  /* __PLUMA_PREFS_MANAGER_H__ */


