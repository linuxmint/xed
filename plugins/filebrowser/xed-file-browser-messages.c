#include "xed-file-browser-messages.h"
#include "xed-file-browser-store.h"
#include <xed/xed-message.h>

#define MESSAGE_OBJECT_PATH     "/plugins/filebrowser"
#define WINDOW_DATA_KEY         "XedFileBrowserMessagesWindowData"

#define BUS_CONNECT(bus, name, data) xed_message_bus_connect(bus, MESSAGE_OBJECT_PATH, #name, (XedMessageCallback)  message_##name##_cb, data, NULL)

typedef struct
{
    XedWindow *window;
    XedMessage *message;
} MessageCacheData;

typedef struct
{
    guint row_inserted_id;
    guint row_deleted_id;
    guint root_changed_id;
    guint begin_loading_id;
    guint end_loading_id;

    GList *merge_ids;
    GtkActionGroup *merged_actions;

    XedMessageBus *bus;
    XedFileBrowserWidget *widget;
    GHashTable *row_tracking;

    GHashTable *filters;
} WindowData;

typedef struct
{
    gulong id;

    XedWindow *window;
    XedMessage *message;
} FilterData;

static WindowData *
window_data_new (XedWindow            *window,
         XedFileBrowserWidget *widget)
{
    WindowData *data = g_slice_new (WindowData);
    GtkUIManager *manager;
    GList *groups;

    data->bus = xed_window_get_message_bus (window);
    data->widget = widget;
    data->row_tracking = g_hash_table_new_full (g_str_hash,
                            g_str_equal,
                            (GDestroyNotify)g_free,
                            (GDestroyNotify)gtk_tree_row_reference_free);

    data->filters = g_hash_table_new_full (g_str_hash,
                           g_str_equal,
                           (GDestroyNotify)g_free,
                           NULL);

    manager = xed_file_browser_widget_get_ui_manager (widget);

    data->merge_ids = NULL;
    data->merged_actions = gtk_action_group_new ("MessageMergedActions");

    groups = gtk_ui_manager_get_action_groups (manager);
    gtk_ui_manager_insert_action_group (manager, data->merged_actions, g_list_length (groups));

    g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, data);

    return data;
}

static WindowData *
get_window_data (XedWindow * window)
{
    return (WindowData *) (g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY));
}

static void
window_data_free (XedWindow *window)
{
    WindowData *data = get_window_data (window);
    GtkUIManager *manager;
    GList *item;

    g_hash_table_destroy (data->row_tracking);
    g_hash_table_destroy (data->filters);

    manager = xed_file_browser_widget_get_ui_manager (data->widget);
    gtk_ui_manager_remove_action_group (manager, data->merged_actions);

    for (item = data->merge_ids; item; item = item->next)
        gtk_ui_manager_remove_ui (manager, GPOINTER_TO_INT (item->data));

    g_list_free (data->merge_ids);
    g_object_unref (data->merged_actions);

    g_slice_free (WindowData, data);

    g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static FilterData *
filter_data_new (XedWindow  *window,
         XedMessage *message)
{
    FilterData *data = g_slice_new (FilterData);
    WindowData *wdata;

    data->window = window;
    data->id = 0;
    data->message = message;

    wdata = get_window_data (window);

    g_hash_table_insert (wdata->filters,
                 xed_message_type_identifier (xed_message_get_object_path (message),
                                                xed_message_get_method (message)),
                 data);

    return data;
}

static void
filter_data_free (FilterData *data)
{
    WindowData *wdata = get_window_data (data->window);
    gchar *identifier;

    identifier = xed_message_type_identifier (xed_message_get_object_path (data->message),
                                        xed_message_get_method (data->message));

    g_hash_table_remove (wdata->filters, identifier);
    g_free (identifier);

    g_object_unref (data->message);
    g_slice_free (FilterData, data);
}

static GtkTreePath *
track_row_lookup (WindowData  *data,
          const gchar *id)
{
    GtkTreeRowReference *ref;

    ref = (GtkTreeRowReference *)g_hash_table_lookup (data->row_tracking, id);

    if (!ref)
        return NULL;

    return gtk_tree_row_reference_get_path (ref);
}

static void
message_cache_data_free (MessageCacheData *data)
{
    g_object_unref (data->message);
    g_slice_free (MessageCacheData, data);
}

static MessageCacheData *
message_cache_data_new (XedWindow            *window,
            XedMessage           *message)
{
    MessageCacheData *data = g_slice_new (MessageCacheData);

    data->window = window;
    data->message = message;

    return data;
}

static void
message_get_root_cb (XedMessageBus *bus,
             XedMessage    *message,
             WindowData      *data)
{
    XedFileBrowserStore *store;
    GFile *location;

    store = xed_file_browser_widget_get_browser_store (data->widget);
    location = xed_file_browser_store_get_virtual_root (store);

    if (location)
    {
        xed_message_set (message, "location", location, NULL);
        g_object_unref (location);
    }
}

static void
message_set_root_cb (XedMessageBus *bus,
             XedMessage    *message,
             WindowData      *data)
{
    GFile *root;
    GFile *virtual = NULL;

    xed_message_get (message, "location", &root, NULL);

    if (!root)
        return;

    if (xed_message_has_key (message, "virtual"))
        xed_message_get (message, "virtual", &virtual, NULL);

    if (virtual)
        xed_file_browser_widget_set_root_and_virtual_root (data->widget, root, virtual);
    else
        xed_file_browser_widget_set_root (data->widget, root, TRUE);
}

static void
message_set_emblem_cb (XedMessageBus *bus,
               XedMessage    *message,
               WindowData      *data)
{
    gchar *id = NULL;
    gchar *emblem = NULL;
    GtkTreePath *path;
    XedFileBrowserStore *store;

    xed_message_get (message, "id", &id, "emblem", &emblem, NULL);

    if (!id || !emblem)
    {
        g_free (id);
        g_free (emblem);

        return;
    }

    path = track_row_lookup (data, id);

    if (path != NULL)
    {
        GError *error = NULL;
        GdkPixbuf *pixbuf;

        pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
                           emblem,
                           10,
                           0,
                           &error);

        if (pixbuf)
        {
            GValue value = { 0, };
            GtkTreeIter iter;

            store = xed_file_browser_widget_get_browser_store (data->widget);

            if (gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path))
            {
                g_value_init (&value, GDK_TYPE_PIXBUF);
                g_value_set_object (&value, pixbuf);

                xed_file_browser_store_set_value (store,
                                    &iter,
                                    XED_FILE_BROWSER_STORE_COLUMN_EMBLEM,
                                    &value);

                g_value_unset (&value);
            }

            g_object_unref (pixbuf);
        }

        if (error)
            g_error_free (error);
    }

    g_free (id);
    g_free (emblem);
}

static gchar *
item_id (const gchar *path,
     GFile *location)
{
    gchar *uri;
    gchar *id;

    uri = g_file_get_uri (location);
    id = g_strconcat (path, "::", uri, NULL);
    g_free (uri);

    return id;
}

static gchar *
track_row (WindowData            *data,
       XedFileBrowserStore *store,
       GtkTreePath           *path,
       GFile               *location)
{
    GtkTreeRowReference *ref;
    gchar *id;
    gchar *pathstr;

    pathstr = gtk_tree_path_to_string (path);
    id = item_id (pathstr, location);

    ref = gtk_tree_row_reference_new (GTK_TREE_MODEL (store), path);
    g_hash_table_insert (data->row_tracking, g_strdup (id), ref);

    g_free (pathstr);

    return id;
}

static void
set_item_message (WindowData   *data,
          GtkTreeIter  *iter,
          GtkTreePath  *path,
          XedMessage *message)
{
    XedFileBrowserStore *store;
    GFile *location;
    guint flags = 0;
    gchar *track_id;

    store = xed_file_browser_widget_get_browser_store (data->widget);

    gtk_tree_model_get (GTK_TREE_MODEL (store), iter,
                XED_FILE_BROWSER_STORE_COLUMN_LOCATION, &location,
                XED_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags,
                -1);

    if (!location)
        return;

    if (path && gtk_tree_path_get_depth (path) != 0)
        track_id = track_row (data, store, path, location);
    else
        track_id = NULL;

    xed_message_set (message,
               "id", track_id,
               "location", location,
               NULL);

    if (xed_message_has_key (message, "is_directory"))
    {
        xed_message_set (message,
                   "is_directory", FILE_IS_DIR (flags),
                   NULL);
    }

    g_free (track_id);
}

static gboolean
custom_message_filter_func (XedFileBrowserWidget *widget,
                XedFileBrowserStore  *store,
                GtkTreeIter            *iter,
                FilterData             *data)
{
    WindowData *wdata = get_window_data (data->window);
    GFile *location;
    guint flags = 0;
    gboolean filter = FALSE;
    GtkTreePath *path;

    gtk_tree_model_get (GTK_TREE_MODEL (store), iter,
                XED_FILE_BROWSER_STORE_COLUMN_LOCATION, &location,
                XED_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags,
                -1);

    if (!location || FILE_IS_DUMMY (flags))
    {
        return FALSE;
    }

    path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), iter);
    set_item_message (wdata, iter, path, data->message);
    gtk_tree_path_free (path);

    xed_message_set (data->message, "filter", filter, NULL);

    xed_message_bus_send_message_sync (wdata->bus, data->message);
    xed_message_get (data->message, "filter", &filter, NULL);

    return !filter;
}

static void
message_add_filter_cb (XedMessageBus *bus,
               XedMessage    *message,
               XedWindow     *window)
{
    gchar *object_path = NULL;
    gchar *method = NULL;
    gulong id;
    XedMessageType *message_type;
    XedMessage *cbmessage;
    FilterData *filter_data;
    WindowData *data = get_window_data (window);

    xed_message_get (message,
               "object_path", &object_path,
               "method", &method,
               NULL);

    // Check if there exists such a 'callback' message
    if (!object_path || !method)
    {
        g_free (object_path);
        g_free (method);

        return;
    }

    message_type = xed_message_bus_lookup (bus, object_path, method);

    if (!message_type)
    {
        g_free (object_path);
        g_free (method);

        return;
    }

    // Check if the message type has the correct arguments
    if (xed_message_type_lookup (message_type, "id") != G_TYPE_STRING ||
        xed_message_type_lookup (message_type, "location") != G_TYPE_FILE ||
        xed_message_type_lookup (message_type, "is_directory") != G_TYPE_BOOLEAN ||
        xed_message_type_lookup (message_type, "filter") != G_TYPE_BOOLEAN)
    {
        return;
    }

    cbmessage = xed_message_type_instantiate (message_type,
                            "id", NULL,
                            "location", NULL,
                            "is_directory", FALSE,
                            "filter", FALSE,
                            NULL);

    // Register the custom filter on the widget
    filter_data = filter_data_new (window, cbmessage);
    id = xed_file_browser_widget_add_filter (data->widget,
                           (XedFileBrowserWidgetFilterFunc)custom_message_filter_func,
                           filter_data,
                           (GDestroyNotify)filter_data_free);

    filter_data->id = id;
}

static void
message_remove_filter_cb (XedMessageBus *bus,
                  XedMessage    *message,
                  WindowData      *data)
{
    gulong id = 0;

    xed_message_get (message, "id", &id, NULL);

    if (!id)
        return;

    xed_file_browser_widget_remove_filter (data->widget, id);
}

static void
message_up_cb (XedMessageBus *bus,
           XedMessage    *message,
           WindowData      *data)
{
    XedFileBrowserStore *store = xed_file_browser_widget_get_browser_store (data->widget);

    xed_file_browser_store_set_virtual_root_up (store);
}

static void
message_history_back_cb (XedMessageBus *bus,
                 XedMessage    *message,
                 WindowData      *data)
{
    xed_file_browser_widget_history_back (data->widget);
}

