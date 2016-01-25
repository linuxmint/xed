/*
 * xedit-utils.h
 * This file is part of xedit
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi
 * Copyright (C) 2002 - 2005 Paolo Maggi
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
 * Modified by the xedit Team, 1998-2005. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __XEDIT_UTILS_H__
#define __XEDIT_UTILS_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <atk/atk.h>
#include <xedit/xedit-encodings.h>

G_BEGIN_DECLS

/* useful macro */
#define GBOOLEAN_TO_POINTER(i) (GINT_TO_POINTER ((i) ? 2 : 1))
#define GPOINTER_TO_BOOLEAN(i) ((gboolean) ((GPOINTER_TO_INT(i) == 2) ? TRUE : FALSE))

#define IS_VALID_BOOLEAN(v) (((v == TRUE) || (v == FALSE)) ? TRUE : FALSE)

enum { XEDIT_ALL_WORKSPACES = 0xffffffff };

gboolean	 xedit_utils_uri_has_writable_scheme	(const gchar *uri);
gboolean	 xedit_utils_uri_has_file_scheme	(const gchar *uri);

void		 xedit_utils_menu_position_under_widget (GtkMenu  *menu,
							 gint     *x,
							 gint     *y,
							 gboolean *push_in,
							 gpointer  user_data);

void		 xedit_utils_menu_position_under_tree_view
							(GtkMenu  *menu,
							 gint     *x,
							 gint     *y,
							 gboolean *push_in,
							 gpointer  user_data);

gchar		*xedit_gdk_color_to_string		(GdkColor color);

GtkWidget	*xedit_gtk_button_new_with_stock_icon	(const gchar *label,
							 const gchar *stock_id);

GtkWidget	*xedit_dialog_add_button		(GtkDialog   *dialog,
							 const gchar *text,
							 const gchar *stock_id, 
							 gint         response_id);

gchar		*xedit_utils_escape_underscores		(const gchar *text,
							 gssize       length);

gchar		*xedit_utils_str_middle_truncate	(const gchar *string, 
							 guint        truncate_length);

gchar		*xedit_utils_str_end_truncate		(const gchar *string,
							 guint        truncate_length);

gboolean	 g_utf8_caselessnmatch			(const char *s1,
							 const char *s2,
							 gssize n1,
							 gssize n2);

void		 xedit_utils_set_atk_name_description	(GtkWidget  *widget,
							 const gchar *name,
							 const gchar *description);

void		 xedit_utils_set_atk_relation		(GtkWidget       *obj1,
							 GtkWidget       *obj2,
							 AtkRelationType  rel_type);

gboolean	 xedit_utils_uri_exists			(const gchar* text_uri);

gchar		*xedit_utils_escape_search_text		(const gchar *text);

gchar		*xedit_utils_unescape_search_text	(const gchar *text);

void		 xedit_warning				(GtkWindow  *parent,
							 const gchar *format,
							 ...) G_GNUC_PRINTF(2, 3);

gchar		*xedit_utils_make_valid_utf8		(const char *name);

/* Note that this function replace home dir with ~ */
gchar		*xedit_utils_uri_get_dirname		(const char *uri);

gchar		*xedit_utils_location_get_dirname_for_display
							(GFile *location);

gchar		*xedit_utils_replace_home_dir_with_tilde (const gchar *uri);

guint		 xedit_utils_get_current_workspace	(GdkScreen *screen);

guint		 xedit_utils_get_window_workspace	(GtkWindow *gtkwindow);

void		 xedit_utils_get_current_viewport	(GdkScreen    *screen,
							 gint         *x,
							 gint         *y);

gboolean	 xedit_utils_is_valid_uri		(const gchar *uri);

gboolean	 xedit_utils_get_ui_objects		(const gchar  *filename,
                                                         gchar       **root_objects,
							 GtkWidget   **error_widget,
							 const gchar  *object_name,
							 ...) G_GNUC_NULL_TERMINATED;

gboolean         xedit_utils_file_has_parent            (GFile *gfile);

/* Return NULL if str is not a valid URI and/or filename */
gchar		*xedit_utils_make_canonical_uri_from_shell_arg
							(const gchar *str);
		
gchar		*xedit_utils_uri_for_display 	        (const gchar *uri);
gchar           *xedit_utils_basename_for_display	(const gchar *uri);
gboolean	 xedit_utils_decode_uri 		(const gchar *uri,
							 gchar **scheme,
							 gchar **user,
							 gchar **port,
							 gchar **host,
							 gchar **path);


/* Turns data from a drop into a list of well formatted uris */
gchar 	       **xedit_utils_drop_get_uris		(GtkSelectionData *selection_data);

G_END_DECLS

#endif /* __XEDIT_UTILS_H__ */


