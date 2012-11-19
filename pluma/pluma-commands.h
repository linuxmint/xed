/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pluma-commands.h
 * This file is part of pluma
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
 * Modified by the pluma Team, 1998-2005. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __PLUMA_COMMANDS_H__
#define __PLUMA_COMMANDS_H__

#include <gtk/gtk.h>
#include <pluma/pluma-window.h>

G_BEGIN_DECLS

/* Do nothing if URI does not exist */
void		 pluma_commands_load_uri		(PlumaWindow         *window,
							 const gchar         *uri,
							 const PlumaEncoding *encoding,
							 gint                 line_pos);

/* Ignore non-existing URIs */
gint		 pluma_commands_load_uris		(PlumaWindow         *window,
							 const GSList        *uris,
							 const PlumaEncoding *encoding,
							 gint                 line_pos);

void		 pluma_commands_save_document		(PlumaWindow         *window,
                                                         PlumaDocument       *document);

void		 pluma_commands_save_all_documents 	(PlumaWindow         *window);

/*
 * Non-exported functions
 */

/* Create titled documens for non-existing URIs */
gint		_pluma_cmd_load_files_from_prompt	(PlumaWindow         *window,
							 GSList              *files,
							 const PlumaEncoding *encoding,
							 gint                 line_pos);

void		_pluma_cmd_file_new			(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_file_open			(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_file_save			(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_file_save_as			(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_file_save_all		(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_file_revert			(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_file_open_uri		(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_file_print_preview		(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_file_print			(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_file_close			(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_file_close_all		(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_file_quit			(GtkAction   *action,
							 PlumaWindow *window);

void		_pluma_cmd_edit_undo			(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_edit_redo			(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_edit_cut			(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_edit_copy			(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_edit_paste			(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_edit_delete			(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_edit_select_all		(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_edit_preferences		(GtkAction   *action,
							 PlumaWindow *window);

void		_pluma_cmd_view_show_toolbar		(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_view_show_statusbar		(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_view_show_side_pane		(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_view_show_bottom_pane	(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_view_toggle_fullscreen_mode	(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_view_leave_fullscreen_mode	(GtkAction   *action,
							 PlumaWindow *window);

void		_pluma_cmd_search_find			(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_search_find_next		(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_search_find_prev		(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_search_replace		(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_search_clear_highlight	(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_search_goto_line		(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_search_incremental_search	(GtkAction   *action,
							 PlumaWindow *window);							 
							 
void		_pluma_cmd_documents_previous_document	(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_documents_next_document	(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_documents_move_to_new_window	(GtkAction   *action,
							 PlumaWindow *window);

void		_pluma_cmd_help_contents		(GtkAction   *action,
							 PlumaWindow *window);
void		_pluma_cmd_help_about			(GtkAction   *action,
							 PlumaWindow *window);

void		_pluma_cmd_file_close_tab 		(PlumaTab    *tab,
							 PlumaWindow *window);

void		_pluma_cmd_file_save_documents_list	(PlumaWindow *window,
							 GList       *docs);


#if !GTK_CHECK_VERSION (2, 17, 4)
void		_pluma_cmd_file_page_setup		(GtkAction   *action,
							 PlumaWindow *window);
#endif


G_END_DECLS

#endif /* __PLUMA_COMMANDS_H__ */ 
