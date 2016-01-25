/*
 * xedit-plugin-manager.c
 * This file is part of xedit
 *
 * Copyright (C) 2002 Paolo Maggi and James Willcox
 * Copyright (C) 2003-2006 Paolo Maggi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA 02110-1301, USA. 
 */

/*
 * Modified by the xedit Team, 1998-2006. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <glib/gi18n.h>

#include "xedit-plugin-manager.h"
#include "xedit-utils.h"
#include "xedit-plugins-engine.h"
#include "xedit-plugin.h"
#include "xedit-debug.h"

enum
{
	ACTIVE_COLUMN,
	AVAILABLE_COLUMN,
	INFO_COLUMN,
	N_COLUMNS
};

#define PLUGIN_MANAGER_NAME_TITLE _("Plugin")
#define PLUGIN_MANAGER_ACTIVE_TITLE _("Enabled")

#define XEDIT_PLUGIN_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), XEDIT_TYPE_PLUGIN_MANAGER, XeditPluginManagerPrivate))

struct _XeditPluginManagerPrivate
{
	GtkWidget	*tree;

	GtkWidget	*about_button;
	GtkWidget	*configure_button;

	XeditPluginsEngine *engine;

	GtkWidget 	*about;
	
	GtkWidget	*popup_menu;
};

#if GTK_CHECK_VERSION (3, 0, 0)
G_DEFINE_TYPE(XeditPluginManager, xedit_plugin_manager, GTK_TYPE_BOX)
#else
G_DEFINE_TYPE(XeditPluginManager, xedit_plugin_manager, GTK_TYPE_VBOX)
#endif

static XeditPluginInfo *plugin_manager_get_selected_plugin (XeditPluginManager *pm); 
static void plugin_manager_toggle_active (XeditPluginManager *pm, GtkTreeIter *iter, GtkTreeModel *model);
static void xedit_plugin_manager_finalize (GObject *object);

static void 
xedit_plugin_manager_class_init (XeditPluginManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = xedit_plugin_manager_finalize;

	g_type_class_add_private (object_class, sizeof (XeditPluginManagerPrivate));
}

static void
about_button_cb (GtkWidget          *button,
		 XeditPluginManager *pm)
{
	XeditPluginInfo *info;

	xedit_debug (DEBUG_PLUGINS);

	info = plugin_manager_get_selected_plugin (pm);

	g_return_if_fail (info != NULL);

	/* if there is another about dialog already open destroy it */
	if (pm->priv->about)
		gtk_widget_destroy (pm->priv->about);

	pm->priv->about = g_object_new (GTK_TYPE_ABOUT_DIALOG,
		"program-name", xedit_plugin_info_get_name (info),
		"copyright", xedit_plugin_info_get_copyright (info),
		"authors", xedit_plugin_info_get_authors (info),
		"comments", xedit_plugin_info_get_description (info),
		"website", xedit_plugin_info_get_website (info),
		"logo-icon-name", xedit_plugin_info_get_icon_name (info),
		"version", xedit_plugin_info_get_version (info),
		NULL);

	gtk_window_set_destroy_with_parent (GTK_WINDOW (pm->priv->about),
					    TRUE);

	g_signal_connect (pm->priv->about,
			  "response",
			  G_CALLBACK (gtk_widget_destroy),
			  NULL);
	g_signal_connect (pm->priv->about,
			  "destroy",
			  G_CALLBACK (gtk_widget_destroyed),
			  &pm->priv->about);

	gtk_window_set_transient_for (GTK_WINDOW (pm->priv->about),
				      GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET(pm))));
	gtk_widget_show (pm->priv->about);
}

static void
configure_button_cb (GtkWidget          *button,
		     XeditPluginManager *pm)
{
	XeditPluginInfo *info;
	GtkWindow *toplevel;

	xedit_debug (DEBUG_PLUGINS);

	info = plugin_manager_get_selected_plugin (pm);

	g_return_if_fail (info != NULL);

	xedit_debug_message (DEBUG_PLUGINS, "Configuring: %s\n", 
			     xedit_plugin_info_get_name (info));

	toplevel = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET(pm)));

	xedit_plugins_engine_configure_plugin (pm->priv->engine,
					       info, toplevel);

	xedit_debug_message (DEBUG_PLUGINS, "Done");	
}

static void
plugin_manager_view_info_cell_cb (GtkTreeViewColumn *tree_column,
				  GtkCellRenderer   *cell,
				  GtkTreeModel      *tree_model,
				  GtkTreeIter       *iter,
				  gpointer           data)
{
	XeditPluginInfo *info;
	gchar *text;
	
	g_return_if_fail (tree_model != NULL);
	g_return_if_fail (tree_column != NULL);

	gtk_tree_model_get (tree_model, iter, INFO_COLUMN, &info, -1);

	if (info == NULL)
		return;

	text = g_markup_printf_escaped ("<b>%s</b>\n%s",
					xedit_plugin_info_get_name (info),
					xedit_plugin_info_get_description (info));
	g_object_set (G_OBJECT (cell),
		      "markup", text,
		      "sensitive", xedit_plugin_info_is_available (info),
		      NULL);

	g_free (text);
}

static void
plugin_manager_view_icon_cell_cb (GtkTreeViewColumn *tree_column,
				  GtkCellRenderer   *cell,
				  GtkTreeModel      *tree_model,
				  GtkTreeIter       *iter,
				  gpointer           data)
{
	XeditPluginInfo *info;
	
	g_return_if_fail (tree_model != NULL);
	g_return_if_fail (tree_column != NULL);

	gtk_tree_model_get (tree_model, iter, INFO_COLUMN, &info, -1);

	if (info == NULL)
		return;

	g_object_set (G_OBJECT (cell),
		      "icon-name", xedit_plugin_info_get_icon_name (info),
		      "sensitive", xedit_plugin_info_is_available (info),
		      NULL);
}


static void
active_toggled_cb (GtkCellRendererToggle *cell,
		   gchar                 *path_str,
		   XeditPluginManager    *pm)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	xedit_debug (DEBUG_PLUGINS);

	path = gtk_tree_path_new_from_string (path_str);

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (pm->priv->tree));
	g_return_if_fail (model != NULL);

	gtk_tree_model_get_iter (model, &iter, path);

	if (&iter != NULL)
		plugin_manager_toggle_active (pm, &iter, model);

	gtk_tree_path_free (path);
}

static void
cursor_changed_cb (GtkTreeView *view,
		   gpointer     data)
{
	XeditPluginManager *pm = data;
	XeditPluginInfo *info;

	xedit_debug (DEBUG_PLUGINS);

	info = plugin_manager_get_selected_plugin (pm);

	gtk_widget_set_sensitive (GTK_WIDGET (pm->priv->about_button),
				  info != NULL);
	gtk_widget_set_sensitive (GTK_WIDGET (pm->priv->configure_button),
				  (info != NULL) && 
				   xedit_plugin_info_is_configurable (info));
}

static void
row_activated_cb (GtkTreeView       *tree_view,
		  GtkTreePath       *path,
		  GtkTreeViewColumn *column,
		  gpointer           data)
{
	XeditPluginManager *pm = data;
	GtkTreeIter iter;
	GtkTreeModel *model;

	xedit_debug (DEBUG_PLUGINS);

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (pm->priv->tree));

	g_return_if_fail (model != NULL);

	gtk_tree_model_get_iter (model, &iter, path);

	g_return_if_fail (&iter != NULL);

	plugin_manager_toggle_active (pm, &iter, model);
}

static void
plugin_manager_populate_lists (XeditPluginManager *pm)
{
	const GList *plugins;
	GtkListStore *model;
	GtkTreeIter iter;

	xedit_debug (DEBUG_PLUGINS);

	plugins = xedit_plugins_engine_get_plugin_list (pm->priv->engine);

	model = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (pm->priv->tree)));

	while (plugins)
	{
		XeditPluginInfo *info;
		info = (XeditPluginInfo *)plugins->data;

		gtk_list_store_append (model, &iter);
		gtk_list_store_set (model, &iter,
				    ACTIVE_COLUMN, xedit_plugin_info_is_active (info),
				    AVAILABLE_COLUMN, xedit_plugin_info_is_available (info),
				    INFO_COLUMN, info,
				    -1);

		plugins = plugins->next;
	}

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
	{
		GtkTreeSelection *selection;
		XeditPluginInfo* info;

		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pm->priv->tree));
		g_return_if_fail (selection != NULL);
		
		gtk_tree_selection_select_iter (selection, &iter);

		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
				    INFO_COLUMN, &info, -1);

		gtk_widget_set_sensitive (GTK_WIDGET (pm->priv->configure_button),
					  xedit_plugin_info_is_configurable (info));
	}
}

