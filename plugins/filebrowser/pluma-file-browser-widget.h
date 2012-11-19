/*
 * pluma-file-browser-widget.h - Pluma plugin providing easy file access 
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

#ifndef __PLUMA_FILE_BROWSER_WIDGET_H__
#define __PLUMA_FILE_BROWSER_WIDGET_H__

#include <gtk/gtk.h>
#include "pluma-file-browser-store.h"
#include "pluma-file-bookmarks-store.h"
#include "pluma-file-browser-view.h"

G_BEGIN_DECLS
#define PLUMA_TYPE_FILE_BROWSER_WIDGET			(pluma_file_browser_widget_get_type ())
#define PLUMA_FILE_BROWSER_WIDGET(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_FILE_BROWSER_WIDGET, PlumaFileBrowserWidget))
#define PLUMA_FILE_BROWSER_WIDGET_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_FILE_BROWSER_WIDGET, PlumaFileBrowserWidget const))
#define PLUMA_FILE_BROWSER_WIDGET_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), PLUMA_TYPE_FILE_BROWSER_WIDGET, PlumaFileBrowserWidgetClass))
#define PLUMA_IS_FILE_BROWSER_WIDGET(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUMA_TYPE_FILE_BROWSER_WIDGET))
#define PLUMA_IS_FILE_BROWSER_WIDGET_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_FILE_BROWSER_WIDGET))
#define PLUMA_FILE_BROWSER_WIDGET_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), PLUMA_TYPE_FILE_BROWSER_WIDGET, PlumaFileBrowserWidgetClass))

typedef struct _PlumaFileBrowserWidget        PlumaFileBrowserWidget;
typedef struct _PlumaFileBrowserWidgetClass   PlumaFileBrowserWidgetClass;
typedef struct _PlumaFileBrowserWidgetPrivate PlumaFileBrowserWidgetPrivate;

typedef
gboolean (*PlumaFileBrowserWidgetFilterFunc) (PlumaFileBrowserWidget * obj,
					      PlumaFileBrowserStore *
					      model, GtkTreeIter * iter,
					      gpointer user_data);

struct _PlumaFileBrowserWidget 
{
	GtkVBox parent;

	PlumaFileBrowserWidgetPrivate *priv;
};

struct _PlumaFileBrowserWidgetClass 
{
	GtkVBoxClass parent_class;

	/* Signals */
	void (*uri_activated)        (PlumaFileBrowserWidget * widget,
			              gchar const *uri);
	void (*error)                (PlumaFileBrowserWidget * widget, 
	                              guint code,
		                      gchar const *message);
	gboolean (*confirm_delete)   (PlumaFileBrowserWidget * widget,
	                              PlumaFileBrowserStore * model,
	                              GList *list);
	gboolean (*confirm_no_trash) (PlumaFileBrowserWidget * widget,
	                              GList *list);
};

GType pluma_file_browser_widget_get_type            (void) G_GNUC_CONST;
GType pluma_file_browser_widget_register_type       (GTypeModule * module);

GtkWidget *pluma_file_browser_widget_new            (const gchar *data_dir);

void pluma_file_browser_widget_show_bookmarks       (PlumaFileBrowserWidget * obj);
void pluma_file_browser_widget_show_files           (PlumaFileBrowserWidget * obj);

void pluma_file_browser_widget_set_root             (PlumaFileBrowserWidget * obj,
                                                     gchar const *root,
                                                     gboolean virtual_root);
void
pluma_file_browser_widget_set_root_and_virtual_root (PlumaFileBrowserWidget * obj,
						     gchar const *root,
						     gchar const *virtual_root);

gboolean
pluma_file_browser_widget_get_selected_directory    (PlumaFileBrowserWidget * obj, 
                                                     GtkTreeIter * iter);

PlumaFileBrowserStore * 
pluma_file_browser_widget_get_browser_store         (PlumaFileBrowserWidget * obj);
PlumaFileBookmarksStore * 
pluma_file_browser_widget_get_bookmarks_store       (PlumaFileBrowserWidget * obj);
PlumaFileBrowserView *
pluma_file_browser_widget_get_browser_view          (PlumaFileBrowserWidget * obj);
GtkWidget *
pluma_file_browser_widget_get_filter_entry          (PlumaFileBrowserWidget * obj);

GtkUIManager * 
pluma_file_browser_widget_get_ui_manager            (PlumaFileBrowserWidget * obj);

gulong pluma_file_browser_widget_add_filter         (PlumaFileBrowserWidget * obj,
                                                     PlumaFileBrowserWidgetFilterFunc func, 
                                                     gpointer user_data,
                                                     GDestroyNotify notify);
void pluma_file_browser_widget_remove_filter        (PlumaFileBrowserWidget * obj,
                                                     gulong id);
void pluma_file_browser_widget_set_filter_pattern   (PlumaFileBrowserWidget * obj,
                                                     gchar const *pattern);

void pluma_file_browser_widget_refresh		    (PlumaFileBrowserWidget * obj);
void pluma_file_browser_widget_history_back	    (PlumaFileBrowserWidget * obj);
void pluma_file_browser_widget_history_forward	    (PlumaFileBrowserWidget * obj);

G_END_DECLS
#endif /* __PLUMA_FILE_BROWSER_WIDGET_H__ */

// ex:ts=8:noet:
