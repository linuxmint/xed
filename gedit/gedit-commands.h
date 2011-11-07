/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gedit-commands.h
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */
 
/*
 * Modified by the gedit Team, 1998-2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __GEDIT_COMMANDS_H__
#define __GEDIT_COMMANDS_H__

#include <gtk/gtk.h>
#include <gedit/gedit-window.h>

G_BEGIN_DECLS

/* Do nothing if URI does not exist */
void		 gedit_commands_load_uri		(GeditWindow         *window,
							 const gchar         *uri,
							 const GeditEncoding *encoding,
							 gint                 line_pos);

/* Ignore non-existing URIs */
gint		 gedit_commands_load_uris		(GeditWindow         *window,
							 const GSList        *uris,
							 const GeditEncoding *encoding,
							 gint                 line_pos);

void		 gedit_commands_save_document		(GeditWindow         *window,
                                                         GeditDocument       *document);

void		 gedit_commands_save_all_documents 	(GeditWindow         *window);

/*
 * Non-exported functions
 */

/* Create titled documens for non-existing URIs */
gint		_gedit_cmd_load_files_from_prompt	(GeditWindow         *window,
							 GSList              *files,
							 const GeditEncoding *encoding,
							 gint                 line_pos);

void		_gedit_cmd_file_new			(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_file_open			(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_file_save			(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_file_save_as			(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_file_save_all		(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_file_revert			(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_file_open_uri		(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_file_print_preview		(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_file_print			(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_file_close			(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_file_close_all		(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_file_quit			(GtkAction   *action,
							 GeditWindow *window);

void		_gedit_cmd_edit_undo			(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_edit_redo			(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_edit_cut			(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_edit_copy			(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_edit_paste			(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_edit_delete			(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_edit_select_all		(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_edit_preferences		(GtkAction   *action,
							 GeditWindow *window);

void		_gedit_cmd_view_show_toolbar		(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_view_show_statusbar		(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_view_show_side_pane		(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_view_show_bottom_pane	(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_view_toggle_fullscreen_mode	(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_view_leave_fullscreen_mode	(GtkAction   *action,
							 GeditWindow *window);

void		_gedit_cmd_search_find			(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_search_find_next		(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_search_find_prev		(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_search_replace		(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_search_clear_highlight	(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_search_goto_line		(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_search_incremental_search	(GtkAction   *action,
							 GeditWindow *window);							 
							 
void		_gedit_cmd_documents_previous_document	(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_documents_next_document	(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_documents_move_to_new_window	(GtkAction   *action,
							 GeditWindow *window);

void		_gedit_cmd_help_contents		(GtkAction   *action,
							 GeditWindow *window);
void		_gedit_cmd_help_about			(GtkAction   *action,
							 GeditWindow *window);

void		_gedit_cmd_file_close_tab 		(GeditTab    *tab,
							 GeditWindow *window);

void		_gedit_cmd_file_save_documents_list	(GeditWindow *window,
							 GList       *docs);


#if !GTK_CHECK_VERSION (2, 17, 4)
void		_gedit_cmd_file_page_setup		(GtkAction   *action,
							 GeditWindow *window);
#endif


G_END_DECLS

#endif /* __GEDIT_COMMANDS_H__ */ 
