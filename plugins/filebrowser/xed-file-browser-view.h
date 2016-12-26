/*
 * xed-file-browser-view.h - Xed plugin providing easy file access
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

#ifndef __XED_FILE_BROWSER_VIEW_H__
#define __XED_FILE_BROWSER_VIEW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS
#define XED_TYPE_FILE_BROWSER_VIEW			(xed_file_browser_view_get_type ())
#define XED_FILE_BROWSER_VIEW(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_FILE_BROWSER_VIEW, XedFileBrowserView))
#define XED_FILE_BROWSER_VIEW_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_FILE_BROWSER_VIEW, XedFileBrowserView const))
#define XED_FILE_BROWSER_VIEW_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_FILE_BROWSER_VIEW, XedFileBrowserViewClass))
#define XED_IS_FILE_BROWSER_VIEW(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_FILE_BROWSER_VIEW))
#define XED_IS_FILE_BROWSER_VIEW_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_FILE_BROWSER_VIEW))
#define XED_FILE_BROWSER_VIEW_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), XED_TYPE_FILE_BROWSER_VIEW, XedFileBrowserViewClass))

typedef struct _XedFileBrowserView        XedFileBrowserView;
typedef struct _XedFileBrowserViewClass   XedFileBrowserViewClass;
typedef struct _XedFileBrowserViewPrivate XedFileBrowserViewPrivate;

typedef enum {
	XED_FILE_BROWSER_VIEW_CLICK_POLICY_DOUBLE,
	XED_FILE_BROWSER_VIEW_CLICK_POLICY_SINGLE
} XedFileBrowserViewClickPolicy;

struct _XedFileBrowserView
{
	GtkTreeView parent;

	XedFileBrowserViewPrivate *priv;
};

struct _XedFileBrowserViewClass
{
	GtkTreeViewClass parent_class;

	/* Signals */
	void (*error) (XedFileBrowserView * filetree,
	               guint code,
		       gchar const *message);
	void (*file_activated) (XedFileBrowserView * filetree,
				    GtkTreeIter *iter);
	void (*directory_activated) (XedFileBrowserView * filetree,
				    GtkTreeIter *iter);
	void (*bookmark_activated) (XedFileBrowserView * filetree,
				    GtkTreeIter *iter);
};

GType xed_file_browser_view_get_type			(void) G_GNUC_CONST;
void _xed_file_browser_view_register_type		(GTypeModule 			* module);

GtkWidget *xed_file_browser_view_new			(void);
void xed_file_browser_view_set_model			(XedFileBrowserView 		* tree_view,
							 GtkTreeModel 			* model);
void xed_file_browser_view_start_rename		(XedFileBrowserView 		* tree_view,
							 GtkTreeIter 			* iter);
void xed_file_browser_view_set_click_policy		(XedFileBrowserView 		* tree_view,
							 XedFileBrowserViewClickPolicy  policy);
void xed_file_browser_view_set_restore_expand_state	(XedFileBrowserView 		* tree_view,
							 gboolean 			  restore_expand_state);

G_END_DECLS
#endif				/* __XED_FILE_BROWSER_VIEW_H__ */

// ex:ts=8:noet:
