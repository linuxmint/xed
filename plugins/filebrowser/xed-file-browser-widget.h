/*
 * xed-file-browser-widget.h - Xed plugin providing easy file access
 * from the sidepanel
 *
 * Copyright (C) 2006 - Jesse van den Kieboom <jesse@icecrew.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __XED_FILE_BROWSER_WIDGET_H__
#define __XED_FILE_BROWSER_WIDGET_H__

#include <gtk/gtk.h>
#include "xed-file-browser-store.h"
#include "xed-file-bookmarks-store.h"
#include "xed-file-browser-view.h"

G_BEGIN_DECLS
#define XED_TYPE_FILE_BROWSER_WIDGET            (xed_file_browser_widget_get_type ())
#define XED_FILE_BROWSER_WIDGET(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_FILE_BROWSER_WIDGET, XedFileBrowserWidget))
#define XED_FILE_BROWSER_WIDGET_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_FILE_BROWSER_WIDGET, XedFileBrowserWidget const))
#define XED_FILE_BROWSER_WIDGET_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_FILE_BROWSER_WIDGET, XedFileBrowserWidgetClass))
#define XED_IS_FILE_BROWSER_WIDGET(obj)     (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_FILE_BROWSER_WIDGET))
#define XED_IS_FILE_BROWSER_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_FILE_BROWSER_WIDGET))
#define XED_FILE_BROWSER_WIDGET_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XED_TYPE_FILE_BROWSER_WIDGET, XedFileBrowserWidgetClass))

typedef struct _XedFileBrowserWidget        XedFileBrowserWidget;
typedef struct _XedFileBrowserWidgetClass   XedFileBrowserWidgetClass;
typedef struct _XedFileBrowserWidgetPrivate XedFileBrowserWidgetPrivate;

typedef
gboolean (*XedFileBrowserWidgetFilterFunc) (XedFileBrowserWidget * obj,
                          XedFileBrowserStore *
                          model, GtkTreeIter * iter,
                          gpointer user_data);

struct _XedFileBrowserWidget
{
    GtkBox parent;

    XedFileBrowserWidgetPrivate *priv;
};

struct _XedFileBrowserWidgetClass
{
    GtkBoxClass parent_class;

    /* Signals */
    void (*uri_activated)        (XedFileBrowserWidget * widget,
                          gchar const *uri);
    void (*error)                (XedFileBrowserWidget * widget,
                                  guint code,
                              gchar const *message);
    gboolean (*confirm_delete)   (XedFileBrowserWidget * widget,
                                  XedFileBrowserStore * model,
                                  GList *list);
    gboolean (*confirm_no_trash) (XedFileBrowserWidget * widget,
                                  GList *list);
};

GType xed_file_browser_widget_get_type            (void) G_GNUC_CONST;
void _xed_file_browser_widget_register_type       (GTypeModule *type_module);

GtkWidget *xed_file_browser_widget_new            (const gchar *data_dir);

void xed_file_browser_widget_show_bookmarks       (XedFileBrowserWidget * obj);
void xed_file_browser_widget_show_files           (XedFileBrowserWidget * obj);

void xed_file_browser_widget_set_root             (XedFileBrowserWidget * obj,
                                                     gchar const *root,
                                                     gboolean virtual_root);
void
xed_file_browser_widget_set_root_and_virtual_root (XedFileBrowserWidget * obj,
                             gchar const *root,
                             gchar const *virtual_root);

gboolean
xed_file_browser_widget_get_selected_directory    (XedFileBrowserWidget * obj,
                                                     GtkTreeIter * iter);

XedFileBrowserStore *
xed_file_browser_widget_get_browser_store         (XedFileBrowserWidget * obj);
XedFileBookmarksStore *
xed_file_browser_widget_get_bookmarks_store       (XedFileBrowserWidget * obj);
XedFileBrowserView *
xed_file_browser_widget_get_browser_view          (XedFileBrowserWidget * obj);
GtkWidget *
xed_file_browser_widget_get_filter_entry          (XedFileBrowserWidget * obj);

GtkUIManager *
xed_file_browser_widget_get_ui_manager            (XedFileBrowserWidget * obj);

gulong xed_file_browser_widget_add_filter         (XedFileBrowserWidget * obj,
                                                     XedFileBrowserWidgetFilterFunc func,
                                                     gpointer user_data,
                                                     GDestroyNotify notify);
void xed_file_browser_widget_remove_filter        (XedFileBrowserWidget * obj,
                                                     gulong id);
void xed_file_browser_widget_set_filter_pattern   (XedFileBrowserWidget * obj,
                                                     gchar const *pattern);

void xed_file_browser_widget_refresh            (XedFileBrowserWidget * obj);
void xed_file_browser_widget_history_back       (XedFileBrowserWidget * obj);
void xed_file_browser_widget_history_forward        (XedFileBrowserWidget * obj);

G_END_DECLS
#endif /* __XED_FILE_BROWSER_WIDGET_H__ */

// ex:ts=8:noet:
