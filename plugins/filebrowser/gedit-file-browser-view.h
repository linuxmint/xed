/*
 * gedit-file-browser-view.h - Gedit plugin providing easy file access 
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __GEDIT_FILE_BROWSER_VIEW_H__
#define __GEDIT_FILE_BROWSER_VIEW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS
#define GEDIT_TYPE_FILE_BROWSER_VIEW			(gedit_file_browser_view_get_type ())
#define GEDIT_FILE_BROWSER_VIEW(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_FILE_BROWSER_VIEW, GeditFileBrowserView))
#define GEDIT_FILE_BROWSER_VIEW_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_FILE_BROWSER_VIEW, GeditFileBrowserView const))
#define GEDIT_FILE_BROWSER_VIEW_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_FILE_BROWSER_VIEW, GeditFileBrowserViewClass))
#define GEDIT_IS_FILE_BROWSER_VIEW(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_FILE_BROWSER_VIEW))
#define GEDIT_IS_FILE_BROWSER_VIEW_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_FILE_BROWSER_VIEW))
#define GEDIT_FILE_BROWSER_VIEW_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_TYPE_FILE_BROWSER_VIEW, GeditFileBrowserViewClass))

typedef struct _GeditFileBrowserView        GeditFileBrowserView;
typedef struct _GeditFileBrowserViewClass   GeditFileBrowserViewClass;
typedef struct _GeditFileBrowserViewPrivate GeditFileBrowserViewPrivate;

typedef enum {
	GEDIT_FILE_BROWSER_VIEW_CLICK_POLICY_DOUBLE,
	GEDIT_FILE_BROWSER_VIEW_CLICK_POLICY_SINGLE	
} GeditFileBrowserViewClickPolicy;

struct _GeditFileBrowserView 
{
	GtkTreeView parent;

	GeditFileBrowserViewPrivate *priv;
};

struct _GeditFileBrowserViewClass 
{
	GtkTreeViewClass parent_class;

	/* Signals */
	void (*error) (GeditFileBrowserView * filetree, 
	               guint code,
		       gchar const *message);
	void (*file_activated) (GeditFileBrowserView * filetree,
				    GtkTreeIter *iter);
	void (*directory_activated) (GeditFileBrowserView * filetree,
				    GtkTreeIter *iter);
	void (*bookmark_activated) (GeditFileBrowserView * filetree,
				    GtkTreeIter *iter);
};

GType gedit_file_browser_view_get_type			(void) G_GNUC_CONST;
GType gedit_file_browser_view_register_type		(GTypeModule 			* module);

GtkWidget *gedit_file_browser_view_new			(void);
void gedit_file_browser_view_set_model			(GeditFileBrowserView 		* tree_view,
							 GtkTreeModel 			* model);
void gedit_file_browser_view_start_rename		(GeditFileBrowserView 		* tree_view, 
							 GtkTreeIter 			* iter);
void gedit_file_browser_view_set_click_policy		(GeditFileBrowserView 		* tree_view,
							 GeditFileBrowserViewClickPolicy  policy);
void gedit_file_browser_view_set_restore_expand_state	(GeditFileBrowserView 		* tree_view,
							 gboolean 			  restore_expand_state);

G_END_DECLS
#endif				/* __GEDIT_FILE_BROWSER_VIEW_H__ */

// ex:ts=8:noet:
