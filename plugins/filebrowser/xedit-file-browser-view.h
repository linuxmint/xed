/*
 * xedit-file-browser-view.h - Xedit plugin providing easy file access 
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

#ifndef __XEDIT_FILE_BROWSER_VIEW_H__
#define __XEDIT_FILE_BROWSER_VIEW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS
#define XEDIT_TYPE_FILE_BROWSER_VIEW			(xedit_file_browser_view_get_type ())
#define XEDIT_FILE_BROWSER_VIEW(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_FILE_BROWSER_VIEW, XeditFileBrowserView))
#define XEDIT_FILE_BROWSER_VIEW_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_FILE_BROWSER_VIEW, XeditFileBrowserView const))
#define XEDIT_FILE_BROWSER_VIEW_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_FILE_BROWSER_VIEW, XeditFileBrowserViewClass))
#define XEDIT_IS_FILE_BROWSER_VIEW(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XEDIT_TYPE_FILE_BROWSER_VIEW))
#define XEDIT_IS_FILE_BROWSER_VIEW_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_FILE_BROWSER_VIEW))
#define XEDIT_FILE_BROWSER_VIEW_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), XEDIT_TYPE_FILE_BROWSER_VIEW, XeditFileBrowserViewClass))

typedef struct _XeditFileBrowserView        XeditFileBrowserView;
typedef struct _XeditFileBrowserViewClass   XeditFileBrowserViewClass;
typedef struct _XeditFileBrowserViewPrivate XeditFileBrowserViewPrivate;

typedef enum {
	XEDIT_FILE_BROWSER_VIEW_CLICK_POLICY_DOUBLE,
	XEDIT_FILE_BROWSER_VIEW_CLICK_POLICY_SINGLE	
} XeditFileBrowserViewClickPolicy;

struct _XeditFileBrowserView 
{
	GtkTreeView parent;

	XeditFileBrowserViewPrivate *priv;
};

struct _XeditFileBrowserViewClass 
{
	GtkTreeViewClass parent_class;

	/* Signals */
	void (*error) (XeditFileBrowserView * filetree, 
	               guint code,
		       gchar const *message);
	void (*file_activated) (XeditFileBrowserView * filetree,
				    GtkTreeIter *iter);
	void (*directory_activated) (XeditFileBrowserView * filetree,
				    GtkTreeIter *iter);
	void (*bookmark_activated) (XeditFileBrowserView * filetree,
				    GtkTreeIter *iter);
};

GType xedit_file_browser_view_get_type			(void) G_GNUC_CONST;
GType xedit_file_browser_view_register_type		(GTypeModule 			* module);

GtkWidget *xedit_file_browser_view_new			(void);
void xedit_file_browser_view_set_model			(XeditFileBrowserView 		* tree_view,
							 GtkTreeModel 			* model);
void xedit_file_browser_view_start_rename		(XeditFileBrowserView 		* tree_view, 
							 GtkTreeIter 			* iter);
void xedit_file_browser_view_set_click_policy		(XeditFileBrowserView 		* tree_view,
							 XeditFileBrowserViewClickPolicy  policy);
void xedit_file_browser_view_set_restore_expand_state	(XeditFileBrowserView 		* tree_view,
							 gboolean 			  restore_expand_state);

G_END_DECLS
#endif				/* __XEDIT_FILE_BROWSER_VIEW_H__ */

// ex:ts=8:noet:
