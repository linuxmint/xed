#include "xedit-file-browser-messages.h"
#include "xedit-file-browser-store.h"
#include <xedit/xedit-message.h>

#define MESSAGE_OBJECT_PATH 	"/plugins/filebrowser"
#define WINDOW_DATA_KEY	       	"XeditFileBrowserMessagesWindowData"

#define BUS_CONNECT(bus, name, data) xedit_message_bus_connect(bus, MESSAGE_OBJECT_PATH, #name, (XeditMessageCallback)  message_##name##_cb, data, NULL)

typedef struct
{
	XeditWindow *window;	
	XeditMessage *message;
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
	
	XeditMessageBus *bus;
	XeditFileBrowserWidget *widget;
	GHashTable *row_tracking;
	
	GHashTable *filters;
} WindowData;

typedef struct
{
	gulong id;
	
	XeditWindow *window;
	XeditMessage *message;
} FilterData;

static WindowData *
window_data_new (XeditWindow            *window,
		 XeditFileBrowserWidget *widget)
{
	WindowData *data = g_slice_new (WindowData);
	GtkUIManager *manager;
	GList *groups;
	
	data->bus = xedit_window_get_message_bus (window);
	data->widget = widget;
	data->row_tracking = g_hash_table_new_full (g_str_hash, 
						    g_str_equal,
						    (GDestroyNotify)g_free,
						    (GDestroyNotify)gtk_tree_row_reference_free);

	data->filters = g_hash_table_new_full (g_str_hash,
					       g_str_equal,
					       (GDestroyNotify)g_free,
					       NULL);
	
	manager = xedit_file_browser_widget_get_ui_manager (widget);

	data->merge_ids = NULL;
	data->merged_actions = gtk_action_group_new ("MessageMergedActions");
	
	groups = gtk_ui_manager_get_action_groups (manager);
	gtk_ui_manager_insert_action_group (manager, data->merged_actions, g_list_length (groups));
	
	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, data);

	return data;
}

static WindowData *
get_window_data (XeditWindow * window)
{
	return (WindowData *) (g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY));
}

static void
window_data_free (XeditWindow *window)
{
	WindowData *data = get_window_data (window);
	GtkUIManager *manager;
	GList *item;
		
	g_hash_table_destroy (data->row_tracking);	
	g_hash_table_destroy (data->filters);

	manager = xedit_file_browser_widget_get_ui_manager (data->widget);
	gtk_ui_manager_remove_action_group (manager, data->merged_actions);
	
	for (item = data->merge_ids; item; item = item->next)
		gtk_ui_manager_remove_ui (manager, GPOINTER_TO_INT (item->data));

	g_list_free (data->merge_ids);
	g_object_unref (data->merged_actions);

	g_slice_free (WindowData, data);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static FilterData *
filter_data_new (XeditWindow  *window,  
		 XeditMessage *message)
{
	FilterData *data = g_slice_new (FilterData);
	WindowData *wdata;
	
	data->window = window;
	data->id = 0;
	data->message = message;
	
	wdata = get_window_data (window);
	
	g_hash_table_insert (wdata->filters, 
			     xedit_message_type_identifier (xedit_message_get_object_path (message),
			                                    xedit_message_get_method (message)),
			     data);

	return data;
}

static void
filter_data_free (FilterData *data)
{
	WindowData *wdata = get_window_data (data->window);
	gchar *identifier;
	
	identifier = xedit_message_type_identifier (xedit_message_get_object_path (data->message),
			                            xedit_message_get_method (data->message));
			                            
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
message_cache_data_new (XeditWindow            *window,
			XeditMessage           *message)
{
	MessageCacheData *data = g_slice_new (MessageCacheData);
	
	data->window = window;
	data->message = message;
	
	return data;
}

static void
message_get_root_cb (XeditMessageBus *bus,
		     XeditMessage    *message,
		     WindowData      *data)
{
	XeditFileBrowserStore *store;
	gchar *uri;
	
	store = xedit_file_browser_widget_get_browser_store (data->widget);
	uri = xedit_file_browser_store_get_virtual_root (store);
	
	xedit_message_set (message, "uri", uri, NULL);
	g_free (uri);
}

static void
message_set_root_cb (XeditMessageBus *bus,
		     XeditMessage    *message,
		     WindowData      *data)
{
	gchar *root = NULL;
	gchar *virtual = NULL;
	
	xedit_message_get (message, "uri", &root, NULL);
	
	if (!root)
		return;
	
	if (xedit_message_has_key (message, "virtual"))
		xedit_message_get (message, "virtual", &virtual, NULL);

	if (virtual)
		xedit_file_browser_widget_set_root_and_virtual_root (data->widget, root, virtual);
	else
		xedit_file_browser_widget_set_root (data->widget, root, TRUE);
	
	g_free (root);
	g_free (virtual);
}

static void
message_set_emblem_cb (XeditMessageBus *bus,
		       XeditMessage    *message,
		       WindowData      *data)
{
	gchar *id = NULL;
	gchar *emblem = NULL;
	GtkTreePath *path;
	XeditFileBrowserStore *store;
	
	xedit_message_get (message, "id", &id, "emblem", &emblem, NULL);
	
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
			
			store = xedit_file_browser_widget_get_browser_store (data->widget);
			
			if (gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path))
			{
				g_value_init (&value, GDK_TYPE_PIXBUF);
				g_value_set_object (&value, pixbuf);
			
				xedit_file_browser_store_set_value (store, 
								    &iter,
								    XEDIT_FILE_BROWSER_STORE_COLUMN_EMBLEM,
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
	 const gchar *uri)
{
	return g_strconcat (path, "::", uri, NULL);
}

static gchar *
track_row (WindowData            *data,
	   XeditFileBrowserStore *store,
	   GtkTreePath           *path,
	   const gchar		 *uri)
{
	GtkTreeRowReference *ref;
	gchar *id;
	gchar *pathstr;
	
	pathstr = gtk_tree_path_to_string (path);
	id = item_id (pathstr, uri);
	
	ref = gtk_tree_row_reference_new (GTK_TREE_MODEL (store), path);
	g_hash_table_insert (data->row_tracking, g_strdup (id), ref);
	
	g_free (pathstr);
	
	return id;
}

static void
set_item_message (WindowData   *data, 
		  GtkTreeIter  *iter,
		  GtkTreePath  *path,
		  XeditMessage *message)
{
	XeditFileBrowserStore *store;
	gchar *uri = NULL;
	guint flags = 0;
	gchar *track_id;
	
	store = xedit_file_browser_widget_get_browser_store (data->widget);
	
	gtk_tree_model_get (GTK_TREE_MODEL (store), iter,
			    XEDIT_FILE_BROWSER_STORE_COLUMN_URI, &uri,
			    XEDIT_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags,
			    -1);
	
	if (!uri)
		return;

	if (path && gtk_tree_path_get_depth (path) != 0)
		track_id = track_row (data, store, path, uri);
	else
		track_id = NULL;

	xedit_message_set (message,
			   "id", track_id,
			   "uri", uri,
			   NULL);
	
	if (xedit_message_has_key (message, "is_directory"))
	{
		xedit_message_set (message, 
				   "is_directory", FILE_IS_DIR (flags),
				   NULL);
	}			   

	g_free (uri);
	g_free (track_id);
}

static gboolean
custom_message_filter_func (XeditFileBrowserWidget *widget,
			    XeditFileBrowserStore  *store,
			    GtkTreeIter            *iter,
			    FilterData             *data)
{
	WindowData *wdata = get_window_data (data->window);
	gchar *uri = NULL;
	guint flags = 0;
	gboolean filter = FALSE;
	GtkTreePath *path;
	
	gtk_tree_model_get (GTK_TREE_MODEL (store), iter, 
			    XEDIT_FILE_BROWSER_STORE_COLUMN_URI, &uri,
			    XEDIT_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags,
			    -1);
	
	if (!uri || FILE_IS_DUMMY (flags))
	{
		g_free (uri);
		return FALSE;
	}
	
	path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), iter);
	set_item_message (wdata, iter, path, data->message);
	gtk_tree_path_free (path);
	
	xedit_message_set (data->message, "filter", filter, NULL);

	xedit_message_bus_send_message_sync (wdata->bus, data->message);
	xedit_message_get (data->message, "filter", &filter, NULL);
	
	return !filter;
}

static void
message_add_filter_cb (XeditMessageBus *bus,
		       XeditMessage    *message,
		       XeditWindow     *window)
{
	gchar *object_path = NULL;
	gchar *method = NULL;
	gulong id;
	XeditMessageType *message_type;
	XeditMessage *cbmessage;
	FilterData *filter_data;
	WindowData *data = get_window_data (window);
	
	xedit_message_get (message, 
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
	
	message_type = xedit_message_bus_lookup (bus, object_path, method);
	
	if (!message_type)
	{
		g_free (object_path);
		g_free (method);
		
		return;
	}
	
	// Check if the message type has the correct arguments
	if (xedit_message_type_lookup (message_type, "id") != G_TYPE_STRING ||
	    xedit_message_type_lookup (message_type, "uri") != G_TYPE_STRING ||
	    xedit_message_type_lookup (message_type, "is_directory") != G_TYPE_BOOLEAN ||
	    xedit_message_type_lookup (message_type, "filter") != G_TYPE_BOOLEAN)
	{
		return;
	}
	
	cbmessage = xedit_message_type_instantiate (message_type,
						    "id", NULL,
						    "uri", NULL,
						    "is_directory", FALSE,
						    "filter", FALSE,
						    NULL);

	// Register the custom filter on the widget
	filter_data = filter_data_new (window, cbmessage);
	id = xedit_file_browser_widget_add_filter (data->widget, 
						   (XeditFileBrowserWidgetFilterFunc)custom_message_filter_func,
						   filter_data,
						   (GDestroyNotify)filter_data_free);

	filter_data->id = id;
}

static void
message_remove_filter_cb (XeditMessageBus *bus,
		          XeditMessage    *message,
		          WindowData      *data)
{
	gulong id = 0;
	
	xedit_message_get (message, "id", &id, NULL);
	
	if (!id)
		return;
	
	xedit_file_browser_widget_remove_filter (data->widget, id);
}

static void
message_up_cb (XeditMessageBus *bus,
	       XeditMessage    *message,
	       WindowData      *data)
{
	XeditFileBrowserStore *store = xedit_file_browser_widget_get_browser_store (data->widget);
	
	xedit_file_browser_store_set_virtual_root_up (store);
}

static void
message_history_back_cb (XeditMessageBus *bus,
		         XeditMessage    *message,
		         WindowData      *data)
{
	xedit_file_browser_widget_history_back (data->widget);
}

static void
message_history_forward_cb (XeditMessageBus *bus,
		            XeditMessage    *message,
		            WindowData      *data)
{
	xedit_file_browser_widget_history_forward (data->widget);
}

static void
message_refresh_cb (XeditMessageBus *bus,
		    XeditMessage    *message,
		    WindowData      *data)
{
	xedit_file_browser_widget_refresh (data->widget);
}

static void
message_set_show_hidden_cb (XeditMessageBus *bus,
		            XeditMessage    *message,
		            WindowData      *data)
{
	gboolean active = FALSE;
	XeditFileBrowserStore *store;
	XeditFileBrowserStoreFilterMode mode;
	
	xedit_message_get (message, "active", &active, NULL);
	
	store = xedit_file_browser_widget_get_browser_store (data->widget);
	mode = xedit_file_browser_store_get_filter_mode (store);
	
	if (active)
		mode &= ~XEDIT_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN;
	else
		mode |= XEDIT_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN;

	xedit_file_browser_store_set_filter_mode (store, mode);
}

static void
message_set_show_binary_cb (XeditMessageBus *bus,
		            XeditMessage    *message,
		            WindowData      *data)
{
	gboolean active = FALSE;
	XeditFileBrowserStore *store;
	XeditFileBrowserStoreFilterMode mode;
	
	xedit_message_get (message, "active", &active, NULL);
	
	store = xedit_file_browser_widget_get_browser_store (data->widget);
	mode = xedit_file_browser_store_get_filter_mode (store);
	
	if (active)
		mode &= ~XEDIT_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY;
	else
		mode |= XEDIT_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY;

	xedit_file_browser_store_set_filter_mode (store, mode);
}

static void
message_show_bookmarks_cb (XeditMessageBus *bus,
		           XeditMessage    *message,
		           WindowData      *data)
{
	xedit_file_browser_widget_show_bookmarks (data->widget);
}

static void
message_show_files_cb (XeditMessageBus *bus,
		       XeditMessage    *message,
		       WindowData      *data)
{
	xedit_file_browser_widget_show_files (data->widget);
}

static void
message_add_context_item_cb (XeditMessageBus *bus,
			     XeditMessage    *message,
			     WindowData      *data)
{
	GtkAction *action = NULL;
	gchar *path = NULL;
	gchar *name;
	GtkUIManager *manager;
	guint merge_id;
	
	xedit_message_get (message, 
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
	manager = xedit_file_browser_widget_get_ui_manager (data->widget);
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
		xedit_message_set (message, "id", merge_id, NULL);
	}
	else
	{
		xedit_message_set (message, "id", 0, NULL);
	}
	
	g_object_unref (action);
	g_free (path);
	g_free (name);
}

static void
message_remove_context_item_cb (XeditMessageBus *bus,
				XeditMessage    *message,
				WindowData      *data)
{
	guint merge_id = 0;
	GtkUIManager *manager;
	
	xedit_message_get (message, "id", &merge_id, NULL);
	
	if (merge_id == 0)
		return;
	
	manager = xedit_file_browser_widget_get_ui_manager (data->widget);
		
	data->merge_ids = g_list_remove (data->merge_ids, GINT_TO_POINTER (merge_id));
	gtk_ui_manager_remove_ui (manager, merge_id);
}

static void
message_get_view_cb (XeditMessageBus *bus,
		     XeditMessage    *message,
		     WindowData      *data)
{
	XeditFileBrowserView *view;
	view = xedit_file_browser_widget_get_browser_view (data->widget);

	xedit_message_set (message, "view", view, NULL);
}

static void
register_methods (XeditWindow            *window,
		  XeditFileBrowserWidget *widget)
{
	XeditMessageBus *bus = xedit_window_get_message_bus (window);
	WindowData *data = get_window_data (window);

	/* Register method calls */
	xedit_message_bus_register (bus, 
				    MESSAGE_OBJECT_PATH, "get_root", 
				    1,
				    "uri", G_TYPE_STRING,
				    NULL);
 
	xedit_message_bus_register (bus, 
				    MESSAGE_OBJECT_PATH, "set_root", 
				    1, 
				    "uri", G_TYPE_STRING,
				    "virtual", G_TYPE_STRING,
				    NULL);
				    
	xedit_message_bus_register (bus,
				    MESSAGE_OBJECT_PATH, "set_emblem",
				    0,
				    "id", G_TYPE_STRING,
				    "emblem", G_TYPE_STRING,
				    NULL);
	
	xedit_message_bus_register (bus,
				    MESSAGE_OBJECT_PATH, "add_filter",
				    1,
				    "object_path", G_TYPE_STRING,
				    "method", G_TYPE_STRING,
				    "id", G_TYPE_ULONG,
				    NULL);
	
	xedit_message_bus_register (bus,
				    MESSAGE_OBJECT_PATH, "remove_filter",
				    0,
				    "id", G_TYPE_ULONG,
				    NULL);
	
	xedit_message_bus_register (bus,
				    MESSAGE_OBJECT_PATH, "add_context_item",
				    1,
				    "action", GTK_TYPE_ACTION,
				    "path", G_TYPE_STRING,
				    "id", G_TYPE_UINT,
				    NULL);

	xedit_message_bus_register (bus,
				    MESSAGE_OBJECT_PATH, "remove_context_item",
				    0,
				    "id", G_TYPE_UINT,
				    NULL);
	
	xedit_message_bus_register (bus, MESSAGE_OBJECT_PATH, "up", 0, NULL);
	
	xedit_message_bus_register (bus, MESSAGE_OBJECT_PATH, "history_back", 0, NULL);
	xedit_message_bus_register (bus, MESSAGE_OBJECT_PATH, "history_forward", 0, NULL);
	
	xedit_message_bus_register (bus, MESSAGE_OBJECT_PATH, "refresh", 0, NULL);

	xedit_message_bus_register (bus, 
				    MESSAGE_OBJECT_PATH, "set_show_hidden", 
				    0, 
				    "active", G_TYPE_BOOLEAN,
				    NULL);
	xedit_message_bus_register (bus, 
				    MESSAGE_OBJECT_PATH, "set_show_binary",
				    0, 
				    "active", G_TYPE_BOOLEAN,
				    NULL);

	xedit_message_bus_register (bus, MESSAGE_OBJECT_PATH, "show_bookmarks", 0, NULL);
	xedit_message_bus_register (bus, MESSAGE_OBJECT_PATH, "show_files", 0, NULL);

	xedit_message_bus_register (bus,
				    MESSAGE_OBJECT_PATH, "get_view",
				    1,
				    "view", XEDIT_TYPE_FILE_BROWSER_VIEW,
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
store_row_inserted (XeditFileBrowserStore *store,
		    GtkTreePath		  *path,
		    GtkTreeIter           *iter,
		    MessageCacheData      *data)
{
	gchar *uri = NULL;
	guint flags = 0;

	gtk_tree_model_get (GTK_TREE_MODEL (store), iter, 
			    XEDIT_FILE_BROWSER_STORE_COLUMN_URI, &uri,
			    XEDIT_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags,
			    -1);
	
	if (!FILE_IS_DUMMY (flags) && !FILE_IS_FILTERED (flags))
	{
		WindowData *wdata = get_window_data (data->window);
		
		set_item_message (wdata, iter, path, data->message);
		xedit_message_bus_send_message_sync (wdata->bus, data->message);
	}
	
	g_free (uri);
}

static void
store_row_deleted (XeditFileBrowserStore *store,
		   GtkTreePath		 *path,
		   MessageCacheData      *data)
{
	GtkTreeIter iter;
	gchar *uri = NULL;
	guint flags = 0;
	
	if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path))
		return;
	
	gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, 
			    XEDIT_FILE_BROWSER_STORE_COLUMN_URI, &uri,
			    XEDIT_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags,
			    -1);
	
	if (!FILE_IS_DUMMY (flags) && !FILE_IS_FILTERED (flags))
	{
		WindowData *wdata = get_window_data (data->window);
		
		set_item_message (wdata, &iter, path, data->message);
		xedit_message_bus_send_message_sync (wdata->bus, data->message);
	}
	
	g_free (uri);
}

static void
store_virtual_root_changed (XeditFileBrowserStore *store,
			    GParamSpec            *spec,
			    MessageCacheData      *data)
{
	WindowData *wdata = get_window_data (data->window);
	gchar *uri;
	
	uri = xedit_file_browser_store_get_virtual_root (store);
	
	if (!uri)
		return;
	
	xedit_message_set (data->message,
			   "uri", uri,
			   NULL);
			   
	xedit_message_bus_send_message_sync (wdata->bus, data->message);
	
	g_free (uri);
}

static void
store_begin_loading (XeditFileBrowserStore *store,
		     GtkTreeIter           *iter,
		     MessageCacheData      *data)
{
	GtkTreePath *path;
	WindowData *wdata = get_window_data (data->window);
	
	path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), iter);
	
	set_item_message (wdata, iter, path, data->message);
	
	xedit_message_bus_send_message_sync (wdata->bus, data->message);
	gtk_tree_path_free (path);
}

static void
store_end_loading (XeditFileBrowserStore *store,
		   GtkTreeIter           *iter,
		   MessageCacheData      *data)
{
	GtkTreePath *path;
	WindowData *wdata = get_window_data (data->window);
	
	path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), iter);
	
	set_item_message (wdata, iter, path, data->message);
	
	xedit_message_bus_send_message_sync (wdata->bus, data->message);
	gtk_tree_path_free (path);
}
		    
static void
register_signals (XeditWindow            *window,
		  XeditFileBrowserWidget *widget)
{
	XeditMessageBus *bus = xedit_window_get_message_bus (window);
	XeditFileBrowserStore *store;
	XeditMessageType *inserted_type;
	XeditMessageType *deleted_type;
	XeditMessageType *begin_loading_type;
	XeditMessageType *end_loading_type;
	XeditMessageType *root_changed_type;
	
	XeditMessage *message;
	WindowData *data;

	/* Register signals */
	root_changed_type = xedit_message_bus_register (bus,
				    MESSAGE_OBJECT_PATH, "root_changed",
				    0,
				    "id", G_TYPE_STRING,
				    "uri", G_TYPE_STRING,
				    NULL);
	
	begin_loading_type = xedit_message_bus_register (bus,
				    MESSAGE_OBJECT_PATH, "begin_loading",
				    0,
				    "id", G_TYPE_STRING,
				    "uri", G_TYPE_STRING,
				    NULL);

	end_loading_type = xedit_message_bus_register (bus,
				    MESSAGE_OBJECT_PATH, "end_loading",
				    0,
				    "id", G_TYPE_STRING,
				    "uri", G_TYPE_STRING,
				    NULL);

	inserted_type = xedit_message_bus_register (bus,
						    MESSAGE_OBJECT_PATH, "inserted",
						    0,
						    "id", G_TYPE_STRING,
						    "uri", G_TYPE_STRING,
						    "is_directory", G_TYPE_BOOLEAN,
						    NULL);

	deleted_type = xedit_message_bus_register (bus,
						   MESSAGE_OBJECT_PATH, "deleted",
						   0,
						   "id", G_TYPE_STRING,
						   "uri", G_TYPE_STRING,
						   "is_directory", G_TYPE_BOOLEAN,
						   NULL);

	store = xedit_file_browser_widget_get_browser_store (widget);
	
	message = xedit_message_type_instantiate (inserted_type, 
						  "id", NULL,
						  "uri", NULL, 
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

	message = xedit_message_type_instantiate (deleted_type, 
						  "id", NULL, 
						  "uri", NULL,
						  "is_directory", FALSE, 
						  NULL);
	data->row_deleted_id = 
		g_signal_connect_data (store, 
				       "row-deleted", 
				       G_CALLBACK (store_row_deleted), 
				       message_cache_data_new (window, message),
				       (GClosureNotify)message_cache_data_free,
				       0);
	
	message = xedit_message_type_instantiate (root_changed_type,
						  "id", NULL,
						  "uri", NULL,
						  NULL);
	data->root_changed_id = 
		g_signal_connect_data (store,
				       "notify::virtual-root",
				       G_CALLBACK (store_virtual_root_changed),
				       message_cache_data_new (window, message),
				       (GClosureNotify)message_cache_data_free,
				       0);

	message = xedit_message_type_instantiate (begin_loading_type,
						  "id", NULL,
						  "uri", NULL,
						  NULL);	
	data->begin_loading_id = 
		g_signal_connect_data (store,
				      "begin_loading",
				       G_CALLBACK (store_begin_loading),
				       message_cache_data_new (window, message),
				       (GClosureNotify)message_cache_data_free,
				       0);

	message = xedit_message_type_instantiate (end_loading_type,
						  "id", NULL,
						  "uri", NULL,
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
message_unregistered (XeditMessageBus  *bus,
		      XeditMessageType *message_type,
		      XeditWindow      *window)
{
	gchar *identifier = xedit_message_type_identifier (xedit_message_type_get_object_path (message_type),
							   xedit_message_type_get_method (message_type));
	FilterData *data;
	WindowData *wdata = get_window_data (window);
	
	data = g_hash_table_lookup (wdata->filters, identifier);
	
	if (data)
		xedit_file_browser_widget_remove_filter (wdata->widget, data->id);
	
	g_free (identifier);
}

void 
xedit_file_browser_messages_register (XeditWindow            *window, 
				      XeditFileBrowserWidget *widget)
{
	window_data_new (window, widget);
	
	register_methods (window, widget);
	register_signals (window, widget);
	
	g_signal_connect (xedit_window_get_message_bus (window),
			  "unregistered",
			  G_CALLBACK (message_unregistered),
			  window);	
}

static void
cleanup_signals (XeditWindow *window)
{
	WindowData *data = get_window_data (window);
	XeditFileBrowserStore *store;
	
	store = xedit_file_browser_widget_get_browser_store (data->widget);
	
	g_signal_handler_disconnect (store, data->row_inserted_id);
	g_signal_handler_disconnect (store, data->row_deleted_id);
	g_signal_handler_disconnect (store, data->root_changed_id);
	g_signal_handler_disconnect (store, data->begin_loading_id);
	g_signal_handler_disconnect (store, data->end_loading_id);
	
	g_signal_handlers_disconnect_by_func (data->bus, message_unregistered, window);
}

void
xedit_file_browser_messages_unregister (XeditWindow *window)
{
	XeditMessageBus *bus = xedit_window_get_message_bus (window);
		
	cleanup_signals (window);
	xedit_message_bus_unregister_all (bus, MESSAGE_OBJECT_PATH);

	window_data_free (window);
}
