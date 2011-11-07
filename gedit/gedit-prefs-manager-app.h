/*
 * gedit-prefs-manager-app.h
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */
 
/*
 * Modified by the gedit Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 *
 */

#ifndef __GEDIT_PREFS_MANAGER_APP_H__
#define __GEDIT_PREFS_MANAGER_APP_H__

#include <glib.h>
#include <gedit/gedit-prefs-manager.h>

/** LIFE CYCLE MANAGEMENT FUNCTIONS **/

gboolean	 gedit_prefs_manager_app_init			(void);

/* This function must be called before exiting gedit */
void		 gedit_prefs_manager_app_shutdown		(void);


/* Window state */
gint		 gedit_prefs_manager_get_window_state		(void);
void 		 gedit_prefs_manager_set_window_state		(gint ws);
gboolean	 gedit_prefs_manager_window_state_can_set	(void);

/* Window size */
void		 gedit_prefs_manager_get_window_size		(gint *width,
								 gint *height);
void		 gedit_prefs_manager_get_default_window_size	(gint *width,
								 gint *height);
void 		 gedit_prefs_manager_set_window_size		(gint width,
								 gint height);
gboolean	 gedit_prefs_manager_window_size_can_set	(void);

/* Side panel */
gint	 	 gedit_prefs_manager_get_side_panel_size	(void);
gint	 	 gedit_prefs_manager_get_default_side_panel_size(void);
void 		 gedit_prefs_manager_set_side_panel_size	(gint ps);
gboolean	 gedit_prefs_manager_side_panel_size_can_set	(void);
gint		 gedit_prefs_manager_get_side_panel_active_page (void);
void 		 gedit_prefs_manager_set_side_panel_active_page (gint id);
gboolean	 gedit_prefs_manager_side_panel_active_page_can_set (void);

/* Bottom panel */
gint	 	 gedit_prefs_manager_get_bottom_panel_size	(void);
gint	 	 gedit_prefs_manager_get_default_bottom_panel_size(void);
void 		 gedit_prefs_manager_set_bottom_panel_size	(gint ps);
gboolean	 gedit_prefs_manager_bottom_panel_size_can_set	(void);
gint		 gedit_prefs_manager_get_bottom_panel_active_page (void);
void 		 gedit_prefs_manager_set_bottom_panel_active_page (gint id);
gboolean	 gedit_prefs_manager_bottom_panel_active_page_can_set (void);

/* File filter */
gint		 gedit_prefs_manager_get_active_file_filter	(void);
void 		 gedit_prefs_manager_set_active_file_filter	(gint id);
gboolean	 gedit_prefs_manager_active_file_filter_can_set	(void);


#endif /* __GEDIT_PREFS_MANAGER_APP_H__ */
