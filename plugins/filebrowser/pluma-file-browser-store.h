/*
 * pluma-file-browser-store.h - Pluma plugin providing easy file access 
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

#ifndef __PLUMA_FILE_BROWSER_STORE_H__
#define __PLUMA_FILE_BROWSER_STORE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS
#define PLUMA_TYPE_FILE_BROWSER_STORE			(pluma_file_browser_store_get_type ())
#define PLUMA_FILE_BROWSER_STORE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_FILE_BROWSER_STORE, PlumaFileBrowserStore))
#define PLUMA_FILE_BROWSER_STORE_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_FILE_BROWSER_STORE, PlumaFileBrowserStore const))
#define PLUMA_FILE_BROWSER_STORE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), PLUMA_TYPE_FILE_BROWSER_STORE, PlumaFileBrowserStoreClass))
#define PLUMA_IS_FILE_BROWSER_STORE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUMA_TYPE_FILE_BROWSER_STORE))
#define PLUMA_IS_FILE_BROWSER_STORE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_FILE_BROWSER_STORE))
#define PLUMA_FILE_BROWSER_STORE_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), PLUMA_TYPE_FILE_BROWSER_STORE, PlumaFileBrowserStoreClass))

typedef enum 
{
	PLUMA_FILE_BROWSER_STORE_COLUMN_ICON = 0,
	PLUMA_FILE_BROWSER_STORE_COLUMN_NAME,
	PLUMA_FILE_BROWSER_STORE_COLUMN_URI,
	PLUMA_FILE_BROWSER_STORE_COLUMN_FLAGS,
	PLUMA_FILE_BROWSER_STORE_COLUMN_EMBLEM,
	PLUMA_FILE_BROWSER_STORE_COLUMN_NUM
} PlumaFileBrowserStoreColumn;

typedef enum 
{
	PLUMA_FILE_BROWSER_STORE_FLAG_IS_DIRECTORY = 1 << 0,
	PLUMA_FILE_BROWSER_STORE_FLAG_IS_HIDDEN    = 1 << 1,
	PLUMA_FILE_BROWSER_STORE_FLAG_IS_TEXT      = 1 << 2,
	PLUMA_FILE_BROWSER_STORE_FLAG_LOADED       = 1 << 3,
	PLUMA_FILE_BROWSER_STORE_FLAG_IS_FILTERED  = 1 << 4,
	PLUMA_FILE_BROWSER_STORE_FLAG_IS_DUMMY     = 1 << 5
} PlumaFileBrowserStoreFlag;

typedef enum 
{
	PLUMA_FILE_BROWSER_STORE_RESULT_OK,
	PLUMA_FILE_BROWSER_STORE_RESULT_NO_CHANGE,
	PLUMA_FILE_BROWSER_STORE_RESULT_ERROR,
	PLUMA_FILE_BROWSER_STORE_RESULT_NO_TRASH,
	PLUMA_FILE_BROWSER_STORE_RESULT_MOUNTING,
	PLUMA_FILE_BROWSER_STORE_RESULT_NUM
} PlumaFileBrowserStoreResult;

typedef enum 
{
	PLUMA_FILE_BROWSER_STORE_FILTER_MODE_NONE        = 0,
	PLUMA_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN = 1 << 0,
	PLUMA_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY = 1 << 1
} PlumaFileBrowserStoreFilterMode;

#define FILE_IS_DIR(flags)	(flags & PLUMA_FILE_BROWSER_STORE_FLAG_IS_DIRECTORY)
#define FILE_IS_HIDDEN(flags)	(flags & PLUMA_FILE_BROWSER_STORE_FLAG_IS_HIDDEN)
#define FILE_IS_TEXT(flags)	(flags & PLUMA_FILE_BROWSER_STORE_FLAG_IS_TEXT)
#define FILE_LOADED(flags)	(flags & PLUMA_FILE_BROWSER_STORE_FLAG_LOADED)
#define FILE_IS_FILTERED(flags)	(flags & PLUMA_FILE_BROWSER_STORE_FLAG_IS_FILTERED)
#define FILE_IS_DUMMY(flags)	(flags & PLUMA_FILE_BROWSER_STORE_FLAG_IS_DUMMY)

typedef struct _PlumaFileBrowserStore        PlumaFileBrowserStore;
typedef struct _PlumaFileBrowserStoreClass   PlumaFileBrowserStoreClass;
typedef struct _PlumaFileBrowserStorePrivate PlumaFileBrowserStorePrivate;

typedef gboolean (*PlumaFileBrowserStoreFilterFunc) (PlumaFileBrowserStore
						     * model,
						     GtkTreeIter * iter,
						     gpointer user_data);

struct _PlumaFileBrowserStore 
{
	GObject parent;

	PlumaFileBrowserStorePrivate *priv;
};

struct _PlumaFileBrowserStoreClass {
	GObjectClass parent_class;

	/* Signals */
	void (*begin_loading)        (PlumaFileBrowserStore * model,
			              GtkTreeIter * iter);
	void (*end_loading)          (PlumaFileBrowserStore * model,
			              GtkTreeIter * iter);
	void (*error)                (PlumaFileBrowserStore * model, 
	                              guint code,
		                      gchar * message);
	gboolean (*no_trash)	     (PlumaFileBrowserStore * model,
				      GList * files);
	void (*rename)		     (PlumaFileBrowserStore * model,
				      const gchar * olduri,
				      const gchar * newuri);
	void (*begin_refresh)	     (PlumaFileBrowserStore * model);
	void (*end_refresh)	     (PlumaFileBrowserStore * model);
	void (*unload)		     (PlumaFileBrowserStore * model,
				      const gchar * uri);
};

GType pluma_file_browser_store_get_type               (void) G_GNUC_CONST;
GType pluma_file_browser_store_register_type          (GTypeModule * module);

PlumaFileBrowserStore *pluma_file_browser_store_new   (gchar const *root);

PlumaFileBrowserStoreResult
pluma_file_browser_store_set_root_and_virtual_root    (PlumaFileBrowserStore * model,
						       gchar const *root,
			  			       gchar const *virtual_root);
PlumaFileBrowserStoreResult
pluma_file_browser_store_set_root                     (PlumaFileBrowserStore * model,
				                       gchar const *root);
PlumaFileBrowserStoreResult
pluma_file_browser_store_set_virtual_root             (PlumaFileBrowserStore * model,
					               GtkTreeIter * iter);
PlumaFileBrowserStoreResult
pluma_file_browser_store_set_virtual_root_from_string (PlumaFileBrowserStore * model, 
                                                       gchar const *root);
PlumaFileBrowserStoreResult
pluma_file_browser_store_set_virtual_root_up          (PlumaFileBrowserStore * model);
PlumaFileBrowserStoreResult
pluma_file_browser_store_set_virtual_root_top         (PlumaFileBrowserStore * model);

gboolean
pluma_file_browser_store_get_iter_virtual_root        (PlumaFileBrowserStore * model, 
                                                       GtkTreeIter * iter);
gboolean pluma_file_browser_store_get_iter_root       (PlumaFileBrowserStore * model,
						       GtkTreeIter * iter);
gchar * pluma_file_browser_store_get_root             (PlumaFileBrowserStore * model);
gchar * pluma_file_browser_store_get_virtual_root     (PlumaFileBrowserStore * model);

gboolean pluma_file_browser_store_iter_equal          (PlumaFileBrowserStore * model, 
                                                       GtkTreeIter * iter1,
					               GtkTreeIter * iter2);

void pluma_file_browser_store_set_value               (PlumaFileBrowserStore * tree_model, 
                                                       GtkTreeIter * iter,
                                                       gint column, 
                                                       GValue * value);

void _pluma_file_browser_store_iter_expanded          (PlumaFileBrowserStore * model, 
                                                       GtkTreeIter * iter);
void _pluma_file_browser_store_iter_collapsed         (PlumaFileBrowserStore * model, 
                                                       GtkTreeIter * iter);

PlumaFileBrowserStoreFilterMode
pluma_file_browser_store_get_filter_mode              (PlumaFileBrowserStore * model);
void pluma_file_browser_store_set_filter_mode         (PlumaFileBrowserStore * model,
                                                       PlumaFileBrowserStoreFilterMode mode);
void pluma_file_browser_store_set_filter_func         (PlumaFileBrowserStore * model,
                                                       PlumaFileBrowserStoreFilterFunc func, 
                                                       gpointer user_data);
void pluma_file_browser_store_refilter                (PlumaFileBrowserStore * model);
PlumaFileBrowserStoreFilterMode
pluma_file_browser_store_filter_mode_get_default      (void);

void pluma_file_browser_store_refresh                 (PlumaFileBrowserStore * model);
gboolean pluma_file_browser_store_rename              (PlumaFileBrowserStore * model,
                                                       GtkTreeIter * iter,
                                                       gchar const *new_name,
                                                       GError ** error);
PlumaFileBrowserStoreResult
pluma_file_browser_store_delete                       (PlumaFileBrowserStore * model,
                                                       GtkTreeIter * iter, 
                                                       gboolean trash);
PlumaFileBrowserStoreResult
pluma_file_browser_store_delete_all                   (PlumaFileBrowserStore * model,
                                                       GList *rows, 
                                                       gboolean trash);

gboolean pluma_file_browser_store_new_file            (PlumaFileBrowserStore * model,
                                                       GtkTreeIter * parent,
                                                       GtkTreeIter * iter);
gboolean pluma_file_browser_store_new_directory       (PlumaFileBrowserStore * model,
                                                       GtkTreeIter * parent,
                                                       GtkTreeIter * iter);

void pluma_file_browser_store_cancel_mount_operation  (PlumaFileBrowserStore *store);

G_END_DECLS
#endif				/* __PLUMA_FILE_BROWSER_STORE_H__ */

// ex:ts=8:noet:
