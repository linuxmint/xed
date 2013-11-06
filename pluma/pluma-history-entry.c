/*
 * pluma-history-entry.c
 * This file is part of pluma
 *
 * Copyright (C) 2006 - Paolo Borelli
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
 * Modified by the pluma Team, 2006. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gio/gio.h>

#include "pluma-history-entry.h"
#include "pluma-prefs-manager.h"

enum {
	PROP_0,
	PROP_HISTORY_ID,
	PROP_HISTORY_LENGTH
};

#define MIN_ITEM_LEN 3

#define PLUMA_HISTORY_ENTRY_HISTORY_LENGTH_DEFAULT 10

#define PLUMA_HISTORY_ENTRY_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
						PLUMA_TYPE_HISTORY_ENTRY, \
						PlumaHistoryEntryPrivate))

struct _PlumaHistoryEntryPrivate
{
	gchar              *history_id;
	guint               history_length;
	
	GtkEntryCompletion *completion;
	
	GSettings          *settings;
};

#if GTK_CHECK_VERSION (3, 0, 0)
G_DEFINE_TYPE(PlumaHistoryEntry, pluma_history_entry, GTK_TYPE_COMBO_BOX_TEXT)
#else
G_DEFINE_TYPE(PlumaHistoryEntry, pluma_history_entry, GTK_TYPE_COMBO_BOX_ENTRY)
#endif

static void
pluma_history_entry_set_property (GObject      *object,
				  guint         prop_id,
				  const GValue *value,
				  GParamSpec   *spec)
{
	PlumaHistoryEntry *entry;

	g_return_if_fail (PLUMA_IS_HISTORY_ENTRY (object));

	entry = PLUMA_HISTORY_ENTRY (object);

	switch (prop_id) {
	case PROP_HISTORY_ID:
		entry->priv->history_id = g_value_dup_string (value);
		break;
	case PROP_HISTORY_LENGTH:
		pluma_history_entry_set_history_length (entry,
						     g_value_get_uint (value));
		break;
	default:
		break;
	}
}

static void
pluma_history_entry_get_property (GObject    *object,
				  guint       prop_id,
				  GValue     *value,
				  GParamSpec *spec)
{
	PlumaHistoryEntryPrivate *priv;

	g_return_if_fail (PLUMA_IS_HISTORY_ENTRY (object));

	priv = PLUMA_HISTORY_ENTRY (object)->priv;

	switch (prop_id) {
	case PROP_HISTORY_ID:
		g_value_set_string (value, priv->history_id);
		break;
	case PROP_HISTORY_LENGTH:
		g_value_set_uint (value, priv->history_length);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
	}
}

static void
#if GTK_CHECK_VERSION (3, 0, 0)
pluma_history_entry_dispose (GObject *object)
#else
pluma_history_entry_destroy (GtkObject *object)
#endif
{
	pluma_history_entry_set_enable_completion (PLUMA_HISTORY_ENTRY (object),
						   FALSE);

#if GTK_CHECK_VERSION (3, 0, 0)
	G_OBJECT_CLASS (pluma_history_entry_parent_class)->dispose (object);
#else
	GTK_OBJECT_CLASS (pluma_history_entry_parent_class)->destroy (object);
#endif
}

static void
pluma_history_entry_finalize (GObject *object)
{
	PlumaHistoryEntryPrivate *priv;

	priv = PLUMA_HISTORY_ENTRY (object)->priv;
	
	g_free (priv->history_id);

	if (priv->settings != NULL)
	{
		g_object_unref (G_OBJECT (priv->settings));
		priv->settings = NULL;
	}

	G_OBJECT_CLASS (pluma_history_entry_parent_class)->finalize (object);
}

static void 
pluma_history_entry_class_init (PlumaHistoryEntryClass *klass)
{
	GObjectClass   *object_class = G_OBJECT_CLASS (klass);
#if !GTK_CHECK_VERSION (3, 0, 0)
	GtkObjectClass *gtkobject_class = GTK_OBJECT_CLASS (klass);
#endif
	
	object_class->set_property = pluma_history_entry_set_property;
	object_class->get_property = pluma_history_entry_get_property;
	object_class->finalize = pluma_history_entry_finalize;
#if GTK_CHECK_VERSION (3, 0, 0)
	object_class->dispose = pluma_history_entry_dispose;
#else
	gtkobject_class->destroy = pluma_history_entry_destroy;
#endif
	
	g_object_class_install_property (object_class,
					 PROP_HISTORY_ID,
					 g_param_spec_string ("history-id",
							      "History ID",
							      "History ID",
							      NULL,
							      G_PARAM_READWRITE |
							      G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class,
					 PROP_HISTORY_LENGTH,
					 g_param_spec_uint ("history-length",
							    "Max History Length",
							    "Max History Length",
							    0,
							    G_MAXUINT,
							    PLUMA_HISTORY_ENTRY_HISTORY_LENGTH_DEFAULT,
							    G_PARAM_READWRITE |
							    G_PARAM_STATIC_STRINGS));

	/* TODO: Add enable-completion property */

	g_type_class_add_private (object_class, sizeof(PlumaHistoryEntryPrivate));
}

static GtkListStore *
get_history_store (PlumaHistoryEntry *entry)
{
	GtkTreeModel *store;

	store = gtk_combo_box_get_model (GTK_COMBO_BOX (entry));
	g_return_val_if_fail (GTK_IS_LIST_STORE (store), NULL);

	return (GtkListStore *) store;
}

static GSList *
get_history_list (PlumaHistoryEntry *entry)
{
	GtkListStore *store;
	GtkTreeIter iter;
	gboolean valid;
	GSList *list = NULL;

	store = get_history_store (entry);

	valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store),
					       &iter);

	while (valid)
	{
		gchar *str;

		gtk_tree_model_get (GTK_TREE_MODEL (store),
				    &iter,
				    0, &str,
				    -1);

		list = g_slist_prepend (list, str);

		valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (store),
						  &iter);
	}

	return g_slist_reverse (list);
}

static void
pluma_history_entry_save_history (PlumaHistoryEntry *entry)
{
	GSList *settings_items;

	g_return_if_fail (PLUMA_IS_HISTORY_ENTRY (entry));

	settings_items = get_history_list (entry);

	pluma_prefs_manager_set_gslist (entry->priv->settings,
			      entry->priv->history_id,
			      settings_items);

	g_slist_foreach (settings_items, (GFunc) g_free, NULL);
	g_slist_free (settings_items);
}

static gboolean
remove_item (GtkListStore *store,
	     const gchar  *text)
{
	GtkTreeIter iter;

	g_return_val_if_fail (text != NULL, FALSE);

	if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter))
		return FALSE;

	do
	{
		gchar *item_text;

		gtk_tree_model_get (GTK_TREE_MODEL (store),
				    &iter,
				    0,
				    &item_text,
				    -1);

		if (item_text != NULL &&
		    strcmp (item_text, text) == 0)
		{
			gtk_list_store_remove (store, &iter);
			g_free (item_text);
			return TRUE;
		}

		g_free (item_text);

	} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter));

	return FALSE;
}

static void
clamp_list_store (GtkListStore *store,
		  guint         max)
{
	GtkTreePath *path;
	GtkTreeIter iter;

	/* -1 because TreePath counts from 0 */
	path = gtk_tree_path_new_from_indices (max - 1, -1);

	if (gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path))
	{
		while (1)
		{
			if (!gtk_list_store_remove (store, &iter))
				break;
		}
	}

	gtk_tree_path_free (path);
}

static void
insert_history_item (PlumaHistoryEntry *entry,
		     const gchar       *text,
		     gboolean           prepend)
{
	GtkListStore *store;
	GtkTreeIter iter;

	if (g_utf8_strlen (text, -1) <= MIN_ITEM_LEN)
		return;
		
	store = get_history_store (entry);

	/* remove the text from the store if it was already
	 * present. If it wasn't, clamp to max history - 1
	 * before inserting the new row, otherwise appending
	 * would not work */

	if (!remove_item (store, text))
		clamp_list_store (store,
				  entry->priv->history_length - 1);

	if (prepend)
		gtk_list_store_insert (store, &iter, 0);
	else
		gtk_list_store_append (store, &iter);

	gtk_list_store_set (store,
			    &iter,
			    0,
			    text,
			    -1);

	pluma_history_entry_save_history (entry);
}

void
pluma_history_entry_prepend_text (PlumaHistoryEntry *entry,
				  const gchar       *text)
{
	g_return_if_fail (PLUMA_IS_HISTORY_ENTRY (entry));
	g_return_if_fail (text != NULL);

	insert_history_item (entry, text, TRUE);
}

void
pluma_history_entry_append_text (PlumaHistoryEntry *entry,
				 const gchar       *text)
{
	g_return_if_fail (PLUMA_IS_HISTORY_ENTRY (entry));
	g_return_if_fail (text != NULL);

	insert_history_item (entry, text, FALSE);
}

static void
pluma_history_entry_load_history (PlumaHistoryEntry *entry)
{
	GSList *settings_items, *l;
	GtkListStore *store;
	GtkTreeIter iter;
	guint i;

	g_return_if_fail (PLUMA_IS_HISTORY_ENTRY (entry));

	store = get_history_store (entry);

	settings_items = pluma_prefs_manager_get_gslist (entry->priv->settings,
					     entry->priv->history_id);

	gtk_list_store_clear (store);

	for (l = settings_items, i = 0;
	     l != NULL && i < entry->priv->history_length;
	     l = l->next, i++)
	{
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, 
				    &iter,
				    0,
				    l->data,
				    -1);
	}

	g_slist_foreach (settings_items, (GFunc) g_free, NULL);
	g_slist_free (settings_items);
}

void
pluma_history_entry_clear (PlumaHistoryEntry *entry)
{
	GtkListStore *store;

	g_return_if_fail (PLUMA_IS_HISTORY_ENTRY (entry));

	store = get_history_store (entry);
	gtk_list_store_clear (store);

	pluma_history_entry_save_history (entry);
}

static void
pluma_history_entry_init (PlumaHistoryEntry *entry)
{
	PlumaHistoryEntryPrivate *priv;

	priv = PLUMA_HISTORY_ENTRY_GET_PRIVATE (entry);
	entry->priv = priv;

	priv->history_id = NULL;
	priv->history_length = PLUMA_HISTORY_ENTRY_HISTORY_LENGTH_DEFAULT;

	priv->completion = NULL;

	priv->settings = g_settings_new (PLUMA_SCHEMA);
}

void
pluma_history_entry_set_history_length (PlumaHistoryEntry *entry,
					guint              history_length)
{
	g_return_if_fail (PLUMA_IS_HISTORY_ENTRY (entry));
	g_return_if_fail (history_length > 0);

	entry->priv->history_length = history_length;

	/* TODO: update if we currently have more items than max */
}

guint
pluma_history_entry_get_history_length (PlumaHistoryEntry *entry)
{
	g_return_val_if_fail (PLUMA_IS_HISTORY_ENTRY (entry), 0);

	return entry->priv->history_length;
}

gchar *
pluma_history_entry_get_history_id (PlumaHistoryEntry *entry)
{
	g_return_val_if_fail (PLUMA_IS_HISTORY_ENTRY (entry), NULL);

	return g_strdup (entry->priv->history_id);
}

void
pluma_history_entry_set_enable_completion (PlumaHistoryEntry *entry,
					   gboolean           enable)
{
	g_return_if_fail (PLUMA_IS_HISTORY_ENTRY (entry));
	
	if (enable)
	{
		if (entry->priv->completion != NULL)
			return;
		
		entry->priv->completion = gtk_entry_completion_new ();
		gtk_entry_completion_set_model (entry->priv->completion, 
						GTK_TREE_MODEL (get_history_store (entry)));
		
		/* Use model column 0 as the text column */
		gtk_entry_completion_set_text_column (entry->priv->completion, 0);

		gtk_entry_completion_set_minimum_key_length (entry->priv->completion,
							     MIN_ITEM_LEN);

		gtk_entry_completion_set_popup_completion (entry->priv->completion, FALSE);
		gtk_entry_completion_set_inline_completion (entry->priv->completion, TRUE);
	
		/* Assign the completion to the entry */
		gtk_entry_set_completion (GTK_ENTRY (pluma_history_entry_get_entry(entry)), 
					  entry->priv->completion);
	}
	else
	{
		if (entry->priv->completion == NULL)
			return;

		gtk_entry_set_completion (GTK_ENTRY (pluma_history_entry_get_entry (entry)), 
					  NULL);
		
		g_object_unref (entry->priv->completion);
		
		entry->priv->completion = NULL;
	}
}
							 
gboolean
pluma_history_entry_get_enable_completion (PlumaHistoryEntry *entry)
{
	g_return_val_if_fail (PLUMA_IS_HISTORY_ENTRY (entry), FALSE);
	
	return entry->priv->completion != NULL;
}

GtkWidget *
pluma_history_entry_new (const gchar *history_id,
			 gboolean     enable_completion)
{
	GtkWidget *ret;
	GtkListStore *store;

	g_return_val_if_fail (history_id != NULL, NULL);

	/* Note that we are setting the model, so
	 * user must be careful to always manipulate
	 * data in the history through pluma_history_entry_
	 * functions.
	 */

	store = gtk_list_store_new (1, G_TYPE_STRING);

	ret = g_object_new (PLUMA_TYPE_HISTORY_ENTRY,
			    "history-id", history_id,
	                    "model", store,
#if GTK_CHECK_VERSION (3, 0, 0)
			    "has-entry", TRUE,
			    "id-column", 0,
#else
			    "text-column", 0,
#endif
	                    NULL);

	g_object_unref (store);

	/* loading has to happen after the model
	 * has been set. However the model is not a 
	 * G_PARAM_CONSTRUCT property of GtkComboBox
	 * so we cannot do this in the constructor.
	 * For now we simply do here since this widget is 
	 * not bound to other programming languages.
	 * A maybe better alternative is to override the 
	 * model property of combobox and mark CONTRUCT_ONLY.
	 * This would also ensure that the model cannot be
	 * set explicitely at a later time.
	 */
	pluma_history_entry_load_history (PLUMA_HISTORY_ENTRY (ret));

	pluma_history_entry_set_enable_completion (PLUMA_HISTORY_ENTRY (ret),
						   enable_completion);
						   
	return ret;
}

/*
 * Utility function to get the editable text entry internal widget.
 * I would prefer to not expose this implementation detail and 
 * simply make the PlumaHistoryEntry widget implement the 
 * GtkEditable interface. Unfortunately both GtkEditable and
 * GtkComboBox have a "changed" signal and I am not sure how to
 * handle the conflict.
 */
GtkWidget *
pluma_history_entry_get_entry (PlumaHistoryEntry *entry)
{
	g_return_val_if_fail (PLUMA_IS_HISTORY_ENTRY (entry), NULL);

	return gtk_bin_get_child (GTK_BIN (entry));
}

static void
escape_cell_data_func (GtkTreeViewColumn           *col,
		       GtkCellRenderer             *renderer,
		       GtkTreeModel                *model,
		       GtkTreeIter                 *iter,
		       PlumaHistoryEntryEscapeFunc  escape_func)
{
	gchar *str;
	gchar *escaped;

	gtk_tree_model_get (model, iter, 0, &str, -1);
	escaped = escape_func (str);
	g_object_set (renderer, "text", escaped, NULL);

	g_free (str);
	g_free (escaped);
}

void
pluma_history_entry_set_escape_func (PlumaHistoryEntry           *entry,
				     PlumaHistoryEntryEscapeFunc  escape_func)
{
	GList *cells;

	g_return_if_fail (PLUMA_IS_HISTORY_ENTRY (entry));

	cells = gtk_cell_layout_get_cells (GTK_CELL_LAYOUT (entry));

	/* We only have one cell renderer */
	g_return_if_fail (cells->data != NULL && cells->next == NULL);

	if (escape_func != NULL)
		gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (entry),
						    GTK_CELL_RENDERER (cells->data),
						    (GtkCellLayoutDataFunc) escape_cell_data_func,
						    escape_func,
						    NULL);
	else
		gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (entry),
						    GTK_CELL_RENDERER (cells->data),
						    NULL,
						    NULL,
						    NULL);

	g_list_free (cells);
}
