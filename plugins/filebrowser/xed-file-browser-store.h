/*
 * xed-file-browser-store.h - Xed plugin providing easy file access
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

#ifndef __XED_FILE_BROWSER_STORE_H__
#define __XED_FILE_BROWSER_STORE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS
#define XED_TYPE_FILE_BROWSER_STORE         (xed_file_browser_store_get_type ())
#define XED_FILE_BROWSER_STORE(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_FILE_BROWSER_STORE, XedFileBrowserStore))
#define XED_FILE_BROWSER_STORE_CONST(obj)       (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_FILE_BROWSER_STORE, XedFileBrowserStore const))
#define XED_FILE_BROWSER_STORE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_FILE_BROWSER_STORE, XedFileBrowserStoreClass))
#define XED_IS_FILE_BROWSER_STORE(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_FILE_BROWSER_STORE))
#define XED_IS_FILE_BROWSER_STORE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_FILE_BROWSER_STORE))
#define XED_FILE_BROWSER_STORE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), XED_TYPE_FILE_BROWSER_STORE, XedFileBrowserStoreClass))

typedef enum
{
    XED_FILE_BROWSER_STORE_COLUMN_ICON = 0,
    XED_FILE_BROWSER_STORE_COLUMN_NAME,
    XED_FILE_BROWSER_STORE_COLUMN_URI,
    XED_FILE_BROWSER_STORE_COLUMN_FLAGS,
    XED_FILE_BROWSER_STORE_COLUMN_EMBLEM,
    XED_FILE_BROWSER_STORE_COLUMN_NUM
} XedFileBrowserStoreColumn;

typedef enum
{
    XED_FILE_BROWSER_STORE_FLAG_IS_DIRECTORY = 1 << 0,
    XED_FILE_BROWSER_STORE_FLAG_IS_HIDDEN    = 1 << 1,
    XED_FILE_BROWSER_STORE_FLAG_IS_TEXT      = 1 << 2,
    XED_FILE_BROWSER_STORE_FLAG_LOADED       = 1 << 3,
    XED_FILE_BROWSER_STORE_FLAG_IS_FILTERED  = 1 << 4,
    XED_FILE_BROWSER_STORE_FLAG_IS_DUMMY     = 1 << 5
} XedFileBrowserStoreFlag;

typedef enum
{
    XED_FILE_BROWSER_STORE_RESULT_OK,
    XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE,
    XED_FILE_BROWSER_STORE_RESULT_ERROR,
    XED_FILE_BROWSER_STORE_RESULT_NO_TRASH,
    XED_FILE_BROWSER_STORE_RESULT_MOUNTING,
    XED_FILE_BROWSER_STORE_RESULT_NUM
} XedFileBrowserStoreResult;

typedef enum
{
    XED_FILE_BROWSER_STORE_FILTER_MODE_NONE        = 0,
    XED_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN = 1 << 0,
    XED_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY = 1 << 1
} XedFileBrowserStoreFilterMode;

#define FILE_IS_DIR(flags)  (flags & XED_FILE_BROWSER_STORE_FLAG_IS_DIRECTORY)
#define FILE_IS_HIDDEN(flags)   (flags & XED_FILE_BROWSER_STORE_FLAG_IS_HIDDEN)
#define FILE_IS_TEXT(flags) (flags & XED_FILE_BROWSER_STORE_FLAG_IS_TEXT)
#define FILE_LOADED(flags)  (flags & XED_FILE_BROWSER_STORE_FLAG_LOADED)
#define FILE_IS_FILTERED(flags) (flags & XED_FILE_BROWSER_STORE_FLAG_IS_FILTERED)
#define FILE_IS_DUMMY(flags)    (flags & XED_FILE_BROWSER_STORE_FLAG_IS_DUMMY)

typedef struct _XedFileBrowserStore        XedFileBrowserStore;
typedef struct _XedFileBrowserStoreClass   XedFileBrowserStoreClass;
typedef struct _XedFileBrowserStorePrivate XedFileBrowserStorePrivate;

typedef gboolean (*XedFileBrowserStoreFilterFunc) (XedFileBrowserStore
                             * model,
                             GtkTreeIter * iter,
                             gpointer user_data);

struct _XedFileBrowserStore
{
    GObject parent;

    XedFileBrowserStorePrivate *priv;
};

struct _XedFileBrowserStoreClass {
    GObjectClass parent_class;

    /* Signals */
    void (*begin_loading)        (XedFileBrowserStore * model,
                          GtkTreeIter * iter);
    void (*end_loading)          (XedFileBrowserStore * model,
                          GtkTreeIter * iter);
    void (*error)                (XedFileBrowserStore * model,
                                  guint code,
                              gchar * message);
    gboolean (*no_trash)         (XedFileBrowserStore * model,
                      GList * files);
    void (*rename)           (XedFileBrowserStore * model,
                      const gchar * olduri,
                      const gchar * newuri);
    void (*begin_refresh)        (XedFileBrowserStore * model);
    void (*end_refresh)      (XedFileBrowserStore * model);
    void (*unload)           (XedFileBrowserStore * model,
                      const gchar * uri);
};

