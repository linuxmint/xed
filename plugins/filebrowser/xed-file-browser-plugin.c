/*
 * xed-file-browser-plugin.c - Xed plugin providing easy file access
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

#include <xed/xed-commands.h>
#include <xed/xed-utils.h>
#include <xed/xed-app.h>
#include <glib/gi18n-lib.h>
#include <xed/xed-debug.h>
#include <gio/gio.h>
#include <string.h>

#include "xed-file-browser-enum-types.h"
#include "xed-file-browser-plugin.h"
#include "xed-file-browser-utils.h"
#include "xed-file-browser-error.h"
#include "xed-file-browser-widget.h"
#include "xed-file-browser-messages.h"

#define WINDOW_DATA_KEY	        	"XedFileBrowserPluginWindowData"

#define FILE_BROWSER_SCHEMA 		"org.x.editor.plugins.filebrowser"
#define FILE_BROWSER_ONLOAD_SCHEMA 	"org.x.editor.plugins.filebrowser.on-load"

#define XED_FILE_BROWSER_PLUGIN_GET_PRIVATE(object)	(G_TYPE_INSTANCE_GET_PRIVATE ((object), XED_TYPE_FILE_BROWSER_PLUGIN, XedFileBrowserPluginPrivate))

struct _XedFileBrowserPluginPrivate
{
	gpointer *dummy;
};

typedef struct _XedFileBrowserPluginData
{
	XedFileBrowserWidget * tree_widget;
	gulong                   merge_id;
	GtkActionGroup         * action_group;
	GtkActionGroup	       * single_selection_action_group;
	gboolean	         auto_root;
	gulong                   end_loading_handle;

	GSettings *settings;
	GSettings *onload_settings;
} XedFileBrowserPluginData;

static void on_uri_activated_cb          (XedFileBrowserWidget * widget,
                                          gchar const *uri,
                                          XedWindow * window);
static void on_error_cb                  (XedFileBrowserWidget * widget,
                                          guint code,
                                          gchar const *message,
                                          XedWindow * window);
static void on_model_set_cb              (XedFileBrowserView * widget,
                                          GParamSpec *arg1,
                                          XedWindow * window);
static void on_virtual_root_changed_cb   (XedFileBrowserStore * model,
                                          GParamSpec * param,
                                          XedWindow * window);
static void on_filter_mode_changed_cb    (XedFileBrowserStore * model,
                                          GParamSpec * param,
                                          XedWindow * window);
static void on_rename_cb		 (XedFileBrowserStore * model,
					  const gchar * olduri,
					  const gchar * newuri,
					  XedWindow * window);
static void on_filter_pattern_changed_cb (XedFileBrowserWidget * widget,
                                          GParamSpec * param,
                                          XedWindow * window);
static void on_tab_added_cb              (XedWindow * window,
                                          XedTab * tab,
                                          XedFileBrowserPluginData * data);
static gboolean on_confirm_delete_cb     (XedFileBrowserWidget * widget,
                                          XedFileBrowserStore * store,
                                          GList * rows,
                                          XedWindow * window);
static gboolean on_confirm_no_trash_cb   (XedFileBrowserWidget * widget,
                                          GList * files,
                                          XedWindow * window);

XED_PLUGIN_REGISTER_TYPE_WITH_CODE (XedFileBrowserPlugin, filetree_plugin, 	\
	xed_file_browser_enum_and_flag_register_type (type_module);		\
	xed_file_browser_store_register_type         (type_module);		\
	xed_file_bookmarks_store_register_type       (type_module);		\
	xed_file_browser_view_register_type	       (type_module);		\
	xed_file_browser_widget_register_type	       (type_module);		\
)


static void
filetree_plugin_init (XedFileBrowserPlugin * plugin)
{
	plugin->priv = XED_FILE_BROWSER_PLUGIN_GET_PRIVATE (plugin);
}

static void
filetree_plugin_finalize (GObject * object)
{
	//XedFileBrowserPlugin * plugin = XED_FILE_BROWSER_PLUGIN (object);

	G_OBJECT_CLASS (filetree_plugin_parent_class)->finalize (object);
}

static XedFileBrowserPluginData *
get_plugin_data (XedWindow * window)
{
	return (XedFileBrowserPluginData *) (g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY));
}

static void
on_end_loading_cb (XedFileBrowserStore      * store,
                   GtkTreeIter                * iter,
                   XedFileBrowserPluginData * data)
{
	/* Disconnect the signal */
	g_signal_handler_disconnect (store, data->end_loading_handle);
	data->end_loading_handle = 0;
	data->auto_root = FALSE;
}

static void
prepare_auto_root (XedFileBrowserPluginData *data)
{
	XedFileBrowserStore *store;

	data->auto_root = TRUE;

	store = xed_file_browser_widget_get_browser_store (data->tree_widget);

	if (data->end_loading_handle != 0) {
		g_signal_handler_disconnect (store, data->end_loading_handle);
		data->end_loading_handle = 0;
	}

	data->end_loading_handle = g_signal_connect (store,
	                                             "end-loading",
	                                             G_CALLBACK (on_end_loading_cb),
	                                             data);
}

static void
restore_default_location (XedFileBrowserPluginData *data)
{
	gchar * root;
	gchar * virtual_root;
	gboolean bookmarks;
	gboolean remote;

	bookmarks = !g_settings_get_boolean (data->onload_settings, "tree-view");

	if (bookmarks) {
		xed_file_browser_widget_show_bookmarks (data->tree_widget);
		return;
	}

	root = g_settings_get_string (data->onload_settings, "root");
	virtual_root = g_settings_get_string (data->onload_settings, "virtual-root");

	remote = g_settings_get_boolean (data->onload_settings, "enable-remote");

	if (root != NULL && *root != '\0') {
		GFile *file;

		file = g_file_new_for_uri (root);

		if (remote || g_file_is_native (file)) {
			if (virtual_root != NULL && *virtual_root != '\0') {
				prepare_auto_root (data);
				xed_file_browser_widget_set_root_and_virtual_root (data->tree_widget,
					                                             root,
					                                             virtual_root);
			} else {
				prepare_auto_root (data);
				xed_file_browser_widget_set_root (data->tree_widget,
					                            root,
					                            TRUE);
			}
		}

		g_object_unref (file);
	}

	g_free (root);
	g_free (virtual_root);
}

static void
restore_filter (XedFileBrowserPluginData * data)
{
	gchar *filter_mode;
	XedFileBrowserStoreFilterMode mode;
	gchar *pattern;

	/* Get filter_mode */
	filter_mode = g_settings_get_string (data->settings, "filter-mode");

	/* Filter mode */
	mode = xed_file_browser_store_filter_mode_get_default ();

	if (filter_mode != NULL) {
		if (strcmp (filter_mode, "hidden") == 0) {
			mode = XED_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN;
		} else if (strcmp (filter_mode, "binary") == 0) {
			mode = XED_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY;
		} else if (strcmp (filter_mode, "hidden_and_binary") == 0 ||
		         strcmp (filter_mode, "binary_and_hidden") == 0) {
			mode = XED_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN |
			       XED_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY;
		} else if (strcmp (filter_mode, "none") == 0 ||
		           *filter_mode == '\0') {
			mode = XED_FILE_BROWSER_STORE_FILTER_MODE_NONE;
		}
	}

	/* Set the filter mode */
	xed_file_browser_store_set_filter_mode (
	    xed_file_browser_widget_get_browser_store (data->tree_widget),
	    mode);

	pattern = g_settings_get_string (data->settings, "filter-pattern");

	xed_file_browser_widget_set_filter_pattern (data->tree_widget,
	                                              pattern);

	g_free (filter_mode);
	g_free (pattern);
}

static void
set_root_from_doc (XedFileBrowserPluginData * data,
                   XedDocument * doc)
{
	GFile *file;
	GFile *parent;

	if (doc == NULL)
		return;

	file = xed_document_get_location (doc);
	if (file == NULL)
		return;

	parent = g_file_get_parent (file);

	if (parent != NULL) {
		gchar * root;

		root = g_file_get_uri (parent);

		xed_file_browser_widget_set_root (data->tree_widget,
				                    root,
				                    TRUE);

		g_object_unref (parent);
		g_free (root);
	}

	g_object_unref (file);
}

static void
on_action_set_active_root (GtkAction * action,
                           XedWindow * window)
{
	XedFileBrowserPluginData *data;

	data = get_plugin_data (window);
	set_root_from_doc (data,
	                   xed_window_get_active_document (window));
}

static gchar *
get_terminal (XedFileBrowserPluginData * data)
{
	// TODO : Identify the DE, find the preferred terminal application (xterminal shouldn't be hardcoded here, it should be set as default in the DE prefs)
	return g_strdup ("xterminal");
}

static void
on_action_open_terminal (GtkAction * action,
                         XedWindow * window)
{
	XedFileBrowserPluginData * data;
	gchar * terminal;
	gchar * wd = NULL;
	gchar * local;
	gchar * argv[2];
	GFile * file;

	GtkTreeIter iter;
	XedFileBrowserStore * store;

	data = get_plugin_data (window);

	/* Get the current directory */
	if (!xed_file_browser_widget_get_selected_directory (data->tree_widget, &iter))
		return;

	store = xed_file_browser_widget_get_browser_store (data->tree_widget);
	gtk_tree_model_get (GTK_TREE_MODEL (store),
	                    &iter,
	                    XED_FILE_BROWSER_STORE_COLUMN_URI,
	                    &wd,
	                    -1);

	if (wd == NULL)
		return;

	terminal = get_terminal (data);

	file = g_file_new_for_uri (wd);
	local = g_file_get_path (file);
	g_object_unref (file);

	argv[0] = terminal;
	argv[1] = NULL;

	g_spawn_async (local,
	               argv,
	               NULL,
	               G_SPAWN_SEARCH_PATH,
	               NULL,
	               NULL,
	               NULL,
	               NULL);

	g_free (terminal);
	g_free (wd);
	g_free (local);
}

static void
on_selection_changed_cb (GtkTreeSelection *selection,
			 XedWindow      *window)
{
	XedFileBrowserPluginData * data;
	GtkTreeView * tree_view;
	GtkTreeModel * model;
	GtkTreeIter iter;
	gboolean sensitive;
	gchar * uri;

	data = get_plugin_data (window);

	tree_view = GTK_TREE_VIEW (xed_file_browser_widget_get_browser_view (data->tree_widget));
	model = gtk_tree_view_get_model (tree_view);

	if (!XED_IS_FILE_BROWSER_STORE (model))
		return;

	sensitive = xed_file_browser_widget_get_selected_directory (data->tree_widget, &iter);

	if (sensitive) {
		gtk_tree_model_get (model, &iter,
				    XED_FILE_BROWSER_STORE_COLUMN_URI,
				    &uri, -1);

		sensitive = xed_utils_uri_has_file_scheme (uri);
		g_free (uri);
	}

	gtk_action_set_sensitive (
		gtk_action_group_get_action (data->single_selection_action_group,
                                            "OpenTerminal"),
		sensitive);
}

#define POPUP_UI ""                             \
"<ui>"                                          \
"  <popup name=\"FilePopup\">"                  \
"    <placeholder name=\"FilePopup_Opt1\">"     \
"      <menuitem action=\"SetActiveRoot\"/>"    \
"    </placeholder>"                            \
"    <placeholder name=\"FilePopup_Opt4\">"     \
"      <menuitem action=\"OpenTerminal\"/>"     \
"    </placeholder>"                            \
"  </popup>"                                    \
"  <popup name=\"BookmarkPopup\">"              \
"    <placeholder name=\"BookmarkPopup_Opt1\">" \
"      <menuitem action=\"SetActiveRoot\"/>"    \
"    </placeholder>"                            \
"  </popup>"                                    \
"</ui>"

static GtkActionEntry extra_actions[] =
{
	{"SetActiveRoot", "go-jump-symbolic", N_("_Set root to active document"),
	 NULL,
	 N_("Set the root to the active document location"),
	 G_CALLBACK (on_action_set_active_root)}
};

static GtkActionEntry extra_single_selection_actions[] = {
	{"OpenTerminal", "utilities-terminal", N_("_Open terminal here"),
	 NULL,
	 N_("Open a terminal at the currently opened directory"),
	 G_CALLBACK (on_action_open_terminal)}
};

static void
add_popup_ui (XedWindow * window)
{
	XedFileBrowserPluginData * data;
	GtkUIManager * manager;
	GtkActionGroup * action_group;
	GError * error = NULL;

	data = get_plugin_data (window);
	manager = xed_file_browser_widget_get_ui_manager (data->tree_widget);

	action_group = gtk_action_group_new ("FileBrowserPluginExtra");
	gtk_action_group_set_translation_domain (action_group, NULL);
	gtk_action_group_add_actions (action_group,
				      extra_actions,
				      G_N_ELEMENTS (extra_actions),
				      window);
	gtk_ui_manager_insert_action_group (manager, action_group, 0);
	data->action_group = action_group;

	action_group = gtk_action_group_new ("FileBrowserPluginSingleSelectionExtra");
	gtk_action_group_set_translation_domain (action_group, NULL);
	gtk_action_group_add_actions (action_group,
				      extra_single_selection_actions,
				      G_N_ELEMENTS (extra_single_selection_actions),
				      window);
	gtk_ui_manager_insert_action_group (manager, action_group, 0);
	data->single_selection_action_group = action_group;

	data->merge_id = gtk_ui_manager_add_ui_from_string (manager,
	                                                    POPUP_UI,
	                                                    -1,
	                                                    &error);

	if (data->merge_id == 0) {
		g_warning("Unable to merge UI: %s", error->message);
		g_error_free(error);
	}
}

static void
remove_popup_ui (XedWindow * window)
{
	XedFileBrowserPluginData * data;
	GtkUIManager * manager;

	data = get_plugin_data (window);
	manager = xed_file_browser_widget_get_ui_manager (data->tree_widget);
	gtk_ui_manager_remove_ui (manager, data->merge_id);

	gtk_ui_manager_remove_action_group (manager, data->action_group);
	g_object_unref (data->action_group);

	gtk_ui_manager_remove_action_group (manager, data->single_selection_action_group);
	g_object_unref (data->single_selection_action_group);
}

static void
impl_updateui (XedPlugin * plugin, XedWindow * window)
{
	XedFileBrowserPluginData * data;
	XedDocument * doc;

	data = get_plugin_data (window);

	doc = xed_window_get_active_document (window);

	gtk_action_set_sensitive (gtk_action_group_get_action (data->action_group,
	                                                       "SetActiveRoot"),
	                          doc != NULL &&
	                          !xed_document_is_untitled (doc));
}

static void
impl_activate (XedPlugin * plugin, XedWindow * window)
{
	XedPanel * panel;
	XedFileBrowserPluginData * data;
	GtkWidget * image;
	GdkPixbuf * pixbuf;
	XedFileBrowserStore * store;
	gchar *data_dir;
	GSettingsSchemaSource *schema_source;
	GSettingsSchema *schema;

	data = g_new0 (XedFileBrowserPluginData, 1);

	data_dir = xed_plugin_get_data_dir (plugin);
	data->tree_widget = XED_FILE_BROWSER_WIDGET (xed_file_browser_widget_new (data_dir));
	g_free (data_dir);

	data->settings = g_settings_new (FILE_BROWSER_SCHEMA);
	data->onload_settings = g_settings_new (FILE_BROWSER_ONLOAD_SCHEMA);

	g_signal_connect (data->tree_widget,
			  "uri-activated",
			  G_CALLBACK (on_uri_activated_cb), window);

	g_signal_connect (data->tree_widget,
			  "error", G_CALLBACK (on_error_cb), window);

	g_signal_connect (data->tree_widget,
	                  "notify::filter-pattern",
	                  G_CALLBACK (on_filter_pattern_changed_cb),
	                  window);

	g_signal_connect (data->tree_widget,
	                  "confirm-delete",
	                  G_CALLBACK (on_confirm_delete_cb),
	                  window);

	g_signal_connect (data->tree_widget,
	                  "confirm-no-trash",
	                  G_CALLBACK (on_confirm_no_trash_cb),
	                  window);

	g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW
			  (xed_file_browser_widget_get_browser_view
			  (data->tree_widget))),
			  "changed",
			  G_CALLBACK (on_selection_changed_cb),
			  window);

	panel = xed_window_get_side_panel (window);
	pixbuf = xed_file_browser_utils_pixbuf_from_theme("system-file-manager",
	                                                    GTK_ICON_SIZE_MENU);

	if (pixbuf) {
		image = gtk_image_new_from_pixbuf(pixbuf);
		g_object_unref(pixbuf);
	} else {
		image = gtk_image_new_from_stock(GTK_STOCK_INDEX, GTK_ICON_SIZE_MENU);
	}

	gtk_widget_show(image);
	xed_panel_add_item (panel,
	                      GTK_WIDGET (data->tree_widget),
	                      _("File Browser"),
	                      image);
	gtk_widget_show (GTK_WIDGET (data->tree_widget));
	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, data);

	add_popup_ui (window);

	/* Restore filter options */
	restore_filter (data);

	/* Connect signals to store the last visited location */
	g_signal_connect (xed_file_browser_widget_get_browser_view (data->tree_widget),
	                  "notify::model",
	                  G_CALLBACK (on_model_set_cb),
	                  window);

	store = xed_file_browser_widget_get_browser_store (data->tree_widget);
	g_signal_connect (store,
	                  "notify::virtual-root",
	                  G_CALLBACK (on_virtual_root_changed_cb),
	                  window);

	g_signal_connect (store,
	                  "notify::filter-mode",
	                  G_CALLBACK (on_filter_mode_changed_cb),
	                  window);

	g_signal_connect (store,
			  "rename",
			  G_CALLBACK (on_rename_cb),
			  window);

	g_signal_connect (window,
	                  "tab-added",
	                  G_CALLBACK (on_tab_added_cb),
	                  data);

	/* Register messages on the bus */
	xed_file_browser_messages_register (window, data->tree_widget);

	impl_updateui (plugin, window);
}

static void
impl_deactivate (XedPlugin * plugin, XedWindow * window)
{
	XedFileBrowserPluginData * data;
	XedPanel * panel;

	data = get_plugin_data (window);

	/* Unregister messages from the bus */
	xed_file_browser_messages_unregister (window);

	/* Disconnect signals */
	g_signal_handlers_disconnect_by_func (window,
	                                      G_CALLBACK (on_tab_added_cb),
	                                      data);

	g_object_unref (data->settings);
	g_object_unref (data->onload_settings);

	remove_popup_ui (window);

	panel = xed_window_get_side_panel (window);
	xed_panel_remove_item (panel, GTK_WIDGET (data->tree_widget));

	g_free (data);
	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static void
filetree_plugin_class_init (XedFileBrowserPluginClass * klass)
{
	GObjectClass  *object_class = G_OBJECT_CLASS (klass);
	XedPluginClass * plugin_class = XED_PLUGIN_CLASS (klass);

	object_class->finalize = filetree_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
	plugin_class->update_ui = impl_updateui;

	g_type_class_add_private (object_class,
				  sizeof (XedFileBrowserPluginPrivate));
}

/* Callbacks */
static void
on_uri_activated_cb (XedFileBrowserWidget * tree_widget,
		     gchar const *uri, XedWindow * window)
{
	xed_commands_load_uri (window, uri, NULL, 0);
}

static void
on_error_cb (XedFileBrowserWidget * tree_widget,
	     guint code, gchar const *message, XedWindow * window)
{
	gchar * title;
	GtkWidget * dlg;
	XedFileBrowserPluginData * data;

	data = get_plugin_data (window);

	/* Do not show the error when the root has been set automatically */
	if (data->auto_root && (code == XED_FILE_BROWSER_ERROR_SET_ROOT ||
	                        code == XED_FILE_BROWSER_ERROR_LOAD_DIRECTORY))
	{
		/* Show bookmarks */
		xed_file_browser_widget_show_bookmarks (data->tree_widget);
		return;
	}

	switch (code) {
	case XED_FILE_BROWSER_ERROR_NEW_DIRECTORY:
		title =
		    _("An error occurred while creating a new directory");
		break;
	case XED_FILE_BROWSER_ERROR_NEW_FILE:
		title = _("An error occurred while creating a new file");
		break;
	case XED_FILE_BROWSER_ERROR_RENAME:
		title =
		    _
		    ("An error occurred while renaming a file or directory");
		break;
	case XED_FILE_BROWSER_ERROR_DELETE:
		title =
		    _
		    ("An error occurred while deleting a file or directory");
		break;
	case XED_FILE_BROWSER_ERROR_OPEN_DIRECTORY:
		title =
		    _
		    ("An error occurred while opening a directory in the file manager");
		break;
	case XED_FILE_BROWSER_ERROR_SET_ROOT:
		title =
		    _("An error occurred while setting a root directory");
		break;
	case XED_FILE_BROWSER_ERROR_LOAD_DIRECTORY:
		title =
		    _("An error occurred while loading a directory");
		break;
	default:
		title = _("An error occurred");
		break;
	}

	dlg = gtk_message_dialog_new (GTK_WINDOW (window),
				      GTK_DIALOG_MODAL |
				      GTK_DIALOG_DESTROY_WITH_PARENT,
				      GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				      "%s", title);
	gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dlg),
						  "%s", message);

	gtk_dialog_run (GTK_DIALOG (dlg));
	gtk_widget_destroy (dlg);
}

static void
on_model_set_cb (XedFileBrowserView * widget,
                 GParamSpec *arg1,
                 XedWindow * window)
{
	XedFileBrowserPluginData * data = get_plugin_data (window);
	GtkTreeModel * model;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (xed_file_browser_widget_get_browser_view (data->tree_widget)));

	if (model == NULL)
		return;

	g_settings_set_boolean (data->onload_settings,
	                       "tree-view",
	                       XED_IS_FILE_BROWSER_STORE (model));
}

static void
on_filter_mode_changed_cb (XedFileBrowserStore * model,
                           GParamSpec * param,
                           XedWindow * window)
{
	XedFileBrowserPluginData * data = get_plugin_data (window);
	XedFileBrowserStoreFilterMode mode;

	mode = xed_file_browser_store_get_filter_mode (model);

	if ((mode & XED_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN) &&
	    (mode & XED_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY)) {
		g_settings_set_string (data->settings, "filter-mode", "hidden_and_binary");
	} else if (mode & XED_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN) {
		g_settings_set_string (data->settings, "filter-mode", "hidden");
	} else if (mode & XED_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY) {
		g_settings_set_string (data->settings, "filter-mode", "binary");
	} else {
		g_settings_set_string (data->settings, "filter-mode", "none");
	}
}

static void
on_rename_cb (XedFileBrowserStore * store,
	      const gchar * olduri,
	      const gchar * newuri,
	      XedWindow * window)
{
	XedApp * app;
	GList * documents;
	GList * item;
	XedDocument * doc;
	GFile * docfile;
	GFile * oldfile;
	GFile * newfile;
	gchar * uri;

	/* Find all documents and set its uri to newuri where it matches olduri */
	app = xed_app_get_default ();
	documents = xed_app_get_documents (app);

	oldfile = g_file_new_for_uri (olduri);
	newfile = g_file_new_for_uri (newuri);

	for (item = documents; item; item = item->next) {
		doc = XED_DOCUMENT (item->data);
		uri = xed_document_get_uri (doc);

		if (!uri)
			continue;

		docfile = g_file_new_for_uri (uri);

		if (g_file_equal (docfile, oldfile)) {
			xed_document_set_uri (doc, newuri);
		} else {
			gchar *relative;

			relative = g_file_get_relative_path (oldfile, docfile);

			if (relative) {
				/* relative now contains the part in docfile without
				   the prefix oldfile */

				g_object_unref (docfile);
				g_free (uri);

				docfile = g_file_get_child (newfile, relative);
				uri = g_file_get_uri (docfile);

				xed_document_set_uri (doc, uri);
			}

			g_free (relative);
		}

		g_free (uri);
		g_object_unref (docfile);
	}

	g_object_unref (oldfile);
	g_object_unref (newfile);

	g_list_free (documents);
}

static void
on_filter_pattern_changed_cb (XedFileBrowserWidget * widget,
                              GParamSpec * param,
                              XedWindow * window)
{
	XedFileBrowserPluginData * data = get_plugin_data (window);
	gchar * pattern;

	g_object_get (G_OBJECT (widget), "filter-pattern", &pattern, NULL);

	if (pattern == NULL)
		g_settings_set_string (data->settings, "filter-pattern", "");
	else
		g_settings_set_string (data->settings, "filter-pattern", pattern);

	g_free (pattern);
}

static void
on_virtual_root_changed_cb (XedFileBrowserStore * store,
                            GParamSpec * param,
                            XedWindow * window)
{
	XedFileBrowserPluginData * data = get_plugin_data (window);
	gchar * root;
	gchar * virtual_root;

	root = xed_file_browser_store_get_root (store);

	if (!root)
		return;

	g_settings_set_string (data->onload_settings, "root", root);

	virtual_root = xed_file_browser_store_get_virtual_root (store);

	if (!virtual_root) {
		/* Set virtual to same as root then */
		g_settings_set_string (data->onload_settings, "virtual-root", root);
	} else {
		g_settings_set_string (data->onload_settings, "virtual-root", virtual_root);
	}

	g_signal_handlers_disconnect_by_func (window,
	                                      G_CALLBACK (on_tab_added_cb),
	                                      data);

	g_free (root);
	g_free (virtual_root);
}

static void
on_tab_added_cb (XedWindow * window,
                 XedTab * tab,
                 XedFileBrowserPluginData * data)
{
	gboolean open;
	gboolean load_default = TRUE;

	open = g_settings_get_boolean (data->settings, "open-at-first-doc");

	if (open) {
		XedDocument *doc;
		gchar *uri;

		doc = xed_tab_get_document (tab);

		uri = xed_document_get_uri (doc);

		if (uri != NULL && xed_utils_uri_has_file_scheme (uri)) {
			prepare_auto_root (data);
			set_root_from_doc (data, doc);
			load_default = FALSE;
		}

		g_free (uri);
	}

	if (load_default)
		restore_default_location (data);

	/* Disconnect this signal, it's only called once */
	g_signal_handlers_disconnect_by_func (window,
	                                      G_CALLBACK (on_tab_added_cb),
	                                      data);
}

static gchar *
get_filename_from_path (GtkTreeModel *model, GtkTreePath *path)
{
	GtkTreeIter iter;
	gchar *uri;

	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter,
			    XED_FILE_BROWSER_STORE_COLUMN_URI, &uri,
			    -1);

	return xed_file_browser_utils_uri_basename (uri);
}

static gboolean
on_confirm_no_trash_cb (XedFileBrowserWidget * widget,
                        GList * files,
                        XedWindow * window)
{
	gchar *normal;
	gchar *message;
	gchar *secondary;
	gboolean result;

	message = _("Cannot move file to trash, do you\nwant to delete permanently?");

	if (files->next == NULL) {
		normal = xed_file_browser_utils_file_basename (G_FILE (files->data));
	    	secondary = g_strdup_printf (_("The file \"%s\" cannot be moved to the trash."), normal);
		g_free (normal);
	} else {
		secondary = g_strdup (_("The selected files cannot be moved to the trash."));
	}

	result = xed_file_browser_utils_confirmation_dialog (window,
	                                                       GTK_MESSAGE_QUESTION,
	                                                       message,
	                                                       secondary,
	                                                       GTK_STOCK_DELETE,
	                                                       NULL);
	g_free (secondary);

	return result;
}

static gboolean
on_confirm_delete_cb (XedFileBrowserWidget *widget,
                      XedFileBrowserStore *store,
                      GList *paths,
                      XedWindow *window)
{
	gchar *normal;
	gchar *message;
	gchar *secondary;
	gboolean result;
	XedFileBrowserPluginData *data;

	data = get_plugin_data (window);

	if (paths->next == NULL) {
		normal = get_filename_from_path (GTK_TREE_MODEL (store), (GtkTreePath *)(paths->data));
		message = g_strdup_printf (_("Are you sure you want to permanently delete \"%s\"?"), normal);
		g_free (normal);
	} else {
		message = g_strdup (_("Are you sure you want to permanently delete the selected files?"));
	}

	secondary = _("If you delete an item, it is permanently lost.");

	result = xed_file_browser_utils_confirmation_dialog (window,
	                                                       GTK_MESSAGE_QUESTION,
	                                                       message,
	                                                       secondary,
	                                                       GTK_STOCK_DELETE,
	                                                       NULL);

	g_free (message);

	return result;
}

// ex:ts=8:noet:
