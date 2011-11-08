/*
 * pluma-file-browser-plugin.c - Pluma plugin providing easy file access
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pluma/pluma-commands.h>
#include <pluma/pluma-utils.h>
#include <pluma/pluma-app.h>
#include <glib/gi18n-lib.h>
#include <pluma/pluma-debug.h>
#include <mateconf/mateconf-client.h>
#include <string.h>

#include "pluma-file-browser-enum-types.h"
#include "pluma-file-browser-plugin.h"
#include "pluma-file-browser-utils.h"
#include "pluma-file-browser-error.h"
#include "pluma-file-browser-widget.h"
#include "pluma-file-browser-messages.h"

#define WINDOW_DATA_KEY	        	"PlumaFileBrowserPluginWindowData"
#define FILE_BROWSER_BASE_KEY 		"/apps/pluma/plugins/filebrowser"
#define CAJA_CLICK_POLICY_BASE_KEY 	"/apps/caja/preferences"
#define CAJA_CLICK_POLICY_KEY	"click_policy"
#define CAJA_ENABLE_DELETE_KEY	"enable_delete"
#define CAJA_CONFIRM_TRASH_KEY	"confirm_trash"
#define TERMINAL_EXEC_KEY		"/desktop/mate/applications/terminal/exec"

#define PLUMA_FILE_BROWSER_PLUGIN_GET_PRIVATE(object)	(G_TYPE_INSTANCE_GET_PRIVATE ((object), PLUMA_TYPE_FILE_BROWSER_PLUGIN, PlumaFileBrowserPluginPrivate))

struct _PlumaFileBrowserPluginPrivate
{
	gpointer dummy;
};

typedef struct _PlumaFileBrowserPluginData
{
	PlumaFileBrowserWidget * tree_widget;
	gulong                   merge_id;
	GtkActionGroup         * action_group;
	GtkActionGroup	       * single_selection_action_group;
	gboolean	         auto_root;
	gulong                   end_loading_handle;
	gboolean		 confirm_trash;

	guint			 click_policy_handle;
	guint			 enable_delete_handle;
	guint			 confirm_trash_handle;
} PlumaFileBrowserPluginData;

static void on_uri_activated_cb          (PlumaFileBrowserWidget * widget,
                                          gchar const *uri,
                                          PlumaWindow * window);
static void on_error_cb                  (PlumaFileBrowserWidget * widget,
                                          guint code,
                                          gchar const *message,
                                          PlumaWindow * window);
static void on_model_set_cb              (PlumaFileBrowserView * widget,
                                          GParamSpec *arg1,
                                          PlumaWindow * window);
static void on_virtual_root_changed_cb   (PlumaFileBrowserStore * model,
                                          GParamSpec * param,
                                          PlumaWindow * window);
static void on_filter_mode_changed_cb    (PlumaFileBrowserStore * model,
                                          GParamSpec * param,
                                          PlumaWindow * window);
static void on_rename_cb		 (PlumaFileBrowserStore * model,
					  const gchar * olduri,
					  const gchar * newuri,
					  PlumaWindow * window);
static void on_filter_pattern_changed_cb (PlumaFileBrowserWidget * widget,
                                          GParamSpec * param,
                                          PlumaWindow * window);
static void on_tab_added_cb              (PlumaWindow * window,
                                          PlumaTab * tab,
                                          PlumaFileBrowserPluginData * data);
static gboolean on_confirm_delete_cb     (PlumaFileBrowserWidget * widget,
                                          PlumaFileBrowserStore * store,
                                          GList * rows,
                                          PlumaWindow * window);
static gboolean on_confirm_no_trash_cb   (PlumaFileBrowserWidget * widget,
                                          GList * files,
                                          PlumaWindow * window);

PLUMA_PLUGIN_REGISTER_TYPE_WITH_CODE (PlumaFileBrowserPlugin, filetree_plugin, 	\
	pluma_file_browser_enum_and_flag_register_type (type_module);		\
	pluma_file_browser_store_register_type         (type_module);		\
	pluma_file_bookmarks_store_register_type       (type_module);		\
	pluma_file_browser_view_register_type	       (type_module);		\
	pluma_file_browser_widget_register_type	       (type_module);		\
)


static void
filetree_plugin_init (PlumaFileBrowserPlugin * plugin)
{
	plugin->priv = PLUMA_FILE_BROWSER_PLUGIN_GET_PRIVATE (plugin);
}

static void
filetree_plugin_finalize (GObject * object)
{
	//PlumaFileBrowserPlugin * plugin = PLUMA_FILE_BROWSER_PLUGIN (object);

	G_OBJECT_CLASS (filetree_plugin_parent_class)->finalize (object);
}

static PlumaFileBrowserPluginData *
get_plugin_data (PlumaWindow * window)
{
	return (PlumaFileBrowserPluginData *) (g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY));
}

static void
on_end_loading_cb (PlumaFileBrowserStore      * store,
                   GtkTreeIter                * iter,
                   PlumaFileBrowserPluginData * data)
{
	/* Disconnect the signal */
	g_signal_handler_disconnect (store, data->end_loading_handle);
	data->end_loading_handle = 0;
	data->auto_root = FALSE;
}

