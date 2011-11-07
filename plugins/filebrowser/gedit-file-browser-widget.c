/*
 * gedit-file-browser-widget.c - Gedit plugin providing easy file access 
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

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gdk/gdkkeysyms.h>
#include <gedit/gedit-utils.h>
#include <gedit/gedit-plugin.h>

#include "gedit-file-browser-utils.h"
#include "gedit-file-browser-error.h"
#include "gedit-file-browser-widget.h"
#include "gedit-file-browser-view.h"
#include "gedit-file-browser-store.h"
#include "gedit-file-bookmarks-store.h"
#include "gedit-file-browser-marshal.h"
#include "gedit-file-browser-enum-types.h"

#define GEDIT_FILE_BROWSER_WIDGET_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), \
						      GEDIT_TYPE_FILE_BROWSER_WIDGET, \
						      GeditFileBrowserWidgetPrivate))

#define XML_UI_FILE "gedit-file-browser-widget-ui.xml"
#define LOCATION_DATA_KEY "gedit-file-browser-widget-location"

enum 
{
	BOOKMARKS_ID,
	SEPARATOR_CUSTOM_ID,
	SEPARATOR_ID,
	PATH_ID,
	NUM_DEFAULT_IDS
};

enum 
{
	COLUMN_INDENT,
	COLUMN_ICON,
	COLUMN_NAME,
	COLUMN_FILE,
	COLUMN_ID,
	N_COLUMNS
};

/* Properties */
enum 
{
	PROP_0,
	
	PROP_FILTER_PATTERN,
	PROP_ENABLE_DELETE
};

/* Signals */
enum 
{
	URI_ACTIVATED,
	ERROR,
	CONFIRM_DELETE,
	CONFIRM_NO_TRASH,
	NUM_SIGNALS
};

static guint signals[NUM_SIGNALS] = { 0 };

typedef struct _SignalNode 
{
	GObject *object;
	gulong id;
} SignalNode;

typedef struct
{
	gulong id;
	GeditFileBrowserWidgetFilterFunc func;
	gpointer user_data;
	GDestroyNotify destroy_notify;
} FilterFunc;

typedef struct 
{
	GFile *root;
	GFile *virtual_root;
} Location;

typedef struct
{
	gchar *name;
	GdkPixbuf *icon;
} NameIcon;

struct _GeditFileBrowserWidgetPrivate 
{
	GeditFileBrowserView *treeview;
	GeditFileBrowserStore *file_store;
	GeditFileBookmarksStore *bookmarks_store;

	GHashTable *bookmarks_hash;

	GtkWidget *combo;
	GtkTreeStore *combo_model;

	GtkWidget *filter_expander;
	GtkWidget *filter_entry;

	GtkUIManager *manager;
	GtkActionGroup *action_group;
	GtkActionGroup *action_group_selection;
	GtkActionGroup *action_group_file_selection;
	GtkActionGroup *action_group_single_selection;
	GtkActionGroup *action_group_single_most_selection;
	GtkActionGroup *action_group_sensitive;
	GtkActionGroup *bookmark_action_group;

	GSList *signal_pool;

	GSList *filter_funcs;
	gulong filter_id;
	gulong glob_filter_id;
	GPatternSpec *filter_pattern;
	gchar *filter_pattern_str;

	GList *locations;
	GList *current_location;
	gboolean changing_location;
	GtkWidget *location_previous_menu;
	GtkWidget *location_next_menu;
	GtkWidget *current_location_menu_item;
	
	gboolean enable_delete;
	
	GCancellable *cancellable;
	
	GdkCursor *busy_cursor;
};

static void set_enable_delete		       (GeditFileBrowserWidget *obj,
						gboolean enable);
static void on_model_set                       (GObject * gobject, 
						GParamSpec * arg1,
						GeditFileBrowserWidget * obj);
static void on_treeview_error                  (GeditFileBrowserView * tree_view,
						guint code, 
						gchar * message,
						GeditFileBrowserWidget * obj);
static void on_file_store_error                (GeditFileBrowserStore * store, 
						guint code,
						gchar * message,
						GeditFileBrowserWidget * obj);
static gboolean on_file_store_no_trash 	       (GeditFileBrowserStore * store, 
						GList * files, 
						GeditFileBrowserWidget * obj);
static void on_combo_changed                   (GtkComboBox * combo,
						GeditFileBrowserWidget * obj);
static gboolean on_treeview_popup_menu         (GeditFileBrowserView * treeview,
						GeditFileBrowserWidget * obj);
static gboolean on_treeview_button_press_event (GeditFileBrowserView * treeview,
						GdkEventButton * event,
						GeditFileBrowserWidget * obj);
static gboolean on_treeview_key_press_event    (GeditFileBrowserView * treeview, 
						GdkEventKey * event,
						GeditFileBrowserWidget * obj);
static void on_selection_changed               (GtkTreeSelection * selection,
						GeditFileBrowserWidget * obj);

static void on_virtual_root_changed            (GeditFileBrowserStore * model,
						GParamSpec *param,
						GeditFileBrowserWidget * obj);

static gboolean on_entry_filter_activate       (GeditFileBrowserWidget * obj);
static void on_location_jump_activate          (GtkMenuItem * item,
						GeditFileBrowserWidget * obj);
static void on_bookmarks_row_changed           (GtkTreeModel * model, 
                                                GtkTreePath * path,
                                                GtkTreeIter * iter,
                                                GeditFileBrowserWidget * obj);
static void on_bookmarks_row_deleted           (GtkTreeModel * model, 
                                                GtkTreePath * path,
                                                GeditFileBrowserWidget * obj);
static void on_filter_mode_changed	       (GeditFileBrowserStore * model,
                                                GParamSpec * param,
                                                GeditFileBrowserWidget * obj);
static void on_action_directory_previous       (GtkAction * action,
						GeditFileBrowserWidget * obj);
static void on_action_directory_next           (GtkAction * action,
						GeditFileBrowserWidget * obj);
static void on_action_directory_up             (GtkAction * action,
						GeditFileBrowserWidget * obj);
static void on_action_directory_new            (GtkAction * action,
						GeditFileBrowserWidget * obj);
static void on_action_file_open                (GtkAction * action,
						GeditFileBrowserWidget * obj);
static void on_action_file_new                 (GtkAction * action,
						GeditFileBrowserWidget * obj);
static void on_action_file_rename              (GtkAction * action,
						GeditFileBrowserWidget * obj);
static void on_action_file_delete              (GtkAction * action,
						GeditFileBrowserWidget * obj);
static void on_action_file_move_to_trash       (GtkAction * action,
						GeditFileBrowserWidget * obj);
static void on_action_directory_refresh        (GtkAction * action,
						GeditFileBrowserWidget * obj);
static void on_action_directory_open           (GtkAction * action,
						GeditFileBrowserWidget * obj);
static void on_action_filter_hidden            (GtkAction * action,
						GeditFileBrowserWidget * obj);
static void on_action_filter_binary            (GtkAction * action,
						GeditFileBrowserWidget * obj);
static void on_action_bookmark_open            (GtkAction * action,
						GeditFileBrowserWidget * obj);

GEDIT_PLUGIN_DEFINE_TYPE (GeditFileBrowserWidget, gedit_file_browser_widget,
	                  GTK_TYPE_VBOX)

static void
free_name_icon (gpointer data)
{
	NameIcon * item;

	if (data == NULL)
		return;

	item = (NameIcon *)(data);

	g_free (item->name);

	if (item->icon)
		g_object_unref (item->icon);

	g_free (item);
}

static FilterFunc *
filter_func_new (GeditFileBrowserWidget * obj,
		 GeditFileBrowserWidgetFilterFunc func,
		 gpointer user_data,
		 GDestroyNotify notify)
{
	FilterFunc *result;

	result = g_new (FilterFunc, 1);

	result->id = ++obj->priv->filter_id;
	result->func = func;
	result->user_data = user_data;
	result->destroy_notify = notify;
	return result;
}

static void
location_free (Location * loc)
{
	if (loc->root)
		g_object_unref (loc->root);
	
	if (loc->virtual_root)
		g_object_unref (loc->virtual_root);

	g_free (loc);
}

static gboolean
combo_find_by_id (GeditFileBrowserWidget * obj, guint id,
		  GtkTreeIter * iter)
{
	guint checkid;
	GtkTreeModel *model = GTK_TREE_MODEL (obj->priv->combo_model);

	if (iter == NULL)
		return FALSE;

	if (gtk_tree_model_get_iter_first (model, iter)) {
		do {
			gtk_tree_model_get (model, iter, COLUMN_ID,
					    &checkid, -1);

			if (checkid == id)
				return TRUE;
		} while (gtk_tree_model_iter_next (model, iter));
	}

	return FALSE;
}

static void
remove_path_items (GeditFileBrowserWidget * obj)
{
	GtkTreeIter iter;

	while (combo_find_by_id (obj, PATH_ID, &iter))
		gtk_tree_store_remove (obj->priv->combo_model, &iter);
}

static void
cancel_async_operation (GeditFileBrowserWidget *widget)
{
	if (!widget->priv->cancellable)
		return;
	
	g_cancellable_cancel (widget->priv->cancellable);
	g_object_unref (widget->priv->cancellable);

	widget->priv->cancellable = NULL;
}

static void
gedit_file_browser_widget_finalize (GObject * object)
{
	GeditFileBrowserWidget *obj = GEDIT_FILE_BROWSER_WIDGET (object);
	GList *loc;

	remove_path_items (obj);
	gedit_file_browser_store_set_filter_func (obj->priv->file_store,
						  NULL, NULL);

	g_object_unref (obj->priv->manager);
	g_object_unref (obj->priv->file_store);
	g_object_unref (obj->priv->bookmarks_store);
	g_object_unref (obj->priv->combo_model);

	g_slist_foreach (obj->priv->filter_funcs, (GFunc) g_free, NULL);
	g_slist_free (obj->priv->filter_funcs);

	for (loc = obj->priv->locations; loc; loc = loc->next)
		location_free ((Location *) (loc->data));

	if (obj->priv->current_location_menu_item)
		g_object_unref (obj->priv->current_location_menu_item);

	g_list_free (obj->priv->locations);

	g_hash_table_destroy (obj->priv->bookmarks_hash);
	
	cancel_async_operation (obj);
	
	gdk_cursor_unref (obj->priv->busy_cursor);

	G_OBJECT_CLASS (gedit_file_browser_widget_parent_class)->finalize (object);
}

static void
gedit_file_browser_widget_get_property (GObject    *object,
			               guint       prop_id,
			               GValue     *value,
			               GParamSpec *pspec)
{
	GeditFileBrowserWidget *obj = GEDIT_FILE_BROWSER_WIDGET (object);

	switch (prop_id)
	{
		case PROP_FILTER_PATTERN:
			g_value_set_string (value, obj->priv->filter_pattern_str);
			break;
		case PROP_ENABLE_DELETE:
			g_value_set_boolean (value, obj->priv->enable_delete);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gedit_file_browser_widget_set_property (GObject      *object,
			               guint         prop_id,
			               const GValue *value,
			               GParamSpec   *pspec)
{
	GeditFileBrowserWidget *obj = GEDIT_FILE_BROWSER_WIDGET (object);

	switch (prop_id)
	{
		case PROP_FILTER_PATTERN:
			gedit_file_browser_widget_set_filter_pattern (obj,
			                                              g_value_get_string (value));
			break;
		case PROP_ENABLE_DELETE:
			set_enable_delete (obj, g_value_get_boolean (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gedit_file_browser_widget_class_init (GeditFileBrowserWidgetClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gedit_file_browser_widget_finalize;

	object_class->get_property = gedit_file_browser_widget_get_property;
	object_class->set_property = gedit_file_browser_widget_set_property;

	g_object_class_install_property (object_class, PROP_FILTER_PATTERN,
					 g_param_spec_string ("filter-pattern",
					 		      "Filter Pattern",
					 		      "The filter pattern",
					 		      NULL,
					 		      G_PARAM_READWRITE));
	g_object_class_install_property (object_class, PROP_ENABLE_DELETE,
					 g_param_spec_boolean ("enable-delete",
					 		       "Enable delete",
					 		       "Enable permanently deleting items",
					 		       TRUE,
					 		       G_PARAM_READWRITE |
					 		       G_PARAM_CONSTRUCT));

	signals[URI_ACTIVATED] =
	    g_signal_new ("uri-activated",
			  G_OBJECT_CLASS_TYPE (object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (GeditFileBrowserWidgetClass,
					   uri_activated), NULL, NULL,
			  g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1,
			  G_TYPE_STRING);
	signals[ERROR] =
	    g_signal_new ("error", G_OBJECT_CLASS_TYPE (object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (GeditFileBrowserWidgetClass,
					   error), NULL, NULL,
			  gedit_file_browser_marshal_VOID__UINT_STRING,
			  G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_STRING);

	signals[CONFIRM_DELETE] =
	    g_signal_new ("confirm-delete", G_OBJECT_CLASS_TYPE (object_class),
	                  G_SIGNAL_RUN_LAST,
	                  G_STRUCT_OFFSET (GeditFileBrowserWidgetClass,
	                                   confirm_delete),
	                  g_signal_accumulator_true_handled,
	                  NULL,
	                  gedit_file_browser_marshal_BOOL__OBJECT_POINTER,
	                  G_TYPE_BOOLEAN,
	                  2,
	                  G_TYPE_OBJECT,
	                  G_TYPE_POINTER);

	signals[CONFIRM_NO_TRASH] =
	    g_signal_new ("confirm-no-trash", G_OBJECT_CLASS_TYPE (object_class),
	                  G_SIGNAL_RUN_LAST,
	                  G_STRUCT_OFFSET (GeditFileBrowserWidgetClass,
	                                   confirm_no_trash),
	                  g_signal_accumulator_true_handled,
	                  NULL,
	                  gedit_file_browser_marshal_BOOL__POINTER,
	                  G_TYPE_BOOLEAN,
	                  1,
	                  G_TYPE_POINTER);

	g_type_class_add_private (object_class,
				  sizeof (GeditFileBrowserWidgetPrivate));
}

static void
add_signal (GeditFileBrowserWidget * obj, gpointer object, gulong id)
{
	SignalNode *node = g_new (SignalNode, 1);

	node->object = G_OBJECT (object);
	node->id = id;

	obj->priv->signal_pool =
	    g_slist_prepend (obj->priv->signal_pool, node);
}

static void
clear_signals (GeditFileBrowserWidget * obj)
{
	GSList *item;
	SignalNode *node;

	for (item = obj->priv->signal_pool; item; item = item->next) {
		node = (SignalNode *) (item->data);

		g_signal_handler_disconnect (node->object, node->id);
		g_free (item->data);
	}

	g_slist_free (obj->priv->signal_pool);
	obj->priv->signal_pool = NULL;
}

static gboolean
separator_func (GtkTreeModel * model, GtkTreeIter * iter, gpointer data)
{
	guint id;

	gtk_tree_model_get (model, iter, COLUMN_ID, &id, -1);

	return (id == SEPARATOR_ID);
}

static gboolean
get_from_bookmark_file (GeditFileBrowserWidget * obj, GFile * file,
		       gchar ** name, GdkPixbuf ** icon)
{
	gpointer data;
	NameIcon * item;

	data = g_hash_table_lookup (obj->priv->bookmarks_hash, file);
	
	if (data == NULL)
		return FALSE;
	
	item = (NameIcon *)data;

	*name = g_strdup (item->name);
	*icon = item->icon;

	if (item->icon != NULL)
	{
		g_object_ref (item->icon);
	}

	return TRUE;
}

static void
insert_path_item (GeditFileBrowserWidget * obj, 
                  GFile * file,
		  GtkTreeIter * after, 
		  GtkTreeIter * iter, 
		  guint indent)
{
	gchar * unescape;
	GdkPixbuf * icon = NULL;

	/* Try to get the icon and name from the bookmarks hash */
	if (!get_from_bookmark_file (obj, file, &unescape, &icon)) {
		/* It's not a bookmark, fetch the name and the icon ourselves */
		unescape = gedit_file_browser_utils_file_basename (file);
		
		/* Get the icon */
		icon = gedit_file_browser_utils_pixbuf_from_file (file, GTK_ICON_SIZE_MENU);
	}

	gtk_tree_store_insert_after (obj->priv->combo_model, iter, NULL,
				     after);
	
	gtk_tree_store_set (obj->priv->combo_model, 
	                    iter, 
	                    COLUMN_INDENT, indent,
	                    COLUMN_ICON, icon, 
	                    COLUMN_NAME, unescape, 
	                    COLUMN_FILE, file,
			    COLUMN_ID, PATH_ID, 
			    -1);

	if (icon)
		g_object_unref (icon);

	g_free (unescape);
}

static void
insert_separator_item (GeditFileBrowserWidget * obj)
{
	GtkTreeIter iter;

	gtk_tree_store_insert (obj->priv->combo_model, &iter, NULL, 1);
	gtk_tree_store_set (obj->priv->combo_model, &iter,
			    COLUMN_ICON, NULL,
			    COLUMN_NAME, NULL,
			    COLUMN_ID, SEPARATOR_ID, -1);
}

static void
combo_set_active_by_id (GeditFileBrowserWidget * obj, guint id)
{
	GtkTreeIter iter;

	if (combo_find_by_id (obj, id, &iter))
		gtk_combo_box_set_active_iter (GTK_COMBO_BOX
					       (obj->priv->combo), &iter);
}

static guint
uri_num_parents (GFile * from, GFile * to)
{
	/* Determine the number of 'levels' to get from #from to #to. */
	guint parents = 0;
	GFile * parent;

	if (from == NULL)
		return 0;

	g_object_ref (from);
	
	while ((parent = g_file_get_parent (from)) && !(to && g_file_equal (from, to))) {
		g_object_unref (from);
		from = parent;
		
		++parents;
	}
	
	g_object_unref (from);	
	return parents;
}

static void
insert_location_path (GeditFileBrowserWidget * obj)
{
	Location *loc;
	GFile *current = NULL;
	GFile * tmp;
	GtkTreeIter separator;
	GtkTreeIter iter;
	guint indent;
	
	if (!obj->priv->current_location) {
		g_message ("insert_location_path: no current location");
		return;
	}

	loc = (Location *) (obj->priv->current_location->data);

	current = loc->virtual_root;
	combo_find_by_id (obj, SEPARATOR_ID, &separator);
	
	indent = uri_num_parents (loc->virtual_root, loc->root);

	while (current != NULL) {
		insert_path_item (obj, current, &separator, &iter, indent--);

		if (current == loc->virtual_root) {
			g_signal_handlers_block_by_func (obj->priv->combo,
							 on_combo_changed,
							 obj);
			gtk_combo_box_set_active_iter (GTK_COMBO_BOX
						       (obj->priv->combo),
						       &iter);
			g_signal_handlers_unblock_by_func (obj->priv->
							   combo,
							   on_combo_changed,
							   obj);
		}

		if (g_file_equal (current, loc->root) || !gedit_utils_file_has_parent (current)) {
			if (current != loc->virtual_root)
				g_object_unref (current);
			break;
		}

		tmp = g_file_get_parent (current);
		
		if (current != loc->virtual_root)
			g_object_unref (current);

		current = tmp;
	}
}

static void
check_current_item (GeditFileBrowserWidget * obj, gboolean show_path)
{
	GtkTreeIter separator;
	gboolean has_sep;

	remove_path_items (obj);
	has_sep = combo_find_by_id (obj, SEPARATOR_ID, &separator);

	if (show_path) {
		if (!has_sep)
			insert_separator_item (obj);

		insert_location_path (obj);
	} else if (has_sep)
		gtk_tree_store_remove (obj->priv->combo_model, &separator);
}

static void
fill_combo_model (GeditFileBrowserWidget * obj)
{
	GtkTreeStore *store = obj->priv->combo_model;
	GtkTreeIter iter;
	GdkPixbuf *icon;

	icon = gedit_file_browser_utils_pixbuf_from_theme (GTK_STOCK_HOME, GTK_ICON_SIZE_MENU);

	gtk_tree_store_append (store, &iter, NULL);
	gtk_tree_store_set (store, &iter,
			    COLUMN_ICON, icon,
			    COLUMN_NAME, _("Bookmarks"),
			    COLUMN_ID, BOOKMARKS_ID, -1);
	g_object_unref (icon);

	gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (obj->priv->combo),
					      separator_func, obj, NULL);
	gtk_combo_box_set_active (GTK_COMBO_BOX (obj->priv->combo), 0);
}

static void
indent_cell_data_func (GtkCellLayout * cell_layout, 
                       GtkCellRenderer * cell,
                       GtkTreeModel * model, 
                       GtkTreeIter * iter, 
                       gpointer data)
{
	gchar * indent;
	guint num;
	
	gtk_tree_model_get (model, iter, COLUMN_INDENT, &num, -1);
	
	if (num == 0)
		g_object_set (cell, "text", "", NULL);
	else {
		indent = g_strnfill (num * 2, ' ');
	
		g_object_set (cell, "text", indent, NULL);
		g_free (indent);
	}
}

static void
create_combo (GeditFileBrowserWidget * obj)
{
	GtkCellRenderer *renderer;

	obj->priv->combo_model = gtk_tree_store_new (N_COLUMNS,
						     G_TYPE_UINT,
						     GDK_TYPE_PIXBUF,
						     G_TYPE_STRING,
						     G_TYPE_FILE,
						     G_TYPE_UINT);
	obj->priv->combo =
	    gtk_combo_box_new_with_model (GTK_TREE_MODEL
					  (obj->priv->combo_model));

	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (obj->priv->combo),
	                            renderer, FALSE);
	gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT
					    (obj->priv->combo), renderer,
					    indent_cell_data_func, obj, NULL);


	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (obj->priv->combo),
				    renderer, FALSE);
	gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (obj->priv->combo),
				       renderer, "pixbuf", COLUMN_ICON);

	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (obj->priv->combo),
				    renderer, TRUE);
	gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (obj->priv->combo),
				       renderer, "text", COLUMN_NAME);

	g_object_set (renderer, "ellipsize-set", TRUE,
		      "ellipsize", PANGO_ELLIPSIZE_END, NULL);

	gtk_box_pack_start (GTK_BOX (obj), GTK_WIDGET (obj->priv->combo),
			    FALSE, FALSE, 0);

	fill_combo_model (obj);
	g_signal_connect (obj->priv->combo, "changed",
			  G_CALLBACK (on_combo_changed), obj);

	gtk_widget_show (obj->priv->combo);
}

static GtkActionEntry toplevel_actions[] = 
{
	{"FilterMenuAction", NULL, N_("_Filter")}
};

static const GtkActionEntry tree_actions_selection[] = 
{
	{"FileMoveToTrash", "mate-stock-trash", N_("_Move to Trash"), NULL,
	 N_("Move selected file or folder to trash"),
	 G_CALLBACK (on_action_file_move_to_trash)},
	{"FileDelete", GTK_STOCK_DELETE, N_("_Delete"), NULL,
	 N_("Delete selected file or folder"),
	 G_CALLBACK (on_action_file_delete)}
};

static const GtkActionEntry tree_actions_file_selection[] =
{
	{"FileOpen", GTK_STOCK_OPEN, NULL, NULL,
	 N_("Open selected file"),
	 G_CALLBACK (on_action_file_open)}
};

static const GtkActionEntry tree_actions[] =
{
	{"DirectoryUp", GTK_STOCK_GO_UP, N_("Up"), NULL,
	 N_("Open the parent folder"), G_CALLBACK (on_action_directory_up)}
};

static const GtkActionEntry tree_actions_single_most_selection[] =
{
	{"DirectoryNew", GTK_STOCK_ADD, N_("_New Folder"), NULL,
	 N_("Add new empty folder"),
	 G_CALLBACK (on_action_directory_new)},
	{"FileNew", GTK_STOCK_NEW, N_("New F_ile"), NULL,
	 N_("Add new empty file"), G_CALLBACK (on_action_file_new)}
};

static const GtkActionEntry tree_actions_single_selection[] = 
{
	{"FileRename", NULL, N_("_Rename"), NULL,
	 N_("Rename selected file or folder"),
	 G_CALLBACK (on_action_file_rename)}
};

static const GtkActionEntry tree_actions_sensitive[] = 
{
	{"DirectoryPrevious", GTK_STOCK_GO_BACK, N_("_Previous Location"),
	 NULL,
	 N_("Go to the previous visited location"),
	 G_CALLBACK (on_action_directory_previous)},
	{"DirectoryNext", GTK_STOCK_GO_FORWARD, N_("_Next Location"), NULL,
	 N_("Go to the next visited location"), G_CALLBACK (on_action_directory_next)},
	{"DirectoryRefresh", GTK_STOCK_REFRESH, N_("Re_fresh View"), NULL,
	 N_("Refresh the view"), G_CALLBACK (on_action_directory_refresh)},
	{"DirectoryOpen", GTK_STOCK_OPEN, N_("_View Folder"), NULL,
	 N_("View folder in file manager"),
	 G_CALLBACK (on_action_directory_open)}
};

static const GtkToggleActionEntry tree_actions_toggle[] = 
{
	{"FilterHidden", GTK_STOCK_DIALOG_AUTHENTICATION,
	 N_("Show _Hidden"), NULL,
	 N_("Show hidden files and folders"),
	 G_CALLBACK (on_action_filter_hidden), FALSE},
	{"FilterBinary", NULL, N_("Show _Binary"), NULL,
	 N_("Show binary files"), G_CALLBACK (on_action_filter_binary),
	 FALSE}
};

static const GtkActionEntry bookmark_actions[] =
{
	{"BookmarkOpen", GTK_STOCK_OPEN, N_("_View Folder"), NULL,
	 N_("View folder in file manager"), G_CALLBACK (on_action_bookmark_open)}
};

static void
create_toolbar (GeditFileBrowserWidget * obj,
		const gchar *data_dir)
{
	GtkUIManager *manager;
	GError *error = NULL;
	GtkActionGroup *action_group;
	GtkWidget *toolbar;
	GtkWidget *widget;
	GtkAction *action;
	gchar *ui_file;

	manager = gtk_ui_manager_new ();
	obj->priv->manager = manager;

	ui_file = g_build_filename (data_dir, XML_UI_FILE, NULL);
	gtk_ui_manager_add_ui_from_file (manager, ui_file, &error);

	g_free (ui_file);

	if (error != NULL) {
		g_warning ("Error in adding ui from file %s: %s",
			   XML_UI_FILE, error->message);
		g_error_free (error);
		return;
	}

	action_group = gtk_action_group_new ("FileBrowserWidgetActionGroupToplevel");
	gtk_action_group_set_translation_domain (action_group, NULL);
	gtk_action_group_add_actions (action_group,
				      toplevel_actions,
				      G_N_ELEMENTS (toplevel_actions),
				      obj);
	gtk_ui_manager_insert_action_group (manager, action_group, 0);

	action_group = gtk_action_group_new ("FileBrowserWidgetActionGroup");
	gtk_action_group_set_translation_domain (action_group, NULL);
	gtk_action_group_add_actions (action_group,
				      tree_actions,
				      G_N_ELEMENTS (tree_actions),
				      obj);
	gtk_action_group_add_toggle_actions (action_group,
					     tree_actions_toggle,
					     G_N_ELEMENTS (tree_actions_toggle),
					     obj);
	gtk_ui_manager_insert_action_group (manager, action_group, 0);
	obj->priv->action_group = action_group;

	action_group = gtk_action_group_new ("FileBrowserWidgetSelectionActionGroup");
	gtk_action_group_set_translation_domain (action_group, NULL);
	gtk_action_group_add_actions (action_group,
				      tree_actions_selection,
				      G_N_ELEMENTS (tree_actions_selection),
				      obj);
	gtk_ui_manager_insert_action_group (manager, action_group, 0);
	obj->priv->action_group_selection = action_group;

	action_group = gtk_action_group_new ("FileBrowserWidgetFileSelectionActionGroup");
	gtk_action_group_set_translation_domain (action_group, NULL);
	gtk_action_group_add_actions (action_group,
				      tree_actions_file_selection,
				      G_N_ELEMENTS (tree_actions_file_selection),
				      obj);
	gtk_ui_manager_insert_action_group (manager, action_group, 0);
	obj->priv->action_group_file_selection = action_group;

	action_group = gtk_action_group_new ("FileBrowserWidgetSingleSelectionActionGroup");
	gtk_action_group_set_translation_domain (action_group, NULL);
	gtk_action_group_add_actions (action_group,
				      tree_actions_single_selection,
				      G_N_ELEMENTS (tree_actions_single_selection),
				      obj);
	gtk_ui_manager_insert_action_group (manager, action_group, 0);
	obj->priv->action_group_single_selection = action_group;

	action_group = gtk_action_group_new ("FileBrowserWidgetSingleMostSelectionActionGroup");
	gtk_action_group_set_translation_domain (action_group, NULL);
	gtk_action_group_add_actions (action_group,
				      tree_actions_single_most_selection,
				      G_N_ELEMENTS (tree_actions_single_most_selection),
				      obj);
	gtk_ui_manager_insert_action_group (manager, action_group, 0);
	obj->priv->action_group_single_most_selection = action_group;

	action_group = gtk_action_group_new ("FileBrowserWidgetSensitiveActionGroup");
	gtk_action_group_set_translation_domain (action_group, NULL);
	gtk_action_group_add_actions (action_group,
				      tree_actions_sensitive,
				      G_N_ELEMENTS (tree_actions_sensitive),
				      obj);
	gtk_ui_manager_insert_action_group (manager, action_group, 0);
	obj->priv->action_group_sensitive = action_group;

	action_group = gtk_action_group_new ("FileBrowserWidgetBookmarkActionGroup");
	gtk_action_group_set_translation_domain (action_group, NULL);
	gtk_action_group_add_actions (action_group,
				      bookmark_actions,
				      G_N_ELEMENTS (bookmark_actions),
				      obj);
	gtk_ui_manager_insert_action_group (manager, action_group, 0);
	obj->priv->bookmark_action_group = action_group;

	action = gtk_action_group_get_action (obj->priv->action_group_sensitive,
					      "DirectoryPrevious");
	gtk_action_set_sensitive (action, FALSE);

	action = gtk_action_group_get_action (obj->priv->action_group_sensitive,
					      "DirectoryNext");
	gtk_action_set_sensitive (action, FALSE);

	toolbar = gtk_ui_manager_get_widget (manager, "/ToolBar");
	gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_icon_size (GTK_TOOLBAR (toolbar), GTK_ICON_SIZE_MENU);

	/* Previous directory menu tool item */
	obj->priv->location_previous_menu = gtk_menu_new ();
	gtk_widget_show (obj->priv->location_previous_menu);

	widget = GTK_WIDGET (gtk_menu_tool_button_new_from_stock (GTK_STOCK_GO_BACK));
	gtk_menu_tool_button_set_menu (GTK_MENU_TOOL_BUTTON (widget),
				       obj->priv->location_previous_menu);

	g_object_set (widget, "label", _("Previous location"), NULL);
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (widget),
					_("Go to previous location"));
	gtk_menu_tool_button_set_arrow_tooltip_text (GTK_MENU_TOOL_BUTTON (widget),
						     _("Go to a previously opened location"));

	action = gtk_action_group_get_action (obj->priv->action_group_sensitive,
					      "DirectoryPrevious");
	g_object_set (action, "is_important", TRUE, "short_label",
		      _("Previous location"), NULL);
	gtk_action_connect_proxy (action, widget);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (widget), 0);

	/* Next directory menu tool item */
	obj->priv->location_next_menu = gtk_menu_new ();
	gtk_widget_show (obj->priv->location_next_menu);

	widget = GTK_WIDGET (gtk_menu_tool_button_new_from_stock (GTK_STOCK_GO_FORWARD));
	gtk_menu_tool_button_set_menu (GTK_MENU_TOOL_BUTTON (widget),
				       obj->priv->location_next_menu);

	g_object_set (widget, "label", _("Next location"), NULL);
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (widget),
					_("Go to next location"));
	gtk_menu_tool_button_set_arrow_tooltip_text (GTK_MENU_TOOL_BUTTON (widget),
						     _("Go to a previously opened location"));

	action = gtk_action_group_get_action (obj->priv->action_group_sensitive,
					      "DirectoryNext");
	g_object_set (action, "is_important", TRUE, "short_label",
		      _("Previous location"), NULL);
	gtk_action_connect_proxy (action, widget);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (widget), 1);

	gtk_box_pack_start (GTK_BOX (obj), toolbar, FALSE, FALSE, 0);
	gtk_widget_show (toolbar);
	
	set_enable_delete (obj, obj->priv->enable_delete);
}

static void 
set_enable_delete (GeditFileBrowserWidget *obj,
		   gboolean enable)
{
	GtkAction *action;
	obj->priv->enable_delete = enable;
	
	if (obj->priv->action_group_selection == NULL)
		return;
	
	action =
	    gtk_action_group_get_action (obj->priv->action_group_selection,
					 "FileDelete");
	
	g_object_set (action, "visible", enable, "sensitive", enable, NULL);
}

static gboolean
filter_real (GeditFileBrowserStore * model, GtkTreeIter * iter,
	     GeditFileBrowserWidget * obj)
{
	GSList *item;
	FilterFunc *func;

	for (item = obj->priv->filter_funcs; item; item = item->next) {
		func = (FilterFunc *) (item->data);

		if (!func->func (obj, model, iter, func->user_data))
			return FALSE;
	}

	return TRUE;
}

static void
add_bookmark_hash (GeditFileBrowserWidget * obj,
                   GtkTreeIter * iter)
{
	GtkTreeModel *model;
	GdkPixbuf * pixbuf;
	gchar * name;
	gchar * uri;
	GFile * file;
	NameIcon * item;

	model = GTK_TREE_MODEL (obj->priv->bookmarks_store);

	uri = gedit_file_bookmarks_store_get_uri (obj->priv->
						  bookmarks_store,
						  iter);

	if (uri == NULL)
		return;
	
	file = g_file_new_for_uri (uri);
	g_free (uri);

	gtk_tree_model_get (model, iter,
			    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_ICON,
			    &pixbuf,
			    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_NAME,
			    &name, -1);	
	
	item = g_new (NameIcon, 1);
	item->name = name;
	item->icon = pixbuf;

	g_hash_table_insert (obj->priv->bookmarks_hash,
			     file,
			     item);
}

static void
init_bookmarks_hash (GeditFileBrowserWidget * obj)
{
	GtkTreeIter iter;
	GtkTreeModel *model;

	model = GTK_TREE_MODEL (obj->priv->bookmarks_store);
	
	if (!gtk_tree_model_get_iter_first (model, &iter))
		return;
	
	do {
		add_bookmark_hash (obj, &iter);
	} while (gtk_tree_model_iter_next (model, &iter));
	
	g_signal_connect (obj->priv->bookmarks_store,
		          "row-changed",
		          G_CALLBACK (on_bookmarks_row_changed),
		          obj);

	g_signal_connect (obj->priv->bookmarks_store,
		          "row-deleted",
		          G_CALLBACK (on_bookmarks_row_deleted),
		          obj);
}

static void
on_begin_loading (GeditFileBrowserStore  *model, 
		  GtkTreeIter            *iter,
		  GeditFileBrowserWidget *obj)
{
	if (!GDK_IS_WINDOW (gtk_widget_get_window (GTK_WIDGET (obj->priv->treeview))))
		return;

	gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (obj)),
			       obj->priv->busy_cursor);
}

static void
on_end_loading (GeditFileBrowserStore  *model, 
		GtkTreeIter            *iter,
		GeditFileBrowserWidget *obj)
{
	if (!GDK_IS_WINDOW (gtk_widget_get_window (GTK_WIDGET (obj->priv->treeview))))
		return;

	gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (obj)), NULL);
}

static void
create_tree (GeditFileBrowserWidget * obj)
{
	GtkWidget *sw;

	obj->priv->file_store = gedit_file_browser_store_new (NULL);
	obj->priv->bookmarks_store = gedit_file_bookmarks_store_new ();
	obj->priv->treeview =
	    GEDIT_FILE_BROWSER_VIEW (gedit_file_browser_view_new ());

	gedit_file_browser_view_set_restore_expand_state (obj->priv->treeview, TRUE);

	gedit_file_browser_store_set_filter_mode (obj->priv->file_store,
						  GEDIT_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN
						  |
						  GEDIT_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY);
	gedit_file_browser_store_set_filter_func (obj->priv->file_store,
						  (GeditFileBrowserStoreFilterFunc)
						  filter_real, obj);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
					     GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	gtk_container_add (GTK_CONTAINER (sw),
			   GTK_WIDGET (obj->priv->treeview));
	gtk_box_pack_start (GTK_BOX (obj), sw, TRUE, TRUE, 0);

	g_signal_connect (obj->priv->treeview, "notify::model",
			  G_CALLBACK (on_model_set), obj);
	g_signal_connect (obj->priv->treeview, "error",
			  G_CALLBACK (on_treeview_error), obj);
	g_signal_connect (obj->priv->treeview, "popup-menu",
			  G_CALLBACK (on_treeview_popup_menu), obj);
	g_signal_connect (obj->priv->treeview, "button-press-event",
			  G_CALLBACK (on_treeview_button_press_event),
			  obj);
	g_signal_connect (obj->priv->treeview, "key-press-event",
			  G_CALLBACK (on_treeview_key_press_event), obj);

	g_signal_connect (gtk_tree_view_get_selection
			  (GTK_TREE_VIEW (obj->priv->treeview)), "changed",
			  G_CALLBACK (on_selection_changed), obj);
	g_signal_connect (obj->priv->file_store, "notify::filter-mode",
			  G_CALLBACK (on_filter_mode_changed), obj);

	g_signal_connect (obj->priv->file_store, "notify::virtual-root",
			  G_CALLBACK (on_virtual_root_changed), obj);
			  
	g_signal_connect (obj->priv->file_store, "begin-loading",
			  G_CALLBACK (on_begin_loading), obj);
			  
	g_signal_connect (obj->priv->file_store, "end-loading",
			  G_CALLBACK (on_end_loading), obj);

	g_signal_connect (obj->priv->file_store, "error",
			  G_CALLBACK (on_file_store_error), obj);
	      
	init_bookmarks_hash (obj);

	gtk_widget_show (sw);
	gtk_widget_show (GTK_WIDGET (obj->priv->treeview));
}

static void
create_filter (GeditFileBrowserWidget * obj)
{
	GtkWidget *expander;
	GtkWidget *vbox;
	GtkWidget *entry;

	expander = gtk_expander_new_with_mnemonic (_("_Match Filename"));
	gtk_widget_show (expander);
	gtk_box_pack_start (GTK_BOX (obj), expander, FALSE, FALSE, 0);

	vbox = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vbox);

	obj->priv->filter_expander = expander;

	entry = gtk_entry_new ();
	gtk_widget_show (entry);

	obj->priv->filter_entry = entry;

	g_signal_connect_swapped (entry, "activate",
				  G_CALLBACK (on_entry_filter_activate),
				  obj);
	g_signal_connect_swapped (entry, "focus_out_event",
				  G_CALLBACK (on_entry_filter_activate),
				  obj);

	gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);
	gtk_container_add (GTK_CONTAINER (expander), vbox);
}

static void
gedit_file_browser_widget_init (GeditFileBrowserWidget * obj)
{
	obj->priv = GEDIT_FILE_BROWSER_WIDGET_GET_PRIVATE (obj);

	obj->priv->bookmarks_hash = g_hash_table_new_full (g_file_hash,
			                                   (GEqualFunc)g_file_equal,
			                                   g_object_unref,
			                                   free_name_icon);

	gtk_box_set_spacing (GTK_BOX (obj), 3);
	
	obj->priv->busy_cursor = gdk_cursor_new (GDK_WATCH);
}

/* Private */

static void
update_sensitivity (GeditFileBrowserWidget * obj)
{
	GtkTreeModel *model =
	    gtk_tree_view_get_model (GTK_TREE_VIEW (obj->priv->treeview));
	GtkAction *action;
	gint mode;

	if (GEDIT_IS_FILE_BROWSER_STORE (model)) {
		gtk_action_group_set_sensitive (obj->priv->action_group,
						TRUE);
		gtk_action_group_set_sensitive (obj->priv->bookmark_action_group,
						FALSE);

		mode =
		    gedit_file_browser_store_get_filter_mode
		    (GEDIT_FILE_BROWSER_STORE (model));

		action =
		    gtk_action_group_get_action (obj->priv->action_group,
						 "FilterHidden");
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
					      !(mode &
						GEDIT_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN));
	} else if (GEDIT_IS_FILE_BOOKMARKS_STORE (model)) {
		gtk_action_group_set_sensitive (obj->priv->action_group,
						FALSE);
		gtk_action_group_set_sensitive (obj->priv->bookmark_action_group,
						TRUE);

		/* Set the filter toggle to normal up state, just for visual pleasure */
		action =
		    gtk_action_group_get_action (obj->priv->action_group,
						 "FilterHidden");
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
					      FALSE);
	}

	on_selection_changed (gtk_tree_view_get_selection
			      (GTK_TREE_VIEW (obj->priv->treeview)), obj);
}

static gboolean
gedit_file_browser_widget_get_first_selected (GeditFileBrowserWidget *obj,
					      GtkTreeIter *iter)
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (obj->priv->treeview));
	GtkTreeModel *model;
	GList *rows = gtk_tree_selection_get_selected_rows (selection, &model);
	gboolean result;
	
	if (!rows)
		return FALSE;
	
	result = gtk_tree_model_get_iter(model, iter, (GtkTreePath *)(rows->data));

	g_list_foreach (rows, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (rows);
	
	return result;
}

static gboolean
popup_menu (GeditFileBrowserWidget * obj, GdkEventButton * event, GtkTreeModel * model)
{
	GtkWidget *menu;

	if (GEDIT_IS_FILE_BROWSER_STORE (model))
		menu = gtk_ui_manager_get_widget (obj->priv->manager, "/FilePopup");
	else if (GEDIT_IS_FILE_BOOKMARKS_STORE (model))
		menu = gtk_ui_manager_get_widget (obj->priv->manager, "/BookmarkPopup");
	else
		return FALSE;

	g_return_val_if_fail (menu != NULL, FALSE);

	if (event != NULL) {
		GtkTreeSelection *selection;
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (obj->priv->treeview));

		if (gtk_tree_selection_count_selected_rows (selection) <= 1) {
			GtkTreePath *path;

			if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (obj->priv->treeview),
			                                   (gint)event->x, (gint)event->y,
			                                   &path, NULL, NULL, NULL))
			{
				gtk_tree_selection_unselect_all (selection);
				gtk_tree_selection_select_path (selection, path);
				gtk_tree_path_free (path);
			}
		}

		gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
				event->button, event->time);
	} else {
		gtk_menu_popup (GTK_MENU (menu), NULL, NULL,
				gedit_utils_menu_position_under_tree_view,
				obj->priv->treeview, 0,
				gtk_get_current_event_time ());
		gtk_menu_shell_select_first (GTK_MENU_SHELL (menu), FALSE);
	}

	return TRUE;
}

static gboolean
filter_glob (GeditFileBrowserWidget * obj, GeditFileBrowserStore * store,
	     GtkTreeIter * iter, gpointer user_data)
{
	gchar *name;
	gboolean result;
	guint flags;

	if (obj->priv->filter_pattern == NULL)
		return TRUE;

	gtk_tree_model_get (GTK_TREE_MODEL (store), iter,
			    GEDIT_FILE_BROWSER_STORE_COLUMN_NAME, &name,
			    GEDIT_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags,
			    -1);

	if (FILE_IS_DIR (flags) || FILE_IS_DUMMY (flags))
		result = TRUE;
	else
		result =
		    g_pattern_match_string (obj->priv->filter_pattern,
					    name);

	g_free (name);

	return result;
}

static void
rename_selected_file (GeditFileBrowserWidget * obj)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (obj->priv->treeview));

	if (!GEDIT_IS_FILE_BROWSER_STORE (model))
		return;

	if (gedit_file_browser_widget_get_first_selected (obj, &iter))
		gedit_file_browser_view_start_rename (obj->priv->treeview,
						      &iter);
}

static GList *
get_deletable_files (GeditFileBrowserWidget *obj) {
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GList *rows;
	GList *row;
	GList *paths = NULL;
	guint flags;
	GtkTreeIter iter;
	GtkTreePath *path;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (obj->priv->treeview));

	/* Get all selected files */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (obj->priv->treeview));
	rows = gtk_tree_selection_get_selected_rows (selection, &model);
	
	for (row = rows; row; row = row->next) {
		path = (GtkTreePath *)(row->data);
		
		if (!gtk_tree_model_get_iter (model, &iter, path))
			continue;
		
		gtk_tree_model_get (model, &iter, 
				    GEDIT_FILE_BROWSER_STORE_COLUMN_FLAGS, 
				    &flags, -1);

		if (FILE_IS_DUMMY (flags))
			continue;

		paths = g_list_append (paths, gtk_tree_path_copy (path));
	}
	
	g_list_foreach (rows, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (rows);
	
	return paths;
}

static gboolean
delete_selected_files (GeditFileBrowserWidget * obj, gboolean trash)
{
	GtkTreeModel *model;
	gboolean confirm;
	GeditFileBrowserStoreResult result;
	GList *rows;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (obj->priv->treeview));

	if (!GEDIT_IS_FILE_BROWSER_STORE (model))
		return FALSE;
		
	rows = get_deletable_files (obj);

	if (!rows)
		return FALSE;

	if (!trash) {
		g_signal_emit (obj, signals[CONFIRM_DELETE], 0, model, rows, &confirm);

		if (!confirm)
			return FALSE;
	}

	result = gedit_file_browser_store_delete_all (GEDIT_FILE_BROWSER_STORE (model),
						      rows, trash);
	
	g_list_foreach (rows, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (rows);

	return result == GEDIT_FILE_BROWSER_STORE_RESULT_OK;
}

static gboolean
on_file_store_no_trash (GeditFileBrowserStore * store, 
			GList * files, 
			GeditFileBrowserWidget * obj)
{
	gboolean confirm = FALSE;
	
	g_signal_emit (obj, signals[CONFIRM_NO_TRASH], 0, files, &confirm);
	
	return confirm;
}

static GFile *
get_topmost_file (GFile * file)
{
	GFile * tmp;
	GFile * current;
	
	current = g_object_ref (file);
	
	while ((tmp = g_file_get_parent (current)) != NULL) {
		g_object_unref (current);
		current = tmp;
	}
	
	return current;
}

static GtkWidget *
create_goto_menu_item (GeditFileBrowserWidget * obj, GList * item,
		       GdkPixbuf * icon)
{
	GtkWidget *result;
	GtkWidget *image;
	gchar *unescape;
	GdkPixbuf *pixbuf = NULL;
	Location *loc;

	loc = (Location *) (item->data);

	if (!get_from_bookmark_file (obj, loc->virtual_root, &unescape, &pixbuf)) {
		unescape = gedit_file_browser_utils_file_basename (loc->virtual_root);
		
		if (icon)
			pixbuf = g_object_ref (icon);
	}

	if (pixbuf) {
		image = gtk_image_new_from_pixbuf (pixbuf);
		g_object_unref (pixbuf);

		gtk_widget_show (image);

		result = gtk_image_menu_item_new_with_label (unescape);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (result),
					       image);
	} else {
		result = gtk_menu_item_new_with_label (unescape);
	}
	
	g_object_set_data (G_OBJECT (result), LOCATION_DATA_KEY, item);
	g_signal_connect (result, "activate",
			  G_CALLBACK (on_location_jump_activate), obj);

	gtk_widget_show (result);

	g_free (unescape);

	return result;
}

static GList *
list_next_iterator (GList * list)
{
	if (!list)
		return NULL;

	return list->next;
}

static GList *
list_prev_iterator (GList * list)
{
	if (!list)
		return NULL;

	return list->prev;
}

static void
jump_to_location (GeditFileBrowserWidget * obj, GList * item,
		  gboolean previous)
{
	Location *loc;
	GtkWidget *widget;
	GList *children;
	GList *child;
	GList *(*iter_func) (GList *);
	GtkWidget *menu_from;
	GtkWidget *menu_to;
	gchar *root;
	gchar *virtual_root;

	if (!obj->priv->locations)
		return;

	if (previous) {
		iter_func = list_next_iterator;
		menu_from = obj->priv->location_previous_menu;
		menu_to = obj->priv->location_next_menu;
	} else {
		iter_func = list_prev_iterator;
		menu_from = obj->priv->location_next_menu;
		menu_to = obj->priv->location_previous_menu;
	}

	children = gtk_container_get_children (GTK_CONTAINER (menu_from));
	child = children;

	/* This is the menuitem for the current location, which is the first
	   to be added to the menu */
	widget = obj->priv->current_location_menu_item;

	while (obj->priv->current_location != item) {
		if (widget) {
			/* Prepend the menu item to the menu */
			gtk_menu_shell_prepend (GTK_MENU_SHELL (menu_to),
						widget);

			g_object_unref (widget);
		}

		widget = GTK_WIDGET (child->data);

		/* Make sure the widget isn't destroyed when removed */
		g_object_ref (widget);
		gtk_container_remove (GTK_CONTAINER (menu_from), widget);

		obj->priv->current_location_menu_item = widget;

		if (obj->priv->current_location == NULL) {
			obj->priv->current_location = obj->priv->locations;

			if (obj->priv->current_location == item)
				break;
		} else {
			obj->priv->current_location =
			    iter_func (obj->priv->current_location);
		}

		child = child->next;
	}

	g_list_free (children);

	obj->priv->changing_location = TRUE;

	loc = (Location *) (obj->priv->current_location->data);

	/* Set the new root + virtual root */
	root = g_file_get_uri (loc->root);
	virtual_root = g_file_get_uri (loc->virtual_root);
	
	gedit_file_browser_widget_set_root_and_virtual_root (obj,
							     root,
							     virtual_root);

	g_free (root);
	g_free (virtual_root);

	obj->priv->changing_location = FALSE;
}

static void
clear_next_locations (GeditFileBrowserWidget * obj)
{
	GList *children;
	GList *item;

	if (obj->priv->current_location == NULL)
		return;

	while (obj->priv->current_location->prev) {
		location_free ((Location *) (obj->priv->current_location->
					     prev->data));
		obj->priv->locations =
		    g_list_remove_link (obj->priv->locations,
					obj->priv->current_location->prev);
	}

	children =
	    gtk_container_get_children (GTK_CONTAINER
					(obj->priv->location_next_menu));

	for (item = children; item; item = item->next) {
		gtk_container_remove (GTK_CONTAINER
				      (obj->priv->location_next_menu),
				      GTK_WIDGET (item->data));
	}

	g_list_free (children);

	gtk_action_set_sensitive (gtk_action_group_get_action
				  (obj->priv->action_group_sensitive,
				   "DirectoryNext"), FALSE);
}

static void
update_filter_mode (GeditFileBrowserWidget * obj, 
                    GtkAction * action, 
                    GeditFileBrowserStoreFilterMode mode)
{
	gboolean active =
	    gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
	GtkTreeModel *model =
	    gtk_tree_view_get_model (GTK_TREE_VIEW (obj->priv->treeview));
	gint now;

	if (GEDIT_IS_FILE_BROWSER_STORE (model)) {
		now =
		    gedit_file_browser_store_get_filter_mode
		    (GEDIT_FILE_BROWSER_STORE (model));

		if (active)
			now &= ~mode;
		else
			now |= mode;

		gedit_file_browser_store_set_filter_mode
		    (GEDIT_FILE_BROWSER_STORE (model), now);
	}
}

static void
set_filter_pattern_real (GeditFileBrowserWidget * obj,
                        gchar const * pattern,
                        gboolean update_entry)
{
	GtkTreeModel *model;

	model =
	    gtk_tree_view_get_model (GTK_TREE_VIEW (obj->priv->treeview));

	if (pattern != NULL && *pattern == '\0')
		pattern = NULL;

	if (pattern == NULL && obj->priv->filter_pattern_str == NULL)
		return;
	
	if (pattern != NULL && obj->priv->filter_pattern_str != NULL && 
	    strcmp (pattern, obj->priv->filter_pattern_str) == 0)
		return;

	/* Free the old pattern */
	g_free (obj->priv->filter_pattern_str);
	obj->priv->filter_pattern_str = g_strdup (pattern);

	if (obj->priv->filter_pattern) {
		g_pattern_spec_free (obj->priv->filter_pattern);
		obj->priv->filter_pattern = NULL;
	}

	if (pattern == NULL) {
		if (obj->priv->glob_filter_id != 0) {
			gedit_file_browser_widget_remove_filter (obj,
								 obj->
								 priv->
								 glob_filter_id);
			obj->priv->glob_filter_id = 0;
		}
	} else {
		obj->priv->filter_pattern = g_pattern_spec_new (pattern);

		if (obj->priv->glob_filter_id == 0)
			obj->priv->glob_filter_id =
			    gedit_file_browser_widget_add_filter (obj,
								  filter_glob,
								  NULL,
								  NULL);
	}

	if (update_entry) {
		if (obj->priv->filter_pattern_str == NULL)
			gtk_entry_set_text (GTK_ENTRY (obj->priv->filter_entry),
			                    "");
		else {
			gtk_entry_set_text (GTK_ENTRY (obj->priv->filter_entry),
			                    obj->priv->filter_pattern_str);

			gtk_expander_set_expanded (GTK_EXPANDER (obj->priv->filter_expander),
		        	                   TRUE);
		}
	}

	if (GEDIT_IS_FILE_BROWSER_STORE (model))
		gedit_file_browser_store_refilter (GEDIT_FILE_BROWSER_STORE
						   (model));

	g_object_notify (G_OBJECT (obj), "filter-pattern");
}


/* Public */

GtkWidget *
gedit_file_browser_widget_new (const gchar *data_dir)
{
	GeditFileBrowserWidget *obj =
	    g_object_new (GEDIT_TYPE_FILE_BROWSER_WIDGET, NULL);

	create_toolbar (obj, data_dir);
	create_combo (obj);
	create_tree (obj);
	create_filter (obj);

	gedit_file_browser_widget_show_bookmarks (obj);

	return GTK_WIDGET (obj);
}

void
gedit_file_browser_widget_show_bookmarks (GeditFileBrowserWidget * obj)
{
	/* Select bookmarks in the combo box */
	g_signal_handlers_block_by_func (obj->priv->combo,
					 on_combo_changed, obj);
	combo_set_active_by_id (obj, BOOKMARKS_ID);
	g_signal_handlers_unblock_by_func (obj->priv->combo,
					   on_combo_changed, obj);

	check_current_item (obj, FALSE);

	gedit_file_browser_view_set_model (obj->priv->treeview,
					   GTK_TREE_MODEL (obj->priv->
							   bookmarks_store));
}

static void
show_files_real (GeditFileBrowserWidget *obj,
		 gboolean                do_root_changed)
{
	gedit_file_browser_view_set_model (obj->priv->treeview,
					   GTK_TREE_MODEL (obj->priv->
							   file_store));

	if (do_root_changed)
		on_virtual_root_changed (obj->priv->file_store, NULL, obj);
}

void
gedit_file_browser_widget_show_files (GeditFileBrowserWidget * obj)
{
	show_files_real (obj, TRUE);
}

void
gedit_file_browser_widget_set_root_and_virtual_root (GeditFileBrowserWidget *obj,
						     gchar const *root,
						     gchar const *virtual_root)
{
	GeditFileBrowserStoreResult result;

	if (!virtual_root)
		result =
		    gedit_file_browser_store_set_root_and_virtual_root 
		    (obj->priv->file_store, root, root);
	else
		result =
		    gedit_file_browser_store_set_root_and_virtual_root
		    (obj->priv->file_store, root, virtual_root);

	if (result == GEDIT_FILE_BROWSER_STORE_RESULT_NO_CHANGE)
		show_files_real (obj, TRUE);
}

void
gedit_file_browser_widget_set_root (GeditFileBrowserWidget * obj,
				    gchar const *root, 
				    gboolean virtual_root)
{
	GFile *file;
	GFile *parent;
	gchar *str;

	if (!virtual_root) {
		gedit_file_browser_widget_set_root_and_virtual_root (obj,
								     root,
								     NULL);
		return;
	}

	if (!root)
		return;

	file = g_file_new_for_uri (root);
	parent = get_topmost_file (file);
	str = g_file_get_uri (parent);

	gedit_file_browser_widget_set_root_and_virtual_root
	    (obj, str, root);

	g_free (str);
	
	g_object_unref (file);
	g_object_unref (parent);
}

GeditFileBrowserStore *
gedit_file_browser_widget_get_browser_store (GeditFileBrowserWidget * obj)
{
	return obj->priv->file_store;
}

GeditFileBookmarksStore *
gedit_file_browser_widget_get_bookmarks_store (GeditFileBrowserWidget * obj)
{
	return obj->priv->bookmarks_store;
}

GeditFileBrowserView *
gedit_file_browser_widget_get_browser_view (GeditFileBrowserWidget * obj)
{
	return obj->priv->treeview;
}

GtkUIManager *
gedit_file_browser_widget_get_ui_manager (GeditFileBrowserWidget * obj)
{
	return obj->priv->manager;
}

GtkWidget *
gedit_file_browser_widget_get_filter_entry (GeditFileBrowserWidget * obj)
{
	return obj->priv->filter_entry;
}

gulong
gedit_file_browser_widget_add_filter (GeditFileBrowserWidget * obj,
				      GeditFileBrowserWidgetFilterFunc func, 
				      gpointer user_data,
				      GDestroyNotify notify)
{
	FilterFunc *f;
	GtkTreeModel *model =
	    gtk_tree_view_get_model (GTK_TREE_VIEW (obj->priv->treeview));

	f = filter_func_new (obj, func, user_data, notify);
	obj->priv->filter_funcs =
	    g_slist_append (obj->priv->filter_funcs, f);

	if (GEDIT_IS_FILE_BROWSER_STORE (model))
		gedit_file_browser_store_refilter (GEDIT_FILE_BROWSER_STORE
						   (model));

	return f->id;
}

void
gedit_file_browser_widget_remove_filter (GeditFileBrowserWidget * obj,
					 gulong id)
{
	GSList *item;
	FilterFunc *func;

	for (item = obj->priv->filter_funcs; item; item = item->next)
	{
		func = (FilterFunc *) (item->data);

		if (func->id == id)
		{
			if (func->destroy_notify)
				func->destroy_notify (func->user_data);

			obj->priv->filter_funcs =
			    g_slist_remove_link (obj->priv->filter_funcs,
						 item);
			g_free (func);
			break;
		}
	}
}

void 
gedit_file_browser_widget_set_filter_pattern (GeditFileBrowserWidget * obj,
                                              gchar const *pattern)
{
	set_filter_pattern_real (obj, pattern, TRUE);
}

gboolean
gedit_file_browser_widget_get_selected_directory (GeditFileBrowserWidget * obj, 
						  GtkTreeIter * iter)
{
	GtkTreeModel *model =
	    gtk_tree_view_get_model (GTK_TREE_VIEW (obj->priv->treeview));
	GtkTreeIter parent;
	guint flags;

	if (!GEDIT_IS_FILE_BROWSER_STORE (model))
		return FALSE;

	if (!gedit_file_browser_widget_get_first_selected (obj, iter)) {
		if (!gedit_file_browser_store_get_iter_virtual_root
		    (GEDIT_FILE_BROWSER_STORE (model), iter))
			return FALSE;
	}

	gtk_tree_model_get (model, iter,
			    GEDIT_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags,
			    -1);

	if (!FILE_IS_DIR (flags)) {
		/* Get the parent, because the selection is a file */
		gtk_tree_model_iter_parent (model, &parent, iter);
		*iter = parent;
	}

	return TRUE;
}

static guint
gedit_file_browser_widget_get_num_selected_files_or_directories (GeditFileBrowserWidget *obj,
								 guint                  *files,
								 guint                  *dirs)
{
	GList *rows, *row;
	GtkTreePath *path;
	GtkTreeIter iter;
	GeditFileBrowserStoreFlag flags;
	guint result = 0;
	GtkTreeSelection *selection;
	GtkTreeModel *model;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (obj->priv->treeview));
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (obj->priv->treeview));

	if (GEDIT_IS_FILE_BOOKMARKS_STORE (model))
		return 0;
	
	rows = gtk_tree_selection_get_selected_rows (selection, &model);
	
	for (row = rows; row; row = row->next) {
		path = (GtkTreePath *)(row->data);
		
		/* Get iter from path */
		if (!gtk_tree_model_get_iter (model, &iter, path))
			continue;

		gtk_tree_model_get (model, &iter,
				    GEDIT_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags,
				    -1);

		if (!FILE_IS_DUMMY (flags)) {
			if (!FILE_IS_DIR (flags))
				++(*files);
			else
				++(*dirs);

			++result;
		}
	}

	g_list_foreach (rows, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (rows);
	
	return result;
}

typedef struct
{
	GeditFileBrowserWidget *widget;
	GCancellable *cancellable;
} AsyncData;

static AsyncData *
async_data_new (GeditFileBrowserWidget *widget)
{
	AsyncData *ret;
	
	ret = g_new (AsyncData, 1);
	ret->widget = widget;
	
	cancel_async_operation (widget);
	widget->priv->cancellable = g_cancellable_new ();
	
	ret->cancellable = g_object_ref (widget->priv->cancellable);
	
	return ret;
}

static void
async_free (AsyncData *async)
{
	g_object_unref (async->cancellable);
	g_free (async);
}

static void
set_busy (GeditFileBrowserWidget *obj, gboolean busy)
{
	GdkCursor *cursor;
	GdkWindow *window;
	
	window = gtk_widget_get_window (GTK_WIDGET (obj->priv->treeview));
	
	if (!GDK_IS_WINDOW (window))
		return;

	if (busy)
	{
		cursor = gdk_cursor_new (GDK_WATCH);
		gdk_window_set_cursor (window, cursor);
		gdk_cursor_unref (cursor);
	}
	else
	{
		gdk_window_set_cursor (window, NULL);
	}
}

static void try_mount_volume (GeditFileBrowserWidget *widget, GVolume *volume);

static void
activate_mount (GeditFileBrowserWidget *widget,
		GVolume		       *volume,
		GMount		       *mount)
{
	GFile *root;
	gchar *uri;
	
	if (!mount)
	{
		gchar *message;
		gchar *name;
		
		name = g_volume_get_name (volume);
		message = g_strdup_printf (_("No mount object for mounted volume: %s"), name);
		
		g_signal_emit (widget, 
			       signals[ERROR], 
			       0, 
			       GEDIT_FILE_BROWSER_ERROR_SET_ROOT,
			       message);
		
		g_free (name);
		g_free (message);
		return;
	}
	
	root = g_mount_get_root (mount);
	uri = g_file_get_uri (root);
	
	gedit_file_browser_widget_set_root (widget, uri, FALSE);
	
	g_free (uri);
	g_object_unref (root);
}

static void
try_activate_drive (GeditFileBrowserWidget *widget,
		    GDrive 		   *drive)
{
	GList *volumes;
	GVolume *volume;
	GMount *mount;
	
	volumes = g_drive_get_volumes (drive);
	
	volume = G_VOLUME (volumes->data);
	mount = g_volume_get_mount (volume);
	
	if (mount)
	{
		/* try set the root of the mount */
		activate_mount (widget, volume, mount);
		g_object_unref (mount);
	}
	else
	{
		/* try to mount it then? */
		try_mount_volume (widget, volume);
	}
	
	g_list_foreach (volumes, (GFunc)g_object_unref, NULL);
	g_list_free (volumes);
}

static void
poll_for_media_cb (GDrive       *drive,
		   GAsyncResult *res,
		   AsyncData 	*async)
{
	GError *error = NULL;

	/* check for cancelled state */
	if (g_cancellable_is_cancelled (async->cancellable))
	{
		async_free (async);
		return;
	}
	
	/* finish poll operation */
	set_busy (async->widget, FALSE);
	
	if (g_drive_poll_for_media_finish (drive, res, &error) && 
	    g_drive_has_media (drive) &&
	    g_drive_has_volumes (drive))
	{
		try_activate_drive (async->widget, drive);
	}
	else
	{
		gchar *message;
		gchar *name;
		
		name = g_drive_get_name (drive);
		message = g_strdup_printf (_("Could not open media: %s"), name);

		g_signal_emit (async->widget, 
			       signals[ERROR], 
			       0, 
			       GEDIT_FILE_BROWSER_ERROR_SET_ROOT,
			       message);

		g_free (name);
		g_free (message);
		
		g_error_free (error);
	}
	
	async_free (async);
}

static void
mount_volume_cb (GVolume      *volume,
		 GAsyncResult *res,
		 AsyncData    *async)
{
	GError *error = NULL;

	/* check for cancelled state */
	if (g_cancellable_is_cancelled (async->cancellable))
	{
		async_free (async);
		return;
	}
	
	if (g_volume_mount_finish (volume, res, &error))
	{
		GMount *mount;
		
		mount = g_volume_get_mount (volume);
		activate_mount (async->widget, volume, mount);
		
		if (mount)
			g_object_unref (mount);
	}
	else
	{
		gchar *message;
		gchar *name;
		
		name = g_volume_get_name (volume);
		message = g_strdup_printf (_("Could not mount volume: %s"), name);

		g_signal_emit (async->widget, 
			       signals[ERROR], 
			       0, 
			       GEDIT_FILE_BROWSER_ERROR_SET_ROOT,
			       message);

		g_free (name);
		g_free (message);
		
		g_error_free (error);
	}
	
	set_busy (async->widget, FALSE);
	async_free (async);
}

static void
activate_drive (GeditFileBrowserWidget *obj,
		GtkTreeIter	       *iter)
{
	GDrive *drive;
	AsyncData *async;

	gtk_tree_model_get (GTK_TREE_MODEL (obj->priv->bookmarks_store), iter,
			    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_OBJECT,
			    &drive, -1);

	/* most common use case is a floppy drive, we'll poll for media and
	   go from there */
	async = async_data_new (obj);
	g_drive_poll_for_media (drive, 
				async->cancellable, 
				(GAsyncReadyCallback)poll_for_media_cb,
				async);

	g_object_unref (drive);
	set_busy (obj, TRUE);
}

static void 
try_mount_volume (GeditFileBrowserWidget *widget, 
		  GVolume 		 *volume)
{
	GMountOperation *operation;
	AsyncData *async;

	operation = gtk_mount_operation_new (GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (widget))));
	async = async_data_new (widget);
	
	g_volume_mount (volume,
			G_MOUNT_MOUNT_NONE,
			operation,
			async->cancellable,
			(GAsyncReadyCallback)mount_volume_cb,
			async);
			
	g_object_unref (operation);
	set_busy (widget, TRUE);
}

static void
activate_volume (GeditFileBrowserWidget *obj,
		 GtkTreeIter	        *iter)
{
	GVolume *volume;
	
	gtk_tree_model_get (GTK_TREE_MODEL (obj->priv->bookmarks_store), iter,
			    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_OBJECT,
			    &volume, -1);

	/* see if we can mount the volume */
	try_mount_volume (obj, volume);	
	g_object_unref (volume);
}

void 
gedit_file_browser_widget_refresh (GeditFileBrowserWidget *obj)
{
	GtkTreeModel *model =
	    gtk_tree_view_get_model (GTK_TREE_VIEW (obj->priv->treeview));

	if (GEDIT_IS_FILE_BROWSER_STORE (model))
		gedit_file_browser_store_refresh (GEDIT_FILE_BROWSER_STORE
						  (model));
	else if (GEDIT_IS_FILE_BOOKMARKS_STORE (model)) {
		g_hash_table_ref (obj->priv->bookmarks_hash);
		g_hash_table_destroy (obj->priv->bookmarks_hash);

		gedit_file_bookmarks_store_refresh
		    (GEDIT_FILE_BOOKMARKS_STORE (model));
	}
}

void 
gedit_file_browser_widget_history_back (GeditFileBrowserWidget *obj)
{
	if (obj->priv->locations) {
		if (obj->priv->current_location)
			jump_to_location (obj,
					  obj->priv->current_location->
					  next, TRUE);
		else {
			jump_to_location (obj, obj->priv->locations, TRUE);
		}
	}
}

void
gedit_file_browser_widget_history_forward (GeditFileBrowserWidget *obj)
{
	if (obj->priv->locations)
		jump_to_location (obj, obj->priv->current_location->prev,
				  FALSE);
}

static void
bookmark_open (GeditFileBrowserWidget *obj,
	       GtkTreeModel           *model,
	       GtkTreeIter            *iter)
{
	gchar *uri;
	gint flags;

	gtk_tree_model_get (model, iter,
			    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_FLAGS,
			    &flags, -1);
	
	if (flags & GEDIT_FILE_BOOKMARKS_STORE_IS_DRIVE)
	{
		/* handle a drive node */
		gedit_file_browser_store_cancel_mount_operation (obj->priv->file_store);
		activate_drive (obj, iter);
		return;
	}
	else if (flags & GEDIT_FILE_BOOKMARKS_STORE_IS_VOLUME)
	{
		/* handle a volume node */
		gedit_file_browser_store_cancel_mount_operation (obj->priv->file_store);
		activate_volume (obj, iter);
		return;
	}

	uri =
	    gedit_file_bookmarks_store_get_uri
	    (GEDIT_FILE_BOOKMARKS_STORE (model), iter);

	if (uri) {
		/* here we check if the bookmark is a mount point, or if it
		   is a remote bookmark. If that's the case, we will set the
		   root to the uri of the bookmark and not try to set the
		   topmost parent as root (since that may as well not be the
		   mount point anymore) */
		if ((flags & GEDIT_FILE_BOOKMARKS_STORE_IS_MOUNT) ||
		    (flags & GEDIT_FILE_BOOKMARKS_STORE_IS_REMOTE_BOOKMARK)) {
			gedit_file_browser_widget_set_root (obj,
							    uri,
							    FALSE);
		} else {
			gedit_file_browser_widget_set_root (obj,
							    uri,
							    TRUE);
		}
	} else {
		g_warning ("No uri!");
	}

	g_free (uri);
}

static void
file_open  (GeditFileBrowserWidget *obj,
	    GtkTreeModel           *model,
	    GtkTreeIter            *iter)
{
	gchar *uri;
	gint flags;

	gtk_tree_model_get (model, iter,
			    GEDIT_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags,
			    GEDIT_FILE_BROWSER_STORE_COLUMN_URI, &uri,
			    -1);

	if (!FILE_IS_DIR (flags) && !FILE_IS_DUMMY (flags)) {
		g_signal_emit (obj, signals[URI_ACTIVATED], 0, uri);
	}

	g_free (uri);
}

static gboolean
directory_open (GeditFileBrowserWidget *obj,
		GtkTreeModel           *model,
		GtkTreeIter            *iter)
{
	gboolean result = FALSE;
	GError *error = NULL;
	gchar *uri = NULL;
	GeditFileBrowserStoreFlag flags;

	gtk_tree_model_get (model, iter,
			    GEDIT_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags,
			    GEDIT_FILE_BROWSER_STORE_COLUMN_URI, &uri,
			    -1);

	if (FILE_IS_DIR (flags)) {
		result = TRUE;

		if (!gtk_show_uri (gtk_widget_get_screen (GTK_WIDGET (obj)), uri, GDK_CURRENT_TIME, &error)) {
			g_signal_emit (obj, signals[ERROR], 0,
				       GEDIT_FILE_BROWSER_ERROR_OPEN_DIRECTORY,
				       error->message);

			g_error_free (error);
			error = NULL;
		}
	}

	g_free (uri);

	return result;
}

static void
on_bookmark_activated (GeditFileBrowserView   *tree_view,
		       GtkTreeIter            *iter,
		       GeditFileBrowserWidget *obj)
{
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

	bookmark_open (obj, model, iter);
}

static void
on_file_activated (GeditFileBrowserView   *tree_view, 
		   GtkTreeIter            *iter,
		   GeditFileBrowserWidget *obj)
{
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

	file_open (obj, model, iter);
}

static gboolean
virtual_root_is_root (GeditFileBrowserWidget * obj, 
                      GeditFileBrowserStore  * model)
{
	GtkTreeIter root;
	GtkTreeIter virtual_root;
	
	if (!gedit_file_browser_store_get_iter_root (model, &root))
		return TRUE;
	
	if (!gedit_file_browser_store_get_iter_virtual_root (model, &virtual_root))
		return TRUE;
	
	return gedit_file_browser_store_iter_equal (model, &root, &virtual_root);
}

static void
on_virtual_root_changed (GeditFileBrowserStore * model,
			 GParamSpec * param,
			 GeditFileBrowserWidget * obj)
{
	GtkTreeIter iter;
	gchar *uri;
	gchar *root_uri;
	GtkTreeIter root;
	GtkAction *action;
	Location *loc;
	GdkPixbuf *pixbuf;

	if (gtk_tree_view_get_model (GTK_TREE_VIEW (obj->priv->treeview)) !=
	    GTK_TREE_MODEL (obj->priv->file_store))
	{
		show_files_real (obj, FALSE);
	}

	if (gedit_file_browser_store_get_iter_virtual_root (model, &iter)) {
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
				    GEDIT_FILE_BROWSER_STORE_COLUMN_URI,
				    &uri, -1);

		if (gedit_file_browser_store_get_iter_root (model, &root)) {
			if (!obj->priv->changing_location) {
				/* Remove all items from obj->priv->current_location on */
				if (obj->priv->current_location)
					clear_next_locations (obj);

				root_uri =
				    gedit_file_browser_store_get_root
				    (model);

				loc = g_new (Location, 1);
				loc->root = g_file_new_for_uri (root_uri);
				loc->virtual_root = g_file_new_for_uri (uri);
				g_free (root_uri);

				if (obj->priv->current_location) {
					/* Add current location to the menu so we can go back
					   to it later */
					gtk_menu_shell_prepend
					    (GTK_MENU_SHELL
					     (obj->priv->
					      location_previous_menu),
					     obj->priv->
					     current_location_menu_item);
				}

				obj->priv->locations =
				    g_list_prepend (obj->priv->locations,
						    loc);

				gtk_tree_model_get (GTK_TREE_MODEL (model),
						    &iter,
						    GEDIT_FILE_BROWSER_STORE_COLUMN_ICON,
						    &pixbuf, -1);

				obj->priv->current_location =
				    obj->priv->locations;
				obj->priv->current_location_menu_item =
				    create_goto_menu_item (obj,
							   obj->priv->
							   current_location,
							   pixbuf);

				g_object_ref_sink (obj->priv->
						   current_location_menu_item);

				if (pixbuf)
					g_object_unref (pixbuf);

			}
			
			action =
			    gtk_action_group_get_action (obj->priv->
			                                 action_group,
			                                 "DirectoryUp");
			gtk_action_set_sensitive (action, 
			                          !virtual_root_is_root (obj, model));

			action =
			    gtk_action_group_get_action (obj->priv->
							 action_group_sensitive,
							 "DirectoryPrevious");
			gtk_action_set_sensitive (action,
						  obj->priv->
						  current_location != NULL
						  && obj->priv->
						  current_location->next !=
						  NULL);

			action =
			    gtk_action_group_get_action (obj->priv->
							 action_group_sensitive,
							 "DirectoryNext");
			gtk_action_set_sensitive (action,
						  obj->priv->
						  current_location != NULL
						  && obj->priv->
						  current_location->prev !=
						  NULL);
		}

		check_current_item (obj, TRUE);
		g_free (uri);
	} else {
		g_message ("NO!");
	}
}

static void
on_model_set (GObject * gobject, GParamSpec * arg1,
	      GeditFileBrowserWidget * obj)
{
	GtkTreeModel *model;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (gobject));

	clear_signals (obj);

	if (GEDIT_IS_FILE_BOOKMARKS_STORE (model)) {
		clear_next_locations (obj);

		/* Add the current location to the back menu */
		if (obj->priv->current_location) {
			GtkAction *action;

			gtk_menu_shell_prepend (GTK_MENU_SHELL (obj->priv->location_previous_menu),
						obj->priv->current_location_menu_item);

			g_object_unref (obj->priv->current_location_menu_item);
			obj->priv->current_location = NULL;
			obj->priv->current_location_menu_item = NULL;

			action = gtk_action_group_get_action (obj->priv->action_group_sensitive,
							      "DirectoryPrevious");
			gtk_action_set_sensitive (action, TRUE);
		}

		gtk_widget_set_sensitive (obj->priv->filter_expander, FALSE);

		add_signal (obj, gobject,
			    g_signal_connect (gobject, "bookmark-activated",
					      G_CALLBACK
					      (on_bookmark_activated), obj));
	} else if (GEDIT_IS_FILE_BROWSER_STORE (model)) {
		/* make sure any async operation is cancelled */
		cancel_async_operation (obj);

		add_signal (obj, gobject,
			    g_signal_connect (gobject, "file-activated",
					      G_CALLBACK
					      (on_file_activated), obj));

		add_signal (obj, model,
			    g_signal_connect (model, "no-trash",
			    		      G_CALLBACK
			    		      (on_file_store_no_trash), obj));

		gtk_widget_set_sensitive (obj->priv->filter_expander, TRUE);
	}

	update_sensitivity (obj);
}

static void
on_file_store_error (GeditFileBrowserStore * store, guint code,
		     gchar * message, GeditFileBrowserWidget * obj)
{
	g_signal_emit (obj, signals[ERROR], 0, code, message);
}

static void
on_treeview_error (GeditFileBrowserView * tree_view, guint code,
		   gchar * message, GeditFileBrowserWidget * obj)
{
	g_signal_emit (obj, signals[ERROR], 0, code, message);
}

static void
on_combo_changed (GtkComboBox * combo, GeditFileBrowserWidget * obj)
{
	GtkTreeIter iter;
	guint id;
	gchar * uri;
	GFile * file;

	if (!gtk_combo_box_get_active_iter (combo, &iter))
		return;

	gtk_tree_model_get (GTK_TREE_MODEL (obj->priv->combo_model), &iter,
			    COLUMN_ID, &id, -1);

	switch (id) {
	case BOOKMARKS_ID:
		gedit_file_browser_widget_show_bookmarks (obj);
		break;

	case PATH_ID:
		gtk_tree_model_get (GTK_TREE_MODEL
				    (obj->priv->combo_model), &iter,
				    COLUMN_FILE, &file, -1);
		
		uri = g_file_get_uri (file);
		gedit_file_browser_store_set_virtual_root_from_string
		    (obj->priv->file_store, uri);
		
		g_free (uri);
		g_object_unref (file);
		break;
	}
}

static gboolean
on_treeview_popup_menu (GeditFileBrowserView * treeview,
			GeditFileBrowserWidget * obj)
{
	return popup_menu (obj, NULL, gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));
}

static gboolean
on_treeview_button_press_event (GeditFileBrowserView * treeview,
				GdkEventButton * event,
				GeditFileBrowserWidget * obj)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
		return popup_menu (obj, event,
				   gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));
	}

	return FALSE;
}

static gboolean
do_change_directory (GeditFileBrowserWidget * obj,
                     GdkEventKey            * event)
{
	GtkAction * action = NULL;

	if ((event->state & 
	    (~GDK_CONTROL_MASK & ~GDK_SHIFT_MASK & ~GDK_MOD1_MASK)) == 
	     event->state && event->keyval == GDK_BackSpace)
		action = gtk_action_group_get_action (obj->priv->
		                                      action_group_sensitive,
		                                      "DirectoryPrevious");
	else if (!((event->state & GDK_MOD1_MASK) && 
	    (event->state & (~GDK_CONTROL_MASK & ~GDK_SHIFT_MASK)) == event->state))
		return FALSE;

	switch (event->keyval) {
		case GDK_Left:
			action = gtk_action_group_get_action (obj->priv->
			                                      action_group_sensitive,
			                                      "DirectoryPrevious");
		break;
		case GDK_Right:
			action = gtk_action_group_get_action (obj->priv->
			                                      action_group_sensitive,
			                                      "DirectoryNext");
		break;
		case GDK_Up:
			action = gtk_action_group_get_action (obj->priv->
			                                      action_group,
			                                      "DirectoryUp");
		break;
		default:
		break;
	}
	
	if (action != NULL) {
		gtk_action_activate (action);
		return TRUE;
	}
	
	return FALSE;
}

static gboolean
on_treeview_key_press_event (GeditFileBrowserView * treeview,
			     GdkEventKey * event,
			     GeditFileBrowserWidget * obj)
{
	guint modifiers;

	if (do_change_directory (obj, event))
		return TRUE;

	if (!GEDIT_IS_FILE_BROWSER_STORE
	    (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview))))
		return FALSE;

	modifiers = gtk_accelerator_get_default_mod_mask ();

	if (event->keyval == GDK_Delete
	    || event->keyval == GDK_KP_Delete) {

		if ((event->state & modifiers) == GDK_SHIFT_MASK) {
			if (obj->priv->enable_delete) {
				delete_selected_files (obj, FALSE);
				return TRUE;
			}
		} else if ((event->state & modifiers) == 0) {
			delete_selected_files (obj, TRUE);
			return TRUE;
		}
	}

	if ((event->keyval == GDK_F2)
	    && (event->state & modifiers) == 0) {
		rename_selected_file (obj);

		return TRUE;
	}

	return FALSE;
}

static void
on_selection_changed (GtkTreeSelection * selection,
		      GeditFileBrowserWidget * obj)
{
	GtkTreeModel *model;
	guint selected = 0;
	guint files = 0;
	guint dirs = 0;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (obj->priv->treeview));

	if (GEDIT_IS_FILE_BROWSER_STORE (model))
	{
		selected = gedit_file_browser_widget_get_num_selected_files_or_directories (obj,
											    &files,
											    &dirs);
	}

	gtk_action_group_set_sensitive (obj->priv->action_group_selection,
					selected > 0);
	gtk_action_group_set_sensitive (obj->priv->action_group_file_selection,
					(selected > 0) && (selected == files));
	gtk_action_group_set_sensitive (obj->priv->action_group_single_selection,
					selected == 1);
	gtk_action_group_set_sensitive (obj->priv->action_group_single_most_selection,
					selected <= 1);
}

static gboolean
on_entry_filter_activate (GeditFileBrowserWidget * obj)
{
	gchar const *text;

	text = gtk_entry_get_text (GTK_ENTRY (obj->priv->filter_entry));
	set_filter_pattern_real (obj, text, FALSE);

	return FALSE;
}

static void
on_location_jump_activate (GtkMenuItem * item,
			   GeditFileBrowserWidget * obj)
{
	GList *location;

	location = g_object_get_data (G_OBJECT (item), LOCATION_DATA_KEY);

	if (obj->priv->current_location) {
		jump_to_location (obj, location,
				  g_list_position (obj->priv->locations,
						   location) >
				  g_list_position (obj->priv->locations,
						   obj->priv->
						   current_location));
	} else {
		jump_to_location (obj, location, TRUE);
	}

}

static void
on_bookmarks_row_changed (GtkTreeModel * model, 
                          GtkTreePath * path,
                          GtkTreeIter * iter,
                          GeditFileBrowserWidget *obj)
{
	add_bookmark_hash (obj, iter);
}

static void
on_bookmarks_row_deleted (GtkTreeModel * model, 
                          GtkTreePath * path,
                          GeditFileBrowserWidget *obj)
{
	GtkTreeIter iter;
	gchar * uri;
	GFile * file;
	
	if (!gtk_tree_model_get_iter (model, &iter, path))
		return;

	uri = gedit_file_bookmarks_store_get_uri (obj->priv->bookmarks_store, &iter);
	
	if (!uri)
		return;

	file = g_file_new_for_uri (uri);
	g_hash_table_remove (obj->priv->bookmarks_hash, file);

	g_object_unref (file);
	g_free (uri);
}

static void 
on_filter_mode_changed (GeditFileBrowserStore * model,
                        GParamSpec * param,
                        GeditFileBrowserWidget * obj)
{
	gint mode;
	GtkToggleAction * action;
	gboolean active;

	mode = gedit_file_browser_store_get_filter_mode (model);
	
	action = GTK_TOGGLE_ACTION (gtk_action_group_get_action (obj->priv->action_group, 
	                                                         "FilterHidden"));
	active = !(mode & GEDIT_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN);
	
	if (active != gtk_toggle_action_get_active (action))
		gtk_toggle_action_set_active (action, active);

	action = GTK_TOGGLE_ACTION (gtk_action_group_get_action (obj->priv->action_group, 
	                                                         "FilterBinary"));
	active = !(mode & GEDIT_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY);
	
	if (active != gtk_toggle_action_get_active (action))
		gtk_toggle_action_set_active (action, active);	
}

static void
on_action_directory_next (GtkAction * action, GeditFileBrowserWidget * obj)
{
	gedit_file_browser_widget_history_forward (obj);
}

static void
on_action_directory_previous (GtkAction * action,
			      GeditFileBrowserWidget * obj)
{
	gedit_file_browser_widget_history_back (obj);
}

static void 
on_action_directory_up (GtkAction              * action,
			GeditFileBrowserWidget * obj)
{
	GtkTreeModel *model;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (obj->priv->treeview));

	if (!GEDIT_IS_FILE_BROWSER_STORE (model))
		return;
	
	gedit_file_browser_store_set_virtual_root_up (GEDIT_FILE_BROWSER_STORE (model));	
}

static void
on_action_directory_new (GtkAction * action, GeditFileBrowserWidget * obj)
{
	GtkTreeModel *model;
	GtkTreeIter parent;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (obj->priv->treeview));

	if (!GEDIT_IS_FILE_BROWSER_STORE (model))
		return;

	if (!gedit_file_browser_widget_get_selected_directory (obj, &parent))
		return;

	if (gedit_file_browser_store_new_directory
	    (GEDIT_FILE_BROWSER_STORE (model), &parent, &iter)) {
		gedit_file_browser_view_start_rename (obj->priv->treeview,
						      &iter);
	}
}

static void
on_action_file_open (GtkAction * action, GeditFileBrowserWidget * obj)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GList *rows;
	GList *row;
	GtkTreeIter iter;
	GtkTreePath *path;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (obj->priv->treeview));
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (obj->priv->treeview));	

	if (!GEDIT_IS_FILE_BROWSER_STORE (model))
		return;

	rows = gtk_tree_selection_get_selected_rows (selection, &model);

	for (row = rows; row; row = row->next) {
		path = (GtkTreePath *)(row->data);

		if (gtk_tree_model_get_iter (model, &iter, path))
			file_open (obj, model, &iter);

		gtk_tree_path_free (path);
	}

	g_list_free (rows);
}

static void
on_action_file_new (GtkAction * action, GeditFileBrowserWidget * obj)
{
	GtkTreeModel *model;
	GtkTreeIter parent;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (obj->priv->treeview));

	if (!GEDIT_IS_FILE_BROWSER_STORE (model))
		return;

	if (!gedit_file_browser_widget_get_selected_directory (obj, &parent))
		return;

	if (gedit_file_browser_store_new_file
	    (GEDIT_FILE_BROWSER_STORE (model), &parent, &iter)) {
		gedit_file_browser_view_start_rename (obj->priv->treeview,
						      &iter);
	}
}

static void
on_action_file_rename (GtkAction * action, GeditFileBrowserWidget * obj)
{
	rename_selected_file (obj);
}

static void
on_action_file_delete (GtkAction * action, GeditFileBrowserWidget * obj)
{
	delete_selected_files (obj, FALSE);
}

static void
on_action_file_move_to_trash (GtkAction * action, GeditFileBrowserWidget * obj)
{
	delete_selected_files (obj, TRUE);
}

static void
on_action_directory_refresh (GtkAction * action,
			     GeditFileBrowserWidget * obj)
{
	gedit_file_browser_widget_refresh (obj);
}

static void
on_action_directory_open (GtkAction * action, GeditFileBrowserWidget * obj)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GList *rows;
	GList *row;
	gboolean directory_opened = FALSE;
	GtkTreeIter iter;
	GtkTreePath *path;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (obj->priv->treeview));
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (obj->priv->treeview));	

	if (!GEDIT_IS_FILE_BROWSER_STORE (model))
		return;

	rows = gtk_tree_selection_get_selected_rows (selection, &model);
	
	for (row = rows; row; row = row->next) {
		path = (GtkTreePath *)(row->data);

		if (gtk_tree_model_get_iter (model, &iter, path))
			directory_opened |= directory_open (obj, model, &iter);
		
		gtk_tree_path_free (path);
	}
	
	if (!directory_opened) {
		if (gedit_file_browser_widget_get_selected_directory (obj, &iter))
			directory_open (obj, model, &iter);
	}

	g_list_free (rows);
}

static void
on_action_filter_hidden (GtkAction * action, GeditFileBrowserWidget * obj)
{
	update_filter_mode (obj, 
	                    action, 
	                    GEDIT_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN);
}

static void
on_action_filter_binary (GtkAction * action, GeditFileBrowserWidget * obj)
{
	update_filter_mode (obj, 
	                    action, 
	                    GEDIT_FILE_BROWSER_STORE_FILTER_MODE_HIDE_BINARY);
}

static void
on_action_bookmark_open (GtkAction * action, GeditFileBrowserWidget * obj)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (obj->priv->treeview));
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (obj->priv->treeview));

	if (!GEDIT_IS_FILE_BOOKMARKS_STORE (model))
		return;

	if (gtk_tree_selection_get_selected (selection, NULL, &iter))
		bookmark_open (obj, model, &iter);
}

// ex:ts=8:noet:
