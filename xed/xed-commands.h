/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xed-commands.h
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

#ifndef __XED_COMMANDS_H__
#define __XED_COMMANDS_H__

#include <gtk/gtk.h>
#include <xed/xed-window.h>

G_BEGIN_DECLS

/* Do nothing if URI does not exist */
void		 xed_commands_load_uri		(XedWindow         *window,
							 const gchar         *uri,
							 const XedEncoding *encoding,
							 gint                 line_pos);

/* Ignore non-existing URIs */
gint		 xed_commands_load_uris		(XedWindow         *window,
							 const GSList        *uris,
							 const XedEncoding *encoding,
							 gint                 line_pos);

void		 xed_commands_save_document		(XedWindow         *window,
                                                         XedDocument       *document);

void		 xed_commands_save_all_documents 	(XedWindow         *window);

/*
 * Non-exported functions
 */

/* Create titled documens for non-existing URIs */
gint		_xed_cmd_load_files_from_prompt	(XedWindow         *window,
							 GSList              *files,
							 const XedEncoding *encoding,
							 gint                 line_pos);

void		_xed_cmd_file_new			(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_file_open			(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_file_save			(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_file_save_as			(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_file_save_all		(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_file_revert			(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_file_open_uri		(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_file_print_preview		(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_file_print			(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_file_close			(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_file_close_all		(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_file_quit			(GtkAction   *action,
							 XedWindow *window);

void		_xed_cmd_edit_undo			(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_edit_redo			(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_edit_cut			(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_edit_copy			(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_edit_paste			(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_edit_delete			(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_edit_select_all		(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_edit_preferences		(GtkAction   *action,
							 XedWindow *window);

void		_xed_cmd_view_show_toolbar		(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_view_show_statusbar		(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_view_show_side_pane		(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_view_show_bottom_pane	(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_view_toggle_fullscreen_mode	(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_view_leave_fullscreen_mode	(GtkAction   *action,
							 XedWindow *window);

void		_xed_cmd_search_find			(GtkAction   *action,
							 XedWindow *window);

void		_xed_cmd_search_find_next		(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_search_find_prev		(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_search_replace		(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_search_clear_highlight	(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_search_goto_line		(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_search_incremental_search	(GtkAction   *action,
							 XedWindow *window);							 
							 
void		_xed_cmd_documents_previous_document	(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_documents_next_document	(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_documents_move_to_new_window	(GtkAction   *action,
							 XedWindow *window);

void		_xed_cmd_help_contents		(GtkAction   *action,
							 XedWindow *window);
void		_xed_cmd_help_about			(GtkAction   *action,
							 XedWindow *window);

void		_xed_cmd_file_close_tab 		(XedTab    *tab,
							 XedWindow *window);

void		_xed_cmd_file_save_documents_list	(XedWindow *window,
							 GList       *docs);

G_END_DECLS

#endif /* __XED_COMMANDS_H__ */ 
