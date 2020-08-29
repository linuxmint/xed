/*
 * xed-history-entry.c
 * This file is part of xed
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
 * Modified by the xed Team, 2006. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#include <config.h>
#include <string.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gio/gio.h>

#include "xed-history-entry.h"

enum
{
    PROP_0,
    PROP_HISTORY_ID,
    PROP_HISTORY_LENGTH
};

#define MIN_ITEM_LEN 3

#define XED_HISTORY_ENTRY_HISTORY_LENGTH_DEFAULT 10

struct _XedHistoryEntryPrivate
{
    gchar *history_id;
    guint  history_length;
    GtkEntryCompletion *completion;
    GSettings *settings;
};

G_DEFINE_TYPE_WITH_PRIVATE (XedHistoryEntry, xed_history_entry, GTK_TYPE_COMBO_BOX_TEXT)

static void
xed_history_entry_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *spec)
{
    XedHistoryEntry *entry;

    g_return_if_fail (XED_IS_HISTORY_ENTRY (object));

    entry = XED_HISTORY_ENTRY (object);

    switch (prop_id)
    {
        case PROP_HISTORY_ID:
            entry->priv->history_id = g_value_dup_string (value);
            break;
        case PROP_HISTORY_LENGTH:
            xed_history_entry_set_history_length (entry, g_value_get_uint (value));
            break;
        default:
            break;
    }
}

static void
xed_history_entry_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *spec)
{
    XedHistoryEntryPrivate *priv;

    g_return_if_fail (XED_IS_HISTORY_ENTRY (object));

    priv = XED_HISTORY_ENTRY (object)->priv;

    switch (prop_id)
    {
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
xed_history_entry_dispose (GObject *object)
{
    XedHistoryEntryPrivate *priv;

    priv = XED_HISTORY_ENTRY (object)->priv;
    xed_history_entry_set_enable_completion (XED_HISTORY_ENTRY (object), FALSE);

    g_clear_object (&priv->settings);

    G_OBJECT_CLASS (xed_history_entry_parent_class)->dispose (object);
}

static void
xed_history_entry_finalize (GObject *object)
{
    XedHistoryEntryPrivate *priv;

    priv = XED_HISTORY_ENTRY (object)->priv;

    g_free (priv->history_id);

    G_OBJECT_CLASS (xed_history_entry_parent_class)->finalize (object);
}

static void
xed_history_entry_class_init (XedHistoryEntryClass *klass)
{
    GObjectClass   *object_class = G_OBJECT_CLASS (klass);

    object_class->set_property = xed_history_entry_set_property;
    object_class->get_property = xed_history_entry_get_property;
    object_class->finalize = xed_history_entry_finalize;
    object_class->dispose = xed_history_entry_dispose;

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
                                                XED_HISTORY_ENTRY_HISTORY_LENGTH_DEFAULT,
                                                G_PARAM_READWRITE |
                                                G_PARAM_STATIC_STRINGS));

    /* TODO: Add enable-completion property */
}

static GtkListStore *
get_history_store (XedHistoryEntry *entry)
{
    GtkTreeModel *store;

    store = gtk_combo_box_get_model (GTK_COMBO_BOX (entry));
    g_return_val_if_fail (GTK_IS_LIST_STORE (store), NULL);

    return (GtkListStore *) store;
}

static gchar **
get_history_items (XedHistoryEntry *entry)
{
    GtkListStore *store;
    GtkTreeIter iter;
    GPtrArray *array;
    gboolean valid;
    gint n_children;
    gint text_column;

    store = get_history_store (entry);
    text_column = gtk_combo_box_get_entry_text_column (GTK_COMBO_BOX (entry));
    valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter);
    n_children = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (store), NULL);
    array = g_ptr_array_sized_new (n_children + 1);

    while (valid)
    {
        gchar *str;

        gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, text_column, &str, -1);

        g_ptr_array_add (array, str);
        valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter);
    }

    g_ptr_array_add (array, NULL);

    return (gchar **)g_ptr_array_free (array, FALSE);
}

static void
xed_history_entry_save_history (XedHistoryEntry *entry)
{
    gchar **items;

    g_return_if_fail (XED_IS_HISTORY_ENTRY (entry));

    items = get_history_items (entry);

    g_settings_set_strv (entry->priv->settings, entry->priv->history_id, (const gchar * const *)items);

    g_strfreev (items);
}

static gboolean
remove_item (XedHistoryEntry *entry,
             const gchar     *text)
{
    GtkListStore *store;
    GtkTreeIter iter;
    gint text_column;

    g_return_val_if_fail (text != NULL, FALSE);

    store = get_history_store (entry);
    text_column = gtk_combo_box_get_entry_text_column (GTK_COMBO_BOX (entry));

    if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter))
    {
        return FALSE;
    }

    do
    {
        gchar *item_text;

        gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, text_column, &item_text, -1);

        if (item_text != NULL && strcmp (item_text, text) == 0)
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
            {
                break;
            }
        }
    }

    gtk_tree_path_free (path);
}

static void
insert_history_item (XedHistoryEntry *entry,
                     const gchar     *text,
                     gboolean         prepend)
{
    GtkListStore *store;

    if (g_utf8_strlen (text, -1) <= MIN_ITEM_LEN)
    {
        return;
    }

    store = get_history_store (entry);

    /* remove the text from the store if it was already
     * present. If it wasn't, clamp to max history - 1
     * before inserting the new row, otherwise appending
     * would not work */

    if (!remove_item (entry, text))
    {
        clamp_list_store (store, entry->priv->history_length - 1);
    }

    if (prepend)
    {
        gtk_combo_box_text_prepend_text (GTK_COMBO_BOX_TEXT (entry), text);
    }
    else
    {
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (entry), text);
    }

    xed_history_entry_save_history (entry);
}

void
xed_history_entry_prepend_text (XedHistoryEntry *entry,
                                const gchar     *text)
{
    g_return_if_fail (XED_IS_HISTORY_ENTRY (entry));
    g_return_if_fail (text != NULL);

    insert_history_item (entry, text, TRUE);
}

void
xed_history_entry_append_text (XedHistoryEntry *entry,
                               const gchar     *text)
{
    g_return_if_fail (XED_IS_HISTORY_ENTRY (entry));
    g_return_if_fail (text != NULL);

    insert_history_item (entry, text, FALSE);
}

static void
xed_history_entry_load_history (XedHistoryEntry *entry)
{
    gchar **items;
    gsize i;

    g_return_if_fail (XED_IS_HISTORY_ENTRY (entry));

    items = g_settings_get_strv (entry->priv->settings, entry->priv->history_id);
    i = 0;

    gtk_combo_box_text_remove_all (GTK_COMBO_BOX_TEXT (entry));

    /* Now the default value is an empty string so we have to take care of it to not
       add the empty string in the search list */
    while (items[i] != NULL && *items[i] != '\0' && i < entry->priv->history_length)
    {
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (entry), items[i]);
        i++;
    }

    g_strfreev (items);
}

void
xed_history_entry_clear (XedHistoryEntry *entry)
{
    g_return_if_fail (XED_IS_HISTORY_ENTRY (entry));

    gtk_combo_box_text_remove_all (GTK_COMBO_BOX_TEXT (entry));

    xed_history_entry_save_history (entry);
}

static void
xed_history_entry_init (XedHistoryEntry *entry)
{
    XedHistoryEntryPrivate *priv;

    priv = xed_history_entry_get_instance_private (entry);
    entry->priv = priv;

    priv->history_id = NULL;
    priv->history_length = XED_HISTORY_ENTRY_HISTORY_LENGTH_DEFAULT;

    priv->completion = NULL;

    priv->settings = g_settings_new ("org.x.editor.state.history-entry");
}

void
xed_history_entry_set_history_length (XedHistoryEntry *entry,
                                      guint            history_length)
{
    g_return_if_fail (XED_IS_HISTORY_ENTRY (entry));
    g_return_if_fail (history_length > 0);

    entry->priv->history_length = history_length;

    /* TODO: update if we currently have more items than max */
}

guint
xed_history_entry_get_history_length (XedHistoryEntry *entry)
{
    g_return_val_if_fail (XED_IS_HISTORY_ENTRY (entry), 0);

    return entry->priv->history_length;
}

void
xed_history_entry_set_enable_completion (XedHistoryEntry *entry,
                                         gboolean         enable)
{
    g_return_if_fail (XED_IS_HISTORY_ENTRY (entry));

    if (enable)
    {
        if (entry->priv->completion != NULL)
        {
            return;
        }

        entry->priv->completion = gtk_entry_completion_new ();
        gtk_entry_completion_set_model (entry->priv->completion, GTK_TREE_MODEL (get_history_store (entry)));

        /* Use model column 0 as the text column */
        gtk_entry_completion_set_text_column (entry->priv->completion, 0);

        gtk_entry_completion_set_minimum_key_length (entry->priv->completion, MIN_ITEM_LEN);

        gtk_entry_completion_set_popup_completion (entry->priv->completion, FALSE);
        gtk_entry_completion_set_inline_completion (entry->priv->completion, TRUE);

        /* Assign the completion to the entry */
        gtk_entry_set_completion (GTK_ENTRY (xed_history_entry_get_entry(entry)), entry->priv->completion);
    }
    else
    {
        if (entry->priv->completion == NULL)
        {
            return;
        }

        gtk_entry_set_completion (GTK_ENTRY (xed_history_entry_get_entry (entry)), NULL);

        g_object_unref (entry->priv->completion);

        entry->priv->completion = NULL;
    }
}

gboolean
xed_history_entry_get_enable_completion (XedHistoryEntry *entry)
{
    g_return_val_if_fail (XED_IS_HISTORY_ENTRY (entry), FALSE);

    return entry->priv->completion != NULL;
}

GtkWidget *
xed_history_entry_new (const gchar *history_id,
             gboolean     enable_completion)
{
    GtkWidget *ret;
    GtkEntry *real_entry;

    g_return_val_if_fail (history_id != NULL, NULL);

    ret = g_object_new (XED_TYPE_HISTORY_ENTRY,
                        "has-entry", TRUE,
                        "entry-text-column", 0,
                        "id-column", 1,
                        "history-id", history_id,
                        NULL);

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
    xed_history_entry_load_history (XED_HISTORY_ENTRY (ret));

    xed_history_entry_set_enable_completion (XED_HISTORY_ENTRY (ret), enable_completion);

    real_entry = GTK_ENTRY (xed_history_entry_get_entry (XED_HISTORY_ENTRY (ret)));
    gtk_entry_set_width_chars (real_entry, 6);

    return ret;
}

/*
 * Utility function to get the editable text entry internal widget.
 * I would prefer to not expose this implementation detail and
 * simply make the XedHistoryEntry widget implement the
 * GtkEditable interface. Unfortunately both GtkEditable and
 * GtkComboBox have a "changed" signal and I am not sure how to
 * handle the conflict.
 */
GtkWidget *
xed_history_entry_get_entry (XedHistoryEntry *entry)
{
    g_return_val_if_fail (XED_IS_HISTORY_ENTRY (entry), NULL);

    return gtk_bin_get_child (GTK_BIN (entry));
}