static gboolean
plugin_manager_set_active (XeditPluginManager *pm,
			   GtkTreeIter        *iter,
			   GtkTreeModel       *model,
			   gboolean            active)
{
	XeditPluginInfo *info;
	gboolean res = TRUE;
	
	xedit_debug (DEBUG_PLUGINS);

	gtk_tree_model_get (model, iter, INFO_COLUMN, &info, -1);

	g_return_val_if_fail (info != NULL, FALSE);

	if (active)
	{
		/* activate the plugin */
		if (!xedit_plugins_engine_activate_plugin (pm->priv->engine, info)) {
			xedit_debug_message (DEBUG_PLUGINS, "Could not activate %s.\n", 
					     xedit_plugin_info_get_name (info));

			res = FALSE;
		}
	}
	else
	{
		/* deactivate the plugin */
		if (!xedit_plugins_engine_deactivate_plugin (pm->priv->engine, info)) {
			xedit_debug_message (DEBUG_PLUGINS, "Could not deactivate %s.\n", 
					     xedit_plugin_info_get_name (info));

			res = FALSE;
		}
	}

	return res;
}

static void
plugin_manager_toggle_active (XeditPluginManager *pm,
			      GtkTreeIter        *iter,
			      GtkTreeModel       *model)
{
	gboolean active;
	
	xedit_debug (DEBUG_PLUGINS);

	gtk_tree_model_get (model, iter, ACTIVE_COLUMN, &active, -1);

	active ^= 1;

	plugin_manager_set_active (pm, iter, model, active);
}

static XeditPluginInfo *
plugin_manager_get_selected_plugin (XeditPluginManager *pm)
{
	XeditPluginInfo *info = NULL;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *selection;

	xedit_debug (DEBUG_PLUGINS);

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (pm->priv->tree));
	g_return_val_if_fail (model != NULL, NULL);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pm->priv->tree));
	g_return_val_if_fail (selection != NULL, NULL);

	if (gtk_tree_selection_get_selected (selection, NULL, &iter))
	{
		gtk_tree_model_get (model, &iter, INFO_COLUMN, &info, -1);
	}
	
	return info;
}

static void
plugin_manager_set_active_all (XeditPluginManager *pm,
			       gboolean            active)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	xedit_debug (DEBUG_PLUGINS);

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (pm->priv->tree));

	g_return_if_fail (model != NULL);

	gtk_tree_model_get_iter_first (model, &iter);

	do {
		plugin_manager_set_active (pm, &iter, model, active);
	}
	while (gtk_tree_model_iter_next (model, &iter));
}

/* Callback used as the interactive search comparison function */
static gboolean
name_search_cb (GtkTreeModel *model,
		gint          column,
		const gchar  *key,
		GtkTreeIter  *iter,
		gpointer      data)
{
	XeditPluginInfo *info;
	gchar *normalized_string;
	gchar *normalized_key;
	gchar *case_normalized_string;
	gchar *case_normalized_key;
	gint key_len;
	gboolean retval;

	gtk_tree_model_get (model, iter, INFO_COLUMN, &info, -1);
	if (!info)
		return FALSE;

	normalized_string = g_utf8_normalize (xedit_plugin_info_get_name (info), -1, G_NORMALIZE_ALL);
	normalized_key = g_utf8_normalize (key, -1, G_NORMALIZE_ALL);
	case_normalized_string = g_utf8_casefold (normalized_string, -1);
	case_normalized_key = g_utf8_casefold (normalized_key, -1);

	key_len = strlen (case_normalized_key);

	/* Oddly enough, this callback must return whether to stop the search
	 * because we found a match, not whether we actually matched.
	 */
	retval = (strncmp (case_normalized_key, case_normalized_string, key_len) != 0);

	g_free (normalized_key);
	g_free (normalized_string);
	g_free (case_normalized_key);
	g_free (case_normalized_string);

	return retval;
}

static void
enable_plugin_menu_cb (GtkMenu            *menu,
		       XeditPluginManager *pm)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *selection;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (pm->priv->tree));
	g_return_if_fail (model != NULL);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pm->priv->tree));
	g_return_if_fail (selection != NULL);

	if (gtk_tree_selection_get_selected (selection, NULL, &iter))
		plugin_manager_toggle_active (pm, &iter, model);
}

static void
enable_all_menu_cb (GtkMenu            *menu,
		    XeditPluginManager *pm)
{
	plugin_manager_set_active_all (pm, TRUE);
}

static void
disable_all_menu_cb (GtkMenu            *menu,
		     XeditPluginManager *pm)
{
	plugin_manager_set_active_all (pm, FALSE);
}

static GtkWidget *
create_tree_popup_menu (XeditPluginManager *pm)
{
	GtkWidget *menu;
	GtkWidget *item;
	GtkWidget *image;
	XeditPluginInfo *info;

	info = plugin_manager_get_selected_plugin (pm);

	menu = gtk_menu_new ();

	item = gtk_image_menu_item_new_with_mnemonic (_("_About"));
	image = gtk_image_new_from_stock (GTK_STOCK_ABOUT,
					  GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
	g_signal_connect (item, "activate",
			  G_CALLBACK (about_button_cb), pm);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	item = gtk_image_menu_item_new_with_mnemonic (_("C_onfigure"));
	image = gtk_image_new_from_stock (GTK_STOCK_PREFERENCES,
					  GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
	g_signal_connect (item, "activate",
			  G_CALLBACK (configure_button_cb), pm);
	gtk_widget_set_sensitive (item, xedit_plugin_info_is_configurable (info));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	item = gtk_check_menu_item_new_with_mnemonic (_("A_ctivate"));
	gtk_widget_set_sensitive (item, xedit_plugin_info_is_available (info));
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item),
					xedit_plugin_info_is_active (info));
	g_signal_connect (item, "toggled",
			  G_CALLBACK (enable_plugin_menu_cb), pm);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	item = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	item = gtk_menu_item_new_with_mnemonic (_("Ac_tivate All"));
	g_signal_connect (item, "activate",
			  G_CALLBACK (enable_all_menu_cb), pm);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	item = gtk_menu_item_new_with_mnemonic (_("_Deactivate All"));
	g_signal_connect (item, "activate",
			  G_CALLBACK (disable_all_menu_cb), pm);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	
	gtk_widget_show_all (menu);
	
	return menu;
}

static void
tree_popup_menu_detach (XeditPluginManager *pm,
			GtkMenu            *menu)
{
	pm->priv->popup_menu = NULL;
}

static void
show_tree_popup_menu (GtkTreeView        *tree,
		      XeditPluginManager *pm,
		      GdkEventButton     *event)
{
	if (pm->priv->popup_menu)
		gtk_widget_destroy (pm->priv->popup_menu);

	pm->priv->popup_menu = create_tree_popup_menu (pm);
	
	gtk_menu_attach_to_widget (GTK_MENU (pm->priv->popup_menu),
				   GTK_WIDGET (pm),
				   (GtkMenuDetachFunc) tree_popup_menu_detach);

	if (event != NULL)
	{
		gtk_menu_popup (GTK_MENU (pm->priv->popup_menu), NULL, NULL,
				NULL, NULL,
				event->button, event->time);
	}
	else
	{
		gtk_menu_popup (GTK_MENU (pm->priv->popup_menu), NULL, NULL,
				xedit_utils_menu_position_under_tree_view, tree,
				0, gtk_get_current_event_time ());

		gtk_menu_shell_select_first (GTK_MENU_SHELL (pm->priv->popup_menu),
					     FALSE);
	}
}

static gboolean
button_press_event_cb (GtkWidget          *tree,
		       GdkEventButton     *event,
		       XeditPluginManager *pm)
{
	/* We want the treeview selection to be updated before showing the menu.
	 * This code is evil, thanks to Federico Mena Quintero's black magic.
	 * See: http://mail.gnome.org/archives/gtk-devel-list/2006-February/msg00168.html
	 * FIXME: Let's remove it asap.
	 */

	static gboolean in_press = FALSE;
	gboolean handled;

	if (in_press)
		return FALSE; /* we re-entered */

	if (GDK_BUTTON_PRESS != event->type || 3 != event->button)
		return FALSE; /* let the normal handler run */

	in_press = TRUE;
	handled = gtk_widget_event (tree, (GdkEvent *) event);
	in_press = FALSE;

	if (!handled)
		return FALSE;
		
	/* The selection is fully updated by now */
	show_tree_popup_menu (GTK_TREE_VIEW (tree), pm, event);
	return TRUE;
}

static gboolean
popup_menu_cb (GtkTreeView        *tree,
	       XeditPluginManager *pm)
{
	show_tree_popup_menu (tree, pm, NULL);
	return TRUE;
}

static gint 
model_name_sort_func (GtkTreeModel *model,
		      GtkTreeIter  *iter1,
		      GtkTreeIter  *iter2,
		      gpointer      user_data)
{
	XeditPluginInfo *info1, *info2;
	
	gtk_tree_model_get (model, iter1, INFO_COLUMN, &info1, -1);
	gtk_tree_model_get (model, iter2, INFO_COLUMN, &info2, -1);

	return g_utf8_collate (xedit_plugin_info_get_name (info1),
			       xedit_plugin_info_get_name (info2));
}

static void
plugin_manager_construct_tree (XeditPluginManager *pm)
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;
	GtkListStore *model;

	xedit_debug (DEBUG_PLUGINS);

	model = gtk_list_store_new (N_COLUMNS,
				    G_TYPE_BOOLEAN,
				    G_TYPE_BOOLEAN,
				    G_TYPE_POINTER);

	gtk_tree_view_set_model (GTK_TREE_VIEW (pm->priv->tree),
				 GTK_TREE_MODEL (model));
	g_object_unref (model);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (pm->priv->tree), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (pm->priv->tree), FALSE);

	/* first column */
	cell = gtk_cell_renderer_toggle_new ();
	g_object_set (cell, "xpad", 6, NULL);
	g_signal_connect (cell,
			  "toggled",
			  G_CALLBACK (active_toggled_cb),
			  pm);
	column = gtk_tree_view_column_new_with_attributes (PLUGIN_MANAGER_ACTIVE_TITLE,
							   cell,
							   "active",
							   ACTIVE_COLUMN,
							   "activatable",
							   AVAILABLE_COLUMN,
							   "sensitive",
							   AVAILABLE_COLUMN,
							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (pm->priv->tree), column);

	/* second column */
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, PLUGIN_MANAGER_NAME_TITLE);
	gtk_tree_view_column_set_resizable (column, TRUE);

	cell = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, cell, FALSE);
	g_object_set (cell, "stock-size", GTK_ICON_SIZE_SMALL_TOOLBAR, NULL);
	gtk_tree_view_column_set_cell_data_func (column, cell,
						 plugin_manager_view_icon_cell_cb,
						 pm, NULL);
	
	cell = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, cell, TRUE);
	g_object_set (cell, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_column_set_cell_data_func (column, cell,
						 plugin_manager_view_info_cell_cb,
						 pm, NULL);
	
	
	gtk_tree_view_column_set_spacing (column, 6);
	gtk_tree_view_append_column (GTK_TREE_VIEW (pm->priv->tree), column);

	/* Sort on the plugin names */
	gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE (model),
	                                         model_name_sort_func,
        	                                 NULL,
                	                         NULL);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model),
					      GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
					      GTK_SORT_ASCENDING);

	/* Enable search for our non-string column */
	gtk_tree_view_set_search_column (GTK_TREE_VIEW (pm->priv->tree),
					 INFO_COLUMN);
	gtk_tree_view_set_search_equal_func (GTK_TREE_VIEW (pm->priv->tree),
					     name_search_cb,
					     NULL,
					     NULL);

	g_signal_connect (pm->priv->tree,
			  "cursor_changed",
			  G_CALLBACK (cursor_changed_cb),
			  pm);
	g_signal_connect (pm->priv->tree,
			  "row_activated",
			  G_CALLBACK (row_activated_cb),
			  pm);

	g_signal_connect (pm->priv->tree,
			  "button-press-event",
			  G_CALLBACK (button_press_event_cb),
			  pm);
	g_signal_connect (pm->priv->tree,
			  "popup-menu",
			  G_CALLBACK (popup_menu_cb),
			  pm);
	gtk_widget_show (pm->priv->tree);
}

static void
plugin_toggled_cb (XeditPluginsEngine *engine,
		   XeditPluginInfo    *info,
		   XeditPluginManager *pm)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean info_found = FALSE;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pm->priv->tree));

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		/* There is an item selected: it's probably the one we want! */
		XeditPluginInfo *tinfo;
		gtk_tree_model_get (model, &iter, INFO_COLUMN, &tinfo, -1);
		info_found = info == tinfo;
	}

	if (!info_found)
	{
		gtk_tree_model_get_iter_first (model, &iter);

		do
		{
			XeditPluginInfo *tinfo;
			gtk_tree_model_get (model, &iter, INFO_COLUMN, &tinfo, -1);
			info_found = info == tinfo;
		}
		while (!info_found && gtk_tree_model_iter_next (model, &iter));
	}

	if (!info_found)
	{
		g_warning ("XeditPluginManager: plugin '%s' not found in the tree model",
			   xedit_plugin_info_get_name (info));
		return;
	}

	gtk_list_store_set (GTK_LIST_STORE (model), &iter, ACTIVE_COLUMN, xedit_plugin_info_is_active (info), -1);
}

static void 
xedit_plugin_manager_init (XeditPluginManager *pm)
{
	GtkWidget *label;
	GtkWidget *viewport;
	GtkWidget *hbuttonbox;

	xedit_debug (DEBUG_PLUGINS);

	pm->priv = XEDIT_PLUGIN_MANAGER_GET_PRIVATE (pm);

#if GTK_CHECK_VERSION (3, 0, 0)
	gtk_orientable_set_orientation (GTK_ORIENTABLE (pm),
	                                GTK_ORIENTATION_VERTICAL);
#endif

	/*
	 * Always we create the manager, firstly we rescan the plugins directory
	 */
	xedit_plugins_engine_rescan_plugins (xedit_plugins_engine_get_default ());

	gtk_box_set_spacing (GTK_BOX (pm), 6);

	label = gtk_label_new_with_mnemonic (_("Active _Plugins:"));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
#if GTK_CHECK_VERSION (3, 16, 0)
	gtk_label_set_xalign (GTK_LABEL (label), 0.0);
#else
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
	
	gtk_box_pack_start (GTK_BOX (pm), label, FALSE, TRUE, 0);
	
	viewport = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (viewport),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (viewport), 
					     GTK_SHADOW_IN);

	gtk_box_pack_start (GTK_BOX (pm), viewport, TRUE, TRUE, 0);

	pm->priv->tree = gtk_tree_view_new ();
	gtk_container_add (GTK_CONTAINER (viewport), pm->priv->tree);

	gtk_label_set_mnemonic_widget (GTK_LABEL (label), pm->priv->tree);

#if GTK_CHECK_VERSION (3, 0, 0)
	hbuttonbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
#else
	hbuttonbox = gtk_hbutton_box_new ();
#endif
	gtk_box_pack_start (GTK_BOX (pm), hbuttonbox, FALSE, FALSE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
	gtk_box_set_spacing (GTK_BOX (hbuttonbox), 8);

	pm->priv->about_button = xedit_gtk_button_new_with_stock_icon (_("_About Plugin"),
								       GTK_STOCK_ABOUT);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), pm->priv->about_button);

	pm->priv->configure_button = xedit_gtk_button_new_with_stock_icon (_("C_onfigure Plugin"),
									   GTK_STOCK_PREFERENCES);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), pm->priv->configure_button);

	/* setup a window of a sane size. */
	gtk_widget_set_size_request (GTK_WIDGET (viewport), 270, 100);

	g_signal_connect (pm->priv->about_button,
			  "clicked",
			  G_CALLBACK (about_button_cb),
			  pm);
	g_signal_connect (pm->priv->configure_button,
			  "clicked",
			  G_CALLBACK (configure_button_cb),
			  pm);

	plugin_manager_construct_tree (pm);

	/* get the plugin engine and populate the treeview */
	pm->priv->engine = xedit_plugins_engine_get_default ();

	g_signal_connect_after (pm->priv->engine,
				"activate-plugin",
				G_CALLBACK (plugin_toggled_cb),
				pm);
	g_signal_connect_after (pm->priv->engine,
				"deactivate-plugin",
				G_CALLBACK (plugin_toggled_cb),
				pm);

	if (xedit_plugins_engine_get_plugin_list (pm->priv->engine) != NULL)
	{
		plugin_manager_populate_lists (pm);
	}
	else
	{
		gtk_widget_set_sensitive (pm->priv->about_button, FALSE);
		gtk_widget_set_sensitive (pm->priv->configure_button, FALSE);		
	}
}

static void
xedit_plugin_manager_finalize (GObject *object)
{
	XeditPluginManager *pm = XEDIT_PLUGIN_MANAGER (object);

	g_signal_handlers_disconnect_by_func (pm->priv->engine,
					      plugin_toggled_cb,
					      pm);

	if (pm->priv->popup_menu)
		gtk_widget_destroy (pm->priv->popup_menu);

	G_OBJECT_CLASS (xedit_plugin_manager_parent_class)->finalize (object);

}

GtkWidget *xedit_plugin_manager_new (void)
{
	return g_object_new (XEDIT_TYPE_PLUGIN_MANAGER,0);
}
