/*
 * xed-utils.h
 * This file is part of xed
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
 * Modified by the xed Team, 1998-2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __XED_UTILS_H__
#define __XED_UTILS_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <atk/atk.h>
#include <gtksourceview/gtksource.h>

G_BEGIN_DECLS

/* useful macro */
#define GBOOLEAN_TO_POINTER(i) (GINT_TO_POINTER ((i) ? 2 : 1))
#define GPOINTER_TO_BOOLEAN(i) ((gboolean) ((GPOINTER_TO_INT(i) == 2) ? TRUE : FALSE))

#define IS_VALID_BOOLEAN(v) (((v == TRUE) || (v == FALSE)) ? TRUE : FALSE)

enum { XED_ALL_WORKSPACES = 0xffffffff };

gchar *xed_gdk_color_to_string (GdkColor color);

gint xed_string_to_clamped_gint (const gchar *text);

gchar *xed_utils_escape_underscores (const gchar *text,
                                     gssize       length);

gchar *xed_utils_str_middle_truncate (const gchar *string,
                                      guint        truncate_length);

gchar *xed_utils_str_end_truncate (const gchar *string,
                                   guint        truncate_length);

gboolean g_utf8_caselessnmatch (const char *s1,
                                const char *s2,
                                gssize      n1,
                                gssize      n2);

void xed_utils_set_atk_name_description (GtkWidget   *widget,
                                         const gchar *name,
                                         const gchar *description);

void xed_utils_set_atk_relation (GtkWidget       *obj1,
                                 GtkWidget       *obj2,
                                 AtkRelationType  rel_type);

void xed_warning (GtkWindow   *parent,
                  const gchar *format,
                  ...) G_GNUC_PRINTF(2, 3);

gchar *xed_utils_make_valid_utf8 (const char *name);

/* Note that this function replace home dir with ~ */
gchar *xed_utils_uri_get_dirname (const char *uri);

gchar *xed_utils_location_get_dirname_for_display (GFile *location);

gchar *xed_utils_replace_home_dir_with_tilde (const gchar *uri);

guint xed_utils_get_current_workspace (GdkScreen *screen);

guint xed_utils_get_window_workspace (GtkWindow *gtkwindow);

void xed_utils_get_current_viewport (GdkScreen    *screen,
                                     gint         *x,
                                     gint         *y);

gboolean xed_utils_is_valid_location (GFile *location);

gboolean xed_utils_get_ui_objects (const gchar  *filename,
                                   gchar       **root_objects,
                                   GtkWidget   **error_widget,
                                   const gchar  *object_name,
                                   ...) G_GNUC_NULL_TERMINATED;

gboolean xed_utils_file_has_parent (GFile *gfile);

/* Return NULL if str is not a valid URI and/or filename */
gchar *xed_utils_make_canonical_uri_from_shell_arg (const gchar *str);

gchar *xed_utils_basename_for_display (GFile *location);

gboolean xed_utils_decode_uri (const gchar  *uri,
                               gchar       **scheme,
                               gchar       **user,
                               gchar       **port,
                               gchar       **host,
                               gchar       **path);


/* Turns data from a drop into a list of well formatted uris */
gchar **xed_utils_drop_get_uris (GtkSelectionData *selection_data);

/* Private */
GSList *_xed_utils_encoding_strv_to_list (const gchar * const *enc_str);

gchar **_xed_utils_encoding_list_to_strv (const GSList *enc_list);

G_END_DECLS

#endif /* __XED_UTILS_H__ */