static void
prepare_auto_root (PlumaFileBrowserPluginData *data)
{
	PlumaFileBrowserStore *store;

	data->auto_root = TRUE;

	store = pluma_file_browser_widget_get_browser_store (data->tree_widget);

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
restore_default_location (PlumaFileBrowserPluginData *data)
{
	gchar * root;
	gchar * virtual_root;
	gboolean bookmarks;
	gboolean remote;
	MateConfClient * client;

	client = mateconf_client_get_default ();
	if (!client)
		return;

	bookmarks = !mateconf_client_get_bool (client,
	                                    FILE_BROWSER_BASE_KEY "/on_load/tree_view",
	                                    NULL);

	if (bookmarks) {
		g_object_unref (client);
		pluma_file_browser_widget_show_bookmarks (data->tree_widget);
		return;
	}

	root = mateconf_client_get_string (client,
	                                FILE_BROWSER_BASE_KEY "/on_load/root",
	                                NULL);
	virtual_root = mateconf_client_get_string (client,
	                                   FILE_BROWSER_BASE_KEY "/on_load/virtual_root",
	                                   NULL);

	remote = mateconf_client_get_bool (client,
	                                FILE_BROWSER_BASE_KEY "/on_load/enable_remote",
	                                NULL);

	if (root != NULL && *root != '\0') {
		GFile *file;

		file = g_file_new_for_uri (root);

		if (remote || g_file_is_native (file)) {
			if (virtual_root != NULL && *virtual_root != '\0') {
				prepare_auto_root (data);
				pluma_file_browser_widget_set_root_and_virtual_root (data->tree_widget,
					                                             root,
					                                             virtual_root);
			} else {
				prepare_auto_root (data);
				pluma_file_browser_widget_set_root (data->tree_widget,
					                            root,
					                            TRUE);
			}
		}

		g_object_unref (file);
	}

	g_object_unref (client);
	g_free (root);
	g_free (virtual_root);
}

static void
restore_filter (PlumaFileBrowserPluginData * data)
{
	MateConfClient * client;
	gchar *filter_mode;
	PlumaFileBrowserStoreFilterMode mode;
	gchar *pattern;

	client = mateconf_client_get_default ();
	if (!client)
		return;

	/* Get filter_mode */
	filter_mode = mateconf_client_get_string (client,
	                                       FILE_BROWSER_BASE_KEY "/filter_mode",
	                                       NULL);

	/* Filter mode */
	mode = pluma_file_browser_store_filter_mode_get_default ();

	if (filter_mode != NULL) {
		if (strcmp (filter_mode, "hidden") == 0) {
			mode = PLUMA_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN;
		} else if (strcmp (filter_mode, "binary") == 0) {
			mode = PLUMA_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY;
		} else if (strcmp (filter_mode, "hidden_and_binary") == 0 ||
		         strcmp (filter_mode, "binary_and_hidden") == 0) {
			mode = PLUMA_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN |
			       PLUMA_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY;
		} else if (strcmp (filter_mode, "none") == 0 ||
		           *filter_mode == '\0') {
			mode = PLUMA_FILE_BROWSER_STORE_FILTER_MODE_NONE;
		}
	}

	/* Set the filter mode */
	pluma_file_browser_store_set_filter_mode (
	    pluma_file_browser_widget_get_browser_store (data->tree_widget),
	    mode);

	pattern = mateconf_client_get_string (client,
	                                   FILE_BROWSER_BASE_KEY "/filter_pattern",
	                                   NULL);

	pluma_file_browser_widget_set_filter_pattern (data->tree_widget,
	                                              pattern);

	g_object_unref (client);
	g_free (filter_mode);
	g_free (pattern);
}

static PlumaFileBrowserViewClickPolicy
click_policy_from_string (gchar const *click_policy)
{
	if (click_policy && strcmp (click_policy, "single") == 0)
		return PLUMA_FILE_BROWSER_VIEW_CLICK_POLICY_SINGLE;
	else
		return PLUMA_FILE_BROWSER_VIEW_CLICK_POLICY_DOUBLE;
}

static void
on_click_policy_changed (MateConfClient *client,
			 guint cnxn_id,
			 MateConfEntry *entry,
			 gpointer user_data)
{
	MateConfValue *value;
	PlumaFileBrowserPluginData * data;
	gchar const *click_policy;
	PlumaFileBrowserViewClickPolicy policy = PLUMA_FILE_BROWSER_VIEW_CLICK_POLICY_DOUBLE;
	PlumaFileBrowserView *view;

	data = (PlumaFileBrowserPluginData *)(user_data);
	value = mateconf_entry_get_value (entry);

	if (value && value->type == MATECONF_VALUE_STRING) {
		click_policy = mateconf_value_get_string (value);

		policy = click_policy_from_string (click_policy);
	}

	view = pluma_file_browser_widget_get_browser_view (data->tree_widget);
	pluma_file_browser_view_set_click_policy (view, policy);
}

static void
on_enable_delete_changed (MateConfClient *client,
		 	  guint cnxn_id,
			  MateConfEntry *entry,
			  gpointer user_data)
{
	MateConfValue *value;
	PlumaFileBrowserPluginData *data;
	gboolean enable = FALSE;

	data = (PlumaFileBrowserPluginData *)(user_data);
	value = mateconf_entry_get_value (entry);

	if (value && value->type == MATECONF_VALUE_BOOL)
		enable = mateconf_value_get_bool (value);

	g_object_set (G_OBJECT (data->tree_widget), "enable-delete", enable, NULL);
}

static void
on_confirm_trash_changed (MateConfClient *client,
		 	  guint cnxn_id,
			  MateConfEntry *entry,
			  gpointer user_data)
{
	MateConfValue *value;
	PlumaFileBrowserPluginData *data;
	gboolean enable = FALSE;

	data = (PlumaFileBrowserPluginData *)(user_data);
	value = mateconf_entry_get_value (entry);

	if (value && value->type == MATECONF_VALUE_BOOL)
		enable = mateconf_value_get_bool (value);

	data->confirm_trash = enable;
}

static void
install_caja_prefs (PlumaFileBrowserPluginData *data)
{
	MateConfClient *client;
	gchar *pref;
	gboolean prefb;
	PlumaFileBrowserViewClickPolicy policy;
	PlumaFileBrowserView *view;

	client = mateconf_client_get_default ();
	if (!client)
		return;

	mateconf_client_add_dir (client,
			      CAJA_CLICK_POLICY_BASE_KEY,
			      MATECONF_CLIENT_PRELOAD_NONE,
			      NULL);

	/* Get click_policy */
	pref = mateconf_client_get_string (client,
	                                CAJA_CLICK_POLICY_BASE_KEY "/" CAJA_CLICK_POLICY_KEY,
	                                NULL);

	policy = click_policy_from_string (pref);

	view = pluma_file_browser_widget_get_browser_view (data->tree_widget);
	pluma_file_browser_view_set_click_policy (view, policy);

	if (pref) {
		data->click_policy_handle =
			mateconf_client_notify_add (client,
						 CAJA_CLICK_POLICY_BASE_KEY "/" CAJA_CLICK_POLICY_KEY,
						 on_click_policy_changed,
						 data,
						 NULL,
						 NULL);
		g_free (pref);
	}

	/* Get enable_delete */
	prefb = mateconf_client_get_bool (client,
	                               CAJA_CLICK_POLICY_BASE_KEY "/" CAJA_ENABLE_DELETE_KEY,
	                               NULL);

	g_object_set (G_OBJECT (data->tree_widget), "enable-delete", prefb, NULL);

	data->enable_delete_handle =
			mateconf_client_notify_add (client,
						 CAJA_CLICK_POLICY_BASE_KEY "/" CAJA_ENABLE_DELETE_KEY,
						 on_enable_delete_changed,
						 data,
						 NULL,
						 NULL);

	/* Get confirm_trash */
	prefb = mateconf_client_get_bool (client,
	                               CAJA_CLICK_POLICY_BASE_KEY "/" CAJA_CONFIRM_TRASH_KEY,
	                               NULL);

	data->confirm_trash = prefb;

	data->confirm_trash_handle =
			mateconf_client_notify_add (client,
						 CAJA_CLICK_POLICY_BASE_KEY "/" CAJA_CONFIRM_TRASH_KEY,
						 on_confirm_trash_changed,
						 data,
						 NULL,
						 NULL);
	g_object_unref (client);
}

static void
set_root_from_doc (PlumaFileBrowserPluginData * data,
                   PlumaDocument * doc)
{
	GFile *file;
	GFile *parent;

	if (doc == NULL)
		return;

	file = pluma_document_get_location (doc);
	if (file == NULL)
		return;

	parent = g_file_get_parent (file);

	if (parent != NULL) {
		gchar * root;

		root = g_file_get_uri (parent);

		pluma_file_browser_widget_set_root (data->tree_widget,
				                    root,
				                    TRUE);

		g_object_unref (parent);
		g_free (root);
	}

	g_object_unref (file);
}

static void
on_action_set_active_root (GtkAction * action,
                           PlumaWindow * window)
{
	PlumaFileBrowserPluginData *data;

	data = get_plugin_data (window);
	set_root_from_doc (data,
	                   pluma_window_get_active_document (window));
}

static gchar *
get_terminal (void)
{
	MateConfClient * client;
	gchar * terminal;

	client = mateconf_client_get_default ();
	terminal = mateconf_client_get_string (client,
					    TERMINAL_EXEC_KEY,
					    NULL);
	g_object_unref (client);

	if (terminal == NULL) {
		const gchar *term = g_getenv ("TERM");

		if (term != NULL)
			terminal = g_strdup (term);
		else
			terminal = g_strdup ("xterm");
	}

	return terminal;
}

static void
on_action_open_terminal (GtkAction * action,
                         PlumaWindow * window)
{
	PlumaFileBrowserPluginData * data;
	gchar * terminal;
	gchar * wd = NULL;
	gchar * local;
	gchar * argv[2];
	GFile * file;

	GtkTreeIter iter;
	PlumaFileBrowserStore * store;

	data = get_plugin_data (window);

	/* Get the current directory */
	if (!pluma_file_browser_widget_get_selected_directory (data->tree_widget, &iter))
		return;

	store = pluma_file_browser_widget_get_browser_store (data->tree_widget);
	gtk_tree_model_get (GTK_TREE_MODEL (store),
	                    &iter,
	                    PLUMA_FILE_BROWSER_STORE_COLUMN_URI,
	                    &wd,
	                    -1);

	if (wd == NULL)
		return;

	terminal = get_terminal ();

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
			 PlumaWindow      *window)
{
	PlumaFileBrowserPluginData * data;
	GtkTreeView * tree_view;
	GtkTreeModel * model;
	GtkTreeIter iter;
	gboolean sensitive;
	gchar * uri;

	data = get_plugin_data (window);

	tree_view = GTK_TREE_VIEW (pluma_file_browser_widget_get_browser_view (data->tree_widget));
	model = gtk_tree_view_get_model (tree_view);

	if (!PLUMA_IS_FILE_BROWSER_STORE (model))
		return;

	sensitive = pluma_file_browser_widget_get_selected_directory (data->tree_widget, &iter);

	if (sensitive) {
		gtk_tree_model_get (model, &iter,
				    PLUMA_FILE_BROWSER_STORE_COLUMN_URI,
				    &uri, -1);

		sensitive = pluma_utils_uri_has_file_scheme (uri);
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
	{"SetActiveRoot", GTK_STOCK_JUMP_TO, N_("_Set root to active document"),
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
add_popup_ui (PlumaWindow * window)
{
	PlumaFileBrowserPluginData * data;
	GtkUIManager * manager;
	GtkActionGroup * action_group;
	GError * error = NULL;

	data = get_plugin_data (window);
	manager = pluma_file_browser_widget_get_ui_manager (data->tree_widget);

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
remove_popup_ui (PlumaWindow * window)
{
	PlumaFileBrowserPluginData * data;
	GtkUIManager * manager;

	data = get_plugin_data (window);
	manager = pluma_file_browser_widget_get_ui_manager (data->tree_widget);
	gtk_ui_manager_remove_ui (manager, data->merge_id);

	gtk_ui_manager_remove_action_group (manager, data->action_group);
	g_object_unref (data->action_group);

	gtk_ui_manager_remove_action_group (manager, data->single_selection_action_group);
	g_object_unref (data->single_selection_action_group);
}

static void
impl_updateui (PlumaPlugin * plugin, PlumaWindow * window)
{
	PlumaFileBrowserPluginData * data;
	PlumaDocument * doc;

	data = get_plugin_data (window);

	doc = pluma_window_get_active_document (window);

	gtk_action_set_sensitive (gtk_action_group_get_action (data->action_group,
	                                                       "SetActiveRoot"),
	                          doc != NULL &&
	                          !pluma_document_is_untitled (doc));
}

static void
impl_activate (PlumaPlugin * plugin, PlumaWindow * window)
{
	PlumaPanel * panel;
	PlumaFileBrowserPluginData * data;
	GtkWidget * image;
	GdkPixbuf * pixbuf;
	PlumaFileBrowserStore * store;
	gchar *data_dir;

	data = g_new0 (PlumaFileBrowserPluginData, 1);

	data_dir = pluma_plugin_get_data_dir (plugin);
	data->tree_widget = PLUMA_FILE_BROWSER_WIDGET (pluma_file_browser_widget_new (data_dir));
	g_free (data_dir);

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
			  (pluma_file_browser_widget_get_browser_view
			  (data->tree_widget))),
			  "changed",
			  G_CALLBACK (on_selection_changed_cb),
			  window);

	panel = pluma_window_get_side_panel (window);
	pixbuf = pluma_file_browser_utils_pixbuf_from_theme("system-file-manager",
	                                                    GTK_ICON_SIZE_MENU);

	if (pixbuf) {
		image = gtk_image_new_from_pixbuf(pixbuf);
		g_object_unref(pixbuf);
	} else {
		image = gtk_image_new_from_stock(GTK_STOCK_INDEX, GTK_ICON_SIZE_MENU);
	}

	gtk_widget_show(image);
	pluma_panel_add_item (panel,
	                      GTK_WIDGET (data->tree_widget),
	                      _("File Browser"),
	                      image);
	gtk_widget_show (GTK_WIDGET (data->tree_widget));
	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, data);

	add_popup_ui (window);

	/* Restore filter options */
	restore_filter (data);

	/* Install caja preferences */
	install_caja_prefs (data);

	/* Connect signals to store the last visited location */
	g_signal_connect (pluma_file_browser_widget_get_browser_view (data->tree_widget),
	                  "notify::model",
	                  G_CALLBACK (on_model_set_cb),
	                  window);

	store = pluma_file_browser_widget_get_browser_store (data->tree_widget);
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
	pluma_file_browser_messages_register (window, data->tree_widget);

	impl_updateui (plugin, window);
}

static void
impl_deactivate (PlumaPlugin * plugin, PlumaWindow * window)
{
	PlumaFileBrowserPluginData * data;
	PlumaPanel * panel;
	MateConfClient *client;

	data = get_plugin_data (window);

	/* Unregister messages from the bus */
	pluma_file_browser_messages_unregister (window);

	/* Disconnect signals */
	g_signal_handlers_disconnect_by_func (window,
	                                      G_CALLBACK (on_tab_added_cb),
	                                      data);

	client = mateconf_client_get_default ();
	mateconf_client_remove_dir (client, CAJA_CLICK_POLICY_BASE_KEY, NULL);

	if (data->click_policy_handle)
		mateconf_client_notify_remove (client, data->click_policy_handle);

	if (data->enable_delete_handle)
		mateconf_client_notify_remove (client, data->enable_delete_handle);

	if (data->confirm_trash_handle)
		mateconf_client_notify_remove (client, data->confirm_trash_handle);

	g_object_unref (client);
	remove_popup_ui (window);

	panel = pluma_window_get_side_panel (window);
	pluma_panel_remove_item (panel, GTK_WIDGET (data->tree_widget));

	g_free (data);
	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static void
filetree_plugin_class_init (PlumaFileBrowserPluginClass * klass)
{
	GObjectClass  *object_class = G_OBJECT_CLASS (klass);
	PlumaPluginClass * plugin_class = PLUMA_PLUGIN_CLASS (klass);

	object_class->finalize = filetree_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
	plugin_class->update_ui = impl_updateui;

	g_type_class_add_private (object_class,
				  sizeof (PlumaFileBrowserPluginPrivate));
}

/* Callbacks */
static void
on_uri_activated_cb (PlumaFileBrowserWidget * tree_widget,
		     gchar const *uri, PlumaWindow * window)
{
	pluma_commands_load_uri (window, uri, NULL, 0);
}

static void
on_error_cb (PlumaFileBrowserWidget * tree_widget,
	     guint code, gchar const *message, PlumaWindow * window)
{
	gchar * title;
	GtkWidget * dlg;
	PlumaFileBrowserPluginData * data;

	data = get_plugin_data (window);

	/* Do not show the error when the root has been set automatically */
	if (data->auto_root && (code == PLUMA_FILE_BROWSER_ERROR_SET_ROOT ||
	                        code == PLUMA_FILE_BROWSER_ERROR_LOAD_DIRECTORY))
	{
		/* Show bookmarks */
		pluma_file_browser_widget_show_bookmarks (data->tree_widget);
		return;
	}

	switch (code) {
	case PLUMA_FILE_BROWSER_ERROR_NEW_DIRECTORY:
		title =
		    _("An error occurred while creating a new directory");
		break;
	case PLUMA_FILE_BROWSER_ERROR_NEW_FILE:
		title = _("An error occurred while creating a new file");
		break;
	case PLUMA_FILE_BROWSER_ERROR_RENAME:
		title =
		    _
		    ("An error occurred while renaming a file or directory");
		break;
	case PLUMA_FILE_BROWSER_ERROR_DELETE:
		title =
		    _
		    ("An error occurred while deleting a file or directory");
		break;
	case PLUMA_FILE_BROWSER_ERROR_OPEN_DIRECTORY:
		title =
		    _
		    ("An error occurred while opening a directory in the file manager");
		break;
	case PLUMA_FILE_BROWSER_ERROR_SET_ROOT:
		title =
		    _("An error occurred while setting a root directory");
		break;
	case PLUMA_FILE_BROWSER_ERROR_LOAD_DIRECTORY:
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
on_model_set_cb (PlumaFileBrowserView * widget,
                 GParamSpec *arg1,
                 PlumaWindow * window)
{
	PlumaFileBrowserPluginData * data = get_plugin_data (window);
	GtkTreeModel * model;
	MateConfClient * client;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (pluma_file_browser_widget_get_browser_view (data->tree_widget)));

	if (model == NULL)
		return;

	client = mateconf_client_get_default ();
	mateconf_client_set_bool (client,
	                       FILE_BROWSER_BASE_KEY "/on_load/tree_view",
	                       PLUMA_IS_FILE_BROWSER_STORE (model),
	                       NULL);
	g_object_unref (client);
}

static void
on_filter_mode_changed_cb (PlumaFileBrowserStore * model,
                           GParamSpec * param,
                           PlumaWindow * window)
{
	MateConfClient * client;
	PlumaFileBrowserStoreFilterMode mode;

	client = mateconf_client_get_default ();

	if (!client)
		return;

	mode = pluma_file_browser_store_get_filter_mode (model);

	if ((mode & PLUMA_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN) &&
	    (mode & PLUMA_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY)) {
		mateconf_client_set_string (client,
		                         FILE_BROWSER_BASE_KEY "/filter_mode",
		                         "hidden_and_binary",
		                         NULL);
	} else if (mode & PLUMA_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN) {
		mateconf_client_set_string (client,
		                         FILE_BROWSER_BASE_KEY "/filter_mode",
		                         "hidden",
		                         NULL);
	} else if (mode & PLUMA_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY) {
		mateconf_client_set_string (client,
		                         FILE_BROWSER_BASE_KEY "/filter_mode",
		                         "binary",
		                         NULL);
	} else {
		mateconf_client_set_string (client,
		                         FILE_BROWSER_BASE_KEY "/filter_mode",
		                         "none",
		                         NULL);
	}

	g_object_unref (client);

}

static void
on_rename_cb (PlumaFileBrowserStore * store,
	      const gchar * olduri,
	      const gchar * newuri,
	      PlumaWindow * window)
{
	PlumaApp * app;
	GList * documents;
	GList * item;
	PlumaDocument * doc;
	GFile * docfile;
	GFile * oldfile;
	GFile * newfile;
	gchar * uri;

	/* Find all documents and set its uri to newuri where it matches olduri */
	app = pluma_app_get_default ();
	documents = pluma_app_get_documents (app);

	oldfile = g_file_new_for_uri (olduri);
	newfile = g_file_new_for_uri (newuri);

	for (item = documents; item; item = item->next) {
		doc = PLUMA_DOCUMENT (item->data);
		uri = pluma_document_get_uri (doc);

		if (!uri)
			continue;

		docfile = g_file_new_for_uri (uri);

		if (g_file_equal (docfile, oldfile)) {
			pluma_document_set_uri (doc, newuri);
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

				pluma_document_set_uri (doc, uri);
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
on_filter_pattern_changed_cb (PlumaFileBrowserWidget * widget,
                              GParamSpec * param,
                              PlumaWindow * window)
{
	MateConfClient * client;
	gchar * pattern;

	client = mateconf_client_get_default ();

	if (!client)
		return;

	g_object_get (G_OBJECT (widget), "filter-pattern", &pattern, NULL);

	if (pattern == NULL)
		mateconf_client_set_string (client,
		                         FILE_BROWSER_BASE_KEY "/filter_pattern",
		                         "",
		                         NULL);
	else
		mateconf_client_set_string (client,
		                         FILE_BROWSER_BASE_KEY "/filter_pattern",
		                         pattern,
		                         NULL);

	g_free (pattern);
}

static void
on_virtual_root_changed_cb (PlumaFileBrowserStore * store,
                            GParamSpec * param,
                            PlumaWindow * window)
{
	PlumaFileBrowserPluginData * data = get_plugin_data (window);
	gchar * root;
	gchar * virtual_root;
	MateConfClient * client;

	root = pluma_file_browser_store_get_root (store);

	if (!root)
		return;

	client = mateconf_client_get_default ();

	if (!client)
		return;

	mateconf_client_set_string (client,
	                         FILE_BROWSER_BASE_KEY "/on_load/root",
	                         root,
	                         NULL);

	virtual_root = pluma_file_browser_store_get_virtual_root (store);

	if (!virtual_root) {
		/* Set virtual to same as root then */
		mateconf_client_set_string (client,
	                                 FILE_BROWSER_BASE_KEY "/on_load/virtual_root",
	                                 root,
	                                 NULL);
	} else {
		mateconf_client_set_string (client,
	                                 FILE_BROWSER_BASE_KEY "/on_load/virtual_root",
	                                 virtual_root,
	                                 NULL);
	}

	g_signal_handlers_disconnect_by_func (window,
	                                      G_CALLBACK (on_tab_added_cb),
	                                      data);

	g_object_unref (client);
	g_free (root);
	g_free (virtual_root);
}

static void
on_tab_added_cb (PlumaWindow * window,
                 PlumaTab * tab,
                 PlumaFileBrowserPluginData * data)
{
	MateConfClient *client;
	gboolean open;
	gboolean load_default = TRUE;

	client = mateconf_client_get_default ();

	if (!client)
		return;

	open = mateconf_client_get_bool (client,
	                              FILE_BROWSER_BASE_KEY "/open_at_first_doc",
	                              NULL);

	if (open) {
		PlumaDocument *doc;
		gchar *uri;

		doc = pluma_tab_get_document (tab);

		uri = pluma_document_get_uri (doc);

		if (uri != NULL && pluma_utils_uri_has_file_scheme (uri)) {
			prepare_auto_root (data);
			set_root_from_doc (data, doc);
			load_default = FALSE;
		}

		g_free (uri);
	}

	if (load_default)
		restore_default_location (data);

	g_object_unref (client);

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
			    PLUMA_FILE_BROWSER_STORE_COLUMN_URI, &uri,
			    -1);

	return pluma_file_browser_utils_uri_basename (uri);
}

static gboolean
on_confirm_no_trash_cb (PlumaFileBrowserWidget * widget,
                        GList * files,
                        PlumaWindow * window)
{
	gchar *normal;
	gchar *message;
	gchar *secondary;
	gboolean result;

	message = _("Cannot move file to trash, do you\nwant to delete permanently?");

	if (files->next == NULL) {
		normal = pluma_file_browser_utils_file_basename (G_FILE (files->data));
	    	secondary = g_strdup_printf (_("The file \"%s\" cannot be moved to the trash."), normal);
		g_free (normal);
	} else {
		secondary = g_strdup (_("The selected files cannot be moved to the trash."));
	}

	result = pluma_file_browser_utils_confirmation_dialog (window,
	                                                       GTK_MESSAGE_QUESTION,
	                                                       message,
	                                                       secondary,
	                                                       GTK_STOCK_DELETE,
	                                                       NULL);
	g_free (secondary);

	return result;
}

static gboolean
on_confirm_delete_cb (PlumaFileBrowserWidget *widget,
                      PlumaFileBrowserStore *store,
                      GList *paths,
                      PlumaWindow *window)
{
	gchar *normal;
	gchar *message;
	gchar *secondary;
	gboolean result;
	PlumaFileBrowserPluginData *data;

	data = get_plugin_data (window);

	if (!data->confirm_trash)
		return TRUE;

	if (paths->next == NULL) {
		normal = get_filename_from_path (GTK_TREE_MODEL (store), (GtkTreePath *)(paths->data));
		message = g_strdup_printf (_("Are you sure you want to permanently delete \"%s\"?"), normal);
		g_free (normal);
	} else {
		message = g_strdup (_("Are you sure you want to permanently delete the selected files?"));
	}

	secondary = _("If you delete an item, it is permanently lost.");

	result = pluma_file_browser_utils_confirmation_dialog (window,
	                                                       GTK_MESSAGE_QUESTION,
	                                                       message,
	                                                       secondary,
	                                                       GTK_STOCK_DELETE,
	                                                       NULL);

	g_free (message);

	return result;
}

// ex:ts=8:noet:
