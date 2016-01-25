/*
 * xedit-file-browser-store.h - Xedit plugin providing easy file access 
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

#ifndef __XEDIT_FILE_BROWSER_STORE_H__
#define __XEDIT_FILE_BROWSER_STORE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS
#define XEDIT_TYPE_FILE_BROWSER_STORE			(xedit_file_browser_store_get_type ())
#define XEDIT_FILE_BROWSER_STORE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_FILE_BROWSER_STORE, XeditFileBrowserStore))
#define XEDIT_FILE_BROWSER_STORE_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_FILE_BROWSER_STORE, XeditFileBrowserStore const))
#define XEDIT_FILE_BROWSER_STORE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_FILE_BROWSER_STORE, XeditFileBrowserStoreClass))
#define XEDIT_IS_FILE_BROWSER_STORE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XEDIT_TYPE_FILE_BROWSER_STORE))
#define XEDIT_IS_FILE_BROWSER_STORE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_FILE_BROWSER_STORE))
#define XEDIT_FILE_BROWSER_STORE_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), XEDIT_TYPE_FILE_BROWSER_STORE, XeditFileBrowserStoreClass))

typedef enum 
{
	XEDIT_FILE_BROWSER_STORE_COLUMN_ICON = 0,
	XEDIT_FILE_BROWSER_STORE_COLUMN_NAME,
	XEDIT_FILE_BROWSER_STORE_COLUMN_URI,
	XEDIT_FILE_BROWSER_STORE_COLUMN_FLAGS,
	XEDIT_FILE_BROWSER_STORE_COLUMN_EMBLEM,
	XEDIT_FILE_BROWSER_STORE_COLUMN_NUM
} XeditFileBrowserStoreColumn;

typedef enum 
{
	XEDIT_FILE_BROWSER_STORE_FLAG_IS_DIRECTORY = 1 << 0,
	XEDIT_FILE_BROWSER_STORE_FLAG_IS_HIDDEN    = 1 << 1,
	XEDIT_FILE_BROWSER_STORE_FLAG_IS_TEXT      = 1 << 2,
	XEDIT_FILE_BROWSER_STORE_FLAG_LOADED       = 1 << 3,
	XEDIT_FILE_BROWSER_STORE_FLAG_IS_FILTERED  = 1 << 4,
	XEDIT_FILE_BROWSER_STORE_FLAG_IS_DUMMY     = 1 << 5
} XeditFileBrowserStoreFlag;

typedef enum 
{
	XEDIT_FILE_BROWSER_STORE_RESULT_OK,
	XEDIT_FILE_BROWSER_STORE_RESULT_NO_CHANGE,
	XEDIT_FILE_BROWSER_STORE_RESULT_ERROR,
	XEDIT_FILE_BROWSER_STORE_RESULT_NO_TRASH,
	XEDIT_FILE_BROWSER_STORE_RESULT_MOUNTING,
	XEDIT_FILE_BROWSER_STORE_RESULT_NUM
} XeditFileBrowserStoreResult;

typedef enum 
{
	XEDIT_FILE_BROWSER_STORE_FILTER_MODE_NONE        = 0,
	XEDIT_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN = 1 << 0,
	XEDIT_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY = 1 << 1
} XeditFileBrowserStoreFilterMode;

#define FILE_IS_DIR(flags)	(flags & XEDIT_FILE_BROWSER_STORE_FLAG_IS_DIRECTORY)
#define FILE_IS_HIDDEN(flags)	(flags & XEDIT_FILE_BROWSER_STORE_FLAG_IS_HIDDEN)
#define FILE_IS_TEXT(flags)	(flags & XEDIT_FILE_BROWSER_STORE_FLAG_IS_TEXT)
#define FILE_LOADED(flags)	(flags & XEDIT_FILE_BROWSER_STORE_FLAG_LOADED)
#define FILE_IS_FILTERED(flags)	(flags & XEDIT_FILE_BROWSER_STORE_FLAG_IS_FILTERED)
#define FILE_IS_DUMMY(flags)	(flags & XEDIT_FILE_BROWSER_STORE_FLAG_IS_DUMMY)

typedef struct _XeditFileBrowserStore        XeditFileBrowserStore;
typedef struct _XeditFileBrowserStoreClass   XeditFileBrowserStoreClass;
typedef struct _XeditFileBrowserStorePrivate XeditFileBrowserStorePrivate;

typedef gboolean (*XeditFileBrowserStoreFilterFunc) (XeditFileBrowserStore
						     * model,
						     GtkTreeIter * iter,
						     gpointer user_data);

struct _XeditFileBrowserStore 
{
	GObject parent;

	XeditFileBrowserStorePrivate *priv;
};

struct _XeditFileBrowserStoreClass {
	GObjectClass parent_class;

	/* Signals */
	void (*begin_loading)        (XeditFileBrowserStore * model,
			              GtkTreeIter * iter);
	void (*end_loading)          (XeditFileBrowserStore * model,
			              GtkTreeIter * iter);
	void (*error)                (XeditFileBrowserStore * model, 
	                              guint code,
		                      gchar * message);
	gboolean (*no_trash)	     (XeditFileBrowserStore * model,
				      GList * files);
	void (*rename)		     (XeditFileBrowserStore * model,
				      const gchar * olduri,
				      const gchar * newuri);
	void (*begin_refresh)	     (XeditFileBrowserStore * model);
	void (*end_refresh)	     (XeditFileBrowserStore * model);
	void (*unload)		     (XeditFileBrowserStore * model,
				      const gchar * uri);
};

GType xedit_file_browser_store_get_type               (void) G_GNUC_CONST;
GType xedit_file_browser_store_register_type          (GTypeModule * module);

XeditFileBrowserStore *xedit_file_browser_store_new   (gchar const *root);

XeditFileBrowserStoreResult
xedit_file_browser_store_set_root_and_virtual_root    (XeditFileBrowserStore * model,
						       gchar const *root,
			  			       gchar const *virtual_root);
XeditFileBrowserStoreResult
xedit_file_browser_store_set_root                     (XeditFileBrowserStore * model,
				                       gchar const *root);
XeditFileBrowserStoreResult
xedit_file_browser_store_set_virtual_root             (XeditFileBrowserStore * model,
					               GtkTreeIter * iter);
XeditFileBrowserStoreResult
xedit_file_browser_store_set_virtual_root_from_string (XeditFileBrowserStore * model, 
                                                       gchar const *root);
XeditFileBrowserStoreResult
xedit_file_browser_store_set_virtual_root_up          (XeditFileBrowserStore * model);
XeditFileBrowserStoreResult
xedit_file_browser_store_set_virtual_root_top         (XeditFileBrowserStore * model);

gboolean
xedit_file_browser_store_get_iter_virtual_root        (XeditFileBrowserStore * model, 
                                                       GtkTreeIter * iter);
gboolean xedit_file_browser_store_get_iter_root       (XeditFileBrowserStore * model,
						       GtkTreeIter * iter);
gchar * xedit_file_browser_store_get_root             (XeditFileBrowserStore * model);
gchar * xedit_file_browser_store_get_virtual_root     (XeditFileBrowserStore * model);

gboolean xedit_file_browser_store_iter_equal          (XeditFileBrowserStore * model, 
                                                       GtkTreeIter * iter1,
					               GtkTreeIter * iter2);

void xedit_file_browser_store_set_value               (XeditFileBrowserStore * tree_model, 
                                                       GtkTreeIter * iter,
                                                       gint column, 
                                                       GValue * value);

void _xedit_file_browser_store_iter_expanded          (XeditFileBrowserStore * model, 
                                                       GtkTreeIter * iter);
void _xedit_file_browser_store_iter_collapsed         (XeditFileBrowserStore * model, 
                                                       GtkTreeIter * iter);

XeditFileBrowserStoreFilterMode
xedit_file_browser_store_get_filter_mode              (XeditFileBrowserStore * model);
void xedit_file_browser_store_set_filter_mode         (XeditFileBrowserStore * model,
                                                       XeditFileBrowserStoreFilterMode mode);
void xedit_file_browser_store_set_filter_func         (XeditFileBrowserStore * model,
                                                       XeditFileBrowserStoreFilterFunc func, 
                                                       gpointer user_data);
void xedit_file_browser_store_refilter                (XeditFileBrowserStore * model);
XeditFileBrowserStoreFilterMode
xedit_file_browser_store_filter_mode_get_default      (void);

void xedit_file_browser_store_refresh                 (XeditFileBrowserStore * model);
gboolean xedit_file_browser_store_rename              (XeditFileBrowserStore * model,
                                                       GtkTreeIter * iter,
                                                       gchar const *new_name,
                                                       GError ** error);
XeditFileBrowserStoreResult
xedit_file_browser_store_delete                       (XeditFileBrowserStore * model,
                                                       GtkTreeIter * iter, 
                                                       gboolean trash);
XeditFileBrowserStoreResult
xedit_file_browser_store_delete_all                   (XeditFileBrowserStore * model,
                                                       GList *rows, 
                                                       gboolean trash);

gboolean xedit_file_browser_store_new_file            (XeditFileBrowserStore * model,
                                                       GtkTreeIter * parent,
                                                       GtkTreeIter * iter);
gboolean xedit_file_browser_store_new_directory       (XeditFileBrowserStore * model,
                                                       GtkTreeIter * parent,
                                                       GtkTreeIter * iter);

void xedit_file_browser_store_cancel_mount_operation  (XeditFileBrowserStore *store);

G_END_DECLS
#endif				/* __XEDIT_FILE_BROWSER_STORE_H__ */

// ex:ts=8:noet:
