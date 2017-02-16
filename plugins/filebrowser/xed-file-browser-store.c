/*
 * xed-file-browser-store.c - Xed plugin providing easy file access
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <glib/gi18n-lib.h>
#include <gio/gio.h>
#include <xed/xed-utils.h>

#include "xed-file-browser-store.h"
#include "xed-file-browser-marshal.h"
#include "xed-file-browser-enum-types.h"
#include "xed-file-browser-error.h"
#include "xed-file-browser-utils.h"

#define XED_FILE_BROWSER_STORE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), \
                             XED_TYPE_FILE_BROWSER_STORE, \
                             XedFileBrowserStorePrivate))

#define NODE_IS_DIR(node)      (FILE_IS_DIR((node)->flags))
#define NODE_IS_HIDDEN(node)   (FILE_IS_HIDDEN((node)->flags))
#define NODE_IS_TEXT(node)     (FILE_IS_TEXT((node)->flags))
#define NODE_LOADED(node)      (FILE_LOADED((node)->flags))
#define NODE_IS_FILTERED(node) (FILE_IS_FILTERED((node)->flags))
#define NODE_IS_DUMMY(node)    (FILE_IS_DUMMY((node)->flags))

#define FILE_BROWSER_NODE_DIR(node) ((FileBrowserNodeDir *)(node))

#define DIRECTORY_LOAD_ITEMS_PER_CALLBACK 100
#define STANDARD_ATTRIBUTE_TYPES G_FILE_ATTRIBUTE_STANDARD_TYPE "," \
                                 G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN "," \
                                 G_FILE_ATTRIBUTE_STANDARD_IS_BACKUP "," \
                                 G_FILE_ATTRIBUTE_STANDARD_NAME "," \
                                 G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE "," \
                                 G_FILE_ATTRIBUTE_STANDARD_ICON

typedef struct _FileBrowserNode    FileBrowserNode;
typedef struct _FileBrowserNodeDir FileBrowserNodeDir;
typedef struct _AsyncData AsyncData;
typedef struct _AsyncNode AsyncNode;

typedef gint (*SortFunc) (FileBrowserNode *node1, FileBrowserNode *node2);

struct _AsyncData
{
    XedFileBrowserStore *model;
    GCancellable *cancellable;
    gboolean trash;
    GList *files;
    GList *iter;
    gboolean removed;
};

struct _AsyncNode
{
    FileBrowserNodeDir *dir;
    GCancellable *cancellable;
    GSList *original_children;
};

typedef struct {
    XedFileBrowserStore *model;
    GFile *virtual_root;
    GMountOperation *operation;
    GCancellable *cancellable;
} MountInfo;

struct _FileBrowserNode
{
    GFile *file;
    guint flags;
    gchar *name;

    GdkPixbuf *icon;
    GdkPixbuf *emblem;

    FileBrowserNode *parent;
    gint pos;
    gboolean inserted;
};

struct _FileBrowserNodeDir
{
    FileBrowserNode node;
    GSList *children;

    GCancellable *cancellable;
    GFileMonitor *monitor;
    XedFileBrowserStore *model;
};

struct _XedFileBrowserStorePrivate
{
    FileBrowserNode *root;
    FileBrowserNode *virtual_root;
    GType column_types[XED_FILE_BROWSER_STORE_COLUMN_NUM];

    XedFileBrowserStoreFilterMode filter_mode;
    XedFileBrowserStoreFilterFunc filter_func;
    gpointer filter_user_data;

    SortFunc sort_func;

    GSList *async_handles;
    MountInfo *mount_info;
};

static FileBrowserNode *model_find_node (XedFileBrowserStore *model,
                                         FileBrowserNode     *node,
                                         GFile               *uri);
static void model_remove_node (XedFileBrowserStore *model,
                               FileBrowserNode     *node,
                               GtkTreePath         *path,
                               gboolean             free_nodes);

static void set_virtual_root_from_node (XedFileBrowserStore *model,
                                        FileBrowserNode     *node);

static void xed_file_browser_store_iface_init (GtkTreeModelIface *iface);
static GtkTreeModelFlags xed_file_browser_store_get_flags (GtkTreeModel *tree_model);
static gint xed_file_browser_store_get_n_columns (GtkTreeModel *tree_model);
static GType xed_file_browser_store_get_column_type (GtkTreeModel *tree_model,
                                                     gint          index);
static gboolean xed_file_browser_store_get_iter (GtkTreeModel *tree_model,
                                                 GtkTreeIter  *iter,
                                                 GtkTreePath  *path);
static GtkTreePath *xed_file_browser_store_get_path (GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter);
static void xed_file_browser_store_get_value (GtkTreeModel *tree_model,
                                              GtkTreeIter  *iter,
                                              gint          column,
                                              GValue       *value);
static gboolean xed_file_browser_store_iter_next (GtkTreeModel *tree_model,
                                                  GtkTreeIter *iter);
static gboolean xed_file_browser_store_iter_children (GtkTreeModel *tree_model,
                                                      GtkTreeIter *iter,
                                                      GtkTreeIter *parent);
static gboolean xed_file_browser_store_iter_has_child (GtkTreeModel *tree_model,
                                                       GtkTreeIter  *iter);
static gint xed_file_browser_store_iter_n_children (GtkTreeModel *tree_model,
                                                    GtkTreeIter  *iter);
static gboolean xed_file_browser_store_iter_nth_child (GtkTreeModel *tree_model,
                                                       GtkTreeIter  *iter,
                                                       GtkTreeIter  *parent,
                                                       gint          n);
static gboolean xed_file_browser_store_iter_parent (GtkTreeModel *tree_model,
                                                    GtkTreeIter  *iter,
                                                    GtkTreeIter  *child);
static void xed_file_browser_store_row_inserted (GtkTreeModel *tree_model,
                                                 GtkTreePath  *path,
                                                 GtkTreeIter  *iter);

static void xed_file_browser_store_drag_source_init (GtkTreeDragSourceIface *iface);
static gboolean xed_file_browser_store_row_draggable (GtkTreeDragSource *drag_source,
                                                      GtkTreePath       *path);
static gboolean xed_file_browser_store_drag_data_delete (GtkTreeDragSource *drag_source,
                                                         GtkTreePath       *path);
static gboolean xed_file_browser_store_drag_data_get (GtkTreeDragSource *drag_source,
                                                      GtkTreePath       *path,
                                                      GtkSelectionData  *selection_data);

static void file_browser_node_free (XedFileBrowserStore *model,
                                    FileBrowserNode     *node);
static void model_add_node (XedFileBrowserStore *model,
                            FileBrowserNode     *child,
                            FileBrowserNode     *parent);
static void model_clear (XedFileBrowserStore *model,
                         gboolean             free_nodes);
static gint model_sort_default (FileBrowserNode *node1,
                                FileBrowserNode *node2);
static void model_check_dummy (XedFileBrowserStore *model,
                               FileBrowserNode     *node);
static void next_files_async (GFileEnumerator *enumerator,
                              AsyncNode       *async);

static void delete_files (AsyncData *data);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (XedFileBrowserStore, xed_file_browser_store,
                                G_TYPE_OBJECT,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (GTK_TYPE_TREE_MODEL,
                                                               xed_file_browser_store_iface_init)
                                G_IMPLEMENT_INTERFACE_DYNAMIC (GTK_TYPE_TREE_DRAG_SOURCE,
                                                               xed_file_browser_store_drag_source_init))

/* Properties */
enum {
    PROP_0,

    PROP_ROOT,
    PROP_VIRTUAL_ROOT,
    PROP_FILTER_MODE
};

/* Signals */
enum
{
    BEGIN_LOADING,
    END_LOADING,
    ERROR,
    NO_TRASH,
    RENAME,
    BEGIN_REFRESH,
    END_REFRESH,
    UNLOAD,
    NUM_SIGNALS
};

static guint model_signals[NUM_SIGNALS] = { 0 };

static void
cancel_mount_operation (XedFileBrowserStore *obj)
{
    if (obj->priv->mount_info != NULL)
    {
        obj->priv->mount_info->model = NULL;
        g_cancellable_cancel (obj->priv->mount_info->cancellable);
        obj->priv->mount_info = NULL;
    }
}

static void
xed_file_browser_store_finalize (GObject * object)
{
    XedFileBrowserStore *obj = XED_FILE_BROWSER_STORE (object);
    GSList *item;

    /* Free all the nodes */
    file_browser_node_free (obj, obj->priv->root);

    /* Cancel any asynchronous operations */
    for (item = obj->priv->async_handles; item; item = item->next)
    {
        AsyncData *data = (AsyncData *) (item->data);
        g_cancellable_cancel (data->cancellable);

        data->removed = TRUE;
    }

    cancel_mount_operation (obj);

    g_slist_free (obj->priv->async_handles);
    G_OBJECT_CLASS (xed_file_browser_store_parent_class)->finalize (object);
}

static void
set_gvalue_from_node (GValue          *value,
                      FileBrowserNode *node)
{
    if (node == NULL)
    {
        g_value_set_object (value, NULL);
    }
    else
    {
        g_value_set_object (value, node->file);
    }
}

static void
xed_file_browser_store_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
    XedFileBrowserStore *obj = XED_FILE_BROWSER_STORE (object);

    switch (prop_id)
    {
        case PROP_ROOT:
            set_gvalue_from_node (value, obj->priv->root);
            break;
        case PROP_VIRTUAL_ROOT:
            set_gvalue_from_node (value, obj->priv->virtual_root);
            break;
        case PROP_FILTER_MODE:
            g_value_set_flags (value, obj->priv->filter_mode);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_file_browser_store_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
    XedFileBrowserStore *obj = XED_FILE_BROWSER_STORE (object);

    switch (prop_id)
    {
        case PROP_FILTER_MODE:
            xed_file_browser_store_set_filter_mode (obj, g_value_get_flags (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_file_browser_store_class_init (XedFileBrowserStoreClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = xed_file_browser_store_finalize;

    object_class->get_property = xed_file_browser_store_get_property;
    object_class->set_property = xed_file_browser_store_set_property;

    g_object_class_install_property (object_class, PROP_ROOT,
                                     g_param_spec_object ("root",
                                                          "Root",
                                                          "The root location",
                                                          G_TYPE_FILE,
                                                          G_PARAM_READABLE));

    g_object_class_install_property (object_class, PROP_VIRTUAL_ROOT,
                                     g_param_spec_object ("virtual-root",
                                                          "Virtual Root",
                                                          "The virtual root location",
                                                          G_TYPE_FILE,
                                                          G_PARAM_READABLE));

    g_object_class_install_property (object_class, PROP_FILTER_MODE,
                                     g_param_spec_flags ("filter-mode",
                                                         "Filter Mode",
                                                         "The filter mode",
                                                         XED_TYPE_FILE_BROWSER_STORE_FILTER_MODE,
                                                         xed_file_browser_store_filter_mode_get_default (),
                                                         G_PARAM_READWRITE));

    model_signals[BEGIN_LOADING] =
        g_signal_new ("begin-loading",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedFileBrowserStoreClass,
                               begin_loading), NULL, NULL,
                      g_cclosure_marshal_VOID__BOXED, G_TYPE_NONE, 1,
                      GTK_TYPE_TREE_ITER);
    model_signals[END_LOADING] =
        g_signal_new ("end-loading",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedFileBrowserStoreClass,
                               end_loading), NULL, NULL,
                      g_cclosure_marshal_VOID__BOXED, G_TYPE_NONE, 1,
                      GTK_TYPE_TREE_ITER);
    model_signals[ERROR] =
        g_signal_new ("error",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedFileBrowserStoreClass,
                               error), NULL, NULL,
                      xed_file_browser_marshal_VOID__UINT_STRING,
                      G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_STRING);
    model_signals[NO_TRASH] =
        g_signal_new ("no-trash",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedFileBrowserStoreClass,
                               no_trash), g_signal_accumulator_true_handled, NULL,
                      xed_file_browser_marshal_BOOL__POINTER,
                      G_TYPE_BOOLEAN, 1, G_TYPE_POINTER);
    model_signals[RENAME] =
        g_signal_new ("rename",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedFileBrowserStoreClass,
                               rename), NULL, NULL,
                      xed_file_browser_marshal_VOID__OBJECT_OBJECT,
                      G_TYPE_NONE, 2,
                      G_TYPE_FILE,
                      G_TYPE_FILE);
    model_signals[BEGIN_REFRESH] =
        g_signal_new ("begin-refresh",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedFileBrowserStoreClass,
                               begin_refresh), NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);
    model_signals[END_REFRESH] =
        g_signal_new ("end-refresh",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedFileBrowserStoreClass,
                               end_refresh), NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);
    model_signals[UNLOAD] =
        g_signal_new ("unload",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedFileBrowserStoreClass,
                               unload), NULL, NULL,
                      g_cclosure_marshal_VOID__OBJECT,
                      G_TYPE_NONE, 1,
                      G_TYPE_FILE);

    g_type_class_add_private (object_class, sizeof (XedFileBrowserStorePrivate));
}

static void
xed_file_browser_store_class_finalize (XedFileBrowserStoreClass *klass)
{
    /* dummy function - used by G_DEFINE_DYNAMIC_TYPE_EXTENDED */
}

static void
xed_file_browser_store_iface_init (GtkTreeModelIface * iface)
{
    iface->get_flags = xed_file_browser_store_get_flags;
    iface->get_n_columns = xed_file_browser_store_get_n_columns;
    iface->get_column_type = xed_file_browser_store_get_column_type;
    iface->get_iter = xed_file_browser_store_get_iter;
    iface->get_path = xed_file_browser_store_get_path;
    iface->get_value = xed_file_browser_store_get_value;
    iface->iter_next = xed_file_browser_store_iter_next;
    iface->iter_children = xed_file_browser_store_iter_children;
    iface->iter_has_child = xed_file_browser_store_iter_has_child;
    iface->iter_n_children = xed_file_browser_store_iter_n_children;
    iface->iter_nth_child = xed_file_browser_store_iter_nth_child;
    iface->iter_parent = xed_file_browser_store_iter_parent;
    iface->row_inserted = xed_file_browser_store_row_inserted;
}

static void
xed_file_browser_store_drag_source_init (GtkTreeDragSourceIface * iface)
{
    iface->row_draggable = xed_file_browser_store_row_draggable;
    iface->drag_data_delete = xed_file_browser_store_drag_data_delete;
    iface->drag_data_get = xed_file_browser_store_drag_data_get;
}

static void
xed_file_browser_store_init (XedFileBrowserStore * obj)
{
    obj->priv = XED_FILE_BROWSER_STORE_GET_PRIVATE (obj);

    obj->priv->column_types[XED_FILE_BROWSER_STORE_COLUMN_LOCATION] = G_TYPE_FILE;
    obj->priv->column_types[XED_FILE_BROWSER_STORE_COLUMN_NAME] = G_TYPE_STRING;
    obj->priv->column_types[XED_FILE_BROWSER_STORE_COLUMN_FLAGS] = G_TYPE_UINT;
    obj->priv->column_types[XED_FILE_BROWSER_STORE_COLUMN_ICON] = GDK_TYPE_PIXBUF;
    obj->priv->column_types[XED_FILE_BROWSER_STORE_COLUMN_EMBLEM] = GDK_TYPE_PIXBUF;

    // Default filter mode is hiding the hidden files
    obj->priv->filter_mode = xed_file_browser_store_filter_mode_get_default ();
    obj->priv->sort_func = model_sort_default;
}

static gboolean
node_has_parent (FileBrowserNode *node,
                 FileBrowserNode *parent)
{
    if (node->parent == NULL)
    {
        return FALSE;
    }

    if (node->parent == parent)
    {
        return TRUE;
    }

    return node_has_parent (node->parent, parent);
}

static gboolean
node_in_tree (XedFileBrowserStore *model,
              FileBrowserNode     *node)
{
    return node_has_parent (node, model->priv->virtual_root);
}

static gboolean
model_node_visibility (XedFileBrowserStore *model,
                       FileBrowserNode     *node)
{
    if (node == NULL)
    {
        return FALSE;
    }

    if (NODE_IS_DUMMY (node))
    {
        return !NODE_IS_HIDDEN (node);
    }

    if (node == model->priv->virtual_root)
    {
        return TRUE;
    }

    if (!node_has_parent (node, model->priv->virtual_root))
    {
        return FALSE;
    }

    return !NODE_IS_FILTERED (node);
}

static gboolean
model_node_inserted (XedFileBrowserStore *model,
                     FileBrowserNode     *node)
{
    return node == model->priv->virtual_root || (model_node_visibility (model, node) && node->inserted);
}

/* Interface implementation */

static GtkTreeModelFlags
xed_file_browser_store_get_flags (GtkTreeModel *tree_model)
{
    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (tree_model), (GtkTreeModelFlags) 0);

    return GTK_TREE_MODEL_ITERS_PERSIST;
}

static gint
xed_file_browser_store_get_n_columns (GtkTreeModel *tree_model)
{
    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (tree_model), 0);

    return XED_FILE_BROWSER_STORE_COLUMN_NUM;
}

static GType
xed_file_browser_store_get_column_type (GtkTreeModel *tree_model,
                                        gint          idx)
{
    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (tree_model), G_TYPE_INVALID);
    g_return_val_if_fail (idx < XED_FILE_BROWSER_STORE_COLUMN_NUM && idx >= 0, G_TYPE_INVALID);

    return XED_FILE_BROWSER_STORE (tree_model)->priv->column_types[idx];
}

static gboolean
xed_file_browser_store_get_iter (GtkTreeModel *tree_model,
                                 GtkTreeIter  *iter,
                                 GtkTreePath  *path)
{
    gint *indices, depth, i;
    FileBrowserNode *node;
    XedFileBrowserStore *model;
    gint num;

    g_assert (XED_IS_FILE_BROWSER_STORE (tree_model));
    g_assert (path != NULL);

    model = XED_FILE_BROWSER_STORE (tree_model);
    indices = gtk_tree_path_get_indices (path);
    depth = gtk_tree_path_get_depth (path);
    node = model->priv->virtual_root;

    for (i = 0; i < depth; ++i)
    {
        GSList * item;

        if (node == NULL)
        {
            return FALSE;
        }

        num = 0;

        if (!NODE_IS_DIR (node))
        {
            return FALSE;
        }

        for (item = FILE_BROWSER_NODE_DIR (node)->children; item; item = item->next)
        {
            FileBrowserNode * child;

            child = (FileBrowserNode *) (item->data);

            if (model_node_inserted (model, child))
            {
                if (num == indices[i])
                {
                    node = child;
                    break;
                }

                num++;
            }
        }

        if (item == NULL)
        {
            return FALSE;
        }

        node = (FileBrowserNode *) (item->data);
    }

    iter->user_data = node;
    iter->user_data2 = NULL;
    iter->user_data3 = NULL;

    return node != NULL;
}

static GtkTreePath *
xed_file_browser_store_get_path_real (XedFileBrowserStore *model,
                                      FileBrowserNode     *node)
{
    GtkTreePath *path;
    gint num = 0;

    path = gtk_tree_path_new ();

    while (node != model->priv->virtual_root)
    {
        GSList *item;

        if (node->parent == NULL)
        {
            gtk_tree_path_free (path);
            return NULL;
        }

        num = 0;

        for (item = FILE_BROWSER_NODE_DIR (node->parent)->children; item; item = item->next)
        {
            FileBrowserNode *check;

            check = (FileBrowserNode *) (item->data);

            if (model_node_visibility (model, check) && (check == node || check->inserted))
            {
                if (check == node)
                {
                    gtk_tree_path_prepend_index (path, num);
                    break;
                }

                ++num;
            }
            else if (check == node)
            {
                if (NODE_IS_DUMMY (node))
                {
                    g_warning ("Dummy not visible???");
                }

                gtk_tree_path_free (path);
                return NULL;
            }
        }

        node = node->parent;
    }

    return path;
}

static GtkTreePath *
xed_file_browser_store_get_path (GtkTreeModel *tree_model,
                                 GtkTreeIter  *iter)
{
    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (tree_model), NULL);
    g_return_val_if_fail (iter != NULL, NULL);
    g_return_val_if_fail (iter->user_data != NULL, NULL);

    return xed_file_browser_store_get_path_real (XED_FILE_BROWSER_STORE (tree_model),
                                                 (FileBrowserNode *) (iter->user_data));
}

static void
xed_file_browser_store_get_value (GtkTreeModel *tree_model,
                                  GtkTreeIter  *iter,
                                  gint          column,
                                  GValue       *value)
{
    FileBrowserNode *node;

    g_return_if_fail (XED_IS_FILE_BROWSER_STORE (tree_model));
    g_return_if_fail (iter != NULL);
    g_return_if_fail (iter->user_data != NULL);

    node = (FileBrowserNode *) (iter->user_data);

    g_value_init (value, XED_FILE_BROWSER_STORE (tree_model)->priv->column_types[column]);

    switch (column)
    {
        case XED_FILE_BROWSER_STORE_COLUMN_LOCATION:
            set_gvalue_from_node (value, node);
            break;
        case XED_FILE_BROWSER_STORE_COLUMN_NAME:
            g_value_set_string (value, node->name);
            break;
        case XED_FILE_BROWSER_STORE_COLUMN_FLAGS:
            g_value_set_uint (value, node->flags);
            break;
        case XED_FILE_BROWSER_STORE_COLUMN_ICON:
            g_value_set_object (value, node->icon);
            break;
        case XED_FILE_BROWSER_STORE_COLUMN_EMBLEM:
            g_value_set_object (value, node->emblem);
            break;
        default:
            g_return_if_reached ();
    }
}

static gboolean
xed_file_browser_store_iter_next (GtkTreeModel *tree_model,
                                  GtkTreeIter  *iter)
{
    XedFileBrowserStore *model;
    FileBrowserNode *node;
    GSList *item;
    GSList *first;

    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (tree_model), FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);
    g_return_val_if_fail (iter->user_data != NULL, FALSE);

    model = XED_FILE_BROWSER_STORE (tree_model);
    node = (FileBrowserNode *) (iter->user_data);

    if (node->parent == NULL)
    {
        return FALSE;
    }

    first = g_slist_next (g_slist_find (FILE_BROWSER_NODE_DIR (node->parent)->children, node));

    for (item = first; item; item = item->next)
    {
        if (model_node_inserted (model, (FileBrowserNode *) (item->data)))
        {
            iter->user_data = item->data;
            return TRUE;
        }
    }

    return FALSE;
}

static gboolean
xed_file_browser_store_iter_children (GtkTreeModel *tree_model,
                                      GtkTreeIter  *iter,
                                      GtkTreeIter  *parent)
{
    FileBrowserNode *node;
    XedFileBrowserStore *model;
    GSList *item;

    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (tree_model), FALSE);
    g_return_val_if_fail (parent == NULL || parent->user_data != NULL, FALSE);

    model = XED_FILE_BROWSER_STORE (tree_model);

    if (parent == NULL)
    {
        node = model->priv->virtual_root;
    }
    else
    {
        node = (FileBrowserNode *) (parent->user_data);
    }

    if (node == NULL)
    {
        return FALSE;
    }

    if (!NODE_IS_DIR (node))
    {
        return FALSE;
    }

    for (item = FILE_BROWSER_NODE_DIR (node)->children; item; item = item->next)
    {
        if (model_node_inserted (model, (FileBrowserNode *) (item->data)))
        {
            iter->user_data = item->data;
            return TRUE;
        }
    }

    return FALSE;
}

static gboolean
filter_tree_model_iter_has_child_real (XedFileBrowserStore *model,
                                       FileBrowserNode     *node)
{
    GSList *item;

    if (!NODE_IS_DIR (node))
    {
        return FALSE;
    }

    for (item = FILE_BROWSER_NODE_DIR (node)->children; item; item = item->next)
    {
        if (model_node_inserted (model, (FileBrowserNode *) (item->data)))
        {
            return TRUE;
        }
    }

    return FALSE;
}

static gboolean
xed_file_browser_store_iter_has_child (GtkTreeModel *tree_model,
                                       GtkTreeIter  *iter)
{
    FileBrowserNode *node;
    XedFileBrowserStore *model;

    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (tree_model), FALSE);
    g_return_val_if_fail (iter == NULL || iter->user_data != NULL, FALSE);

    model = XED_FILE_BROWSER_STORE (tree_model);

    if (iter == NULL)
    {
        node = model->priv->virtual_root;
    }
    else
    {
        node = (FileBrowserNode *) (iter->user_data);
    }

    return filter_tree_model_iter_has_child_real (model, node);
}

static gint
xed_file_browser_store_iter_n_children (GtkTreeModel *tree_model,
                                        GtkTreeIter  *iter)
{
    FileBrowserNode *node;
    XedFileBrowserStore *model;
    GSList *item;
    gint num = 0;

    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (tree_model), FALSE);
    g_return_val_if_fail (iter == NULL || iter->user_data != NULL, FALSE);

    model = XED_FILE_BROWSER_STORE (tree_model);

    if (iter == NULL)
    {
        node = model->priv->virtual_root;
    }
    else
    {
        node = (FileBrowserNode *) (iter->user_data);
    }

    if (!NODE_IS_DIR (node))
    {
        return 0;
    }

    for (item = FILE_BROWSER_NODE_DIR (node)->children; item; item = item->next)
    {
        if (model_node_inserted (model, (FileBrowserNode *) (item->data)))
        {
            ++num;
        }
    }

    return num;
}

static gboolean
xed_file_browser_store_iter_nth_child (GtkTreeModel *tree_model,
                                       GtkTreeIter  *iter,
                                       GtkTreeIter  *parent, gint n)
{
    FileBrowserNode *node;
    XedFileBrowserStore *model;
    GSList *item;
    gint num = 0;

    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (tree_model), FALSE);
    g_return_val_if_fail (parent == NULL || parent->user_data != NULL, FALSE);

    model = XED_FILE_BROWSER_STORE (tree_model);

    if (parent == NULL)
    {
        node = model->priv->virtual_root;
    }
    else
    {
        node = (FileBrowserNode *) (parent->user_data);
    }

    if (!NODE_IS_DIR (node))
    {
        return FALSE;
    }

    for (item = FILE_BROWSER_NODE_DIR (node)->children; item; item = item->next)
    {
        if (model_node_inserted (model, (FileBrowserNode *) (item->data)))
        {
            if (num == n)
            {
                iter->user_data = item->data;
                return TRUE;
            }

            ++num;
        }
    }

    return FALSE;
}

static gboolean
xed_file_browser_store_iter_parent (GtkTreeModel *tree_model,
                                    GtkTreeIter  *iter,
                                    GtkTreeIter  *child)
{
    FileBrowserNode *node;
    XedFileBrowserStore *model;

    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (tree_model), FALSE);
    g_return_val_if_fail (child != NULL, FALSE);
    g_return_val_if_fail (child->user_data != NULL, FALSE);

    node = (FileBrowserNode *) (child->user_data);
    model = XED_FILE_BROWSER_STORE (tree_model);

    if (!node_in_tree (model, node))
    {
        return FALSE;
    }

    if (node->parent == NULL)
    {
        return FALSE;
    }

    iter->user_data = node->parent;
    return TRUE;
}

static void
xed_file_browser_store_row_inserted (GtkTreeModel *tree_model,
                                     GtkTreePath  *path,
                                     GtkTreeIter  *iter)
{
    FileBrowserNode * node = (FileBrowserNode *)(iter->user_data);

    node->inserted = TRUE;
}

static gboolean
xed_file_browser_store_row_draggable (GtkTreeDragSource *drag_source,
                                      GtkTreePath       *path)
{
    GtkTreeIter iter;
    XedFileBrowserStoreFlag flags;

    if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (drag_source), &iter, path))
    {
        return FALSE;
    }

    gtk_tree_model_get (GTK_TREE_MODEL (drag_source), &iter, XED_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags, -1);

    return !FILE_IS_DUMMY(flags);
}

static gboolean
xed_file_browser_store_drag_data_delete (GtkTreeDragSource *drag_source,
                                         GtkTreePath       *path)
{
    return FALSE;
}

static gboolean
xed_file_browser_store_drag_data_get (GtkTreeDragSource *drag_source,
                                     GtkTreePath        *path,
                                     GtkSelectionData   *selection_data)
{
    GtkTreeIter iter;
    GFile *location;
    gchar *uris[2] = {0, };
    gboolean ret;

    if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (drag_source), &iter, path))
    {
        return FALSE;
    }

    gtk_tree_model_get (GTK_TREE_MODEL (drag_source), &iter, XED_FILE_BROWSER_STORE_COLUMN_LOCATION, &location, -1);

    g_assert (location);

    uris[0] = g_file_get_uri (location);
    ret = gtk_selection_data_set_uris (selection_data, uris);

    g_free (uris[0]);
    g_object_unref (location);

    return ret;
}

#define FILTER_HIDDEN(mode) (mode & XED_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN)
#define FILTER_BINARY(mode) (mode & XED_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY)

/* Private */
static void
model_begin_loading (XedFileBrowserStore *model,
                     FileBrowserNode     *node)
{
    GtkTreeIter iter;

    iter.user_data = node;
    g_signal_emit (model, model_signals[BEGIN_LOADING], 0, &iter);
}

static void
model_end_loading (XedFileBrowserStore *model,
                   FileBrowserNode     *node)
{
    GtkTreeIter iter;

    iter.user_data = node;
    g_signal_emit (model, model_signals[END_LOADING], 0, &iter);
}

static void
model_node_update_visibility (XedFileBrowserStore *model,
                              FileBrowserNode     *node)
{
    GtkTreeIter iter;

    node->flags &= ~XED_FILE_BROWSER_STORE_FLAG_IS_FILTERED;

    if (FILTER_HIDDEN (model->priv->filter_mode) && NODE_IS_HIDDEN (node))
    {
        node->flags |= XED_FILE_BROWSER_STORE_FLAG_IS_FILTERED;
    }
    else if (FILTER_BINARY (model->priv->filter_mode) && (!NODE_IS_TEXT (node) && !NODE_IS_DIR (node)))
    {
        node->flags |= XED_FILE_BROWSER_STORE_FLAG_IS_FILTERED;
    }
    else if (model->priv->filter_func)
    {
        iter.user_data = node;

        if (!model->priv->filter_func (model, &iter, model->priv->filter_user_data))
        {
            node->flags |= XED_FILE_BROWSER_STORE_FLAG_IS_FILTERED;
        }
    }
}

static gint
collate_nodes (FileBrowserNode *node1,
               FileBrowserNode *node2)
{
    if (node1->name == NULL)
    {
        return -1;
    }
    else if (node2->name == NULL)
    {
        return 1;
    }
    else
    {
        gchar *k1, *k2;
        gint result;

        k1 = g_utf8_collate_key_for_filename (node1->name, -1);
        k2 = g_utf8_collate_key_for_filename (node2->name, -1);

        result = strcmp (k1, k2);

        g_free (k1);
        g_free (k2);

        return result;
    }
}

static gint
model_sort_default (FileBrowserNode *node1,
                    FileBrowserNode *node2)
{
    gint f1;
    gint f2;

    f1 = NODE_IS_DUMMY (node1);
    f2 = NODE_IS_DUMMY (node2);

    if (f1 && f2)
    {
        return 0;
    }
    else if (f1 || f2)
    {
        return f1 ? -1 : 1;
    }

    f1 = NODE_IS_DIR (node1);
    f2 = NODE_IS_DIR (node2);

    if (f1 != f2)
    {
        return f1 ? -1 : 1;
    }

    f1 = NODE_IS_HIDDEN (node1);
    f2 = NODE_IS_HIDDEN (node2);

    if (f1 != f2)
    {
        return f2 ? -1 : 1;
    }

    return collate_nodes (node1, node2);
}

static void
model_resort_node (XedFileBrowserStore *model,
                   FileBrowserNode     *node)
{
    FileBrowserNodeDir *dir;
    GSList *item;
    FileBrowserNode *child;
    gint pos = 0;
    GtkTreeIter iter;
    GtkTreePath *path;
    gint *neworder;

    dir = FILE_BROWSER_NODE_DIR (node->parent);

    if (!model_node_visibility (model, node->parent))
    {
        /* Just sort the children of the parent */
        dir->children = g_slist_sort (dir->children, (GCompareFunc) (model->priv->sort_func));
    }
    else
    {
        /* Store current positions */
        for (item = dir->children; item; item = item->next)
        {
            child = (FileBrowserNode *) (item->data);

            if (model_node_visibility (model, child))
            {
                child->pos = pos++;
            }
        }

        dir->children = g_slist_sort (dir->children, (GCompareFunc) (model->priv->sort_func));
        neworder = g_new (gint, pos);
        pos = 0;

        /* Store the new positions */
        for (item = dir->children; item; item = item->next)
        {
            child = (FileBrowserNode *) (item->data);

            if (model_node_visibility (model, child))
            {
                neworder[pos++] = child->pos;
            }
        }

        iter.user_data = node->parent;
        path = xed_file_browser_store_get_path_real (model, node->parent);

        gtk_tree_model_rows_reordered (GTK_TREE_MODEL (model), path, &iter, neworder);

        g_free (neworder);
        gtk_tree_path_free (path);
    }
}

static void
row_changed (XedFileBrowserStore *model,
             GtkTreePath         **path,
             GtkTreeIter         *iter)
{
    GtkTreeRowReference *ref = gtk_tree_row_reference_new (GTK_TREE_MODEL (model), *path);

    /* Insert a copy of the actual path here because the row-inserted
       signal may alter the path */
    gtk_tree_model_row_changed (GTK_TREE_MODEL(model), *path, iter);
    gtk_tree_path_free (*path);

    *path = gtk_tree_row_reference_get_path (ref);
    gtk_tree_row_reference_free (ref);
}

static void
row_inserted (XedFileBrowserStore *model,
              GtkTreePath         **path,
              GtkTreeIter         *iter)
{
    /* This function creates a row reference for the path because it's
       uncertain what might change the actual model/view when we insert
       a node, maybe another directory load is triggered for example.
       Because functions that use this function rely on the notion that
       the path remains pointed towards the inserted node, we use the
       reference to keep track. */
    GtkTreeRowReference *ref = gtk_tree_row_reference_new (GTK_TREE_MODEL (model), *path);
    GtkTreePath * copy = gtk_tree_path_copy (*path);

    gtk_tree_model_row_inserted (GTK_TREE_MODEL(model), copy, iter);
    gtk_tree_path_free (copy);

    if (ref)
    {
        gtk_tree_path_free (*path);

        /* To restore the path, we get the path from the reference. But, since
           we inserted a row, the path will be one index further than the
           actual path of our node. We therefore call gtk_tree_path_prev */
        *path = gtk_tree_row_reference_get_path (ref);
        gtk_tree_path_prev (*path);
    }

    gtk_tree_row_reference_free (ref);
}

static void
row_deleted (XedFileBrowserStore *model,
             const GtkTreePath   *path)
{
    GtkTreePath *copy = gtk_tree_path_copy (path);

    /* Delete a copy of the actual path here because the row-deleted
       signal may alter the path */
    gtk_tree_model_row_deleted (GTK_TREE_MODEL(model), copy);
    gtk_tree_path_free (copy);
}

static void
model_refilter_node (XedFileBrowserStore *model,
                     FileBrowserNode     *node,
                     GtkTreePath        **path)
{
    gboolean old_visible;
    gboolean new_visible;
    FileBrowserNodeDir *dir;
    GSList *item;
    GtkTreeIter iter;
    GtkTreePath *tmppath = NULL;
    gboolean in_tree;

    if (node == NULL)
    {
        return;
    }

    old_visible = model_node_visibility (model, node);
    model_node_update_visibility (model, node);

    in_tree = node_in_tree (model, node);

    if (path == NULL)
    {
        if (in_tree)
        {
            tmppath = xed_file_browser_store_get_path_real (model, node);
        }
        else
        {
            tmppath = gtk_tree_path_new_first ();
        }

        path = &tmppath;
    }

    if (NODE_IS_DIR (node))
    {
        if (in_tree)
        {
            gtk_tree_path_down (*path);
        }

        dir = FILE_BROWSER_NODE_DIR (node);

        for (item = dir->children; item; item = item->next)
        {
            model_refilter_node (model, (FileBrowserNode *) (item->data), path);
        }

        if (in_tree)
        {
            gtk_tree_path_up (*path);
        }
    }

    if (in_tree)
    {
        new_visible = model_node_visibility (model, node);

        if (old_visible != new_visible)
        {
            if (old_visible)
            {
                node->inserted = FALSE;
                row_deleted (model, *path);
            }
            else
            {
                iter.user_data = node;
                row_inserted (model, path, &iter);
                gtk_tree_path_next (*path);
            }
        }
        else if (old_visible)
        {
            gtk_tree_path_next (*path);
        }
    }

    model_check_dummy (model, node);

    if (tmppath)
    {
        gtk_tree_path_free (tmppath);
    }
}

static void
model_refilter (XedFileBrowserStore *model)
{
    model_refilter_node (model, model->priv->root, NULL);
}

static void
file_browser_node_set_name (FileBrowserNode *node)
{
    g_free (node->name);

    if (node->file)
    {
        node->name = xed_file_browser_utils_file_basename (node->file);
    }
    else
    {
        node->name = NULL;
    }
}

static void
file_browser_node_init (FileBrowserNode *node,
                        GFile           *file,
                        FileBrowserNode *parent)
{
    if (file != NULL)
    {
        node->file = g_object_ref (file);
        file_browser_node_set_name (node);
    }

    node->parent = parent;
}

static FileBrowserNode *
file_browser_node_new (GFile           *file,
                       FileBrowserNode *parent)
{
    FileBrowserNode *node = g_slice_new0 (FileBrowserNode);

    file_browser_node_init (node, file, parent);
    return node;
}

static FileBrowserNode *
file_browser_node_dir_new (XedFileBrowserStore *model,
                           GFile               *file,
                           FileBrowserNode     *parent)
{
    FileBrowserNode *node = (FileBrowserNode *) g_slice_new0 (FileBrowserNodeDir);

    file_browser_node_init (node, file, parent);

    node->flags |= XED_FILE_BROWSER_STORE_FLAG_IS_DIRECTORY;

    FILE_BROWSER_NODE_DIR (node)->model = model;

    return node;
}

static void
file_browser_node_free_children (XedFileBrowserStore *model,
                                 FileBrowserNode     *node)
{
    GSList *item;

    if (node == NULL)
    {
        return;
    }

    if (NODE_IS_DIR (node))
    {
        for (item = FILE_BROWSER_NODE_DIR (node)->children; item; item = item->next)
        {
            file_browser_node_free (model, (FileBrowserNode *) (item->data));
        }

        g_slist_free (FILE_BROWSER_NODE_DIR (node)->children);
        FILE_BROWSER_NODE_DIR (node)->children = NULL;

        /* This node is no longer loaded */
        node->flags &= ~XED_FILE_BROWSER_STORE_FLAG_LOADED;
    }
}

static void
file_browser_node_free (XedFileBrowserStore *model,
                        FileBrowserNode     *node)
{
    if (node == NULL)
    {
        return;
    }

    if (NODE_IS_DIR (node))
    {
        FileBrowserNodeDir *dir;

        dir = FILE_BROWSER_NODE_DIR (node);

        if (dir->cancellable)
        {
            g_cancellable_cancel (dir->cancellable);
            g_object_unref (dir->cancellable);

            model_end_loading (model, node);
        }

        file_browser_node_free_children (model, node);

        if (dir->monitor)
        {
            g_file_monitor_cancel (dir->monitor);
            g_object_unref (dir->monitor);
        }
    }

    if (node->file)
    {
        g_signal_emit (model, model_signals[UNLOAD], 0, node->file);
        g_object_unref (node->file);
    }

    if (node->icon)
    {
        g_object_unref (node->icon);
    }

    if (node->emblem)
    {
        g_object_unref (node->emblem);
    }

    g_free (node->name);

    if (NODE_IS_DIR (node))
    {
        g_slice_free (FileBrowserNodeDir, (FileBrowserNodeDir *)node);
    }
    else
    {
        g_slice_free (FileBrowserNode, (FileBrowserNode *)node);
    }
}

/**
 * model_remove_node_children:
 * @model: the #XedFileBrowserStore
 * @node: the FileBrowserNode to remove
 * @path: the path of the node, or NULL to let the path be calculated
 * @free_nodes: whether to also remove the nodes from memory
 *
 * Removes all the children of node from the model. This function is used
 * to remove the child nodes from the _model_. Don't use it to just free
 * a node.
 **/
static void
model_remove_node_children (XedFileBrowserStore *model,
                            FileBrowserNode     *node,
                            GtkTreePath         *path,
                            gboolean             free_nodes)
{
    FileBrowserNodeDir *dir;
    GtkTreePath *path_child;
    GSList *list;
    GSList *item;

    if (node == NULL || !NODE_IS_DIR (node))
    {
        return;
    }

    dir = FILE_BROWSER_NODE_DIR (node);

    if (dir->children == NULL)
    {
        return;
    }

    if (!model_node_visibility (model, node))
    {
        // Node is invisible and therefore the children can just
        // be freed
        if (free_nodes)
        {
            file_browser_node_free_children (model, node);
        }

        return;
    }

    if (path == NULL)
    {
        path_child = xed_file_browser_store_get_path_real (model, node);
    }
    else
    {
        path_child = gtk_tree_path_copy (path);
    }

    gtk_tree_path_down (path_child);

    list = g_slist_copy (dir->children);

    for (item = list; item; item = item->next)
    {
        model_remove_node (model, (FileBrowserNode *) (item->data), path_child, free_nodes);
    }

    g_slist_free (list);
    gtk_tree_path_free (path_child);
}

/**
 * model_remove_node:
 * @model: the #XedFileBrowserStore
 * @node: the FileBrowserNode to remove
 * @path: the path to use to remove this node, or NULL to use the path
 * calculated from the node itself
 * @free_nodes: whether to also remove the nodes from memory
 *
 * Removes this node and all its children from the model. This function is used
 * to remove the node from the _model_. Don't use it to just free
 * a node.
 **/
static void
model_remove_node (XedFileBrowserStore *model,
                   FileBrowserNode     *node,
                   GtkTreePath         *path,
                   gboolean             free_nodes)
{
    gboolean free_path = FALSE;
    FileBrowserNode *parent;

    if (path == NULL)
    {
        path = xed_file_browser_store_get_path_real (model, node);
        free_path = TRUE;
    }

    model_remove_node_children (model, node, path, free_nodes);

    /* Only delete if the node is visible in the tree (but only when it's
       not the virtual root) */
    if (model_node_visibility (model, node) && node != model->priv->virtual_root)
    {
        node->inserted = FALSE;
        row_deleted (model, path);
    }

    if (free_path)
    {
        gtk_tree_path_free (path);
    }

    parent = node->parent;

    if (free_nodes)
    {
        /* Remove the node from the parents children list */
        if (parent)
        {
            FILE_BROWSER_NODE_DIR (node->parent)->children = g_slist_remove (FILE_BROWSER_NODE_DIR
                                                                             (node->parent)->children,
                                                                             node);
        }
    }

    /* If this is the virtual root, than set the parent as the virtual root */
    if (node == model->priv->virtual_root)
    {
        set_virtual_root_from_node (model, parent);
    }
    else if (parent && model_node_visibility (model, parent) && !(free_nodes && NODE_IS_DUMMY(node)))
    {
        model_check_dummy (model, parent);
    }

    /* Now free the node if necessary */
    if (free_nodes)
    {
        file_browser_node_free (model, node);
    }
}

/**
 * model_clear:
 * @model: the #XedFileBrowserStore
 * @free_nodes: whether to also remove the nodes from memory
 *
 * Removes all nodes from the model. This function is used
 * to remove all the nodes from the _model_. Don't use it to just free the
 * nodes in the model.
 **/
static void
model_clear (XedFileBrowserStore *model,
             gboolean             free_nodes)
{
    GtkTreePath *path;
    FileBrowserNodeDir *dir;
    FileBrowserNode *dummy;

    path = gtk_tree_path_new ();
    model_remove_node_children (model, model->priv->virtual_root, path, free_nodes);
    gtk_tree_path_free (path);

    /* Remove the dummy if there is one */
    if (model->priv->virtual_root)
    {
        dir = FILE_BROWSER_NODE_DIR (model->priv->virtual_root);

        if (dir->children != NULL)
        {
            dummy = (FileBrowserNode *) (dir->children->data);

            if (NODE_IS_DUMMY (dummy) && model_node_visibility (model, dummy))
            {
                path = gtk_tree_path_new_first ();

                dummy->inserted = FALSE;
                row_deleted (model, path);
                gtk_tree_path_free (path);
            }
        }
    }
}

static void
file_browser_node_unload (XedFileBrowserStore *model,
                          FileBrowserNode     *node,
                          gboolean             remove_children)
{
    FileBrowserNodeDir *dir;

    if (node == NULL)
    {
        return;
    }

    if (!NODE_IS_DIR (node) || !NODE_LOADED (node))
    {
        return;
    }

    dir = FILE_BROWSER_NODE_DIR (node);

    if (remove_children)
    {
        model_remove_node_children (model, node, NULL, TRUE);
    }

    if (dir->cancellable)
    {
        g_cancellable_cancel (dir->cancellable);
        g_object_unref (dir->cancellable);

        model_end_loading (model, node);
        dir->cancellable = NULL;
    }

    if (dir->monitor)
    {
        g_file_monitor_cancel (dir->monitor);
        g_object_unref (dir->monitor);

        dir->monitor = NULL;
    }

    node->flags &= ~XED_FILE_BROWSER_STORE_FLAG_LOADED;
}

static void
model_recomposite_icon_real (XedFileBrowserStore *tree_model,
                             FileBrowserNode     *node,
                             GFileInfo           *info)
{
    GdkPixbuf *icon;

    g_return_if_fail (XED_IS_FILE_BROWSER_STORE (tree_model));
    g_return_if_fail (node != NULL);

    if (node->file == NULL)
    {
        return;
    }

    if (info)
    {
        GIcon *gicon = g_file_info_get_icon (info);
        if (gicon != NULL)
        {
            icon = xed_file_browser_utils_pixbuf_from_icon (gicon, GTK_ICON_SIZE_MENU);
        }
        else
        {
            icon = NULL;
        }
    }
    else
    {
        icon = xed_file_browser_utils_pixbuf_from_file (node->file, GTK_ICON_SIZE_MENU);
    }

    if (node->icon)
    {
        g_object_unref (node->icon);
    }

    if (node->emblem)
    {
        gint icon_size;

        gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, NULL, &icon_size);

        if (icon == NULL)
        {
            node->icon = gdk_pixbuf_new (gdk_pixbuf_get_colorspace (node->emblem),
                                         gdk_pixbuf_get_has_alpha (node->emblem),
                                         gdk_pixbuf_get_bits_per_sample (node->emblem),
                                         icon_size,
                                         icon_size);
        }
        else
        {
            node->icon = gdk_pixbuf_copy (icon);
            g_object_unref (icon);
        }

        gdk_pixbuf_composite (node->emblem, node->icon,
                              icon_size - 10, icon_size - 10, 10,
                              10, icon_size - 10, icon_size - 10,
                              1, 1, GDK_INTERP_NEAREST, 255);
    }
    else
    {
        node->icon = icon;
    }
}

static void
model_recomposite_icon (XedFileBrowserStore *tree_model,
                        GtkTreeIter         *iter)
{
    g_return_if_fail (XED_IS_FILE_BROWSER_STORE (tree_model));
    g_return_if_fail (iter != NULL);
    g_return_if_fail (iter->user_data != NULL);

    model_recomposite_icon_real (tree_model, (FileBrowserNode *) (iter->user_data), NULL);
}

static FileBrowserNode *
model_create_dummy_node (XedFileBrowserStore *model,
                         FileBrowserNode     *parent)
{
    FileBrowserNode *dummy;

    dummy = file_browser_node_new (NULL, parent);
    dummy->name = g_strdup (_("(Empty)"));

    dummy->flags |= XED_FILE_BROWSER_STORE_FLAG_IS_DUMMY;
    dummy->flags |= XED_FILE_BROWSER_STORE_FLAG_IS_HIDDEN;

    return dummy;
}

static FileBrowserNode *
model_add_dummy_node (XedFileBrowserStore *model,
                      FileBrowserNode     *parent)
{
    FileBrowserNode *dummy;

    dummy = model_create_dummy_node (model, parent);

    if (model_node_visibility (model, parent))
    {
        dummy->flags &= ~XED_FILE_BROWSER_STORE_FLAG_IS_HIDDEN;
    }

    model_add_node (model, dummy, parent);

    return dummy;
}

static void
model_check_dummy (XedFileBrowserStore *model,
                   FileBrowserNode     *node)
{
    // Hide the dummy child if needed
    if (NODE_IS_DIR (node))
    {
        FileBrowserNode *dummy;
        GtkTreeIter iter;
        GtkTreePath *path;
        guint flags;
        FileBrowserNodeDir *dir;

        dir = FILE_BROWSER_NODE_DIR (node);

        if (dir->children == NULL)
        {
            model_add_dummy_node (model, node);
            return;
        }

        dummy = (FileBrowserNode *) (dir->children->data);

        if (!NODE_IS_DUMMY (dummy))
        {
            dummy = model_create_dummy_node (model, node);
            dir->children = g_slist_prepend (dir->children, dummy);
        }

        if (!model_node_visibility (model, node))
        {
            dummy->flags |= XED_FILE_BROWSER_STORE_FLAG_IS_HIDDEN;
            return;
        }

        /* Temporarily set the node to invisible to check
         * for real children */
        flags = dummy->flags;
        dummy->flags |= XED_FILE_BROWSER_STORE_FLAG_IS_HIDDEN;

        if (!filter_tree_model_iter_has_child_real (model, node))
        {
            dummy->flags &= ~XED_FILE_BROWSER_STORE_FLAG_IS_HIDDEN;

            if (FILE_IS_HIDDEN (flags))
            {
                // Was hidden, needs to be inserted
                iter.user_data = dummy;
                path = xed_file_browser_store_get_path_real (model, dummy);

                row_inserted (model, &path, &iter);
                gtk_tree_path_free (path);
            }
        }
        else
        {
            if (!FILE_IS_HIDDEN (flags))
            {
                // Was shown, needs to be removed

                // To get the path we need to set it to visible temporarily
                dummy->flags &= ~XED_FILE_BROWSER_STORE_FLAG_IS_HIDDEN;
                path = xed_file_browser_store_get_path_real (model, dummy);
                dummy->flags |= XED_FILE_BROWSER_STORE_FLAG_IS_HIDDEN;

                dummy->inserted = FALSE;
                row_deleted (model, path);
                gtk_tree_path_free (path);
            }
        }
    }
}

static void
insert_node_sorted (XedFileBrowserStore *model,
                    FileBrowserNode     *child,
                    FileBrowserNode     *parent)
{
    FileBrowserNodeDir *dir;

    dir = FILE_BROWSER_NODE_DIR (parent);

    if (model->priv->sort_func == NULL)
    {
        dir->children = g_slist_append (dir->children, child);
    }
    else
    {
        dir->children = g_slist_insert_sorted (dir->children, child, (GCompareFunc) (model->priv->sort_func));
    }
}

static void
model_add_node (XedFileBrowserStore *model,
                FileBrowserNode     *child,
                FileBrowserNode     *parent)
{
    /* Add child to parents children */
    insert_node_sorted (model, child, parent);

    if (model_node_visibility (model, parent) && model_node_visibility (model, child))
    {
        GtkTreeIter iter;
        GtkTreePath *path;

        iter.user_data = child;
        path = xed_file_browser_store_get_path_real (model, child);

        /* Emit row inserted */
        row_inserted (model, &path, &iter);
        gtk_tree_path_free (path);
    }

    model_check_dummy (model, parent);
    model_check_dummy (model, child);
}

static void
model_add_nodes_batch (XedFileBrowserStore *model,
                       GSList              *children,
                       FileBrowserNode     *parent)
{
    GSList *sorted_children;
    GSList *child;
    GSList *prev;
    GSList *l;
    FileBrowserNodeDir *dir;

    dir = FILE_BROWSER_NODE_DIR (parent);

    sorted_children = g_slist_sort (children, (GCompareFunc) model->priv->sort_func);

    child = sorted_children;
    l = dir->children;
    prev = NULL;

    model_check_dummy (model, parent);

    while (child)
    {
        FileBrowserNode *node = child->data;
        GtkTreeIter iter;
        GtkTreePath *path;

        /* reached the end of the first list, just append the second */
        if (l == NULL)
        {

            dir->children = g_slist_concat (dir->children, child);

            for (l = child; l; l = l->next)
            {
                if (model_node_visibility (model, parent) && model_node_visibility (model, l->data))
                {
                    iter.user_data = l->data;
                    path = xed_file_browser_store_get_path_real (model, l->data);

                    // Emit row inserted
                    row_inserted (model, &path, &iter);
                    gtk_tree_path_free (path);
                }

                model_check_dummy (model, l->data);
            }

            break;
        }

        if (model->priv->sort_func (l->data, node) > 0)
        {
            GSList *next_child;

            if (prev == NULL)
            {
                /* prepend to the list */
                dir->children = g_slist_prepend (dir->children, child);
            }
            else
            {
                prev->next = child;
            }

            next_child = child->next;
            prev = child;
            child->next = l;
            child = next_child;

            if (model_node_visibility (model, parent) && model_node_visibility (model, node))
            {
                iter.user_data = node;
                path = xed_file_browser_store_get_path_real (model, node);

                // Emit row inserted
                row_inserted (model, &path, &iter);
                gtk_tree_path_free (path);
            }

            model_check_dummy (model, node);

            /* try again at the same l position with the
             * next child */
        }
        else
        {

            /* Move to the next item in the list */
            prev = l;
            l = l->next;
        }
    }
}

static gchar const *
backup_content_type (GFileInfo *info)
{
    gchar const * content;

    if (!g_file_info_get_is_backup (info))
    {
        return NULL;
    }

    content = g_file_info_get_content_type (info);

    if (!content || g_content_type_equals (content, "application/x-trash"))
    {
        return "text/plain";
    }

    return content;
}

static void
file_browser_node_set_from_info (XedFileBrowserStore *model,
                                 FileBrowserNode     *node,
                                 GFileInfo           *info,
                                 gboolean             isadded)
{
    gchar const *content;
    gboolean free_info = FALSE;
    GtkTreePath *path;
    gchar *uri;
    GError *error = NULL;

    if (info == NULL)
    {
        info = g_file_query_info (node->file,
                                  STANDARD_ATTRIBUTE_TYPES,
                                  G_FILE_QUERY_INFO_NONE,
                                  NULL,
                                  &error);

        if (!info)
        {
            if (!(error->domain == G_IO_ERROR && error->code == G_IO_ERROR_NOT_FOUND))
            {
                uri = g_file_get_uri (node->file);
                g_warning ("Could not get info for %s: %s", uri, error->message);
                g_free (uri);
            }
            g_error_free (error);

            return;
        }

        free_info = TRUE;
    }

    if (g_file_info_get_is_hidden (info) || g_file_info_get_is_backup (info))
    {
        node->flags |= XED_FILE_BROWSER_STORE_FLAG_IS_HIDDEN;
    }

    if (g_file_info_get_file_type (info) == G_FILE_TYPE_DIRECTORY)
    {
        node->flags |= XED_FILE_BROWSER_STORE_FLAG_IS_DIRECTORY;
    }
    else
    {
        if (!(content = backup_content_type (info)))
        {
            content = g_file_info_get_content_type (info);
        }

        if (!content ||
            g_content_type_is_unknown (content) ||
            g_content_type_is_a (content, "text/plain"))
        {
            node->flags |= XED_FILE_BROWSER_STORE_FLAG_IS_TEXT;
        }
    }

    model_recomposite_icon_real (model, node, info);

    if (free_info)
    {
        g_object_unref (info);
    }

    if (isadded)
    {
        path = xed_file_browser_store_get_path_real (model, node);
        model_refilter_node (model, node, &path);
        gtk_tree_path_free (path);

        model_check_dummy (model, node->parent);
    }
    else
    {
        model_node_update_visibility (model, node);
    }
}

static FileBrowserNode *
node_list_contains_file (GSList *children,
                         GFile  *file)
{
    GSList *item;

    for (item = children; item; item = item->next)
    {
        FileBrowserNode *node;

        node = (FileBrowserNode *) (item->data);

        if (node->file != NULL && g_file_equal (node->file, file))
        {
            return node;
        }
    }

    return NULL;
}

static FileBrowserNode *
model_add_node_from_file (XedFileBrowserStore *model,
                          FileBrowserNode     *parent,
                          GFile               *file,
                          GFileInfo           *info)
{
    FileBrowserNode *node;
    gboolean free_info = FALSE;
    GError * error = NULL;

    if ((node = node_list_contains_file (FILE_BROWSER_NODE_DIR (parent)->children, file)) == NULL)
    {
        if (info == NULL)
        {
            info = g_file_query_info (file,
                                      STANDARD_ATTRIBUTE_TYPES,
                                      G_FILE_QUERY_INFO_NONE,
                                      NULL,
                                      &error);
            free_info = TRUE;
        }

        if (!info)
        {
            g_warning ("Error querying file info: %s", error->message);
            g_error_free (error);

            /* FIXME: What to do now then... */
            node = file_browser_node_new (file, parent);
        }
        else if (g_file_info_get_file_type (info) == G_FILE_TYPE_DIRECTORY)
        {
            node = file_browser_node_dir_new (model, file, parent);
        }
        else
        {
            node = file_browser_node_new (file, parent);
        }

        file_browser_node_set_from_info (model, node, info, FALSE);
        model_add_node (model, node, parent);

        if (info && free_info)
        {
            g_object_unref (info);
        }
    }

    return node;
}

/* We pass in a copy of the list of parent->children so that we do
 * not have to check if a file already exists among the ones we just
 * added */
static void
model_add_nodes_from_files (XedFileBrowserStore *model,
                            FileBrowserNode     *parent,
                            GSList              *original_children,
                            GList               *files)
{
    GList *item;
    GSList *nodes = NULL;

    for (item = files; item; item = item->next)
    {
        GFileInfo *info = G_FILE_INFO (item->data);
        GFileType type;
        gchar const *name;
        GFile *file;
        FileBrowserNode *node;

        type = g_file_info_get_file_type (info);

        /* Skip all non regular, non directory files */
        if (type != G_FILE_TYPE_REGULAR &&
            type != G_FILE_TYPE_DIRECTORY &&
            type != G_FILE_TYPE_SYMBOLIC_LINK)
        {
            g_object_unref (info);
            continue;
        }

        name = g_file_info_get_name (info);

        /* Skip '.' and '..' directories */
        if (type == G_FILE_TYPE_DIRECTORY &&
            (strcmp (name, ".") == 0 ||
             strcmp (name, "..") == 0))
        {
            g_object_unref (info);
            continue;
        }

        file = g_file_get_child (parent->file, name);

        node = node_list_contains_file (original_children, file);
        if (node == NULL)
        {

            if (type == G_FILE_TYPE_DIRECTORY)
            {
                node = file_browser_node_dir_new (model, file, parent);
            }
            else
            {
                node = file_browser_node_new (file, parent);
            }

            file_browser_node_set_from_info (model, node, info, FALSE);

            nodes = g_slist_prepend (nodes, node);
        }

        g_object_unref (file);
        g_object_unref (info);
    }

    if (nodes)
    {
        model_add_nodes_batch (model, nodes, parent);
    }
}

static FileBrowserNode *
model_add_node_from_dir (XedFileBrowserStore *model,
                         FileBrowserNode     *parent,
                         GFile               *file)
{
    FileBrowserNode *node;

    /* Check if it already exists */
    if ((node = node_list_contains_file (FILE_BROWSER_NODE_DIR (parent)->children, file)) == NULL)
    {
        node = file_browser_node_dir_new (model, file, parent);
        file_browser_node_set_from_info (model, node, NULL, FALSE);

        if (node->name == NULL)
        {
            file_browser_node_set_name (node);
        }

        if (node->icon == NULL)
        {
            node->icon = xed_file_browser_utils_pixbuf_from_theme ("folder", GTK_ICON_SIZE_MENU);
        }

        model_add_node (model, node, parent);
    }

    return node;
}

static void
on_directory_monitor_event (GFileMonitor      *monitor,
                            GFile             *file,
                            GFile             *other_file,
                            GFileMonitorEvent  event_type,
                            FileBrowserNode   *parent)
{
    FileBrowserNode *node;
    FileBrowserNodeDir *dir = FILE_BROWSER_NODE_DIR (parent);

    switch (event_type)
    {
        case G_FILE_MONITOR_EVENT_DELETED:
            node = node_list_contains_file (dir->children, file);

            if (node != NULL)
            {
                model_remove_node (dir->model, node, NULL, TRUE);
            }
            break;
        case G_FILE_MONITOR_EVENT_CREATED:
            if (g_file_query_exists (file, NULL))
            {
                model_add_node_from_file (dir->model, parent, file, NULL);
            }

            break;
        default:
            break;
    }
}

static void
async_node_free (AsyncNode *async)
{
    g_object_unref (async->cancellable);
    g_slist_free (async->original_children);
    g_slice_free (AsyncNode, async);
}

static void
model_iterate_next_files_cb (GFileEnumerator *enumerator,
                             GAsyncResult    *result,
                             AsyncNode       *async)
{
    GList *files;
    GError *error = NULL;
    FileBrowserNodeDir *dir = async->dir;
    FileBrowserNode *parent = (FileBrowserNode *)dir;

    files = g_file_enumerator_next_files_finish (enumerator, result, &error);

    if (files == NULL)
    {
        g_file_enumerator_close (enumerator, NULL, NULL);
        g_object_unref (enumerator);
        async_node_free (async);

        if (!error)
        {
            /* We're done loading */
            g_object_unref (dir->cancellable);
            dir->cancellable = NULL;

/*
 * FIXME: This is temporarly, it is a bug in gio:
 * http://bugzilla.gnome.org/show_bug.cgi?id=565924
 */
            if (g_file_is_native (parent->file) && dir->monitor == NULL)
            {
                dir->monitor = g_file_monitor_directory (parent->file,
                                                         G_FILE_MONITOR_NONE,
                                                         NULL,
                                                         NULL);
                if (dir->monitor != NULL)
                {
                    g_signal_connect (dir->monitor, "changed",
                              G_CALLBACK (on_directory_monitor_event), parent);
                }
            }

            model_check_dummy (dir->model, parent);
            model_end_loading (dir->model, parent);
        }
        else
        {
            /* Simply return if we were cancelled */
            if (error->domain == G_IO_ERROR && error->code == G_IO_ERROR_CANCELLED)
            {
                return;
            }

            /* Otherwise handle the error appropriately */
            g_signal_emit (dir->model,
                           model_signals[ERROR],
                           0,
                           XED_FILE_BROWSER_ERROR_LOAD_DIRECTORY,
                           error->message);

            file_browser_node_unload (dir->model, (FileBrowserNode *)parent, TRUE);
            g_error_free (error);
        }
    }
    else if (g_cancellable_is_cancelled (async->cancellable))
    {
        /* Check cancel state manually */
        g_file_enumerator_close (enumerator, NULL, NULL);
        g_object_unref (enumerator);
        async_node_free (async);
    }
    else
    {
        model_add_nodes_from_files (dir->model, parent, async->original_children, files);

        g_list_free (files);
        next_files_async (enumerator, async);
    }
}

static void
next_files_async (GFileEnumerator *enumerator,
                  AsyncNode       *async)
{
    g_file_enumerator_next_files_async (enumerator,
                                        DIRECTORY_LOAD_ITEMS_PER_CALLBACK,
                                        G_PRIORITY_DEFAULT,
                                        async->cancellable,
                                        (GAsyncReadyCallback)model_iterate_next_files_cb,
                                        async);
}

static void
model_iterate_children_cb (GFile        *file,
                           GAsyncResult *result,
                           AsyncNode    *async)
{
    GError * error = NULL;
    GFileEnumerator * enumerator;

    if (g_cancellable_is_cancelled (async->cancellable))
    {
        async_node_free (async);
        return;
    }

    enumerator = g_file_enumerate_children_finish (file, result, &error);

    if (enumerator == NULL)
    {
        /* Simply return if we were cancelled or if the dir is not there */
        FileBrowserNodeDir *dir = async->dir;

        /* Otherwise handle the error appropriately */
        g_signal_emit (dir->model,
                       model_signals[ERROR],
                       0,
                       XED_FILE_BROWSER_ERROR_LOAD_DIRECTORY,
                       error->message);

        file_browser_node_unload (dir->model, (FileBrowserNode *)dir, TRUE);
        g_error_free (error);
        async_node_free (async);
    }
    else
    {
        next_files_async (enumerator, async);
    }
}

static void
model_load_directory (XedFileBrowserStore *model,
                      FileBrowserNode     *node)
{
    FileBrowserNodeDir *dir;
    AsyncNode *async;

    g_return_if_fail (NODE_IS_DIR (node));

    dir = FILE_BROWSER_NODE_DIR (node);

    /* Cancel a previous load */
    if (dir->cancellable != NULL)
    {
        file_browser_node_unload (dir->model, node, TRUE);
    }

    node->flags |= XED_FILE_BROWSER_STORE_FLAG_LOADED;
    model_begin_loading (model, node);

    dir->cancellable = g_cancellable_new ();

    async = g_slice_new (AsyncNode);
    async->dir = dir;
    async->cancellable = g_object_ref (dir->cancellable);
    async->original_children = g_slist_copy (dir->children);

    /* Start loading async */
    g_file_enumerate_children_async (node->file,
                                     STANDARD_ATTRIBUTE_TYPES,
                                     G_FILE_QUERY_INFO_NONE,
                                     G_PRIORITY_DEFAULT,
                                     async->cancellable,
                                     (GAsyncReadyCallback)model_iterate_children_cb,
                                     async);
}

static GList *
get_parent_files (XedFileBrowserStore *model,
                  GFile               *file)
{
    GList * result = NULL;

    result = g_list_prepend (result, g_object_ref (file));

    while ((file = g_file_get_parent (file)))
    {
        if (g_file_equal (file, model->priv->root->file))
        {
            g_object_unref (file);
            break;
        }

        result = g_list_prepend (result, file);
    }

    return result;
}

static void
model_fill (XedFileBrowserStore *model,
            FileBrowserNode     *node,
            GtkTreePath        **path)
{
    gboolean free_path = FALSE;
    GtkTreeIter iter = {0,};
    GSList *item;
    FileBrowserNode *child;

    if (node == NULL)
    {
        node = model->priv->virtual_root;
        *path = gtk_tree_path_new ();
        free_path = TRUE;
    }

    if (*path == NULL)
    {
        *path = xed_file_browser_store_get_path_real (model, node);
        free_path = TRUE;
    }

    if (!model_node_visibility (model, node))
    {
        if (free_path)
        {
            gtk_tree_path_free (*path);
        }

        return;
    }

    if (node != model->priv->virtual_root)
    {
        /* Insert node */
        iter.user_data = node;

        row_inserted(model, path, &iter);
    }

    if (NODE_IS_DIR (node))
    {
        /* Go to the first child */
        gtk_tree_path_down (*path);

        for (item = FILE_BROWSER_NODE_DIR (node)->children; item; item = item->next) {
            child = (FileBrowserNode *) (item->data);

            if (model_node_visibility (model, child))
            {
                model_fill (model, child, path);

                /* Increase path for next child */
                gtk_tree_path_next (*path);
            }
        }

        /* Move back up to node path */
        gtk_tree_path_up (*path);
    }

    model_check_dummy (model, node);

    if (free_path)
    {
        gtk_tree_path_free (*path);
    }
}

static void
set_virtual_root_from_node (XedFileBrowserStore *model,
                            FileBrowserNode     *node)
{
    FileBrowserNode *next;
    FileBrowserNode *prev;
    FileBrowserNode *check;
    FileBrowserNodeDir *dir;
    GSList *item;
    GSList *copy;
    GtkTreePath *empty = NULL;

    g_assert (node != NULL);

    prev = node;
    next = prev->parent;

    /* Free all the nodes below that we don't need in cache */
    while (prev != model->priv->root)
    {
        dir = FILE_BROWSER_NODE_DIR (next);
        copy = g_slist_copy (dir->children);

        for (item = copy; item; item = item->next)
        {
            check = (FileBrowserNode *) (item->data);

            if (prev == node)
            {
                /* Only free the children, keeping this depth in cache */
                if (check != node)
                {
                    file_browser_node_free_children (model, check);
                    file_browser_node_unload (model, check, FALSE);
                }
            }
            else if (check != prev)
            {
                /* Only free when the node is not in the chain */
                dir->children = g_slist_remove (dir->children, check);
                file_browser_node_free (model, check);
            }
        }

        if (prev != node)
        {
            file_browser_node_unload (model, next, FALSE);
        }

        g_slist_free (copy);
        prev = next;
        next = prev->parent;
    }

    /* Free all the nodes up that we don't need in cache */
    for (item = FILE_BROWSER_NODE_DIR (node)->children; item; item = item->next)
    {
        check = (FileBrowserNode *) (item->data);

        if (NODE_IS_DIR (check))
        {
            for (copy = FILE_BROWSER_NODE_DIR (check)->children; copy; copy = copy->next)
            {
                file_browser_node_free_children (model, (FileBrowserNode *) (copy->data));
                file_browser_node_unload (model, (FileBrowserNode *) (copy->data), FALSE);
            }
        }
        else if (NODE_IS_DUMMY (check))
        {
            check->flags |= XED_FILE_BROWSER_STORE_FLAG_IS_HIDDEN;
        }
    }

    /* Now finally, set the virtual root, and load it up! */
    model->priv->virtual_root = node;

    /* Notify that the virtual-root has changed before loading up new nodes so that the
       "root_changed" signal can be emitted before any "inserted" signals */
    g_object_notify (G_OBJECT (model), "virtual-root");

    model_fill (model, NULL, &empty);

    if (!NODE_LOADED (node))
    {
        model_load_directory (model, node);
    }
}

static void
set_virtual_root_from_file (XedFileBrowserStore *model,
                            GFile               *file)
{
    GList * files;
    GList * item;
    FileBrowserNode * parent;
    GFile * check;

    /* Always clear the model before altering the nodes */
    model_clear (model, FALSE);

    /* Create the node path, get all the uri's */
    files = get_parent_files (model, file);
    parent = model->priv->root;

    for (item = files; item; item = item->next)
    {
        check = G_FILE (item->data);

        parent = model_add_node_from_dir (model, parent, check);
        g_object_unref (check);
    }

    g_list_free (files);
    set_virtual_root_from_node (model, parent);
}

static FileBrowserNode *
model_find_node_children (XedFileBrowserStore *model,
                          FileBrowserNode     *parent,
                          GFile               *file)
{
    FileBrowserNodeDir *dir;
    FileBrowserNode *child;
    FileBrowserNode *result;
    GSList *children;

    if (!NODE_IS_DIR (parent))
    {
        return NULL;
    }

    dir = FILE_BROWSER_NODE_DIR (parent);

    for (children = dir->children; children; children = children->next)
    {
        child = (FileBrowserNode *)(children->data);

        result = model_find_node (model, child, file);

        if (result)
        {
            return result;
        }
    }

    return NULL;
}

static FileBrowserNode *
model_find_node (XedFileBrowserStore *model,
                 FileBrowserNode     *node,
                 GFile               *file)
{
    if (node == NULL)
    {
        node = model->priv->root;
    }

    if (node->file && g_file_equal (node->file, file))
    {
        return node;
    }

    if (NODE_IS_DIR (node) && g_file_has_prefix (file, node->file))
    {
        return model_find_node_children (model, node, file);
    }

    return NULL;
}

static GQuark
xed_file_browser_store_error_quark (void)
{
    static GQuark quark = 0;

    if (G_UNLIKELY (quark == 0))
    {
        quark = g_quark_from_string ("xed_file_browser_store_error");
    }

    return quark;
}

static GFile *
unique_new_name (GFile       *directory,
                 gchar const *name)
{
    GFile * newuri = NULL;
    guint num = 0;
    gchar * newname;

    while (newuri == NULL || g_file_query_exists (newuri, NULL))
    {
        if (newuri != NULL)
        {
            g_object_unref (newuri);
        }

        if (num == 0)
        {
            newname = g_strdup (name);
        }
        else
        {
            newname = g_strdup_printf ("%s(%d)", name, num);
        }

        newuri = g_file_get_child (directory, newname);
        g_free (newname);

        ++num;
    }

    return newuri;
}

static XedFileBrowserStoreResult
model_root_mounted (XedFileBrowserStore *model,
                    GFile               *virtual_root)
{
    model_check_dummy (model, model->priv->root);
    g_object_notify (G_OBJECT (model), "root");

    if (virtual_root != NULL)
    {
        return xed_file_browser_store_set_virtual_root_from_location (model, virtual_root);
    }
    else
    {
        set_virtual_root_from_node (model, model->priv->root);
    }

    return XED_FILE_BROWSER_STORE_RESULT_OK;
}

static void
handle_root_error (XedFileBrowserStore *model,
                   GError              *error)
{
    FileBrowserNode * root;

    g_signal_emit (model,
                   model_signals[ERROR],
                   0,
                   XED_FILE_BROWSER_ERROR_SET_ROOT,
                   error->message);

    /* Set the virtual root to the root */
    root = model->priv->root;
    model->priv->virtual_root = root;

    /* Set the root to be loaded */
    root->flags |= XED_FILE_BROWSER_STORE_FLAG_LOADED;

    /* Check the dummy */
    model_check_dummy (model, root);

    g_object_notify (G_OBJECT (model), "root");
    g_object_notify (G_OBJECT (model), "virtual-root");
}

static void
mount_cb (GFile        *file,
          GAsyncResult *res,
          MountInfo    *mount_info)
{
    gboolean mounted;
    GError *error = NULL;
    XedFileBrowserStore *model = mount_info->model;

    mounted = g_file_mount_enclosing_volume_finish (file, res, &error);

    if (mount_info->model)
    {
        model->priv->mount_info = NULL;
        model_end_loading (model, model->priv->root);
    }

    if (!mount_info->model || g_cancellable_is_cancelled (mount_info->cancellable))
    {
        // Reset because it might be reused?
        g_cancellable_reset (mount_info->cancellable);
    }
    else if (mounted)
    {
        model_root_mounted (model, mount_info->virtual_root);
    }
    else if (error->code != G_IO_ERROR_CANCELLED)
    {
        handle_root_error (model, error);
    }

    if (error)
    {
        g_error_free (error);
    }

    g_object_unref (mount_info->operation);
    g_object_unref (mount_info->cancellable);

    if (mount_info->virtual_root)
    {
        g_object_unref (mount_info->virtual_root);
    }

    g_slice_free (MountInfo, mount_info);
}

static XedFileBrowserStoreResult
model_mount_root (XedFileBrowserStore *model,
                  GFile               *virtual_root)
{
    GFileInfo *info;
    GError *error = NULL;
    MountInfo *mount_info;

    info = g_file_query_info (model->priv->root->file,
                              G_FILE_ATTRIBUTE_STANDARD_TYPE,
                              G_FILE_QUERY_INFO_NONE,
                              NULL,
                              &error);

    if (!info)
    {
        if (error->code == G_IO_ERROR_NOT_MOUNTED)
        {
            /* Try to mount it */
            FILE_BROWSER_NODE_DIR (model->priv->root)->cancellable = g_cancellable_new ();

            mount_info = g_slice_new (MountInfo);
            mount_info->model = model;
            mount_info->virtual_root = g_file_dup (virtual_root);

            /* FIXME: we should be setting the correct window */
            mount_info->operation = gtk_mount_operation_new (NULL);
            mount_info->cancellable = g_object_ref (FILE_BROWSER_NODE_DIR (model->priv->root)->cancellable);

            model_begin_loading (model, model->priv->root);
            g_file_mount_enclosing_volume (model->priv->root->file,
                                           G_MOUNT_MOUNT_NONE,
                                           mount_info->operation,
                                           mount_info->cancellable,
                                           (GAsyncReadyCallback)mount_cb,
                                           mount_info);

            model->priv->mount_info = mount_info;
            return XED_FILE_BROWSER_STORE_RESULT_MOUNTING;
        }
        else
        {
            handle_root_error (model, error);
        }

        g_error_free (error);
    }
    else
    {
        g_object_unref (info);

        return model_root_mounted (model, virtual_root);
    }

    return XED_FILE_BROWSER_STORE_RESULT_OK;
}

/* Public */
XedFileBrowserStore *
xed_file_browser_store_new (GFile *root)
{
    XedFileBrowserStore *obj = XED_FILE_BROWSER_STORE (g_object_new (XED_TYPE_FILE_BROWSER_STORE, NULL));

    xed_file_browser_store_set_root (obj, root);
    return obj;
}

void
xed_file_browser_store_set_value (XedFileBrowserStore *tree_model,
                                  GtkTreeIter         *iter, gint column,
                                  GValue              *value)
{
    gpointer data;
    FileBrowserNode *node;
    GtkTreePath *path;

    g_return_if_fail (XED_IS_FILE_BROWSER_STORE (tree_model));
    g_return_if_fail (column == XED_FILE_BROWSER_STORE_COLUMN_EMBLEM);
    g_return_if_fail (G_VALUE_HOLDS_OBJECT (value));
    g_return_if_fail (iter != NULL);
    g_return_if_fail (iter->user_data != NULL);

    data = g_value_get_object (value);

    if (data)
    {
        g_return_if_fail (GDK_IS_PIXBUF (data));
    }

    node = (FileBrowserNode *) (iter->user_data);

    if (node->emblem)
    {
        g_object_unref (node->emblem);
    }

    if (data)
    {
        node->emblem = g_object_ref (GDK_PIXBUF (data));
    }
    else
    {
        node->emblem = NULL;
    }

    model_recomposite_icon (tree_model, iter);

    if (model_node_visibility (tree_model, node))
    {
        path = xed_file_browser_store_get_path (GTK_TREE_MODEL (tree_model), iter);
        row_changed (tree_model, &path, iter);
        gtk_tree_path_free (path);
    }
}

XedFileBrowserStoreResult
xed_file_browser_store_set_virtual_root (XedFileBrowserStore *model,
                                         GtkTreeIter         *iter)
{
    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (model), XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE);
    g_return_val_if_fail (iter != NULL, XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE);
    g_return_val_if_fail (iter->user_data != NULL, XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE);

    model_clear (model, FALSE);
    set_virtual_root_from_node (model, (FileBrowserNode *) (iter->user_data));

    return TRUE;
}

XedFileBrowserStoreResult
xed_file_browser_store_set_virtual_root_from_location (XedFileBrowserStore *model,
                                                       GFile               *root)
{
    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (model), XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE);

    if (root == NULL)
    {
        gchar *uri;

        uri = g_file_get_uri (root);
        g_warning ("Invalid uri (%s)", uri);
        g_free (uri);
        return XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE;
    }

    /* Check if uri is already the virtual root */
    if (model->priv->virtual_root && g_file_equal (model->priv->virtual_root->file, root))
    {
        return XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE;
    }

    /* Check if uri is the root itself */
    if (g_file_equal (model->priv->root->file, root))
    {
        /* Always clear the model before altering the nodes */
        model_clear (model, FALSE);
        set_virtual_root_from_node (model, model->priv->root);
        return XED_FILE_BROWSER_STORE_RESULT_OK;
    }

    if (!g_file_has_prefix (root, model->priv->root->file))
    {
        gchar *str, *str1;

        str = g_file_get_parse_name (model->priv->root->file);
        str1 = g_file_get_parse_name (root);

        g_warning ("Virtual root (%s) is not below actual root (%s)", str1, str);

        g_free (str);
        g_free (str1);

        return XED_FILE_BROWSER_STORE_RESULT_ERROR;
    }

    set_virtual_root_from_file (model, root);

    return XED_FILE_BROWSER_STORE_RESULT_OK;
}

XedFileBrowserStoreResult
xed_file_browser_store_set_virtual_root_top (XedFileBrowserStore *model)
{
    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (model), XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE);

    if (model->priv->virtual_root == model->priv->root)
    {
        return XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE;
    }

    model_clear (model, FALSE);
    set_virtual_root_from_node (model, model->priv->root);

    return XED_FILE_BROWSER_STORE_RESULT_OK;
}

XedFileBrowserStoreResult
xed_file_browser_store_set_virtual_root_up (XedFileBrowserStore *model)
{
    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (model), XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE);

    if (model->priv->virtual_root == model->priv->root)
    {
        return XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE;
    }

    model_clear (model, FALSE);
    set_virtual_root_from_node (model, model->priv->virtual_root->parent);

    return XED_FILE_BROWSER_STORE_RESULT_OK;
}

gboolean
xed_file_browser_store_get_iter_virtual_root (XedFileBrowserStore *model,
                                              GtkTreeIter         *iter)
{
    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (model), FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);

    if (model->priv->virtual_root == NULL)
    {
        return FALSE;
    }

    iter->user_data = model->priv->virtual_root;
    return TRUE;
}

gboolean
xed_file_browser_store_get_iter_root (XedFileBrowserStore *model,
                                      GtkTreeIter         *iter)
{
    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (model), FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);

    if (model->priv->root == NULL)
    {
        return FALSE;
    }

    iter->user_data = model->priv->root;
    return TRUE;
}

gboolean
xed_file_browser_store_iter_equal (XedFileBrowserStore *model,
                                   GtkTreeIter         *iter1,
                                   GtkTreeIter         *iter2)
{
    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (model), FALSE);
    g_return_val_if_fail (iter1 != NULL, FALSE);
    g_return_val_if_fail (iter2 != NULL, FALSE);
    g_return_val_if_fail (iter1->user_data != NULL, FALSE);
    g_return_val_if_fail (iter2->user_data != NULL, FALSE);

    return (iter1->user_data == iter2->user_data);
}

void
xed_file_browser_store_cancel_mount_operation (XedFileBrowserStore *store)
{
    g_return_if_fail (XED_IS_FILE_BROWSER_STORE (store));

    cancel_mount_operation (store);
}

XedFileBrowserStoreResult
xed_file_browser_store_set_root_and_virtual_root (XedFileBrowserStore *model,
                                                  GFile               *root,
                                                  GFile               *virtual_root)
{
    FileBrowserNode * node;
    gboolean equal = FALSE;

    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (model), XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE);

    if (root == NULL && model->priv->root == NULL)
    {
        return XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE;
    }

    if (root != NULL && model->priv->root != NULL)
    {
        equal = g_file_equal (root, model->priv->root->file);

        if (equal && virtual_root == NULL)
        {
            return XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE;
        }
    }

    if (virtual_root)
    {
        if (equal && g_file_equal (virtual_root, model->priv->virtual_root->file))
        {
            return XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE;
        }
    }

    /* make sure to cancel any previous mount operations */
    cancel_mount_operation (model);

    /* Always clear the model before altering the nodes */
    model_clear (model, TRUE);
    file_browser_node_free (model, model->priv->root);

    model->priv->root = NULL;
    model->priv->virtual_root = NULL;

    if (root != NULL)
    {
        /* Create the root node */
        node = file_browser_node_dir_new (model, root, NULL);

        model->priv->root = node;
        return model_mount_root (model, virtual_root);
    }
    else
    {
        g_object_notify (G_OBJECT (model), "root");
        g_object_notify (G_OBJECT (model), "virtual-root");
    }

    return XED_FILE_BROWSER_STORE_RESULT_OK;
}

XedFileBrowserStoreResult
xed_file_browser_store_set_root (XedFileBrowserStore *model,
                                 GFile               *root)
{
    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (model), XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE);
    return xed_file_browser_store_set_root_and_virtual_root (model, root, NULL);
}

GFile *
xed_file_browser_store_get_root (XedFileBrowserStore * model)
{
    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (model), NULL);

    if (model->priv->root == NULL || model->priv->root->file == NULL)
    {
        return NULL;
    }
    else
    {
        return g_file_dup (model->priv->root->file);
    }
}

GFile *
xed_file_browser_store_get_virtual_root (XedFileBrowserStore *model)
{
    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (model), NULL);

    if (model->priv->virtual_root == NULL || model->priv->virtual_root->file == NULL)
    {
        return NULL;
    }
    else
    {
        return g_file_dup (model->priv->virtual_root->file);
    }
}

void
_xed_file_browser_store_iter_expanded (XedFileBrowserStore *model,
                                       GtkTreeIter         *iter)
{
    FileBrowserNode *node;

    g_return_if_fail (XED_IS_FILE_BROWSER_STORE (model));
    g_return_if_fail (iter != NULL);
    g_return_if_fail (iter->user_data != NULL);

    node = (FileBrowserNode *) (iter->user_data);

    if (NODE_IS_DIR (node) && !NODE_LOADED (node))
    {
        /* Load it now */
        model_load_directory (model, node);
    }
}

void
_xed_file_browser_store_iter_collapsed (XedFileBrowserStore *model,
                                        GtkTreeIter         *iter)
{
    FileBrowserNode *node;
    GSList *item;

    g_return_if_fail (XED_IS_FILE_BROWSER_STORE (model));
    g_return_if_fail (iter != NULL);
    g_return_if_fail (iter->user_data != NULL);

    node = (FileBrowserNode *) (iter->user_data);

    if (NODE_IS_DIR (node) && NODE_LOADED (node))
    {
        /* Unload children of the children, keeping 1 depth in cache */

        for (item = FILE_BROWSER_NODE_DIR (node)->children; item; item = item->next)
        {
            node = (FileBrowserNode *) (item->data);

            if (NODE_IS_DIR (node) && NODE_LOADED (node))
            {
                file_browser_node_unload (model, node, TRUE);
                model_check_dummy (model, node);
            }
        }
    }
}

XedFileBrowserStoreFilterMode
xed_file_browser_store_get_filter_mode (XedFileBrowserStore *model)
{
    return model->priv->filter_mode;
}

void
xed_file_browser_store_set_filter_mode (XedFileBrowserStore           *model,
                                        XedFileBrowserStoreFilterMode  mode)
{
    g_return_if_fail (XED_IS_FILE_BROWSER_STORE (model));

    if (model->priv->filter_mode == mode)
    {
        return;
    }

    model->priv->filter_mode = mode;
    model_refilter (model);

    g_object_notify (G_OBJECT (model), "filter-mode");
}

void
xed_file_browser_store_set_filter_func (XedFileBrowserStore           *model,
                                        XedFileBrowserStoreFilterFunc  func,
                                        gpointer                       user_data)
{
    g_return_if_fail (XED_IS_FILE_BROWSER_STORE (model));

    model->priv->filter_func = func;
    model->priv->filter_user_data = user_data;
    model_refilter (model);
}

void
xed_file_browser_store_refilter (XedFileBrowserStore *model)
{
    model_refilter (model);
}

XedFileBrowserStoreFilterMode
xed_file_browser_store_filter_mode_get_default (void)
{
    return XED_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN;
}

void
xed_file_browser_store_refresh (XedFileBrowserStore *model)
{
    g_return_if_fail (XED_IS_FILE_BROWSER_STORE (model));

    if (model->priv->root == NULL || model->priv->virtual_root == NULL)
    {
        return;
    }

    /* Clear the model */
    g_signal_emit (model, model_signals[BEGIN_REFRESH], 0);
    file_browser_node_unload (model, model->priv->virtual_root, TRUE);
    model_load_directory (model, model->priv->virtual_root);
    g_signal_emit (model, model_signals[END_REFRESH], 0);
}

static void
reparent_node (FileBrowserNode *node,
               gboolean         reparent)
{
    FileBrowserNodeDir *dir;
    GSList *child;
    GFile *parent;
    gchar *base;

    if (!node->file)
    {
        return;
    }

    if (reparent)
    {
        parent = node->parent->file;
        base = g_file_get_basename (node->file);
        g_object_unref (node->file);

        node->file = g_file_get_child (parent, base);
        g_free (base);
    }

    if (NODE_IS_DIR (node))
    {
        dir = FILE_BROWSER_NODE_DIR (node);

        for (child = dir->children; child; child = child->next)
        {
            reparent_node ((FileBrowserNode *)child->data, TRUE);
        }
    }
}

gboolean
xed_file_browser_store_rename (XedFileBrowserStore *model,
                               GtkTreeIter         *iter,
                               const gchar         *new_name,
                               GError             **error)
{
    FileBrowserNode *node;
    GFile *file;
    GFile *parent;
    GFile *previous;
    GError *err = NULL;
    GtkTreePath *path;

    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (model), FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);
    g_return_val_if_fail (iter->user_data != NULL, FALSE);

    node = (FileBrowserNode *) (iter->user_data);

    parent = g_file_get_parent (node->file);
    g_return_val_if_fail (parent != NULL, FALSE);

    file = g_file_get_child (parent, new_name);
    g_object_unref (parent);

    if (g_file_equal (node->file, file)) \
    {
        g_object_unref (file);
        return TRUE;
    }

    if (g_file_move (node->file, file, G_FILE_COPY_NONE, NULL, NULL, NULL, &err))
    {
        previous = node->file;
        node->file = file;

        /* This makes sure the actual info for the node is requeried */
        file_browser_node_set_name (node);
        file_browser_node_set_from_info (model, node, NULL, TRUE);

        reparent_node (node, FALSE);

        if (model_node_visibility (model, node))
        {
            path = xed_file_browser_store_get_path_real (model, node);
            row_changed (model, &path, iter);
            gtk_tree_path_free (path);

            /* Reorder this item */
            model_resort_node (model, node);
        }
        else
        {
            g_object_unref (previous);

            if (error != NULL)
            {
                *error = g_error_new_literal (xed_file_browser_store_error_quark (),
                                              XED_FILE_BROWSER_ERROR_RENAME,
                                              _("The renamed file is currently filtered out. You need to adjust your filter settings to make the file visible"));
            }
            return FALSE;
        }

        g_signal_emit (model, model_signals[RENAME], 0, previous, node->file);

        g_object_unref (previous);

        return TRUE;
    }
    else
    {
        g_object_unref (file);

        if (err)
        {
            if (error != NULL)
            {
                *error = g_error_new_literal (xed_file_browser_store_error_quark (),
                                              XED_FILE_BROWSER_ERROR_RENAME,
                                              err->message);
            }

            g_error_free (err);
        }

        return FALSE;
    }
}

static void
async_data_free (AsyncData *data)
{
    g_object_unref (data->cancellable);

    g_list_foreach (data->files, (GFunc)g_object_unref, NULL);
    g_list_free (data->files);

    if (!data->removed)
    {
        data->model->priv->async_handles = g_slist_remove (data->model->priv->async_handles, data);
    }

    g_slice_free (AsyncData, data);
}

static gboolean
emit_no_trash (AsyncData *data)
{
    /* Emit the no trash error */
    gboolean ret;

    g_signal_emit (data->model, model_signals[NO_TRASH], 0, data->files, &ret);
    return ret;
}

static void
delete_file_finished (GFile        *file,
                      GAsyncResult *res,
                      AsyncData    *data)
{
    GError *error = NULL;
    gboolean ok;

    if (data->trash)
    {
        ok = g_file_trash_finish (file, res, &error);
    }
    else
    {
        ok = g_file_delete_finish (file, res, &error);
    }

    if (ok)
    {
        /* Remove the file from the model */
        FileBrowserNode *node = model_find_node (data->model, NULL, file);

        if (node != NULL)
        {
            model_remove_node (data->model, node, NULL, TRUE);
        }

        /* Process the next file */
        data->iter = data->iter->next;
    }
    else if (!ok && error != NULL)
    {
        gint code = error->code;
        g_error_free (error);

        if (data->trash && code == G_IO_ERROR_NOT_SUPPORTED)
        {
            /* Trash is not supported on this system. Ask the user
             * if he wants to delete completely the files instead.
             */
            if (emit_no_trash (data))
            {
                /* Changes this into a delete job */
                data->trash = FALSE;
                data->iter = data->files;
            }
            else
            {
                /* End the job */
                async_data_free (data);
                return;
            }
        }
        else if (code == G_IO_ERROR_CANCELLED)
        {
            /* Job has been cancelled, end the job */
            async_data_free (data);
            return;
        }
    }

    /* Continue the job */
    delete_files (data);
}

static void
delete_files (AsyncData *data)
{
    GFile *file;

    /* Check if our job is done */
    if (data->iter == NULL)
    {
        async_data_free (data);
        return;
    }

    file = G_FILE (data->iter->data);

    if (data->trash)
    {
        g_file_trash_async (file,
                            G_PRIORITY_DEFAULT,
                            data->cancellable,
                            (GAsyncReadyCallback)delete_file_finished,
                            data);
    }
    else
    {
        g_file_delete_async (file,
                             G_PRIORITY_DEFAULT,
                             data->cancellable,
                             (GAsyncReadyCallback)delete_file_finished,
                             data);
    }
}

XedFileBrowserStoreResult
xed_file_browser_store_delete_all (XedFileBrowserStore *model,
                                   GList               *rows,
                                   gboolean             trash)
{
    FileBrowserNode *node;
    AsyncData *data;
    GList *files = NULL;
    GList *row;
    GtkTreeIter iter;
    GtkTreePath *prev = NULL;
    GtkTreePath *path;

    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (model), XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE);

    if (rows == NULL)
    {
        return XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE;
    }

    /* First we sort the paths so that we can later on remove any
       files/directories that are actually subfiles/directories of
       a directory that's also deleted */
    rows = g_list_sort (g_list_copy (rows), (GCompareFunc)gtk_tree_path_compare);

    for (row = rows; row; row = row->next)
    {
        path = (GtkTreePath *)(row->data);

        if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path))
        {
            continue;
        }

        /* Skip if the current path is actually a descendant of the
           previous path */
        if (prev != NULL && gtk_tree_path_is_descendant (path, prev))
        {
            continue;
        }

        prev = path;
        node = (FileBrowserNode *)(iter.user_data);
        files = g_list_prepend (files, g_object_ref (node->file));
    }

    data = g_slice_new (AsyncData);

    data->model = model;
    data->cancellable = g_cancellable_new ();
    data->files = files;
    data->trash = trash;
    data->iter = files;
    data->removed = FALSE;

    model->priv->async_handles = g_slist_prepend (model->priv->async_handles, data);

    delete_files (data);
    g_list_free (rows);

    return XED_FILE_BROWSER_STORE_RESULT_OK;
}

XedFileBrowserStoreResult
xed_file_browser_store_delete (XedFileBrowserStore *model,
                               GtkTreeIter         *iter,
                               gboolean             trash)
{
    FileBrowserNode *node;
    GList *rows = NULL;
    XedFileBrowserStoreResult result;

    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (model), XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE);
    g_return_val_if_fail (iter != NULL, XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE);
    g_return_val_if_fail (iter->user_data != NULL, XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE);

    node = (FileBrowserNode *) (iter->user_data);

    if (NODE_IS_DUMMY (node))
    {
        return XED_FILE_BROWSER_STORE_RESULT_NO_CHANGE;
    }

    rows = g_list_append(NULL, xed_file_browser_store_get_path_real (model, node));
    result = xed_file_browser_store_delete_all (model, rows, trash);

    g_list_foreach (rows, (GFunc)gtk_tree_path_free, NULL);
    g_list_free (rows);

    return result;
}

gboolean
xed_file_browser_store_new_file (XedFileBrowserStore *model,
                                 GtkTreeIter         *parent,
                                 GtkTreeIter         *iter)
{
    GFile *file;
    GFileOutputStream *stream;
    FileBrowserNodeDir *parent_node;
    gboolean result = FALSE;
    FileBrowserNode *node;
    GError *error = NULL;

    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (model), FALSE);
    g_return_val_if_fail (parent != NULL, FALSE);
    g_return_val_if_fail (parent->user_data != NULL, FALSE);
    g_return_val_if_fail (NODE_IS_DIR ((FileBrowserNode *) (parent->user_data)), FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);

    parent_node = FILE_BROWSER_NODE_DIR (parent->user_data);
    /* Translators: This is the default name of new files created by the file browser pane. */
    file = unique_new_name (((FileBrowserNode *) parent_node)->file, _("Untitled File"));

    stream = g_file_create (file, G_FILE_CREATE_NONE, NULL, &error);

    if (!stream)
    {
        g_signal_emit (model, model_signals[ERROR], 0,
                       XED_FILE_BROWSER_ERROR_NEW_FILE,
                       error->message);
        g_error_free (error);
    }
    else
    {
        g_object_unref (stream);
        node = model_add_node_from_file (model,
                                         (FileBrowserNode *)parent_node,
                                         file,
                                         NULL);

        if (model_node_visibility (model, node))
        {
            iter->user_data = node;
            result = TRUE;
        }
        else
        {
            g_signal_emit (model, model_signals[ERROR], 0,
                           XED_FILE_BROWSER_ERROR_NEW_FILE,
                           _("The new file is currently filtered out. You need to adjust your filter settings to make the file visible"));
        }
    }

    g_object_unref (file);
    return result;
}

gboolean
xed_file_browser_store_new_directory (XedFileBrowserStore *model,
                                      GtkTreeIter         *parent,
                                      GtkTreeIter         *iter)
{
    GFile *file;
    FileBrowserNodeDir *parent_node;
    GError *error = NULL;
    FileBrowserNode *node;
    gboolean result = FALSE;

    g_return_val_if_fail (XED_IS_FILE_BROWSER_STORE (model), FALSE);
    g_return_val_if_fail (parent != NULL, FALSE);
    g_return_val_if_fail (parent->user_data != NULL, FALSE);
    g_return_val_if_fail (NODE_IS_DIR ((FileBrowserNode *) (parent->user_data)), FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);

    parent_node = FILE_BROWSER_NODE_DIR (parent->user_data);
    /* Translators: This is the default name of new directories created by the file browser pane. */
    file = unique_new_name (((FileBrowserNode *) parent_node)->file, _("Untitled Folder"));

    if (!g_file_make_directory (file, NULL, &error))
    {
        g_signal_emit (model, model_signals[ERROR], 0,
                       XED_FILE_BROWSER_ERROR_NEW_DIRECTORY,
                       error->message);
        g_error_free (error);
    }
    else
    {
        node = model_add_node_from_file (model,
                                         (FileBrowserNode *)parent_node,
                                         file,
                                         NULL);

        if (model_node_visibility (model, node))
        {
            iter->user_data = node;
            result = TRUE;
        }
        else
        {
            g_signal_emit (model, model_signals[ERROR], 0,
                           XED_FILE_BROWSER_ERROR_NEW_FILE,
                           _("The new directory is currently filtered out. You need to adjust your filter settings to make the directory visible"));
        }
    }

    g_object_unref (file);
    return result;
}

void
_xed_file_browser_store_register_type (GTypeModule *type_module)
{
    xed_file_browser_store_register_type (type_module);
}

// ex:ts=8:noet:
