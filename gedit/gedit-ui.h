/*
 * gedit-ui.h
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */
 
/*
 * Modified by the gedit Team, 2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __GEDIT_UI_H__
#define __GEDIT_UI_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include "gedit-commands.h"

G_BEGIN_DECLS

static const GtkActionEntry gedit_always_sensitive_menu_entries[] =
{
	/* Toplevel */
	{ "File", NULL, N_("_File") },
	{ "Edit", NULL, N_("_Edit") },
	{ "View", NULL, N_("_View") },
	{ "Search", NULL, N_("_Search") },
	{ "Tools", NULL, N_("_Tools") },
	{ "Documents", NULL, N_("_Documents") },
	{ "Help", NULL, N_("_Help") },

	/* File menu */
	{ "FileNew", GTK_STOCK_NEW, NULL, "<control>N",
	  N_("Create a new document"), G_CALLBACK (_gedit_cmd_file_new) },
	{ "FileOpen", GTK_STOCK_OPEN, N_("_Open..."), "<control>O",
	  N_("Open a file"), G_CALLBACK (_gedit_cmd_file_open) },

	/* Edit menu */
	{ "EditPreferences", GTK_STOCK_PREFERENCES, N_("Pr_eferences"), NULL,
	  N_("Configure the application"), G_CALLBACK (_gedit_cmd_edit_preferences) },

	/* Help menu */
	{"HelpContents", GTK_STOCK_HELP, N_("_Contents"), "F1",
	 N_("Open the gedit manual"), G_CALLBACK (_gedit_cmd_help_contents) },
	{ "HelpAbout", GTK_STOCK_ABOUT, NULL, NULL,
	 N_("About this application"), G_CALLBACK (_gedit_cmd_help_about) },
	
	/* Fullscreen toolbar */
	{ "LeaveFullscreen", GTK_STOCK_LEAVE_FULLSCREEN, NULL,
	  NULL, N_("Leave fullscreen mode"),
	  G_CALLBACK (_gedit_cmd_view_leave_fullscreen_mode) }
};

static const GtkActionEntry gedit_menu_entries[] =
{
	/* File menu */
	{ "FileSave", GTK_STOCK_SAVE, NULL, "<control>S",
	  N_("Save the current file"), G_CALLBACK (_gedit_cmd_file_save) },
	{ "FileSaveAs", GTK_STOCK_SAVE_AS, N_("Save _As..."), "<shift><control>S",
	  N_("Save the current file with a different name"), G_CALLBACK (_gedit_cmd_file_save_as) },
	{ "FileRevert", GTK_STOCK_REVERT_TO_SAVED, NULL, NULL,
	  N_("Revert to a saved version of the file"), G_CALLBACK (_gedit_cmd_file_revert) },
#if !GTK_CHECK_VERSION (2, 17, 4)
	{ "FilePageSetup", GTK_STOCK_PAGE_SETUP, N_("Page Set_up..."), NULL,
	  N_("Set up the page settings"), G_CALLBACK (_gedit_cmd_file_page_setup) },
#endif
	{ "FilePrintPreview", GTK_STOCK_PRINT_PREVIEW, N_("Print Previe_w"),"<control><shift>P",
	  N_("Print preview"), G_CALLBACK (_gedit_cmd_file_print_preview) },
	 { "FilePrint", GTK_STOCK_PRINT, N_("_Print..."), "<control>P",
	  N_("Print the current page"), G_CALLBACK (_gedit_cmd_file_print) },

	/* Edit menu */
	{ "EditUndo", GTK_STOCK_UNDO, NULL, "<control>Z",
	  N_("Undo the last action"), G_CALLBACK (_gedit_cmd_edit_undo) },
	{ "EditRedo", GTK_STOCK_REDO, NULL, "<shift><control>Z",
	  N_("Redo the last undone action"), G_CALLBACK (_gedit_cmd_edit_redo) },
	{ "EditCut", GTK_STOCK_CUT, NULL, "<control>X",
	  N_("Cut the selection"), G_CALLBACK (_gedit_cmd_edit_cut) },
	{ "EditCopy", GTK_STOCK_COPY, NULL, "<control>C",
	  N_("Copy the selection"), G_CALLBACK (_gedit_cmd_edit_copy) },
	{ "EditPaste", GTK_STOCK_PASTE, NULL, "<control>V",
	  N_("Paste the clipboard"), G_CALLBACK (_gedit_cmd_edit_paste) },
	{ "EditDelete", GTK_STOCK_DELETE, NULL, NULL,
	  N_("Delete the selected text"), G_CALLBACK (_gedit_cmd_edit_delete) },
	{ "EditSelectAll", GTK_STOCK_SELECT_ALL, N_("Select _All"), "<control>A",
	  N_("Select the entire document"), G_CALLBACK (_gedit_cmd_edit_select_all) },

	/* View menu */
	{ "ViewHighlightMode", NULL, N_("_Highlight Mode") },

	/* Search menu */
	{ "SearchFind", GTK_STOCK_FIND, N_("_Find..."), "<control>F",
	  N_("Search for text"), G_CALLBACK (_gedit_cmd_search_find) },
	{ "SearchFindNext", NULL, N_("Find Ne_xt"), "<control>G",
	  N_("Search forwards for the same text"), G_CALLBACK (_gedit_cmd_search_find_next) },
	{ "SearchFindPrevious", NULL, N_("Find Pre_vious"), "<shift><control>G",
	  N_("Search backwards for the same text"), G_CALLBACK (_gedit_cmd_search_find_prev) },
#ifndef OS_OSX
	{ "SearchReplace", GTK_STOCK_FIND_AND_REPLACE, N_("_Replace..."), "<control>H",
	  N_("Search for and replace text"), G_CALLBACK (_gedit_cmd_search_replace) },
#else
	{ "SearchReplace", GTK_STOCK_FIND_AND_REPLACE, N_("_Replace..."), "<control><alt>F",
	  N_("Search for and replace text"), G_CALLBACK (_gedit_cmd_search_replace) },
#endif
	{ "SearchClearHighlight", NULL, N_("_Clear Highlight"), "<shift><control>K",
	  N_("Clear highlighting of search matches"), G_CALLBACK (_gedit_cmd_search_clear_highlight) },
	{ "SearchGoToLine", GTK_STOCK_JUMP_TO, N_("Go to _Line..."), "<control>I",
	  N_("Go to a specific line"), G_CALLBACK (_gedit_cmd_search_goto_line) },
	{ "SearchIncrementalSearch", GTK_STOCK_FIND, N_("_Incremental Search..."), "<control>K",
	  N_("Incrementally search for text"), G_CALLBACK (_gedit_cmd_search_incremental_search) },

	/* Documents menu */
	{ "FileSaveAll", GTK_STOCK_SAVE, N_("_Save All"), "<shift><control>L",
	  N_("Save all open files"), G_CALLBACK (_gedit_cmd_file_save_all) },
	{ "FileCloseAll", GTK_STOCK_CLOSE, N_("_Close All"), "<shift><control>W",
	  N_("Close all open files"), G_CALLBACK (_gedit_cmd_file_close_all) },
	{ "DocumentsPreviousDocument", NULL, N_("_Previous Document"), "<alt><control>Page_Up",
	  N_("Activate previous document"), G_CALLBACK (_gedit_cmd_documents_previous_document) },
	{ "DocumentsNextDocument", NULL, N_("_Next Document"), "<alt><control>Page_Down",
	  N_("Activate next document"), G_CALLBACK (_gedit_cmd_documents_next_document) },
	{ "DocumentsMoveToNewWindow", NULL, N_("_Move to New Window"), NULL,
	  N_("Move the current document to a new window"), G_CALLBACK (_gedit_cmd_documents_move_to_new_window) }
};

/* separate group, needs to be sensitive on OS X even when there are no tabs */
static const GtkActionEntry gedit_close_menu_entries[] =
{
	{ "FileClose", GTK_STOCK_CLOSE, NULL, "<control>W",
	  N_("Close the current file"), G_CALLBACK (_gedit_cmd_file_close) }
};

/* separate group, should be sensitive even when there are no tabs */
static const GtkActionEntry gedit_quit_menu_entries[] =
{
	{ "FileQuit", GTK_STOCK_QUIT, NULL, "<control>Q",
	  N_("Quit the program"), G_CALLBACK (_gedit_cmd_file_quit) }
};

static const GtkToggleActionEntry gedit_always_sensitive_toggle_menu_entries[] =
{
	{ "ViewToolbar", NULL, N_("_Toolbar"), NULL,
	  N_("Show or hide the toolbar in the current window"),
	  G_CALLBACK (_gedit_cmd_view_show_toolbar), TRUE },
	{ "ViewStatusbar", NULL, N_("_Statusbar"), NULL,
	  N_("Show or hide the statusbar in the current window"),
	  G_CALLBACK (_gedit_cmd_view_show_statusbar), TRUE },
	{ "ViewFullscreen", GTK_STOCK_FULLSCREEN, NULL, "F11",
	  N_("Edit text in fullscreen"),
	  G_CALLBACK (_gedit_cmd_view_toggle_fullscreen_mode), FALSE }
};

/* separate group, should be always sensitive except when there are no panes */
static const GtkToggleActionEntry gedit_panes_toggle_menu_entries[] =
{
	{ "ViewSidePane", NULL, N_("Side _Pane"), "F9",
	  N_("Show or hide the side pane in the current window"),
	  G_CALLBACK (_gedit_cmd_view_show_side_pane), FALSE },
	{ "ViewBottomPane", NULL, N_("_Bottom Pane"), "<control>F9",
	  N_("Show or hide the bottom pane in the current window"),
	  G_CALLBACK (_gedit_cmd_view_show_bottom_pane), FALSE }
};

G_END_DECLS

#endif  /* __GEDIT_UI_H__  */
