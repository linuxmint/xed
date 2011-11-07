/*
 * gedit-file-browser-store.h - Gedit plugin providing easy file access 
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

#ifndef __GEDIT_FILE_BROWSER_STORE_H__
#define __GEDIT_FILE_BROWSER_STORE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS
#define GEDIT_TYPE_FILE_BROWSER_STORE			(gedit_file_browser_store_get_type ())
#define GEDIT_FILE_BROWSER_STORE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_FILE_BROWSER_STORE, GeditFileBrowserStore))
#define GEDIT_FILE_BROWSER_STORE_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_FILE_BROWSER_STORE, GeditFileBrowserStore const))
#define GEDIT_FILE_BROWSER_STORE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_FILE_BROWSER_STORE, GeditFileBrowserStoreClass))
#define GEDIT_IS_FILE_BROWSER_STORE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_FILE_BROWSER_STORE))
#define GEDIT_IS_FILE_BROWSER_STORE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_FILE_BROWSER_STORE))
#define GEDIT_FILE_BROWSER_STORE_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_TYPE_FILE_BROWSER_STORE, GeditFileBrowserStoreClass))

typedef enum 
{
	GEDIT_FILE_BROWSER_STORE_COLUMN_ICON = 0,
	GEDIT_FILE_BROWSER_STORE_COLUMN_NAME,
	GEDIT_FILE_BROWSER_STORE_COLUMN_URI,
	GEDIT_FILE_BROWSER_STORE_COLUMN_FLAGS,
	GEDIT_FILE_BROWSER_STORE_COLUMN_EMBLEM,
	GEDIT_FILE_BROWSER_STORE_COLUMN_NUM
} GeditFileBrowserStoreColumn;

typedef enum 
{
	GEDIT_FILE_BROWSER_STORE_FLAG_IS_DIRECTORY = 1 << 0,
	GEDIT_FILE_BROWSER_STORE_FLAG_IS_HIDDEN    = 1 << 1,
	GEDIT_FILE_BROWSER_STORE_FLAG_IS_TEXT      = 1 << 2,
	GEDIT_FILE_BROWSER_STORE_FLAG_LOADED       = 1 << 3,
	GEDIT_FILE_BROWSER_STORE_FLAG_IS_FILTERED  = 1 << 4,
	GEDIT_FILE_BROWSER_STORE_FLAG_IS_DUMMY     = 1 << 5
} GeditFileBrowserStoreFlag;

typedef enum 
{
	GEDIT_FILE_BROWSER_STORE_RESULT_OK,
	GEDIT_FILE_BROWSER_STORE_RESULT_NO_CHANGE,
	GEDIT_FILE_BROWSER_STORE_RESULT_ERROR,
	GEDIT_FILE_BROWSER_STORE_RESULT_NO_TRASH,
	GEDIT_FILE_BROWSER_STORE_RESULT_MOUNTING,
	GEDIT_FILE_BROWSER_STORE_RESULT_NUM
} GeditFileBrowserStoreResult;

typedef enum 
{
	GEDIT_FILE_BROWSER_STORE_FILTER_MODE_NONE        = 0,
	GEDIT_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN = 1 << 0,
	GEDIT_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY = 1 << 1
} GeditFileBrowserStoreFilterMode;

#define FILE_IS_DIR(flags)	(flags & GEDIT_FILE_BROWSER_STORE_FLAG_IS_DIRECTORY)
#define FILE_IS_HIDDEN(flags)	(flags & GEDIT_FILE_BROWSER_STORE_FLAG_IS_HIDDEN)
#define FILE_IS_TEXT(flags)	(flags & GEDIT_FILE_BROWSER_STORE_FLAG_IS_TEXT)
#define FILE_LOADED(flags)	(flags & GEDIT_FILE_BROWSER_STORE_FLAG_LOADED)
#define FILE_IS_FILTERED(flags)	(flags & GEDIT_FILE_BROWSER_STORE_FLAG_IS_FILTERED)
#define FILE_IS_DUMMY(flags)	(flags & GEDIT_FILE_BROWSER_STORE_FLAG_IS_DUMMY)

typedef struct _GeditFileBrowserStore        GeditFileBrowserStore;
typedef struct _GeditFileBrowserStoreClass   GeditFileBrowserStoreClass;
typedef struct _GeditFileBrowserStorePrivate GeditFileBrowserStorePrivate;

typedef gboolean (*GeditFileBrowserStoreFilterFunc) (GeditFileBrowserStore
						     * model,
						     GtkTreeIter * iter,
						     gpointer user_data);

struct _GeditFileBrowserStore 
{
	GObject parent;

	GeditFileBrowserStorePrivate *priv;
};

struct _GeditFileBrowserStoreClass {
	GObjectClass parent_class;

	/* Signals */
	void (*begin_loading)        (GeditFileBrowserStore * model,
			              GtkTreeIter * iter);
	void (*end_loading)          (GeditFileBrowserStore * model,
			              GtkTreeIter * iter);
	void (*error)                (GeditFileBrowserStore * model, 
	                              guint code,
		                      gchar * message);
	gboolean (*no_trash)	     (GeditFileBrowserStore * model,
				      GList * files);
	void (*rename)		     (GeditFileBrowserStore * model,
				      const gchar * olduri,
				      const gchar * newuri);
	void (*begin_refresh)	     (GeditFileBrowserStore * model);
	void (*end_refresh)	     (GeditFileBrowserStore * model);
	void (*unload)		     (GeditFileBrowserStore * model,
				      const gchar * uri);
};

GType gedit_file_browser_store_get_type               (void) G_GNUC_CONST;
GType gedit_file_browser_store_register_type          (GTypeModule * module);

GeditFileBrowserStore *gedit_file_browser_store_new   (gchar const *root);

GeditFileBrowserStoreResult
gedit_file_browser_store_set_root_and_virtual_root    (GeditFileBrowserStore * model,
						       gchar const *root,
			  			       gchar const *virtual_root);
GeditFileBrowserStoreResult
gedit_file_browser_store_set_root                     (GeditFileBrowserStore * model,
				                       gchar const *root);
GeditFileBrowserStoreResult
gedit_file_browser_store_set_virtual_root             (GeditFileBrowserStore * model,
					               GtkTreeIter * iter);
GeditFileBrowserStoreResult
gedit_file_browser_store_set_virtual_root_from_string (GeditFileBrowserStore * model, 
                                                       gchar const *root);
GeditFileBrowserStoreResult
gedit_file_browser_store_set_virtual_root_up          (GeditFileBrowserStore * model);
GeditFileBrowserStoreResult
gedit_file_browser_store_set_virtual_root_top         (GeditFileBrowserStore * model);

gboolean
gedit_file_browser_store_get_iter_virtual_root        (GeditFileBrowserStore * model, 
                                                       GtkTreeIter * iter);
gboolean gedit_file_browser_store_get_iter_root       (GeditFileBrowserStore * model,
						       GtkTreeIter * iter);
gchar * gedit_file_browser_store_get_root             (GeditFileBrowserStore * model);
gchar * gedit_file_browser_store_get_virtual_root     (GeditFileBrowserStore * model);

gboolean gedit_file_browser_store_iter_equal          (GeditFileBrowserStore * model, 
                                                       GtkTreeIter * iter1,
					               GtkTreeIter * iter2);

void gedit_file_browser_store_set_value               (GeditFileBrowserStore * tree_model, 
                                                       GtkTreeIter * iter,
                                                       gint column, 
                                                       GValue * value);

void _gedit_file_browser_store_iter_expanded          (GeditFileBrowserStore * model, 
                                                       GtkTreeIter * iter);
void _gedit_file_browser_store_iter_collapsed         (GeditFileBrowserStore * model, 
                                                       GtkTreeIter * iter);

GeditFileBrowserStoreFilterMode
gedit_file_browser_store_get_filter_mode              (GeditFileBrowserStore * model);
void gedit_file_browser_store_set_filter_mode         (GeditFileBrowserStore * model,
                                                       GeditFileBrowserStoreFilterMode mode);
void gedit_file_browser_store_set_filter_func         (GeditFileBrowserStore * model,
                                                       GeditFileBrowserStoreFilterFunc func, 
                                                       gpointer user_data);
void gedit_file_browser_store_refilter                (GeditFileBrowserStore * model);
GeditFileBrowserStoreFilterMode
gedit_file_browser_store_filter_mode_get_default      (void);

void gedit_file_browser_store_refresh                 (GeditFileBrowserStore * model);
gboolean gedit_file_browser_store_rename              (GeditFileBrowserStore * model,
                                                       GtkTreeIter * iter,
                                                       gchar const *new_name,
                                                       GError ** error);
GeditFileBrowserStoreResult
gedit_file_browser_store_delete                       (GeditFileBrowserStore * model,
                                                       GtkTreeIter * iter, 
                                                       gboolean trash);
GeditFileBrowserStoreResult
gedit_file_browser_store_delete_all                   (GeditFileBrowserStore * model,
                                                       GList *rows, 
                                                       gboolean trash);

gboolean gedit_file_browser_store_new_file            (GeditFileBrowserStore * model,
                                                       GtkTreeIter * parent,
                                                       GtkTreeIter * iter);
gboolean gedit_file_browser_store_new_directory       (GeditFileBrowserStore * model,
                                                       GtkTreeIter * parent,
                                                       GtkTreeIter * iter);

void gedit_file_browser_store_cancel_mount_operation  (GeditFileBrowserStore *store);

G_END_DECLS
#endif				/* __GEDIT_FILE_BROWSER_STORE_H__ */

// ex:ts=8:noet:
