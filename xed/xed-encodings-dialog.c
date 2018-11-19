/*
 * xed-encodings-dialog.c
 * This file is part of xed
 *
 * Copyright (C) 2002-2005 Paolo Maggi
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
 * Modified by the xed Team, 2002-2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#include <config.h>
#include <string.h>
#include <glib/gi18n.h>
#include <gtksourceview/gtksource.h>

#include "xed-encodings-dialog.h"
#include "xed-utils.h"
#include "xed-debug.h"
#include "xed-dirs.h"
#include "xed-settings.h"

#define XED_ENCODINGS_DIALOG_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
                                                 XED_TYPE_ENCODINGS_DIALOG,           \
                                                 XedEncodingsDialogPrivate))

struct _XedEncodingsDialogPrivate
{
    GSettings *enc_settings;

    GtkListStore *available_liststore;
    GtkListStore *displayed_liststore;

    GtkWidget *available_treeview;
    GtkWidget *displayed_treeview;
    GtkWidget *add_button;
    GtkWidget *remove_button;

    GSList       *show_in_menu_list;
};

G_DEFINE_TYPE(XedEncodingsDialog, xed_encodings_dialog, GTK_TYPE_DIALOG)

static void
xed_encodings_dialog_finalize (GObject *object)
{
    XedEncodingsDialogPrivate *priv = XED_ENCODINGS_DIALOG (object)->priv;

    g_slist_free (priv->show_in_menu_list);

    G_OBJECT_CLASS (xed_encodings_dialog_parent_class)->finalize (object);
}

static void
xed_encodings_dialog_dispose (GObject *object)
{
    XedEncodingsDialogPrivate *priv = XED_ENCODINGS_DIALOG (object)->priv;

    g_clear_object (&priv->enc_settings);

    G_OBJECT_CLASS (xed_encodings_dialog_parent_class)->dispose (object);
}

static void
xed_encodings_dialog_class_init (XedEncodingsDialogClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = xed_encodings_dialog_finalize;
    object_class->dispose = xed_encodings_dialog_dispose;

    g_type_class_add_private (object_class, sizeof (XedEncodingsDialogPrivate));
}

enum
{
    COLUMN_NAME,
    COLUMN_CHARSET,
    N_COLUMNS
};

static void
count_selected_items_func (GtkTreeModel *model,
                           GtkTreePath  *path,
                           GtkTreeIter  *iter,
                           gpointer      data)
{
    int *count = data;

    *count += 1;
}

static void
available_selection_changed_callback (GtkTreeSelection   *selection,
                                      XedEncodingsDialog *dialogs)
{
    int count;

    count = 0;
    gtk_tree_selection_selected_foreach (selection, count_selected_items_func, &count);

    gtk_widget_set_sensitive (dialogs->priv->add_button, count > 0);
}

static void
displayed_selection_changed_callback (GtkTreeSelection   *selection,
                                      XedEncodingsDialog *dialogs)
{
    int count;

    count = 0;
    gtk_tree_selection_selected_foreach (selection, count_selected_items_func, &count);

    gtk_widget_set_sensitive (dialogs->priv->remove_button, count > 0);
}

static void
get_selected_encodings_func (GtkTreeModel *model,
                             GtkTreePath  *path,
                             GtkTreeIter  *iter,
                             gpointer      data)
{
    GSList **list = data;
    gchar *charset;
    const GtkSourceEncoding *enc;

    charset = NULL;
    gtk_tree_model_get (model, iter, COLUMN_CHARSET, &charset, -1);

    enc = gtk_source_encoding_get_from_charset (charset);
    g_free (charset);

    *list = g_slist_prepend (*list, (gpointer)enc);
}

static void
update_shown_in_menu_tree_model (GtkListStore *store,
                                 GSList       *list)
{
    GtkTreeIter iter;

    gtk_list_store_clear (store);

    while (list != NULL)
    {
        const GtkSourceEncoding *enc;

        enc = list->data;

        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter,
                            COLUMN_CHARSET, gtk_source_encoding_get_charset (enc),
                            COLUMN_NAME, gtk_source_encoding_get_name (enc),
                            -1);

        list = g_slist_next (list);
    }
}

static void
add_button_clicked_callback (GtkWidget          *button,
                             XedEncodingsDialog *dialog)
{
    GtkTreeSelection *selection;
    GSList *encodings;
    GSList *tmp;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->priv->available_treeview));

    encodings = NULL;
    gtk_tree_selection_selected_foreach (selection, get_selected_encodings_func, &encodings);

    tmp = encodings;
    while (tmp != NULL)
    {
        if (g_slist_find (dialog->priv->show_in_menu_list, tmp->data) == NULL)
            dialog->priv->show_in_menu_list = g_slist_prepend (dialog->priv->show_in_menu_list, tmp->data);

        tmp = g_slist_next (tmp);
    }

    g_slist_free (encodings);

    update_shown_in_menu_tree_model (GTK_LIST_STORE (dialog->priv->displayed_liststore),
                                     dialog->priv->show_in_menu_list);
}

static void
remove_button_clicked_callback (GtkWidget          *button,
                                XedEncodingsDialog *dialog)
{
    GtkTreeSelection *selection;
    GSList *encodings;
    GSList *tmp;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->priv->displayed_treeview));

    encodings = NULL;
    gtk_tree_selection_selected_foreach (selection, get_selected_encodings_func, &encodings);

    tmp = encodings;
    while (tmp != NULL)
    {
        dialog->priv->show_in_menu_list = g_slist_remove (dialog->priv->show_in_menu_list, tmp->data);

        tmp = g_slist_next (tmp);
    }

    g_slist_free (encodings);

    update_shown_in_menu_tree_model (GTK_LIST_STORE (dialog->priv->displayed_liststore),
                                     dialog->priv->show_in_menu_list);
}

static void
init_shown_in_menu_tree_model (XedEncodingsDialog *dialog)
{
    GtkTreeIter iter;
    gchar **enc_strv;
    GSList *list, *tmp;

    /* add data to the list store */
    enc_strv = g_settings_get_strv (dialog->priv->enc_settings, XED_SETTINGS_ENCODING_SHOWN_IN_MENU);

    list = _xed_utils_encoding_strv_to_list ((const gchar * const *)enc_strv);

    for (tmp = list; tmp != NULL; tmp = g_slist_next (tmp))
    {
        const GtkSourceEncoding *enc = tmp->data;

        dialog->priv->show_in_menu_list = g_slist_prepend (dialog->priv->show_in_menu_list, tmp->data);

        gtk_list_store_append (dialog->priv->displayed_liststore, &iter);
        gtk_list_store_set (dialog->priv->displayed_liststore, &iter,
                            COLUMN_CHARSET, gtk_source_encoding_get_charset (enc),
                            COLUMN_NAME, gtk_source_encoding_get_name (enc),
                            -1);
    }

    g_slist_free (list);
}

static void
response_handler (GtkDialog          *dialog,
                  gint                response_id,
                  XedEncodingsDialog *dlg)
{
    if (response_id == GTK_RESPONSE_HELP)
    {
        xed_app_show_help (XED_APP (g_application_get_default ()), GTK_WINDOW (dialog), "xed", NULL);
        g_signal_stop_emission_by_name (dialog, "response");
        return;
    }

    if (response_id == GTK_RESPONSE_OK)
    {
        gchar **encs;

        encs = _xed_utils_encoding_list_to_strv (dlg->priv->show_in_menu_list);
        g_settings_set_strv (dlg->priv->enc_settings,
                             XED_SETTINGS_ENCODING_SHOWN_IN_MENU,
                             (const gchar * const *)encs);

        g_strfreev (encs);
    }
}

static void
init_liststore_available (XedEncodingsDialog *dialog)
{
    GSList *all_encodings;
    GSList *l;

    all_encodings = gtk_source_encoding_get_all ();

    for (l = all_encodings; l != NULL; l = l->next)
    {
        const GtkSourceEncoding *encoding = l->data;
        GtkTreeIter iter;

        if (encoding == gtk_source_encoding_get_utf8 ())
        {
            /* The UTF-8 encoding is always added to the combobox. */
            continue;
        }

        gtk_list_store_append (dialog->priv->available_liststore, &iter);

        gtk_list_store_set (dialog->priv->available_liststore,
                            &iter,
                            COLUMN_CHARSET, gtk_source_encoding_get_charset (encoding),
                            COLUMN_NAME, gtk_source_encoding_get_name (encoding),
                            -1);
    }

    g_slist_free (all_encodings);
}

static void
xed_encodings_dialog_init (XedEncodingsDialog *dlg)
{
    GtkBuilder *builder;
    GtkWidget *content;
    GtkCellRenderer *cell_renderer;
    GtkTreeModel *sort_model;
    GtkTreeViewColumn *column;
    GtkTreeSelection *selection;
    gchar *root_objects[] = {
        "encodings-dialog-contents",
        NULL
    };

    dlg->priv = XED_ENCODINGS_DIALOG_GET_PRIVATE (dlg);
    dlg->priv->enc_settings = g_settings_new ("org.x.editor.preferences.encodings");

    gtk_dialog_add_buttons (GTK_DIALOG (dlg),
                            _("_Cancel"), GTK_RESPONSE_CANCEL,
                            _("_OK"), GTK_RESPONSE_OK,
                            _("_Help"), GTK_RESPONSE_HELP,
                            NULL);

    gtk_window_set_title (GTK_WINDOW (dlg), _("Character Encodings"));
    gtk_window_set_default_size (GTK_WINDOW (dlg), 650, 400);

    /* HIG defaults */
    gtk_container_set_border_width (GTK_CONTAINER (dlg), 5);
    gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dlg))), 2); /* 2 * 5 + 2 = 12 */

    gtk_dialog_set_default_response (GTK_DIALOG (dlg), GTK_RESPONSE_OK);

    g_signal_connect (dlg, "response", G_CALLBACK (response_handler), dlg);

    builder = gtk_builder_new ();
    gtk_builder_add_objects_from_resource (builder, "/org/x/editor/ui/xed-encodings-dialog.ui",
                                           root_objects, NULL);
    content = GTK_WIDGET (gtk_builder_get_object (builder, "encodings-dialog-contents"));
    g_object_ref (content);
    dlg->priv->add_button = GTK_WIDGET (gtk_builder_get_object (builder, "add-button"));
    dlg->priv->remove_button = GTK_WIDGET (gtk_builder_get_object (builder, "remove-button"));
    dlg->priv->available_treeview = GTK_WIDGET (gtk_builder_get_object (builder, "available-treeview"));
    dlg->priv->displayed_treeview = GTK_WIDGET (gtk_builder_get_object (builder, "displayed-treeview"));
    g_object_unref (builder);

    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dlg))), content, TRUE, TRUE, 0);
    g_object_unref (content);
    gtk_container_set_border_width (GTK_CONTAINER (content), 5);

    g_signal_connect (dlg->priv->add_button, "clicked", G_CALLBACK (add_button_clicked_callback), dlg);
    g_signal_connect (dlg->priv->remove_button, "clicked", G_CALLBACK (remove_button_clicked_callback), dlg);

    /* Tree view of available encodings */
    dlg->priv->available_liststore = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);

    cell_renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("_Description"),
                                                       cell_renderer,
                                                       "text", COLUMN_NAME,
                                                       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (dlg->priv->available_treeview), column);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_NAME);

    cell_renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("_Encoding"),
                                                       cell_renderer,
                                                       "text",
                                                       COLUMN_CHARSET,
                                                       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (dlg->priv->available_treeview), column);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_CHARSET);

    /* Add the data */
    init_liststore_available (dlg);

    /* Sort model */
    sort_model = gtk_tree_model_sort_new_with_model (GTK_TREE_MODEL (dlg->priv->available_liststore));
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (sort_model), COLUMN_NAME, GTK_SORT_ASCENDING);

    gtk_tree_view_set_model (GTK_TREE_VIEW (dlg->priv->available_treeview), sort_model);
    g_object_unref (G_OBJECT (dlg->priv->available_liststore));
    g_object_unref (G_OBJECT (sort_model));

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dlg->priv->available_treeview));
    gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_MULTIPLE);

    available_selection_changed_callback (selection, dlg);
    g_signal_connect (selection, "changed", G_CALLBACK (available_selection_changed_callback), dlg);

    /* Tree view of selected encodings */
    dlg->priv->displayed_liststore = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);

    cell_renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("_Description"),
                                                       cell_renderer,
                                                       "text", COLUMN_NAME,
                                                       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (dlg->priv->displayed_treeview), column);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_NAME);

    cell_renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("_Encoding"),
                                                       cell_renderer,
                                                       "text",
                                                       COLUMN_CHARSET,
                                                       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (dlg->priv->displayed_treeview), column);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_CHARSET);

    /* Add the data */
    init_shown_in_menu_tree_model (dlg);

    /* Sort model */
    sort_model = gtk_tree_model_sort_new_with_model (GTK_TREE_MODEL (dlg->priv->displayed_liststore));

    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (sort_model), COLUMN_NAME, GTK_SORT_ASCENDING);

    gtk_tree_view_set_model (GTK_TREE_VIEW (dlg->priv->displayed_treeview), sort_model);
    g_object_unref (G_OBJECT (sort_model));
    g_object_unref (G_OBJECT (dlg->priv->displayed_liststore));

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dlg->priv->displayed_treeview));
    gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_MULTIPLE);

    displayed_selection_changed_callback (selection, dlg);
    g_signal_connect (selection, "changed", G_CALLBACK (displayed_selection_changed_callback), dlg);
}

GtkWidget *
xed_encodings_dialog_new (void)
{
    GtkWidget *dlg;

    dlg = GTK_WIDGET (g_object_new (XED_TYPE_ENCODINGS_DIALOG, NULL));

    return dlg;
}

