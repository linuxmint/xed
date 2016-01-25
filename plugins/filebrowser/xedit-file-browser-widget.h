/*
 * xedit-file-browser-widget.h - Xedit plugin providing easy file access 
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

#ifndef __XEDIT_FILE_BROWSER_WIDGET_H__
#define __XEDIT_FILE_BROWSER_WIDGET_H__

#include <gtk/gtk.h>
#include "xedit-file-browser-store.h"
#include "xedit-file-bookmarks-store.h"
#include "xedit-file-browser-view.h"

G_BEGIN_DECLS
#define XEDIT_TYPE_FILE_BROWSER_WIDGET			(xedit_file_browser_widget_get_type ())
#define XEDIT_FILE_BROWSER_WIDGET(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_FILE_BROWSER_WIDGET, XeditFileBrowserWidget))
#define XEDIT_FILE_BROWSER_WIDGET_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_FILE_BROWSER_WIDGET, XeditFileBrowserWidget const))
#define XEDIT_FILE_BROWSER_WIDGET_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_FILE_BROWSER_WIDGET, XeditFileBrowserWidgetClass))
#define XEDIT_IS_FILE_BROWSER_WIDGET(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XEDIT_TYPE_FILE_BROWSER_WIDGET))
#define XEDIT_IS_FILE_BROWSER_WIDGET_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_FILE_BROWSER_WIDGET))
#define XEDIT_FILE_BROWSER_WIDGET_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), XEDIT_TYPE_FILE_BROWSER_WIDGET, XeditFileBrowserWidgetClass))

typedef struct _XeditFileBrowserWidget        XeditFileBrowserWidget;
typedef struct _XeditFileBrowserWidgetClass   XeditFileBrowserWidgetClass;
typedef struct _XeditFileBrowserWidgetPrivate XeditFileBrowserWidgetPrivate;

typedef
gboolean (*XeditFileBrowserWidgetFilterFunc) (XeditFileBrowserWidget * obj,
					      XeditFileBrowserStore *
					      model, GtkTreeIter * iter,
					      gpointer user_data);

struct _XeditFileBrowserWidget 
{
#if GTK_CHECK_VERSION (3, 0, 0)
	GtkBox parent;
#else
	GtkVBox parent;
#endif

	XeditFileBrowserWidgetPrivate *priv;
};

struct _XeditFileBrowserWidgetClass 
{
#if GTK_CHECK_VERSION (3, 0, 0)
	GtkBoxClass parent_class;
#else
	GtkVBoxClass parent_class;
#endif

	/* Signals */
	void (*uri_activated)        (XeditFileBrowserWidget * widget,
			              gchar const *uri);
	void (*error)                (XeditFileBrowserWidget * widget, 
	                              guint code,
		                      gchar const *message);
	gboolean (*confirm_delete)   (XeditFileBrowserWidget * widget,
	                              XeditFileBrowserStore * model,
	                              GList *list);
	gboolean (*confirm_no_trash) (XeditFileBrowserWidget * widget,
	                              GList *list);
};

GType xedit_file_browser_widget_get_type            (void) G_GNUC_CONST;
GType xedit_file_browser_widget_register_type       (GTypeModule * module);

GtkWidget *xedit_file_browser_widget_new            (const gchar *data_dir);

void xedit_file_browser_widget_show_bookmarks       (XeditFileBrowserWidget * obj);
void xedit_file_browser_widget_show_files           (XeditFileBrowserWidget * obj);

void xedit_file_browser_widget_set_root             (XeditFileBrowserWidget * obj,
                                                     gchar const *root,
                                                     gboolean virtual_root);
void
xedit_file_browser_widget_set_root_and_virtual_root (XeditFileBrowserWidget * obj,
						     gchar const *root,
						     gchar const *virtual_root);

gboolean
xedit_file_browser_widget_get_selected_directory    (XeditFileBrowserWidget * obj, 
                                                     GtkTreeIter * iter);

XeditFileBrowserStore * 
xedit_file_browser_widget_get_browser_store         (XeditFileBrowserWidget * obj);
XeditFileBookmarksStore * 
xedit_file_browser_widget_get_bookmarks_store       (XeditFileBrowserWidget * obj);
XeditFileBrowserView *
xedit_file_browser_widget_get_browser_view          (XeditFileBrowserWidget * obj);
GtkWidget *
xedit_file_browser_widget_get_filter_entry          (XeditFileBrowserWidget * obj);

GtkUIManager * 
xedit_file_browser_widget_get_ui_manager            (XeditFileBrowserWidget * obj);

gulong xedit_file_browser_widget_add_filter         (XeditFileBrowserWidget * obj,
                                                     XeditFileBrowserWidgetFilterFunc func, 
                                                     gpointer user_data,
                                                     GDestroyNotify notify);
void xedit_file_browser_widget_remove_filter        (XeditFileBrowserWidget * obj,
                                                     gulong id);
void xedit_file_browser_widget_set_filter_pattern   (XeditFileBrowserWidget * obj,
                                                     gchar const *pattern);

void xedit_file_browser_widget_refresh		    (XeditFileBrowserWidget * obj);
void xedit_file_browser_widget_history_back	    (XeditFileBrowserWidget * obj);
void xedit_file_browser_widget_history_forward	    (XeditFileBrowserWidget * obj);

G_END_DECLS
#endif /* __XEDIT_FILE_BROWSER_WIDGET_H__ */

// ex:ts=8:noet:
