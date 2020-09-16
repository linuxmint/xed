/*
 * xed-ui.h
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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

#ifndef __XED_UI_H__
#define __XED_UI_H__

#include <config.h>
#include <gtk/gtk.h>

#include "xed-commands.h"

G_BEGIN_DECLS

static const GtkActionEntry xed_always_sensitive_menu_entries[] =
{
	/* Toplevel */
	{ "File", NULL, N_("_File") },
	{ "Edit", NULL, N_("_Edit") },
	{ "View", NULL, N_("_View") },
	{ "Search", NULL, N_("_Search") },
	{ "Tools", NULL, N_("_Tools") },
	{ "Documents", NULL, N_("_Documents") },
	{ "Help", NULL, N_("_Help") },
    { "XAppFavoritesMenu", NULL, N_("Favorites")},
    { "FileRecentsMenu", NULL, N_("Recents")},

	/* File menu */
	{ "FileNew", "document-new-symbolic", N_("_New"), "<control>N",
	  N_("Create a new document"), G_CALLBACK (_xed_cmd_file_new) },
	{ "FileOpen", "document-open-symbolic", N_("_Open..."), "<control>O",
	  N_("Open a file"), G_CALLBACK (_xed_cmd_file_open) },

	/* Edit menu */
	{ "EditPreferences", "preferences-other-symbolic", N_("Pr_eferences"), NULL,
	  N_("Configure the application"), G_CALLBACK (_xed_cmd_edit_preferences) },

	/* Help menu */
	{"HelpContents", "help-contents-symbolic", N_("_Contents"), "F1",
	 N_("Open the xed manual"), G_CALLBACK (_xed_cmd_help_contents) },
	{ "HelpAbout", "help-about-symbolic", N_("_About"), NULL,
	 N_("About this application"), G_CALLBACK (_xed_cmd_help_about) },
    { "HelpShortcuts", "preferences-desktop-keyboard-shortcuts-symbolic", N_("_Keyboard Shortcuts"), NULL,
     N_("Show the keyboard shortcuts dialog"), G_CALLBACK (_xed_cmd_help_keyboard_shortcuts) },

	/* Fullscreen toolbar */
	{ "LeaveFullscreen", "view-restore-symbolic", NULL,
	  NULL, N_("Leave fullscreen mode"),
	  G_CALLBACK (_xed_cmd_view_leave_fullscreen_mode) }
};

static const GtkActionEntry xed_menu_entries[] =
{
	/* File menu */
	{ "FileSave", "document-save-symbolic", N_("_Save"), "<control>S",
	  N_("Save the current file"), G_CALLBACK (_xed_cmd_file_save) },
	{ "FileSaveAs", "document-save-as-symbolic", N_("Save _As..."), "<shift><control>S",
	  N_("Save the current file with a different name"), G_CALLBACK (_xed_cmd_file_save_as) },
	{ "FileRevert", "document-revert-symbolic", N_("Revert"), NULL,
	  N_("Revert to a saved version of the file"), G_CALLBACK (_xed_cmd_file_revert) },
	{ "FilePrintPreview", "document-print-preview-symbolic", N_("Print Previe_w"),"<control><shift>P",
	  N_("Print preview"), G_CALLBACK (_xed_cmd_file_print_preview) },
	 { "FilePrint", "document-print-symbolic", N_("_Print..."), "<control>P",
	  N_("Print the current page"), G_CALLBACK (_xed_cmd_file_print) },

	/* Edit menu */
	{ "EditUndo", "edit-undo-symbolic", N_("_Undo"), "<control>Z",
	  N_("Undo the last action"), G_CALLBACK (_xed_cmd_edit_undo) },
	{ "EditRedo", "edit-redo-symbolic", N_("_Redo"), "<control>Y",
	  N_("Redo the last undone action"), G_CALLBACK (_xed_cmd_edit_redo) },
	{ "EditCut", "edit-cut-symbolic", N_("C_ut"), "<control>X",
	  N_("Cut the selection"), G_CALLBACK (_xed_cmd_edit_cut) },
	{ "EditCopy", "edit-copy-symbolic", N_("_Copy"), "<control>C",
	  N_("Copy the selection"), G_CALLBACK (_xed_cmd_edit_copy) },
	{ "EditPaste", "edit-paste-symbolic", N_("_Paste"), "<control>V",
	  N_("Paste the clipboard"), G_CALLBACK (_xed_cmd_edit_paste) },
	{ "EditDelete", "edit-delete-symbolic", N_("_Delete"), NULL,
	  N_("Delete the selected text"), G_CALLBACK (_xed_cmd_edit_delete) },
	{ "EditSelectAll", "edit-select-all-symbolic", N_("Select _All"), "<control>A",
	  N_("Select the entire document"), G_CALLBACK (_xed_cmd_edit_select_all) },
    { "EditToggleComment", NULL, N_("_Toggle Comment"), "<control>slash",
      N_("Comment"), G_CALLBACK (_xed_cmd_edit_toggle_comment) },
    { "EditToggleCommentBlock", NULL, N_("Toggle Comment _Block"), "<shift><control>question",
      N_("Comment Block"), G_CALLBACK (_xed_cmd_edit_toggle_comment_block) },

	/* View menu */
	{ "ViewHighlightMode", NULL, N_("_Highlight Mode"), "<shift><control>H",
	  N_("Change syntax hightlight mode"),
	  G_CALLBACK (_xed_cmd_view_change_highlight_mode) },

	/* Search menu */
	{ "SearchFind", "edit-find-symbolic", N_("_Find"), "<control>F",
	  N_("Search for text"), G_CALLBACK (_xed_cmd_search_find) },
	{ "SearchFindNext", NULL, N_("Find Ne_xt"), "<control>G",
	  N_("Search forwards for the same text"), G_CALLBACK (_xed_cmd_search_find_next) },
	{ "SearchFindPrevious", NULL, N_("Find Pre_vious"), "<shift><control>G",
	  N_("Search backwards for the same text"), G_CALLBACK (_xed_cmd_search_find_prev) },
	{ "SearchReplace", "edit-find-replace-symbolic", N_("_Replace"), "<control>H",
	  N_("Search for and replace text"), G_CALLBACK (_xed_cmd_search_replace) },
	{ "SearchGoToLine", "go-jump-symbolic", N_("Go to _Line..."), "<control>I",
	  N_("Go to a specific line"), G_CALLBACK (_xed_cmd_search_goto_line) },

	/* Documents menu */
	{ "FileSaveAll", "document-save-symbolic", N_("_Save All"), "<shift><control>L",
	  N_("Save all open files"), G_CALLBACK (_xed_cmd_file_save_all) },
	{ "FileCloseAll", "window-close-symbolic", N_("_Close All"), "<shift><control>W",
	  N_("Close all open files"), G_CALLBACK (_xed_cmd_file_close_all) },
	{ "DocumentsPreviousDocument", NULL, N_("_Previous Document"), "<alt><control>Page_Up",
	  N_("Activate previous document"), G_CALLBACK (_xed_cmd_documents_previous_document) },
	{ "DocumentsNextDocument", NULL, N_("_Next Document"), "<alt><control>Page_Down",
	  N_("Activate next document"), G_CALLBACK (_xed_cmd_documents_next_document) },
	{ "DocumentsMoveToNewWindow", NULL, N_("_Move to New Window"), NULL,
	  N_("Move the current document to a new window"), G_CALLBACK (_xed_cmd_documents_move_to_new_window) }
};

/* separate group, needs to be sensitive on OS X even when there are no tabs */
static const GtkActionEntry xed_close_menu_entries[] =
{
	{ "FileClose", "window-close-symbolic", N_("_Close"), "<control>W",
	  N_("Close the current file"), G_CALLBACK (_xed_cmd_file_close) }
};

/* separate group, should be sensitive even when there are no tabs */
static const GtkActionEntry xed_quit_menu_entries[] =
{
	{ "FileQuit", "application-exit-symbolic", N_("_Quit"), "<control>Q",
	  N_("Quit the program"), G_CALLBACK (_xed_cmd_file_quit) }
};

static const GtkToggleActionEntry xed_always_sensitive_toggle_menu_entries[] =
{
	{ "ViewToolbar", NULL, N_("_Toolbar"), NULL,
	  N_("Show or hide the toolbar in the current window"),
	  G_CALLBACK (_xed_cmd_view_show_toolbar), TRUE },
	{ "ViewStatusbar", NULL, N_("_Statusbar"), NULL,
	  N_("Show or hide the statusbar in the current window"),
	  G_CALLBACK (_xed_cmd_view_show_statusbar), TRUE },
	{ "ViewFullscreen", "view-fullscreen", N_("Fullscreen"), "F11",
	  N_("Edit text in fullscreen"),
	  G_CALLBACK (_xed_cmd_view_toggle_fullscreen_mode), FALSE },
    { "ViewWordWrap", NULL, N_("_Word wrap"), "<control>R",
      N_("Set word wrap for the current document"),
      G_CALLBACK (_xed_cmd_view_toggle_word_wrap), FALSE },
    { "ViewOverviewMap", NULL, N_("_Overview Map"), NULL,
      N_("Show or hide the overview map for the current view"),
      G_CALLBACK (_xed_cmd_view_toggle_overview_map), FALSE }
};

/* separate group, should be always sensitive except when there are no panes */
static const GtkToggleActionEntry xed_panes_toggle_menu_entries[] =
{
	{ "ViewSidePane", NULL, N_("Side _Pane"), "F9",
	  N_("Show or hide the side pane in the current window"),
	  G_CALLBACK (_xed_cmd_view_show_side_pane), FALSE },
	{ "ViewBottomPane", NULL, N_("_Bottom Pane"), "<control>F9",
	  N_("Show or hide the bottom pane in the current window"),
	  G_CALLBACK (_xed_cmd_view_show_bottom_pane), FALSE }
};

G_END_DECLS

#endif  /* __XED_UI_H__  */