static void
message_history_forward_cb (XedMessageBus *bus,
                    XedMessage    *message,
                    WindowData      *data)
{
    xed_file_browser_widget_history_forward (data->widget);
}

static void
message_refresh_cb (XedMessageBus *bus,
            XedMessage    *message,
            WindowData      *data)
{
    xed_file_browser_widget_refresh (data->widget);
}

static void
message_set_show_hidden_cb (XedMessageBus *bus,
                    XedMessage    *message,
                    WindowData      *data)
{
    gboolean active = FALSE;
    XedFileBrowserStore *store;
    XedFileBrowserStoreFilterMode mode;

    xed_message_get (message, "active", &active, NULL);

    store = xed_file_browser_widget_get_browser_store (data->widget);
    mode = xed_file_browser_store_get_filter_mode (store);

    if (active)
        mode &= ~XED_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN;
    else
        mode |= XED_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN;

    xed_file_browser_store_set_filter_mode (store, mode);
}

static void
message_set_show_binary_cb (XedMessageBus *bus,
                    XedMessage    *message,
                    WindowData      *data)
{
    gboolean active = FALSE;
    XedFileBrowserStore *store;
    XedFileBrowserStoreFilterMode mode;

    xed_message_get (message, "active", &active, NULL);

    store = xed_file_browser_widget_get_browser_store (data->widget);
    mode = xed_file_browser_store_get_filter_mode (store);

    if (active)
        mode &= ~XED_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY;
    else
        mode |= XED_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY;

    xed_file_browser_store_set_filter_mode (store, mode);
}

static void
message_show_bookmarks_cb (XedMessageBus *bus,
                   XedMessage    *message,
                   WindowData      *data)
{
    xed_file_browser_widget_show_bookmarks (data->widget);
}

static void
message_show_files_cb (XedMessageBus *bus,
               XedMessage    *message,
               WindowData      *data)
{
    xed_file_browser_widget_show_files (data->widget);
}

static void
message_add_context_item_cb (XedMessageBus *bus,
                 XedMessage    *message,
                 WindowData      *data)
{
    GtkAction *action = NULL;
    gchar *path = NULL;
    gchar *name;
    GtkUIManager *manager;
    guint merge_id;

    xed_message_get (message,
               "action", &action,
               "path", &path,
               NULL);

    if (!action || !path)
    {
        if (action)
            g_object_unref (action);

        g_free (path);
        return;
    }

    gtk_action_group_add_action (data->merged_actions, action);
    manager = xed_file_browser_widget_get_ui_manager (data->widget);
    name = g_strconcat (gtk_action_get_name (action), "MenuItem", NULL);
    merge_id = gtk_ui_manager_new_merge_id (manager);

    gtk_ui_manager_add_ui (manager,
                   merge_id,
                   path,
                   name,
                   gtk_action_get_name (action),
                   GTK_UI_MANAGER_AUTO,
                   FALSE);

    if (gtk_ui_manager_get_widget (manager, path))
    {
        data->merge_ids = g_list_prepend (data->merge_ids, GINT_TO_POINTER (merge_id));
        xed_message_set (message, "id", merge_id, NULL);
    }
    else
    {
        xed_message_set (message, "id", 0, NULL);
    }

    g_object_unref (action);
    g_free (path);
    g_free (name);
}

static void
message_remove_context_item_cb (XedMessageBus *bus,
                XedMessage    *message,
                WindowData      *data)
{
    guint merge_id = 0;
    GtkUIManager *manager;

    xed_message_get (message, "id", &merge_id, NULL);

    if (merge_id == 0)
        return;

    manager = xed_file_browser_widget_get_ui_manager (data->widget);

    data->merge_ids = g_list_remove (data->merge_ids, GINT_TO_POINTER (merge_id));
    gtk_ui_manager_remove_ui (manager, merge_id);
}

static void
message_get_view_cb (XedMessageBus *bus,
             XedMessage    *message,
             WindowData      *data)
{
    XedFileBrowserView *view;
    view = xed_file_browser_widget_get_browser_view (data->widget);

    xed_message_set (message, "view", view, NULL);
}

static void
register_methods (XedWindow            *window,
          XedFileBrowserWidget *widget)
{
    XedMessageBus *bus = xed_window_get_message_bus (window);
    WindowData *data = get_window_data (window);

    /* Register method calls */
    xed_message_bus_register (bus,
                    MESSAGE_OBJECT_PATH, "get_root",
                    1,
                    "location", G_TYPE_FILE,
                    NULL);

    xed_message_bus_register (bus,
                    MESSAGE_OBJECT_PATH, "set_root",
                    1,
                    "location", G_TYPE_FILE,
                    "virtual", G_TYPE_STRING,
                    NULL);

    xed_message_bus_register (bus,
                    MESSAGE_OBJECT_PATH, "set_emblem",
                    0,
                    "id", G_TYPE_STRING,
                    "emblem", G_TYPE_STRING,
                    NULL);

    xed_message_bus_register (bus,
                    MESSAGE_OBJECT_PATH, "add_filter",
                    1,
                    "object_path", G_TYPE_STRING,
                    "method", G_TYPE_STRING,
                    "id", G_TYPE_ULONG,
                    NULL);

    xed_message_bus_register (bus,
                    MESSAGE_OBJECT_PATH, "remove_filter",
                    0,
                    "id", G_TYPE_ULONG,
                    NULL);

    xed_message_bus_register (bus,
                    MESSAGE_OBJECT_PATH, "add_context_item",
                    1,
                    "action", GTK_TYPE_ACTION,
                    "path", G_TYPE_STRING,
                    "id", G_TYPE_UINT,
                    NULL);

    xed_message_bus_register (bus,
                    MESSAGE_OBJECT_PATH, "remove_context_item",
                    0,
                    "id", G_TYPE_UINT,
                    NULL);

    xed_message_bus_register (bus, MESSAGE_OBJECT_PATH, "up", 0, NULL);

    xed_message_bus_register (bus, MESSAGE_OBJECT_PATH, "history_back", 0, NULL);
    xed_message_bus_register (bus, MESSAGE_OBJECT_PATH, "history_forward", 0, NULL);

    xed_message_bus_register (bus, MESSAGE_OBJECT_PATH, "refresh", 0, NULL);

    xed_message_bus_register (bus,
                    MESSAGE_OBJECT_PATH, "set_show_hidden",
                    0,
                    "active", G_TYPE_BOOLEAN,
                    NULL);
    xed_message_bus_register (bus,
                    MESSAGE_OBJECT_PATH, "set_show_binary",
                    0,
                    "active", G_TYPE_BOOLEAN,
                    NULL);

    xed_message_bus_register (bus, MESSAGE_OBJECT_PATH, "show_bookmarks", 0, NULL);
    xed_message_bus_register (bus, MESSAGE_OBJECT_PATH, "show_files", 0, NULL);

    xed_message_bus_register (bus,
                    MESSAGE_OBJECT_PATH, "get_view",
                    1,
                    "view", XED_TYPE_FILE_BROWSER_VIEW,
                    NULL);

    BUS_CONNECT (bus, get_root, data);
    BUS_CONNECT (bus, set_root, data);
    BUS_CONNECT (bus, set_emblem, data);
    BUS_CONNECT (bus, add_filter, window);
    BUS_CONNECT (bus, remove_filter, data);

    BUS_CONNECT (bus, add_context_item, data);
    BUS_CONNECT (bus, remove_context_item, data);

    BUS_CONNECT (bus, up, data);
    BUS_CONNECT (bus, history_back, data);
    BUS_CONNECT (bus, history_forward, data);

    BUS_CONNECT (bus, refresh, data);

    BUS_CONNECT (bus, set_show_hidden, data);
    BUS_CONNECT (bus, set_show_binary, data);

    BUS_CONNECT (bus, show_bookmarks, data);
    BUS_CONNECT (bus, show_files, data);

    BUS_CONNECT (bus, get_view, data);
}

static void
store_row_inserted (XedFileBrowserStore *store,
            GtkTreePath       *path,
            GtkTreeIter           *iter,
            MessageCacheData      *data)
{
    guint flags = 0;

    gtk_tree_model_get (GTK_TREE_MODEL (store), iter,
                XED_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags,
                -1);

    if (!FILE_IS_DUMMY (flags) && !FILE_IS_FILTERED (flags))
    {
        WindowData *wdata = get_window_data (data->window);

        set_item_message (wdata, iter, path, data->message);
        xed_message_bus_send_message_sync (wdata->bus, data->message);
    }
}

static void
store_row_deleted (XedFileBrowserStore *store,
           GtkTreePath       *path,
           MessageCacheData      *data)
{
    GtkTreeIter iter;
    guint flags = 0;

    if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path))
        return;

    gtk_tree_model_get (GTK_TREE_MODEL (store), &iter,
                XED_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags,
                -1);

    if (!FILE_IS_DUMMY (flags) && !FILE_IS_FILTERED (flags))
    {
        WindowData *wdata = get_window_data (data->window);

        set_item_message (wdata, &iter, path, data->message);
        xed_message_bus_send_message_sync (wdata->bus, data->message);
    }
}

static void
store_virtual_root_changed (XedFileBrowserStore *store,
                GParamSpec            *spec,
                MessageCacheData      *data)
{
    WindowData *wdata = get_window_data (data->window);
    GFile *vroot;

    vroot = xed_file_browser_store_get_virtual_root (store);

    if (!vroot)
        return;

    xed_message_set (data->message,
               "location", vroot,
               NULL);

    xed_message_bus_send_message_sync (wdata->bus, data->message);

    g_object_unref (vroot);
}

static void
store_begin_loading (XedFileBrowserStore *store,
             GtkTreeIter           *iter,
             MessageCacheData      *data)
{
    GtkTreePath *path;
    WindowData *wdata = get_window_data (data->window);

    path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), iter);

    set_item_message (wdata, iter, path, data->message);

    xed_message_bus_send_message_sync (wdata->bus, data->message);
    gtk_tree_path_free (path);
}

static void
store_end_loading (XedFileBrowserStore *store,
           GtkTreeIter           *iter,
           MessageCacheData      *data)
{
    GtkTreePath *path;
    WindowData *wdata = get_window_data (data->window);

    path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), iter);

    set_item_message (wdata, iter, path, data->message);

    xed_message_bus_send_message_sync (wdata->bus, data->message);
    gtk_tree_path_free (path);
}

