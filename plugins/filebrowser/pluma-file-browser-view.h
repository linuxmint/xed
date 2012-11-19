/*
 * pluma-file-browser-view.h - Pluma plugin providing easy file access 
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

#ifndef __PLUMA_FILE_BROWSER_VIEW_H__
#define __PLUMA_FILE_BROWSER_VIEW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS
#define PLUMA_TYPE_FILE_BROWSER_VIEW			(pluma_file_browser_view_get_type ())
#define PLUMA_FILE_BROWSER_VIEW(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_FILE_BROWSER_VIEW, PlumaFileBrowserView))
#define PLUMA_FILE_BROWSER_VIEW_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_FILE_BROWSER_VIEW, PlumaFileBrowserView const))
#define PLUMA_FILE_BROWSER_VIEW_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), PLUMA_TYPE_FILE_BROWSER_VIEW, PlumaFileBrowserViewClass))
#define PLUMA_IS_FILE_BROWSER_VIEW(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUMA_TYPE_FILE_BROWSER_VIEW))
#define PLUMA_IS_FILE_BROWSER_VIEW_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_FILE_BROWSER_VIEW))
#define PLUMA_FILE_BROWSER_VIEW_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), PLUMA_TYPE_FILE_BROWSER_VIEW, PlumaFileBrowserViewClass))

typedef struct _PlumaFileBrowserView        PlumaFileBrowserView;
typedef struct _PlumaFileBrowserViewClass   PlumaFileBrowserViewClass;
typedef struct _PlumaFileBrowserViewPrivate PlumaFileBrowserViewPrivate;

typedef enum {
	PLUMA_FILE_BROWSER_VIEW_CLICK_POLICY_DOUBLE,
	PLUMA_FILE_BROWSER_VIEW_CLICK_POLICY_SINGLE	
} PlumaFileBrowserViewClickPolicy;

struct _PlumaFileBrowserView 
{
	GtkTreeView parent;

	PlumaFileBrowserViewPrivate *priv;
};

struct _PlumaFileBrowserViewClass 
{
	GtkTreeViewClass parent_class;

	/* Signals */
	void (*error) (PlumaFileBrowserView * filetree, 
	               guint code,
		       gchar const *message);
	void (*file_activated) (PlumaFileBrowserView * filetree,
				    GtkTreeIter *iter);
	void (*directory_activated) (PlumaFileBrowserView * filetree,
				    GtkTreeIter *iter);
	void (*bookmark_activated) (PlumaFileBrowserView * filetree,
				    GtkTreeIter *iter);
};

GType pluma_file_browser_view_get_type			(void) G_GNUC_CONST;
GType pluma_file_browser_view_register_type		(GTypeModule 			* module);

GtkWidget *pluma_file_browser_view_new			(void);
void pluma_file_browser_view_set_model			(PlumaFileBrowserView 		* tree_view,
							 GtkTreeModel 			* model);
void pluma_file_browser_view_start_rename		(PlumaFileBrowserView 		* tree_view, 
							 GtkTreeIter 			* iter);
void pluma_file_browser_view_set_click_policy		(PlumaFileBrowserView 		* tree_view,
							 PlumaFileBrowserViewClickPolicy  policy);
void pluma_file_browser_view_set_restore_expand_state	(PlumaFileBrowserView 		* tree_view,
							 gboolean 			  restore_expand_state);

G_END_DECLS
#endif				/* __PLUMA_FILE_BROWSER_VIEW_H__ */

// ex:ts=8:noet:
