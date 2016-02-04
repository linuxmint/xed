/*
 * xed-prefs-manager-app.h
 * This file is part of xed
 *
 * Copyright (C) 2002-2005  Paolo Maggi 
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
 * Modified by the xed Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the xed Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 *
 */

#ifndef __XED_PREFS_MANAGER_APP_H__
#define __XED_PREFS_MANAGER_APP_H__

#include <glib.h>
#include <xed/xed-prefs-manager.h>

/** LIFE CYCLE MANAGEMENT FUNCTIONS **/

gboolean	 xed_prefs_manager_app_init			(void);

/* This function must be called before exiting xed */
void		 xed_prefs_manager_app_shutdown		(void);


/* Window state */
gint		 xed_prefs_manager_get_window_state		(void);
void 		 xed_prefs_manager_set_window_state		(gint ws);
gboolean	 xed_prefs_manager_window_state_can_set	(void);

/* Window size */
void		 xed_prefs_manager_get_window_size		(gint *width,
								 gint *height);
void		 xed_prefs_manager_get_default_window_size	(gint *width,
								 gint *height);
void 		 xed_prefs_manager_set_window_size		(gint width,
								 gint height);
gboolean	 xed_prefs_manager_window_size_can_set	(void);

/* Side panel */
gint	 	 xed_prefs_manager_get_side_panel_size	(void);
gint	 	 xed_prefs_manager_get_default_side_panel_size(void);
void 		 xed_prefs_manager_set_side_panel_size	(gint ps);
gboolean	 xed_prefs_manager_side_panel_size_can_set	(void);
gint		 xed_prefs_manager_get_side_panel_active_page (void);
void 		 xed_prefs_manager_set_side_panel_active_page (gint id);
gboolean	 xed_prefs_manager_side_panel_active_page_can_set (void);

/* Bottom panel */
gint	 	 xed_prefs_manager_get_bottom_panel_size	(void);
gint	 	 xed_prefs_manager_get_default_bottom_panel_size(void);
void 		 xed_prefs_manager_set_bottom_panel_size	(gint ps);
gboolean	 xed_prefs_manager_bottom_panel_size_can_set	(void);
gint		 xed_prefs_manager_get_bottom_panel_active_page (void);
void 		 xed_prefs_manager_set_bottom_panel_active_page (gint id);
gboolean	 xed_prefs_manager_bottom_panel_active_page_can_set (void);

/* File filter */
gint		 xed_prefs_manager_get_active_file_filter	(void);
void 		 xed_prefs_manager_set_active_file_filter	(gint id);
gboolean	 xed_prefs_manager_active_file_filter_can_set	(void);


#endif /* __XED_PREFS_MANAGER_APP_H__ */