static void
register_signals (XedWindow            *window,
          XedFileBrowserWidget *widget)
{
    XedMessageBus *bus = xed_window_get_message_bus (window);
    XedFileBrowserStore *store;
    XedMessageType *inserted_type;
    XedMessageType *deleted_type;
    XedMessageType *begin_loading_type;
    XedMessageType *end_loading_type;
    XedMessageType *root_changed_type;

    XedMessage *message;
    WindowData *data;

    /* Register signals */
    root_changed_type = xed_message_bus_register (bus,
                    MESSAGE_OBJECT_PATH, "root_changed",
                    0,
                    "id", G_TYPE_STRING,
                    "location", G_TYPE_FILE,
                    NULL);

    begin_loading_type = xed_message_bus_register (bus,
                    MESSAGE_OBJECT_PATH, "begin_loading",
                    0,
                    "id", G_TYPE_STRING,
                    "location", G_TYPE_FILE,
                    NULL);

    end_loading_type = xed_message_bus_register (bus,
                    MESSAGE_OBJECT_PATH, "end_loading",
                    0,
                    "id", G_TYPE_STRING,
                    "location", G_TYPE_FILE,
                    NULL);

    inserted_type = xed_message_bus_register (bus,
                            MESSAGE_OBJECT_PATH, "inserted",
                            0,
                            "id", G_TYPE_STRING,
                            "location", G_TYPE_FILE,
                            "is_directory", G_TYPE_BOOLEAN,
                            NULL);

    deleted_type = xed_message_bus_register (bus,
                           MESSAGE_OBJECT_PATH, "deleted",
                           0,
                           "id", G_TYPE_STRING,
                           "location", G_TYPE_FILE,
                           "is_directory", G_TYPE_BOOLEAN,
                           NULL);

    store = xed_file_browser_widget_get_browser_store (widget);

    message = xed_message_type_instantiate (inserted_type,
                          "id", NULL,
                          "location", NULL,
                          "is_directory", FALSE,
                          NULL);

    data = get_window_data (window);

    data->row_inserted_id =
        g_signal_connect_data (store,
                       "row-inserted",
                       G_CALLBACK (store_row_inserted),
                       message_cache_data_new (window, message),
                       (GClosureNotify)message_cache_data_free,
                       0);

    message = xed_message_type_instantiate (deleted_type,
                          "id", NULL,
                          "location", NULL,
                          "is_directory", FALSE,
                          NULL);
    data->row_deleted_id =
        g_signal_connect_data (store,
                       "row-deleted",
                       G_CALLBACK (store_row_deleted),
                       message_cache_data_new (window, message),
                       (GClosureNotify)message_cache_data_free,
                       0);

    message = xed_message_type_instantiate (root_changed_type,
                          "id", NULL,
                          "location", NULL,
                          NULL);
    data->root_changed_id =
        g_signal_connect_data (store,
                       "notify::virtual-root",
                       G_CALLBACK (store_virtual_root_changed),
                       message_cache_data_new (window, message),
                       (GClosureNotify)message_cache_data_free,
                       0);

    message = xed_message_type_instantiate (begin_loading_type,
                          "id", NULL,
                          "location", NULL,
                          NULL);
    data->begin_loading_id =
        g_signal_connect_data (store,
                      "begin_loading",
                       G_CALLBACK (store_begin_loading),
                       message_cache_data_new (window, message),
                       (GClosureNotify)message_cache_data_free,
                       0);

    message = xed_message_type_instantiate (end_loading_type,
                          "id", NULL,
                          "location", NULL,
                          NULL);
    data->end_loading_id =
        g_signal_connect_data (store,
                       "end_loading",
                       G_CALLBACK (store_end_loading),
                       message_cache_data_new (window, message),
                       (GClosureNotify)message_cache_data_free,
                       0);
}

static void
message_unregistered (XedMessageBus  *bus,
              XedMessageType *message_type,
              XedWindow      *window)
{
    gchar *identifier = xed_message_type_identifier (xed_message_type_get_object_path (message_type),
                               xed_message_type_get_method (message_type));
    FilterData *data;
    WindowData *wdata = get_window_data (window);

    data = g_hash_table_lookup (wdata->filters, identifier);

    if (data)
        xed_file_browser_widget_remove_filter (wdata->widget, data->id);

    g_free (identifier);
}

void
xed_file_browser_messages_register (XedWindow            *window,
                      XedFileBrowserWidget *widget)
{
    window_data_new (window, widget);

    register_methods (window, widget);
    register_signals (window, widget);

    g_signal_connect (xed_window_get_message_bus (window),
              "unregistered",
              G_CALLBACK (message_unregistered),
              window);
}

static void
cleanup_signals (XedWindow *window)
{
    WindowData *data = get_window_data (window);
    XedFileBrowserStore *store;

    store = xed_file_browser_widget_get_browser_store (data->widget);

    g_signal_handler_disconnect (store, data->row_inserted_id);
    g_signal_handler_disconnect (store, data->row_deleted_id);
    g_signal_handler_disconnect (store, data->root_changed_id);
    g_signal_handler_disconnect (store, data->begin_loading_id);
    g_signal_handler_disconnect (store, data->end_loading_id);

    g_signal_handlers_disconnect_by_func (data->bus, message_unregistered, window);
}

void
xed_file_browser_messages_unregister (XedWindow *window)
{
    XedMessageBus *bus = xed_window_get_message_bus (window);

    cleanup_signals (window);
    xed_message_bus_unregister_all (bus, MESSAGE_OBJECT_PATH);

    window_data_free (window);
}
