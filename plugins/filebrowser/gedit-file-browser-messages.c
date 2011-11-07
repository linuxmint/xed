#include "gedit-file-browser-messages.h"
#include "gedit-file-browser-store.h"
#include <gedit/gedit-message.h>

#define MESSAGE_OBJECT_PATH 	"/plugins/filebrowser"
#define WINDOW_DATA_KEY	       	"GeditFileBrowserMessagesWindowData"

#define BUS_CONNECT(bus, name, data) gedit_message_bus_connect(bus, MESSAGE_OBJECT_PATH, #name, (GeditMessageCallback)  message_##name##_cb, data, NULL)

typedef struct
{
	GeditWindow *window;	
	GeditMessage *message;
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
	
	GeditMessageBus *bus;
	GeditFileBrowserWidget *widget;
	GHashTable *row_tracking;
	
	GHashTable *filters;
} WindowData;

typedef struct
{
	gulong id;
	
	GeditWindow *window;
	GeditMessage *message;
} FilterData;

static WindowData *
window_data_new (GeditWindow            *window,
		 GeditFileBrowserWidget *widget)
{
	WindowData *data = g_slice_new (WindowData);
	GtkUIManager *manager;
	GList *groups;
	
	data->bus = gedit_window_get_message_bus (window);
	data->widget = widget;
	data->row_tracking = g_hash_table_new_full (g_str_hash, 
						    g_str_equal,
						    (GDestroyNotify)g_free,
						    (GDestroyNotify)gtk_tree_row_reference_free);

	data->filters = g_hash_table_new_full (g_str_hash,
					       g_str_equal,
					       (GDestroyNotify)g_free,
					       NULL);
	
	manager = gedit_file_browser_widget_get_ui_manager (widget);

	data->merge_ids = NULL;
	data->merged_actions = gtk_action_group_new ("MessageMergedActions");
	
	groups = gtk_ui_manager_get_action_groups (manager);
	gtk_ui_manager_insert_action_group (manager, data->merged_actions, g_list_length (groups));
	
	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, data);

	return data;
}

static WindowData *
get_window_data (GeditWindow * window)
{
	return (WindowData *) (g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY));
}

static void
window_data_free (GeditWindow *window)
{
	WindowData *data = get_window_data (window);
	GtkUIManager *manager;
	GList *item;
		
	g_hash_table_destroy (data->row_tracking);	
	g_hash_table_destroy (data->filters);

	manager = gedit_file_browser_widget_get_ui_manager (data->widget);
	gtk_ui_manager_remove_action_group (manager, data->merged_actions);
	
	for (item = data->merge_ids; item; item = item->next)
		gtk_ui_manager_remove_ui (manager, GPOINTER_TO_INT (item->data));

	g_list_free (data->merge_ids);
	g_object_unref (data->merged_actions);

	g_slice_free (WindowData, data);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static FilterData *
filter_data_new (GeditWindow  *window,  
		 GeditMessage *message)
{
	FilterData *data = g_slice_new (FilterData);
	WindowData *wdata;
	
	data->window = window;
	data->id = 0;
	data->message = message;
	
	wdata = get_window_data (window);
	
	g_hash_table_insert (wdata->filters, 
			     gedit_message_type_identifier (gedit_message_get_object_path (message),
			                                    gedit_message_get_method (message)),
			     data);

	return data;
}

static void
filter_data_free (FilterData *data)
{
	WindowData *wdata = get_window_data (data->window);
	gchar *identifier;
	
	identifier = gedit_message_type_identifier (gedit_message_get_object_path (data->message),
			                            gedit_message_get_method (data->message));
			                            
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
message_cache_data_new (GeditWindow            *window,
			GeditMessage           *message)
{
	MessageCacheData *data = g_slice_new (MessageCacheData);
	
	data->window = window;
	data->message = message;
	
	return data;
}

static void
message_get_root_cb (GeditMessageBus *bus,
		     GeditMessage    *message,
		     WindowData      *data)
{
	GeditFileBrowserStore *store;
	gchar *uri;
	
	store = gedit_file_browser_widget_get_browser_store (data->widget);
	uri = gedit_file_browser_store_get_virtual_root (store);
	
	gedit_message_set (message, "uri", uri, NULL);
	g_free (uri);
}

static void
message_set_root_cb (GeditMessageBus *bus,
		     GeditMessage    *message,
		     WindowData      *data)
{
	gchar *root = NULL;
	gchar *virtual = NULL;
	
	gedit_message_get (message, "uri", &root, NULL);
	
	if (!root)
		return;
	
	if (gedit_message_has_key (message, "virtual"))
		gedit_message_get (message, "virtual", &virtual, NULL);

	if (virtual)
		gedit_file_browser_widget_set_root_and_virtual_root (data->widget, root, virtual);
	else
		gedit_file_browser_widget_set_root (data->widget, root, TRUE);
	
	g_free (root);
	g_free (virtual);
}

static void
message_set_emblem_cb (GeditMessageBus *bus,
		       GeditMessage    *message,
		       WindowData      *data)
{
	gchar *id = NULL;
	gchar *emblem = NULL;
	GtkTreePath *path;
	GeditFileBrowserStore *store;
	
	gedit_message_get (message, "id", &id, "emblem", &emblem, NULL);
	
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
			
			store = gedit_file_browser_widget_get_browser_store (data->widget);
			
			if (gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path))
			{
				g_value_init (&value, GDK_TYPE_PIXBUF);
				g_value_set_object (&value, pixbuf);
			
				gedit_file_browser_store_set_value (store, 
								    &iter,
								    GEDIT_FILE_BROWSER_STORE_COLUMN_EMBLEM,
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
	   GeditFileBrowserStore *store,
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
		  GeditMessage *message)
{
	GeditFileBrowserStore *store;
	gchar *uri = NULL;
	guint flags = 0;
	gchar *track_id;
	
	store = gedit_file_browser_widget_get_browser_store (data->widget);
	
	gtk_tree_model_get (GTK_TREE_MODEL (store), iter,
			    GEDIT_FILE_BROWSER_STORE_COLUMN_URI, &uri,
			    GEDIT_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags,
			    -1);
	
	if (!uri)
		return;

	if (path && gtk_tree_path_get_depth (path) != 0)
		track_id = track_row (data, store, path, uri);
	else
		track_id = NULL;

	gedit_message_set (message,
			   "id", track_id,
			   "uri", uri,
			   NULL);
	
	if (gedit_message_has_key (message, "is_directory"))
	{
		gedit_message_set (message, 
				   "is_directory", FILE_IS_DIR (flags),
				   NULL);
	}			   

	g_free (uri);
	g_free (track_id);
}

static gboolean
custom_message_filter_func (GeditFileBrowserWidget *widget,
			    GeditFileBrowserStore  *store,
			    GtkTreeIter            *iter,
			    FilterData             *data)
{
	WindowData *wdata = get_window_data (data->window);
	gchar *uri = NULL;
	guint flags = 0;
	gboolean filter = FALSE;
	GtkTreePath *path;
	
	gtk_tree_model_get (GTK_TREE_MODEL (store), iter, 
			    GEDIT_FILE_BROWSER_STORE_COLUMN_URI, &uri,
			    GEDIT_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags,
			    -1);
	
	if (!uri || FILE_IS_DUMMY (flags))
	{
		g_free (uri);
		return FALSE;
	}
	
	path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), iter);
	set_item_message (wdata, iter, path, data->message);
	gtk_tree_path_free (path);
	
	gedit_message_set (data->message, "filter", filter, NULL);

	gedit_message_bus_send_message_sync (wdata->bus, data->message);
	gedit_message_get (data->message, "filter", &filter, NULL);
	
	return !filter;
}

static void
message_add_filter_cb (GeditMessageBus *bus,
		       GeditMessage    *message,
		       GeditWindow     *window)
{
	gchar *object_path = NULL;
	gchar *method = NULL;
	gulong id;
	GeditMessageType *message_type;
	GeditMessage *cbmessage;
	FilterData *filter_data;
	WindowData *data = get_window_data (window);
	
	gedit_message_get (message, 
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
	
	message_type = gedit_message_bus_lookup (bus, object_path, method);
	
	if (!message_type)
	{
		g_free (object_path);
		g_free (method);
		
		return;
	}
	
	// Check if the message type has the correct arguments
	if (gedit_message_type_lookup (message_type, "id") != G_TYPE_STRING ||
	    gedit_message_type_lookup (message_type, "uri") != G_TYPE_STRING ||
	    gedit_message_type_lookup (message_type, "is_directory") != G_TYPE_BOOLEAN ||
	    gedit_message_type_lookup (message_type, "filter") != G_TYPE_BOOLEAN)
	{
		return;
	}
	
	cbmessage = gedit_message_type_instantiate (message_type,
						    "id", NULL,
						    "uri", NULL,
						    "is_directory", FALSE,
						    "filter", FALSE,
						    NULL);

	// Register the custom filter on the widget
	filter_data = filter_data_new (window, cbmessage);
	id = gedit_file_browser_widget_add_filter (data->widget, 
						   (GeditFileBrowserWidgetFilterFunc)custom_message_filter_func,
						   filter_data,
						   (GDestroyNotify)filter_data_free);

	filter_data->id = id;
}

static void
message_remove_filter_cb (GeditMessageBus *bus,
		          GeditMessage    *message,
		          WindowData      *data)
{
	gulong id = 0;
	
	gedit_message_get (message, "id", &id, NULL);
	
	if (!id)
		return;
	
	gedit_file_browser_widget_remove_filter (data->widget, id);
}

static void
message_up_cb (GeditMessageBus *bus,
	       GeditMessage    *message,
	       WindowData      *data)
{
	GeditFileBrowserStore *store = gedit_file_browser_widget_get_browser_store (data->widget);
	
	gedit_file_browser_store_set_virtual_root_up (store);
}

static void
message_history_back_cb (GeditMessageBus *bus,
		         GeditMessage    *message,
		         WindowData      *data)
{
	gedit_file_browser_widget_history_back (data->widget);
}

static void
message_history_forward_cb (GeditMessageBus *bus,
		            GeditMessage    *message,
		            WindowData      *data)
{
	gedit_file_browser_widget_history_forward (data->widget);
}

static void
message_refresh_cb (GeditMessageBus *bus,
		    GeditMessage    *message,
		    WindowData      *data)
{
	gedit_file_browser_widget_refresh (data->widget);
}

static void
message_set_show_hidden_cb (GeditMessageBus *bus,
		            GeditMessage    *message,
		            WindowData      *data)
{
	gboolean active = FALSE;
	GeditFileBrowserStore *store;
	GeditFileBrowserStoreFilterMode mode;
	
	gedit_message_get (message, "active", &active, NULL);
	
	store = gedit_file_browser_widget_get_browser_store (data->widget);
	mode = gedit_file_browser_store_get_filter_mode (store);
	
	if (active)
		mode &= ~GEDIT_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN;
	else
		mode |= GEDIT_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN;

	gedit_file_browser_store_set_filter_mode (store, mode);
}

static void
message_set_show_binary_cb (GeditMessageBus *bus,
		            GeditMessage    *message,
		            WindowData      *data)
{
	gboolean active = FALSE;
	GeditFileBrowserStore *store;
	GeditFileBrowserStoreFilterMode mode;
	
	gedit_message_get (message, "active", &active, NULL);
	
	store = gedit_file_browser_widget_get_browser_store (data->widget);
	mode = gedit_file_browser_store_get_filter_mode (store);
	
	if (active)
		mode &= ~GEDIT_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY;
	else
		mode |= GEDIT_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY;

	gedit_file_browser_store_set_filter_mode (store, mode);
}

static void
message_show_bookmarks_cb (GeditMessageBus *bus,
		           GeditMessage    *message,
		           WindowData      *data)
{
	gedit_file_browser_widget_show_bookmarks (data->widget);
}

static void
message_show_files_cb (GeditMessageBus *bus,
		       GeditMessage    *message,
		       WindowData      *data)
{
	gedit_file_browser_widget_show_files (data->widget);
}

static void
message_add_context_item_cb (GeditMessageBus *bus,
			     GeditMessage    *message,
			     WindowData      *data)
{
	GtkAction *action = NULL;
	gchar *path = NULL;
	gchar *name;
	GtkUIManager *manager;
	guint merge_id;
	
	gedit_message_get (message, 
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
	manager = gedit_file_browser_widget_get_ui_manager (data->widget);
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
		gedit_message_set (message, "id", merge_id, NULL);
	}
	else
	{
		gedit_message_set (message, "id", 0, NULL);
	}
	
	g_object_unref (action);
	g_free (path);
	g_free (name);
}

static void
message_remove_context_item_cb (GeditMessageBus *bus,
				GeditMessage    *message,
				WindowData      *data)
{
	guint merge_id = 0;
	GtkUIManager *manager;
	
	gedit_message_get (message, "id", &merge_id, NULL);
	
	if (merge_id == 0)
		return;
	
	manager = gedit_file_browser_widget_get_ui_manager (data->widget);
		
	data->merge_ids = g_list_remove (data->merge_ids, GINT_TO_POINTER (merge_id));
	gtk_ui_manager_remove_ui (manager, merge_id);
}

static void
message_get_view_cb (GeditMessageBus *bus,
		     GeditMessage    *message,
		     WindowData      *data)
{
	GeditFileBrowserView *view;
	view = gedit_file_browser_widget_get_browser_view (data->widget);

	gedit_message_set (message, "view", view, NULL);
}

static void
register_methods (GeditWindow            *window,
		  GeditFileBrowserWidget *widget)
{
	GeditMessageBus *bus = gedit_window_get_message_bus (window);
	WindowData *data = get_window_data (window);

	/* Register method calls */
	gedit_message_bus_register (bus, 
				    MESSAGE_OBJECT_PATH, "get_root", 
				    1,
				    "uri", G_TYPE_STRING,
				    NULL);
 
	gedit_message_bus_register (bus, 
				    MESSAGE_OBJECT_PATH, "set_root", 
				    1, 
				    "uri", G_TYPE_STRING,
				    "virtual", G_TYPE_STRING,
				    NULL);
				    
	gedit_message_bus_register (bus,
				    MESSAGE_OBJECT_PATH, "set_emblem",
				    0,
				    "id", G_TYPE_STRING,
				    "emblem", G_TYPE_STRING,
				    NULL);
	
	gedit_message_bus_register (bus,
				    MESSAGE_OBJECT_PATH, "add_filter",
				    1,
				    "object_path", G_TYPE_STRING,
				    "method", G_TYPE_STRING,
				    "id", G_TYPE_ULONG,
				    NULL);
	
	gedit_message_bus_register (bus,
				    MESSAGE_OBJECT_PATH, "remove_filter",
				    0,
				    "id", G_TYPE_ULONG,
				    NULL);
	
	gedit_message_bus_register (bus,
				    MESSAGE_OBJECT_PATH, "add_context_item",
				    1,
				    "action", GTK_TYPE_ACTION,
				    "path", G_TYPE_STRING,
				    "id", G_TYPE_UINT,
				    NULL);

	gedit_message_bus_register (bus,
				    MESSAGE_OBJECT_PATH, "remove_context_item",
				    0,
				    "id", G_TYPE_UINT,
				    NULL);
	
	gedit_message_bus_register (bus, MESSAGE_OBJECT_PATH, "up", 0, NULL);
	
	gedit_message_bus_register (bus, MESSAGE_OBJECT_PATH, "history_back", 0, NULL);
	gedit_message_bus_register (bus, MESSAGE_OBJECT_PATH, "history_forward", 0, NULL);
	
	gedit_message_bus_register (bus, MESSAGE_OBJECT_PATH, "refresh", 0, NULL);

	gedit_message_bus_register (bus, 
				    MESSAGE_OBJECT_PATH, "set_show_hidden", 
				    0, 
				    "active", G_TYPE_BOOLEAN,
				    NULL);
	gedit_message_bus_register (bus, 
				    MESSAGE_OBJECT_PATH, "set_show_binary",
				    0, 
				    "active", G_TYPE_BOOLEAN,
				    NULL);

	gedit_message_bus_register (bus, MESSAGE_OBJECT_PATH, "show_bookmarks", 0, NULL);
	gedit_message_bus_register (bus, MESSAGE_OBJECT_PATH, "show_files", 0, NULL);

	gedit_message_bus_register (bus,
				    MESSAGE_OBJECT_PATH, "get_view",
				    1,
				    "view", GEDIT_TYPE_FILE_BROWSER_VIEW,
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
store_row_inserted (GeditFileBrowserStore *store,
		    GtkTreePath		  *path,
		    GtkTreeIter           *iter,
		    MessageCacheData      *data)
{
	gchar *uri = NULL;
	guint flags = 0;

	gtk_tree_model_get (GTK_TREE_MODEL (store), iter, 
			    GEDIT_FILE_BROWSER_STORE_COLUMN_URI, &uri,
			    GEDIT_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags,
			    -1);
	
	if (!FILE_IS_DUMMY (flags) && !FILE_IS_FILTERED (flags))
	{
		WindowData *wdata = get_window_data (data->window);
		
		set_item_message (wdata, iter, path, data->message);
		gedit_message_bus_send_message_sync (wdata->bus, data->message);
	}
	
	g_free (uri);
}

static void
store_row_deleted (GeditFileBrowserStore *store,
		   GtkTreePath		 *path,
		   MessageCacheData      *data)
{
	GtkTreeIter iter;
	gchar *uri = NULL;
	guint flags = 0;
	
	if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path))
		return;
	
	gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, 
			    GEDIT_FILE_BROWSER_STORE_COLUMN_URI, &uri,
			    GEDIT_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags,
			    -1);
	
	if (!FILE_IS_DUMMY (flags) && !FILE_IS_FILTERED (flags))
	{
		WindowData *wdata = get_window_data (data->window);
		
		set_item_message (wdata, &iter, path, data->message);
		gedit_message_bus_send_message_sync (wdata->bus, data->message);
	}
	
	g_free (uri);
}

static void
store_virtual_root_changed (GeditFileBrowserStore *store,
			    GParamSpec            *spec,
			    MessageCacheData      *data)
{
	WindowData *wdata = get_window_data (data->window);
	gchar *uri;
	
	uri = gedit_file_browser_store_get_virtual_root (store);
	
	if (!uri)
		return;
	
	gedit_message_set (data->message,
			   "uri", uri,
			   NULL);
			   
	gedit_message_bus_send_message_sync (wdata->bus, data->message);
	
	g_free (uri);
}

static void
store_begin_loading (GeditFileBrowserStore *store,
		     GtkTreeIter           *iter,
		     MessageCacheData      *data)
{
	GtkTreePath *path;
	WindowData *wdata = get_window_data (data->window);
	
	path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), iter);
	
	set_item_message (wdata, iter, path, data->message);
	
	gedit_message_bus_send_message_sync (wdata->bus, data->message);
	gtk_tree_path_free (path);
}

static void
store_end_loading (GeditFileBrowserStore *store,
		   GtkTreeIter           *iter,
		   MessageCacheData      *data)
{
	GtkTreePath *path;
	WindowData *wdata = get_window_data (data->window);
	
	path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), iter);
	
	set_item_message (wdata, iter, path, data->message);
	
	gedit_message_bus_send_message_sync (wdata->bus, data->message);
	gtk_tree_path_free (path);
}
		    
static void
register_signals (GeditWindow            *window,
		  GeditFileBrowserWidget *widget)
{
	GeditMessageBus *bus = gedit_window_get_message_bus (window);
	GeditFileBrowserStore *store;
	GeditMessageType *inserted_type;
	GeditMessageType *deleted_type;
	GeditMessageType *begin_loading_type;
	GeditMessageType *end_loading_type;
	GeditMessageType *root_changed_type;
	
	GeditMessage *message;
	WindowData *data;

	/* Register signals */
	root_changed_type = gedit_message_bus_register (bus,
				    MESSAGE_OBJECT_PATH, "root_changed",
				    0,
				    "id", G_TYPE_STRING,
				    "uri", G_TYPE_STRING,
				    NULL);
	
	begin_loading_type = gedit_message_bus_register (bus,
				    MESSAGE_OBJECT_PATH, "begin_loading",
				    0,
				    "id", G_TYPE_STRING,
				    "uri", G_TYPE_STRING,
				    NULL);

	end_loading_type = gedit_message_bus_register (bus,
				    MESSAGE_OBJECT_PATH, "end_loading",
				    0,
				    "id", G_TYPE_STRING,
				    "uri", G_TYPE_STRING,
				    NULL);

	inserted_type = gedit_message_bus_register (bus,
						    MESSAGE_OBJECT_PATH, "inserted",
						    0,
						    "id", G_TYPE_STRING,
						    "uri", G_TYPE_STRING,
						    "is_directory", G_TYPE_BOOLEAN,
						    NULL);

	deleted_type = gedit_message_bus_register (bus,
						   MESSAGE_OBJECT_PATH, "deleted",
						   0,
						   "id", G_TYPE_STRING,
						   "uri", G_TYPE_STRING,
						   "is_directory", G_TYPE_BOOLEAN,
						   NULL);

	store = gedit_file_browser_widget_get_browser_store (widget);
	
	message = gedit_message_type_instantiate (inserted_type, 
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

	message = gedit_message_type_instantiate (deleted_type, 
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
	
	message = gedit_message_type_instantiate (root_changed_type,
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

	message = gedit_message_type_instantiate (begin_loading_type,
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

	message = gedit_message_type_instantiate (end_loading_type,
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
message_unregistered (GeditMessageBus  *bus,
		      GeditMessageType *message_type,
		      GeditWindow      *window)
{
	gchar *identifier = gedit_message_type_identifier (gedit_message_type_get_object_path (message_type),
							   gedit_message_type_get_method (message_type));
	FilterData *data;
	WindowData *wdata = get_window_data (window);
	
	data = g_hash_table_lookup (wdata->filters, identifier);
	
	if (data)
		gedit_file_browser_widget_remove_filter (wdata->widget, data->id);
	
	g_free (identifier);
}

void 
gedit_file_browser_messages_register (GeditWindow            *window, 
				      GeditFileBrowserWidget *widget)
{
	window_data_new (window, widget);
	
	register_methods (window, widget);
	register_signals (window, widget);
	
	g_signal_connect (gedit_window_get_message_bus (window),
			  "unregistered",
			  G_CALLBACK (message_unregistered),
			  window);	
}

static void
cleanup_signals (GeditWindow *window)
{
	WindowData *data = get_window_data (window);
	GeditFileBrowserStore *store;
	
	store = gedit_file_browser_widget_get_browser_store (data->widget);
	
	g_signal_handler_disconnect (store, data->row_inserted_id);
	g_signal_handler_disconnect (store, data->row_deleted_id);
	g_signal_handler_disconnect (store, data->root_changed_id);
	g_signal_handler_disconnect (store, data->begin_loading_id);
	g_signal_handler_disconnect (store, data->end_loading_id);
	
	g_signal_handlers_disconnect_by_func (data->bus, message_unregistered, window);
}

void
gedit_file_browser_messages_unregister (GeditWindow *window)
{
	GeditMessageBus *bus = gedit_window_get_message_bus (window);
		
	cleanup_signals (window);
	gedit_message_bus_unregister_all (bus, MESSAGE_OBJECT_PATH);

	window_data_free (window);
}
