/*
 * gedit-taglist-plugin-panel.c
 * This file is part of gedit
 *
 * Copyright (C) 2005 - Paolo Maggi
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
 * Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the gedit Team, 2005. See the AUTHORS file for a
 * list of people on the gedit Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "gedit-taglist-plugin-panel.h"
#include "gedit-taglist-plugin-parser.h"

#include <gedit/gedit-utils.h>
#include <gedit/gedit-debug.h>
#include <gedit/gedit-plugin.h>

#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>

#define GEDIT_TAGLIST_PLUGIN_PANEL_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
						       GEDIT_TYPE_TAGLIST_PLUGIN_PANEL, \
						       GeditTaglistPluginPanelPrivate))

enum
{
	COLUMN_TAG_NAME,
	COLUMN_TAG_INDEX_IN_GROUP,
	NUM_COLUMNS
};

struct _GeditTaglistPluginPanelPrivate
{
	GeditWindow  *window;

	GtkWidget *tag_groups_combo;
	GtkWidget *tags_list;
	GtkWidget *preview;

	TagGroup *selected_tag_group;
	
	gchar *data_dir;
};

GEDIT_PLUGIN_DEFINE_TYPE (GeditTaglistPluginPanel, gedit_taglist_plugin_panel, GTK_TYPE_VBOX)

enum
{
	PROP_0,
	PROP_WINDOW,
};

static void
set_window (GeditTaglistPluginPanel *panel,
	    GeditWindow             *window)
{
	g_return_if_fail (panel->priv->window == NULL);
	g_return_if_fail (GEDIT_IS_WINDOW (window));

	panel->priv->window = window;

	/* TODO */
}

static void
gedit_taglist_plugin_panel_set_property (GObject      *object,
					 guint         prop_id,
					 const GValue *value,
					 GParamSpec   *pspec)
{
	GeditTaglistPluginPanel *panel = GEDIT_TAGLIST_PLUGIN_PANEL (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			set_window (panel, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gedit_taglist_plugin_panel_get_property (GObject    *object,
					 guint       prop_id,
					 GValue     *value,
					 GParamSpec *pspec)
{
	GeditTaglistPluginPanel *panel = GEDIT_TAGLIST_PLUGIN_PANEL (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			g_value_set_object (value,
					    GEDIT_TAGLIST_PLUGIN_PANEL_GET_PRIVATE (panel)->window);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gedit_taglist_plugin_panel_finalize (GObject *object)
{
	GeditTaglistPluginPanel *panel = GEDIT_TAGLIST_PLUGIN_PANEL (object);
	
	g_free (panel->priv->data_dir);

	G_OBJECT_CLASS (gedit_taglist_plugin_panel_parent_class)->finalize (object);
}

static void
gedit_taglist_plugin_panel_class_init (GeditTaglistPluginPanelClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gedit_taglist_plugin_panel_finalize;
	object_class->get_property = gedit_taglist_plugin_panel_get_property;
	object_class->set_property = gedit_taglist_plugin_panel_set_property;

	g_object_class_install_property (object_class,
					 PROP_WINDOW,
					 g_param_spec_object ("window",
							 "Window",
							 "The GeditWindow this GeditTaglistPluginPanel is associated with",
							 GEDIT_TYPE_WINDOW,
							 G_PARAM_READWRITE |
							 G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private (object_class, sizeof(GeditTaglistPluginPanelPrivate));
}

static void
insert_tag (GeditTaglistPluginPanel *panel,
	    Tag                     *tag,
	    gboolean                 grab_focus)
{
	GeditView *view;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;
	GtkTextIter cursor;
	gboolean sel = FALSE;

	gedit_debug (DEBUG_PLUGINS);

	view = gedit_window_get_active_view (panel->priv->window);
	g_return_if_fail (view != NULL);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	gtk_text_buffer_begin_user_action (buffer);

	/* always insert the begin tag at the beginning of the selection
	 * and the end tag at the end, if there is no selection they will
	 * be automatically inserted at the cursor position.
	 */

	if (tag->begin != NULL)
	{
		sel = gtk_text_buffer_get_selection_bounds (buffer,
						 	    &start,
						 	    &end);

		gtk_text_buffer_insert (buffer,
					&start,
					(gchar *)tag->begin,
					-1);

		/* get iterators again since they have been invalidated and move
		 * the cursor after the selection */
		gtk_text_buffer_get_selection_bounds (buffer,
						      &start,
						      &cursor);
	}

	if (tag->end != NULL)
	{
		sel = gtk_text_buffer_get_selection_bounds (buffer,
							    &start,
							    &end);

		gtk_text_buffer_insert (buffer,
					&end,
					(gchar *)tag->end,
					-1);

		/* if there is no selection and we have a paired tag, move the
		 * cursor between the pair, otherwise move it at the end */
		if (!sel)
		{
			gint offset;

			offset = gtk_text_iter_get_offset (&end) -
				 g_utf8_strlen ((gchar *)tag->end, -1);

			gtk_text_buffer_get_iter_at_offset (buffer,
							    &end,
							    offset);
		}

		cursor = end;
	}

	gtk_text_buffer_place_cursor (buffer, &cursor);

	gtk_text_buffer_end_user_action (buffer);

	if (grab_focus)
		gtk_widget_grab_focus (GTK_WIDGET (view));
}

static void
tag_list_row_activated_cb (GtkTreeView             *tag_list,
			   GtkTreePath             *path,
			   GtkTreeViewColumn       *column,
			   GeditTaglistPluginPanel *panel)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	gint index;

	gedit_debug (DEBUG_PLUGINS);

	model = gtk_tree_view_get_model (tag_list);

	gtk_tree_model_get_iter (model, &iter, path);
	g_return_if_fail (&iter != NULL);

	gtk_tree_model_get (model, &iter, COLUMN_TAG_INDEX_IN_GROUP, &index, -1);

	gedit_debug_message (DEBUG_PLUGINS, "Index: %d", index);

	insert_tag (panel,
		    (Tag*)g_list_nth_data (panel->priv->selected_tag_group->tags, index),
		    TRUE);
}

static gboolean
tag_list_key_press_event_cb (GtkTreeView             *tag_list,
			     GdkEventKey             *event,
			     GeditTaglistPluginPanel *panel)
{
	gboolean grab_focus;

	grab_focus = (event->state & GDK_CONTROL_MASK) != 0;

	if (event->keyval == GDK_Return)
	{
		GtkTreeModel *model;
		GtkTreeSelection *selection;
		GtkTreeIter iter;
		gint index;

		gedit_debug_message (DEBUG_PLUGINS, "RETURN Pressed");

		model = gtk_tree_view_get_model (tag_list);

		selection = gtk_tree_view_get_selection (tag_list);

		if (gtk_tree_selection_get_selected (selection, NULL, &iter))
		{
			gtk_tree_model_get (model, &iter, COLUMN_TAG_INDEX_IN_GROUP, &index, -1);

			gedit_debug_message (DEBUG_PLUGINS, "Index: %d", index);

			insert_tag (panel,
				    (Tag*)g_list_nth_data (panel->priv->selected_tag_group->tags, index),
				    grab_focus);
		}

		return TRUE;
	}

	return FALSE;
}

static GtkTreeModel*
create_model (GeditTaglistPluginPanel *panel)
{
	gint i = 0;
	GtkListStore *store;
	GtkTreeIter iter;
	GList *list;

	gedit_debug (DEBUG_PLUGINS);

	/* create list store */
	store = gtk_list_store_new (NUM_COLUMNS, G_TYPE_STRING, G_TYPE_INT);

	/* add data to the list store */
	list = panel->priv->selected_tag_group->tags;

	while (list != NULL)
	{
		const gchar* tag_name;

		tag_name = (gchar *)((Tag*)list->data)->name;

		gedit_debug_message (DEBUG_PLUGINS, "%d : %s", i, tag_name);

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
				    COLUMN_TAG_NAME, tag_name,
				    COLUMN_TAG_INDEX_IN_GROUP, i,
				    -1);
		++i;

		list = g_list_next (list);
	}

	gedit_debug_message (DEBUG_PLUGINS, "Rows: %d ",
			     gtk_tree_model_iter_n_children (GTK_TREE_MODEL (store), NULL));

	return GTK_TREE_MODEL (store);
}

static void
populate_tags_list (GeditTaglistPluginPanel *panel)
{
	GtkTreeModel* model;

	gedit_debug (DEBUG_PLUGINS);

	g_return_if_fail (taglist != NULL);

	model = create_model (panel);
	gtk_tree_view_set_model (GTK_TREE_VIEW (panel->priv->tags_list),
			         model);
	g_object_unref (model);
}

static TagGroup *
find_tag_group (const gchar *name)
{
	GList *l;

	gedit_debug (DEBUG_PLUGINS);

	g_return_val_if_fail (taglist != NULL, NULL);

	for (l = taglist->tag_groups; l != NULL; l = g_list_next (l))
	{
		if (strcmp (name, (gchar *)((TagGroup*)l->data)->name) == 0)
			return (TagGroup*)l->data;
	}

	return NULL;
}

static void
populate_tag_groups_combo (GeditTaglistPluginPanel *panel)
{
	GList *l;
	GtkComboBox *combo;

	gedit_debug (DEBUG_PLUGINS);

	combo = GTK_COMBO_BOX (panel->priv->tag_groups_combo);

	if (taglist == NULL)
		return;

	for (l = taglist->tag_groups; l != NULL; l = g_list_next (l))
	{
		gtk_combo_box_append_text (combo,
					   (gchar *)((TagGroup*)l->data)->name);
	}

	gtk_combo_box_set_active (combo, 0);

	return;
}

static void
selected_group_changed (GtkComboBox             *combo,
			GeditTaglistPluginPanel *panel)
{
	gchar* group_name;

	gedit_debug (DEBUG_PLUGINS);

	group_name = gtk_combo_box_get_active_text (combo);

	if ((group_name == NULL) || (strlen (group_name) <= 0))
	{
		g_free (group_name);
		return;
	}

	if ((panel->priv->selected_tag_group == NULL) ||
	    (strcmp (group_name, (gchar *)panel->priv->selected_tag_group->name) != 0))
	{
		panel->priv->selected_tag_group = find_tag_group (group_name);
		g_return_if_fail (panel->priv->selected_tag_group != NULL);

		gedit_debug_message (DEBUG_PLUGINS,
				     "New selected group: %s",
				     panel->priv->selected_tag_group->name);

		populate_tags_list (panel);
	}
	
	/* Clean up preview */
	gtk_label_set_text (GTK_LABEL (panel->priv->preview),
			    "");

	g_free (group_name);
}

static gchar *
create_preview_string (Tag *tag)
{
	GString *str;

	str = g_string_new ("<tt><small>");

	if (tag->begin != NULL)
	{
		gchar *markup;

		markup = g_markup_escape_text ((gchar *)tag->begin, -1);
		g_string_append (str, markup);
		g_free (markup);
	}

	if (tag->end != NULL)
	{
		gchar *markup;

		markup = g_markup_escape_text ((gchar *)tag->end, -1);
		g_string_append (str, markup);
		g_free (markup);
	}

	g_string_append (str, "</small></tt>");

	return g_string_free (str, FALSE);
}

static void
update_preview (GeditTaglistPluginPanel *panel,
		Tag                     *tag)
{
	gchar *str;

	str = create_preview_string (tag);

	gtk_label_set_markup (GTK_LABEL (panel->priv->preview),
			      str);

	g_free (str);
}

static void
tag_list_cursor_changed_cb (GtkTreeView *tag_list,
			    gpointer     data)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	gint index;

	GeditTaglistPluginPanel *panel = (GeditTaglistPluginPanel *)data;

	model = gtk_tree_view_get_model (tag_list);

	selection = gtk_tree_view_get_selection (tag_list);

	if (gtk_tree_selection_get_selected (selection, NULL, &iter))
	{
		gtk_tree_model_get (model, &iter, COLUMN_TAG_INDEX_IN_GROUP, &index, -1);

		gedit_debug_message (DEBUG_PLUGINS, "Index: %d", index);

		update_preview (panel,
			        (Tag*)g_list_nth_data (panel->priv->selected_tag_group->tags, index));
	}
}

static gboolean
tags_list_query_tooltip_cb (GtkWidget               *widget,
			    gint                     x,
			    gint                     y,
			    gboolean                 keyboard_tip,
			    GtkTooltip              *tooltip,
			    GeditTaglistPluginPanel *panel)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	gint index;
	Tag *tag;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (widget));

	if (keyboard_tip)
	{
		gtk_tree_view_get_cursor (GTK_TREE_VIEW (widget),
					  &path,
					  NULL);

		if (path == NULL)
		{
			return FALSE;
		}
	}
	else
	{
		gint bin_x, bin_y;

		gtk_tree_view_convert_widget_to_bin_window_coords (GTK_TREE_VIEW (widget),
								   x, y,
								   &bin_x, &bin_y);

		if (!gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget),
						    bin_x, bin_y,
						    &path,
						    NULL, NULL, NULL))
		{
			return FALSE;
		}
	}

	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter,
			    COLUMN_TAG_INDEX_IN_GROUP, &index,
			    -1);

	tag = g_list_nth_data (panel->priv->selected_tag_group->tags, index);
	if (tag != NULL)
	{
		gchar *tip;

		tip = create_preview_string (tag);
		gtk_tooltip_set_markup (tooltip, tip);
		g_free (tip);
		gtk_tree_path_free (path);

		return TRUE;
	}

	gtk_tree_path_free (path);

	return FALSE;
}

static gboolean
expose_event_cb (GtkWidget      *panel,
                 GdkEventExpose *event,
                 gpointer        user_data)
{
	GeditTaglistPluginPanel *ppanel = GEDIT_TAGLIST_PLUGIN_PANEL (panel);

	gedit_debug (DEBUG_PLUGINS);

	/* If needed load taglists from files at the first expose */
	if (taglist == NULL)
		create_taglist (ppanel->priv->data_dir);

	/* And populate combo box */
	populate_tag_groups_combo (GEDIT_TAGLIST_PLUGIN_PANEL (panel));

	/* We need to manage only the first expose event -> disconnect */
	g_signal_handlers_disconnect_by_func (panel, expose_event_cb, NULL);

	return FALSE;
}

static void
set_combo_tooltip (GtkWidget *widget,
		   gpointer   data)
{
	if (GTK_IS_BUTTON (widget))
	{
		gtk_widget_set_tooltip_text (widget,
					     _("Select the group of tags you want to use"));
	}
}

static void
realize_tag_groups_combo (GtkWidget *combo,
			  gpointer   data)
{
	gtk_container_forall (GTK_CONTAINER (combo),
			      set_combo_tooltip,
			      NULL);
}

static void
add_preview_widget (GeditTaglistPluginPanel *panel)
{
	GtkWidget *expander;
	GtkWidget *frame;

	expander = gtk_expander_new_with_mnemonic (_("_Preview"));

	panel->priv->preview = 	gtk_label_new (NULL);
	gtk_widget_set_size_request (panel->priv->preview, -1, 80);

	gtk_label_set_line_wrap	(GTK_LABEL (panel->priv->preview), TRUE);
	gtk_label_set_use_markup (GTK_LABEL (panel->priv->preview), TRUE);
	gtk_misc_set_alignment (GTK_MISC (panel->priv->preview), 0, 0);
	gtk_misc_set_padding (GTK_MISC (panel->priv->preview), 6, 6);	
	gtk_label_set_selectable (GTK_LABEL (panel->priv->preview), TRUE);
	gtk_label_set_selectable (GTK_LABEL (panel->priv->preview), TRUE);
	gtk_label_set_ellipsize  (GTK_LABEL (panel->priv->preview),
				  PANGO_ELLIPSIZE_END);

	frame = gtk_frame_new (0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

	gtk_container_add (GTK_CONTAINER (frame),
			   panel->priv->preview);

	gtk_container_add (GTK_CONTAINER (expander),
			   frame);

	gtk_box_pack_start (GTK_BOX (panel), expander, FALSE, FALSE, 0);

	gtk_widget_show_all (expander);
}

static void
gedit_taglist_plugin_panel_init (GeditTaglistPluginPanel *panel)
{
	GtkWidget *sw;
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;
	GList *focus_chain = NULL;

	gedit_debug (DEBUG_PLUGINS);

	panel->priv = GEDIT_TAGLIST_PLUGIN_PANEL_GET_PRIVATE (panel);
	panel->priv->data_dir = NULL;

	/* Build the window content */
	panel->priv->tag_groups_combo = gtk_combo_box_new_text ();
	gtk_box_pack_start (GTK_BOX (panel),
			    panel->priv->tag_groups_combo,
			    FALSE,
			    TRUE,
			    0);

	g_signal_connect (panel->priv->tag_groups_combo,
			  "realize",
			  G_CALLBACK (realize_tag_groups_combo),
			  panel);

	sw = gtk_scrolled_window_new (NULL, NULL);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
                                             GTK_SHADOW_IN);
	gtk_box_pack_start (GTK_BOX (panel), sw, TRUE, TRUE, 0);

	/* Create tree view */
	panel->priv->tags_list = gtk_tree_view_new ();

	gedit_utils_set_atk_name_description (panel->priv->tag_groups_combo,
					      _("Available Tag Lists"),
					      NULL);
	gedit_utils_set_atk_name_description (panel->priv->tags_list,
					      _("Tags"),
					      NULL);
	gedit_utils_set_atk_relation (panel->priv->tag_groups_combo,
				      panel->priv->tags_list,
				      ATK_RELATION_CONTROLLER_FOR);
	gedit_utils_set_atk_relation (panel->priv->tags_list,
				      panel->priv->tag_groups_combo,
				      ATK_RELATION_CONTROLLED_BY);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (panel->priv->tags_list), FALSE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (panel->priv->tags_list), FALSE);

	g_object_set (panel->priv->tags_list, "has-tooltip", TRUE, NULL);

	/* Add the tags column */
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Tags"),
							   cell,
							   "text",
							   COLUMN_TAG_NAME,
							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (panel->priv->tags_list),
				     column);

	gtk_tree_view_set_search_column (GTK_TREE_VIEW (panel->priv->tags_list),
					 COLUMN_TAG_NAME);

	gtk_container_add (GTK_CONTAINER (sw), panel->priv->tags_list);

	focus_chain = g_list_prepend (focus_chain, panel->priv->tags_list);
	focus_chain = g_list_prepend (focus_chain, panel->priv->tag_groups_combo);

	gtk_container_set_focus_chain (GTK_CONTAINER (panel),
				       focus_chain);
	g_list_free (focus_chain);

	add_preview_widget (panel);

	gtk_widget_show_all (GTK_WIDGET (sw));
	gtk_widget_show (GTK_WIDGET (panel->priv->tag_groups_combo));

	g_signal_connect_after (panel->priv->tags_list,
				"row_activated",
				G_CALLBACK (tag_list_row_activated_cb),
				panel);
	g_signal_connect (panel->priv->tags_list,
			  "key_press_event",
			  G_CALLBACK (tag_list_key_press_event_cb),
			  panel);
	g_signal_connect (panel->priv->tags_list,
			  "query-tooltip",
			  G_CALLBACK (tags_list_query_tooltip_cb),
			  panel);
	g_signal_connect (panel->priv->tags_list,
			  "cursor_changed",
			  G_CALLBACK (tag_list_cursor_changed_cb),
			  panel);
	g_signal_connect (panel->priv->tag_groups_combo,
			  "changed",
			  G_CALLBACK (selected_group_changed),
			  panel);
	g_signal_connect (panel,
			  "expose-event",
			  G_CALLBACK (expose_event_cb),
			  NULL);
}

GtkWidget *
gedit_taglist_plugin_panel_new (GeditWindow *window,
				const gchar *data_dir)
{
	GeditTaglistPluginPanel *panel;

	g_return_val_if_fail (GEDIT_IS_WINDOW (window), NULL);

	panel = g_object_new (GEDIT_TYPE_TAGLIST_PLUGIN_PANEL,
			      "window", window,
			      NULL);
	
	panel->priv->data_dir = g_strdup (data_dir);
	
	return GTK_WIDGET (panel);
}