GType xed_file_browser_store_get_type               (void) G_GNUC_CONST;
void _xed_file_browser_store_register_type          (GTypeModule * module);

XedFileBrowserStore *xed_file_browser_store_new   (gchar const *root);

XedFileBrowserStoreResult
xed_file_browser_store_set_root_and_virtual_root    (XedFileBrowserStore * model,
                               gchar const *root,
                               gchar const *virtual_root);
XedFileBrowserStoreResult
xed_file_browser_store_set_root                     (XedFileBrowserStore * model,
                                       gchar const *root);
XedFileBrowserStoreResult
xed_file_browser_store_set_virtual_root             (XedFileBrowserStore * model,
                                   GtkTreeIter * iter);
XedFileBrowserStoreResult
xed_file_browser_store_set_virtual_root_from_string (XedFileBrowserStore * model,
                                                       gchar const *root);
XedFileBrowserStoreResult
xed_file_browser_store_set_virtual_root_up          (XedFileBrowserStore * model);
XedFileBrowserStoreResult
xed_file_browser_store_set_virtual_root_top         (XedFileBrowserStore * model);

gboolean
xed_file_browser_store_get_iter_virtual_root        (XedFileBrowserStore * model,
                                                       GtkTreeIter * iter);
gboolean xed_file_browser_store_get_iter_root       (XedFileBrowserStore * model,
                               GtkTreeIter * iter);
gchar * xed_file_browser_store_get_root             (XedFileBrowserStore * model);
gchar * xed_file_browser_store_get_virtual_root     (XedFileBrowserStore * model);

gboolean xed_file_browser_store_iter_equal          (XedFileBrowserStore * model,
                                                       GtkTreeIter * iter1,
                                   GtkTreeIter * iter2);

void xed_file_browser_store_set_value               (XedFileBrowserStore * tree_model,
                                                       GtkTreeIter * iter,
                                                       gint column,
                                                       GValue * value);

void _xed_file_browser_store_iter_expanded          (XedFileBrowserStore * model,
                                                       GtkTreeIter * iter);
void _xed_file_browser_store_iter_collapsed         (XedFileBrowserStore * model,
                                                       GtkTreeIter * iter);

XedFileBrowserStoreFilterMode
xed_file_browser_store_get_filter_mode              (XedFileBrowserStore * model);
void xed_file_browser_store_set_filter_mode         (XedFileBrowserStore * model,
                                                       XedFileBrowserStoreFilterMode mode);
void xed_file_browser_store_set_filter_func         (XedFileBrowserStore * model,
                                                       XedFileBrowserStoreFilterFunc func,
                                                       gpointer user_data);
void xed_file_browser_store_refilter                (XedFileBrowserStore * model);
XedFileBrowserStoreFilterMode
xed_file_browser_store_filter_mode_get_default      (void);

void xed_file_browser_store_refresh                 (XedFileBrowserStore * model);
gboolean xed_file_browser_store_rename              (XedFileBrowserStore * model,
                                                       GtkTreeIter * iter,
                                                       gchar const *new_name,
                                                       GError ** error);
XedFileBrowserStoreResult
xed_file_browser_store_delete                       (XedFileBrowserStore * model,
                                                       GtkTreeIter * iter,
                                                       gboolean trash);
XedFileBrowserStoreResult
xed_file_browser_store_delete_all                   (XedFileBrowserStore * model,
                                                       GList *rows,
                                                       gboolean trash);

gboolean xed_file_browser_store_new_file            (XedFileBrowserStore * model,
                                                       GtkTreeIter * parent,
                                                       GtkTreeIter * iter);
gboolean xed_file_browser_store_new_directory       (XedFileBrowserStore * model,
                                                       GtkTreeIter * parent,
                                                       GtkTreeIter * iter);

void xed_file_browser_store_cancel_mount_operation  (XedFileBrowserStore *store);

G_END_DECLS
#endif              /* __XED_FILE_BROWSER_STORE_H__ */

// ex:ts=8:noet:
