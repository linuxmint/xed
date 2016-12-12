/*
 * xed-close-confirmation-dialog.c
 * This file is part of xed
 *
 * Copyright (C) 2004-2005 GNOME Foundation
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
 * Modified by the xed Team, 2004-2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>

#include "xed-close-confirmation-dialog.h"
#include <xed/xed-app.h>
#include <xed/xed-utils.h>
#include <xed/xed-window.h>


/* Properties */
enum
{
    PROP_0,
    PROP_UNSAVED_DOCUMENTS,
    PROP_LOGOUT_MODE
};

/* Mode */
enum
{
    SINGLE_DOC_MODE,
    MULTIPLE_DOCS_MODE
};

/* Columns */
enum
{
    SAVE_COLUMN,
    NAME_COLUMN,
    DOC_COLUMN, /* a handy pointer to the document */
    N_COLUMNS
};

struct _XedCloseConfirmationDialogPrivate
{
    gboolean      logout_mode;
    GList        *unsaved_documents;
    GList        *selected_documents;
    GtkTreeModel *list_store;
};

#define XED_CLOSE_CONFIRMATION_DIALOG_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                                      XED_TYPE_CLOSE_CONFIRMATION_DIALOG, \
                                                      XedCloseConfirmationDialogPrivate))

#define GET_MODE(priv) (((priv->unsaved_documents != NULL) && \
                        (priv->unsaved_documents->next == NULL)) ? \
                        SINGLE_DOC_MODE : MULTIPLE_DOCS_MODE)

G_DEFINE_TYPE(XedCloseConfirmationDialog, xed_close_confirmation_dialog, GTK_TYPE_DIALOG)

static void  set_unsaved_document (XedCloseConfirmationDialog *dlg,
                                   const GList                *list);
static GList *get_selected_docs (GtkTreeModel *store);

/*  Since we connect in the costructor we are sure this handler will be called
 *  before the user ones
 */
static void
response_cb (XedCloseConfirmationDialog *dlg,
             gint                        response_id,
             gpointer                    data)
{
    XedCloseConfirmationDialogPrivate *priv;

    g_return_if_fail (XED_IS_CLOSE_CONFIRMATION_DIALOG (dlg));

    priv = dlg->priv;

    if (priv->selected_documents != NULL)
    {
        g_list_free (priv->selected_documents);
    }

    if (response_id == GTK_RESPONSE_YES)
    {
        if (GET_MODE (priv) == SINGLE_DOC_MODE)
        {
            priv->selected_documents = g_list_copy (priv->unsaved_documents);
        }
        else
        {
            g_return_if_fail (priv->list_store);

            priv->selected_documents = get_selected_docs (priv->list_store);
        }
    }
    else
        priv->selected_documents = NULL;
}

static void
set_logout_mode (XedCloseConfirmationDialog *dlg,
                 gboolean                    logout_mode)
{
    dlg->priv->logout_mode = logout_mode;

    if (logout_mode)
    {
        gtk_dialog_add_button (GTK_DIALOG (dlg), _("Log Out _without Saving"), GTK_RESPONSE_NO);
        xed_dialog_add_button (GTK_DIALOG (dlg), _("_Cancel Logout"), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
    }
    else
    {
        gtk_dialog_add_button (GTK_DIALOG (dlg), _("Close _without Saving"), GTK_RESPONSE_NO);
        gtk_dialog_add_button (GTK_DIALOG (dlg), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
    }


    const gchar *stock_id = GTK_STOCK_SAVE;

    if (GET_MODE (dlg->priv) == SINGLE_DOC_MODE)
    {
        XedDocument *doc;

        doc = XED_DOCUMENT (dlg->priv->unsaved_documents->data);

        if (xed_document_get_readonly (doc) || xed_document_is_untitled (doc))
        {
            stock_id = GTK_STOCK_SAVE_AS;
        }
    }

    gtk_dialog_add_button (GTK_DIALOG (dlg), stock_id, GTK_RESPONSE_YES);
    gtk_dialog_set_default_response (GTK_DIALOG (dlg), GTK_RESPONSE_YES);
}

static void
xed_close_confirmation_dialog_init (XedCloseConfirmationDialog *dlg)
{
    AtkObject *atk_obj;

    dlg->priv = XED_CLOSE_CONFIRMATION_DIALOG_GET_PRIVATE (dlg);

    gtk_container_set_border_width (GTK_CONTAINER (dlg), 5);
    gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dlg))), 14);
    gtk_window_set_resizable (GTK_WINDOW (dlg), FALSE);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dlg), TRUE);

    gtk_window_set_title (GTK_WINDOW (dlg), "");

    gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
    gtk_window_set_destroy_with_parent (GTK_WINDOW (dlg), TRUE);

    atk_obj = gtk_widget_get_accessible (GTK_WIDGET (dlg));
    atk_object_set_role (atk_obj, ATK_ROLE_ALERT);
    atk_object_set_name (atk_obj, _("Question"));

    g_signal_connect (dlg, "response", G_CALLBACK (response_cb), NULL);
}

static void
xed_close_confirmation_dialog_finalize (GObject *object)
{
    XedCloseConfirmationDialogPrivate *priv;

    priv = XED_CLOSE_CONFIRMATION_DIALOG (object)->priv;

    if (priv->unsaved_documents != NULL)
    {
        g_list_free (priv->unsaved_documents);
    }

    if (priv->selected_documents != NULL)
    {
        g_list_free (priv->selected_documents);
    }

    /* Call the parent's destructor */
    G_OBJECT_CLASS (xed_close_confirmation_dialog_parent_class)->finalize (object);
}

static void
xed_close_confirmation_dialog_set_property (GObject      *object,
                                            guint         prop_id,
                                            const GValue *value,
                                            GParamSpec   *pspec)
{
    XedCloseConfirmationDialog *dlg;

    dlg = XED_CLOSE_CONFIRMATION_DIALOG (object);

    switch (prop_id)
    {
        case PROP_UNSAVED_DOCUMENTS:
            set_unsaved_document (dlg, g_value_get_pointer (value));
            break;

        case PROP_LOGOUT_MODE:
            set_logout_mode (dlg, g_value_get_boolean (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_close_confirmation_dialog_get_property (GObject    *object,
                                            guint       prop_id,
                                            GValue     *value,
                                            GParamSpec *pspec)
{
    XedCloseConfirmationDialogPrivate *priv;

    priv = XED_CLOSE_CONFIRMATION_DIALOG (object)->priv;

    switch( prop_id )
    {
        case PROP_UNSAVED_DOCUMENTS:
            g_value_set_pointer (value, priv->unsaved_documents);
            break;

        case PROP_LOGOUT_MODE:
            g_value_set_boolean (value, priv->logout_mode);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_close_confirmation_dialog_class_init (XedCloseConfirmationDialogClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = xed_close_confirmation_dialog_set_property;
    gobject_class->get_property = xed_close_confirmation_dialog_get_property;
    gobject_class->finalize = xed_close_confirmation_dialog_finalize;

    g_type_class_add_private (klass, sizeof (XedCloseConfirmationDialogPrivate));

    g_object_class_install_property (gobject_class,
                                     PROP_UNSAVED_DOCUMENTS,
                                     g_param_spec_pointer ("unsaved_documents",
                                                           "Unsaved Documents",
                                                           "List of Unsaved Documents",
                                                           (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY)));

    g_object_class_install_property (gobject_class,
                                     PROP_LOGOUT_MODE,
                                     g_param_spec_boolean ("logout_mode",
                                                           "Logout Mode",
                                                           "Whether the dialog is in logout mode",
                                                           FALSE,
                                                           (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY)));
}

static GList *
get_selected_docs (GtkTreeModel *store)
{
    GList      *list;
    gboolean     valid;
    GtkTreeIter  iter;

    list = NULL;
    valid = gtk_tree_model_get_iter_first (store, &iter);

    while (valid)
    {
        gboolean to_save;
        XedDocument *doc;

        gtk_tree_model_get (store, &iter, SAVE_COLUMN, &to_save, DOC_COLUMN, &doc, -1);
        if (to_save)
        {
            list = g_list_prepend (list, doc);
        }

        valid = gtk_tree_model_iter_next (store, &iter);
    }

    list = g_list_reverse (list);

    return list;
}

GList *
xed_close_confirmation_dialog_get_selected_documents (XedCloseConfirmationDialog *dlg)
{
    g_return_val_if_fail (XED_IS_CLOSE_CONFIRMATION_DIALOG (dlg), NULL);

    return g_list_copy (dlg->priv->selected_documents);
}

GtkWidget *
xed_close_confirmation_dialog_new (GtkWindow *parent,
                                   GList     *unsaved_documents,
                                   gboolean   logout_mode)
{
    GtkWidget *dlg;
    g_return_val_if_fail (unsaved_documents != NULL, NULL);

    dlg = GTK_WIDGET (g_object_new (XED_TYPE_CLOSE_CONFIRMATION_DIALOG,
                      "unsaved_documents", unsaved_documents,
                      "logout_mode", logout_mode,
                      NULL));
    g_return_val_if_fail (dlg != NULL, NULL);

    if (parent != NULL)
    {
        gtk_window_group_add_window (xed_window_get_group (XED_WINDOW (parent)), GTK_WINDOW (dlg));
        gtk_window_set_transient_for (GTK_WINDOW (dlg), parent);
    }

    return dlg;
}

GtkWidget *
xed_close_confirmation_dialog_new_single (GtkWindow     *parent,
                                          XedDocument *doc,
                                          gboolean       logout_mode)
{
    GtkWidget *dlg;
    GList *unsaved_documents;
    g_return_val_if_fail (doc != NULL, NULL);

    unsaved_documents = g_list_prepend (NULL, doc);

    dlg = xed_close_confirmation_dialog_new (parent, unsaved_documents, logout_mode);

    g_list_free (unsaved_documents);

    return dlg;
}

static gchar *
get_text_secondary_label (XedDocument *doc)
{
    glong  seconds;
    gchar *secondary_msg;

    seconds = MAX (1, _xed_document_get_seconds_since_last_save_or_load (doc));

    if (seconds < 55)
    {
        secondary_msg = g_strdup_printf (ngettext ("If you don't save, changes from the last %ld second "
                                                   "will be permanently lost.",
                                                   "If you don't save, changes from the last %ld seconds "
                                                   "will be permanently lost.",
                                                   seconds),
                                                   seconds);
    }
    else if (seconds < 75) /* 55 <= seconds < 75 */
    {
        secondary_msg = g_strdup (_("If you don't save, changes from the last minute "
                                    "will be permanently lost."));
    }
    else if (seconds < 110) /* 75 <= seconds < 110 */
    {
        secondary_msg = g_strdup_printf (ngettext ("If you don't save, changes from the last minute and %ld "
                                                   "second will be permanently lost.",
                                                   "If you don't save, changes from the last minute and %ld "
                                                   "seconds will be permanently lost.",
                                                   seconds - 60 ),
                                                   seconds - 60);
    }
    else if (seconds < 3600)
    {
        secondary_msg = g_strdup_printf (ngettext ("If you don't save, changes from the last %ld minute "
                                                   "will be permanently lost.",
                                                   "If you don't save, changes from the last %ld minutes "
                                                   "will be permanently lost.",
                                                   seconds / 60),
                                                   seconds / 60);
    }
    else if (seconds < 7200)
    {
        gint minutes;
        seconds -= 3600;

        minutes = seconds / 60;
        if (minutes < 5)
        {
            secondary_msg = g_strdup (_("If you don't save, changes from the last hour "
                                        "will be permanently lost."));
        }
        else
        {
            secondary_msg = g_strdup_printf (ngettext ("If you don't save, changes from the last hour and %d "
                                                       "minute will be permanently lost.",
                                                       "If you don't save, changes from the last hour and %d "
                                                       "minutes will be permanently lost.",
                                                       minutes),
                                                       minutes);
        }
    }
    else
    {
        gint hours;

        hours = seconds / 3600;

        secondary_msg = g_strdup_printf (ngettext ("If you don't save, changes from the last %d hour "
                                                   "will be permanently lost.",
                                                   "If you don't save, changes from the last %d hours "
                                                   "will be permanently lost.",
                                                   hours),
                                                   hours);
    }

    return secondary_msg;
}

static void
build_single_doc_dialog (XedCloseConfirmationDialog *dlg)
{
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *primary_label;
    GtkWidget *secondary_label;
    GtkWidget *image;
    XedDocument *doc;
    gchar *doc_name;
    gchar *str;
    gchar *markup_str;

    g_return_if_fail (dlg->priv->unsaved_documents->data != NULL);
    doc = XED_DOCUMENT (dlg->priv->unsaved_documents->data);

    /* Image */
    image = gtk_image_new_from_icon_name ("dialog-warning", GTK_ICON_SIZE_DIALOG);
    gtk_widget_set_halign (image, GTK_ALIGN_START);
    gtk_widget_set_valign (image, GTK_ALIGN_END);

    /* Primary label */
    primary_label = gtk_label_new (NULL);
    gtk_label_set_line_wrap (GTK_LABEL (primary_label), TRUE);
    gtk_label_set_use_markup (GTK_LABEL (primary_label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (primary_label), 0.0, 0.5);
    gtk_label_set_selectable (GTK_LABEL (primary_label), TRUE);
    gtk_widget_set_can_focus (GTK_WIDGET (primary_label), FALSE);

    doc_name = xed_document_get_short_name_for_display (doc);

    str = g_markup_printf_escaped (_("Save changes to document \"%s\" before closing?"), doc_name);

    g_free (doc_name);

    markup_str = g_strconcat ("<span weight=\"bold\" size=\"larger\">", str, "</span>", NULL);
    g_free (str);

    gtk_label_set_markup (GTK_LABEL (primary_label), markup_str);
    g_free (markup_str);

    /* Secondary label */
    str = get_text_secondary_label (doc);
    secondary_label = gtk_label_new (str);
    g_free (str);
    gtk_label_set_line_wrap (GTK_LABEL (secondary_label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (secondary_label), 0.0, 0.5);
    gtk_label_set_selectable (GTK_LABEL (secondary_label), TRUE);
    gtk_widget_set_can_focus (GTK_WIDGET (secondary_label), FALSE);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

    gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);

    gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), primary_label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), secondary_label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dlg))), hbox, FALSE, FALSE, 0);

    gtk_widget_show_all (hbox);
}

static void
populate_model (GtkTreeModel *store,
                GList        *docs)
{
    GtkTreeIter iter;

    while (docs != NULL)
    {
        XedDocument *doc;
        gchar *name;

        doc = XED_DOCUMENT (docs->data);

        name = xed_document_get_short_name_for_display (doc);

        gtk_list_store_append (GTK_LIST_STORE (store), &iter);
        gtk_list_store_set (GTK_LIST_STORE (store), &iter,
                            SAVE_COLUMN, TRUE,
                            NAME_COLUMN, name,
                            DOC_COLUMN, doc,
                            -1);

        g_free (name);

        docs = g_list_next (docs);
    }
}

static void
save_toggled (GtkCellRendererToggle *renderer,
              gchar                 *path_str,
              GtkTreeModel          *store)
{
    GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
    GtkTreeIter iter;
    gboolean active;

    gtk_tree_model_get_iter (store, &iter, path);
    gtk_tree_model_get (store, &iter, SAVE_COLUMN, &active, -1);

    active ^= 1;

    gtk_list_store_set (GTK_LIST_STORE (store), &iter, SAVE_COLUMN, active, -1);

    gtk_tree_path_free (path);
}

static GtkWidget *
create_treeview (XedCloseConfirmationDialogPrivate *priv)
{
    GtkListStore *store;
    GtkWidget *treeview;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    treeview = gtk_tree_view_new ();
    gtk_widget_set_size_request (treeview, 260, 120);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
    gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview), FALSE);

    /* Create and populate the model */
    store = gtk_list_store_new (N_COLUMNS, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_POINTER);
    populate_model (GTK_TREE_MODEL (store), priv->unsaved_documents);

    /* Set model to the treeview */
    gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (store));
    g_object_unref (store);

    priv->list_store = GTK_TREE_MODEL (store);

    /* Add columns */

    renderer = gtk_cell_renderer_toggle_new ();
    g_signal_connect (renderer, "toggled", G_CALLBACK (save_toggled), store);

    column = gtk_tree_view_column_new_with_attributes ("Save?",
                                                       renderer,
                                                       "active",
                                                       SAVE_COLUMN,
                                                       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Name",
                                                       renderer,
                                                       "text",
                                                       NAME_COLUMN,
                                                       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

    return treeview;
}

static void
build_multiple_docs_dialog (XedCloseConfirmationDialog *dlg)
{
    XedCloseConfirmationDialogPrivate *priv;
    GtkWidget *hbox;
    GtkWidget *image;
    GtkWidget *vbox;
    GtkWidget *primary_label;
    GtkWidget *vbox2;
    GtkWidget *select_label;
    GtkWidget *scrolledwindow;
    GtkWidget *treeview;
    GtkWidget *secondary_label;
    gchar *str;
    gchar *markup_str;

    priv = dlg->priv;

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dlg))), hbox, TRUE, TRUE, 0);

    /* Image */
    image = gtk_image_new_from_icon_name ("dialog-warning", GTK_ICON_SIZE_DIALOG);
    gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (image, GTK_ALIGN_START);
    gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

    /* Primary label */
    primary_label = gtk_label_new (NULL);
    gtk_label_set_line_wrap (GTK_LABEL (primary_label), TRUE);
    gtk_label_set_use_markup (GTK_LABEL (primary_label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (primary_label), 0.0, 0.5);
    gtk_label_set_selectable (GTK_LABEL (primary_label), TRUE);

    str = g_strdup_printf (ngettext ("There is %d document with unsaved changes. "
                                     "Save changes before closing?",
                                     "There are %d documents with unsaved changes. "
                                     "Save changes before closing?",
                                     g_list_length (priv->unsaved_documents)),
                                     g_list_length (priv->unsaved_documents));

    markup_str = g_strconcat ("<span weight=\"bold\" size=\"larger\">", str, "</span>", NULL);
    g_free (str);

    gtk_label_set_markup (GTK_LABEL (primary_label), markup_str);
    g_free (markup_str);
    gtk_box_pack_start (GTK_BOX (vbox), primary_label, FALSE, FALSE, 0);

    vbox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
    gtk_box_pack_start (GTK_BOX (vbox), vbox2, FALSE, FALSE, 0);

    select_label = gtk_label_new_with_mnemonic (_("S_elect the documents you want to save:"));

    gtk_box_pack_start (GTK_BOX (vbox2), select_label, FALSE, FALSE, 0);
    gtk_label_set_line_wrap (GTK_LABEL (select_label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (select_label), 0.0, 0.5);

    scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_box_pack_start (GTK_BOX (vbox2), scrolledwindow, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_SHADOW_IN);
    gtk_scrolled_window_set_min_content_height (GTK_SCROLLED_WINDOW (scrolledwindow), 60);

    treeview = create_treeview (priv);
    gtk_container_add (GTK_CONTAINER (scrolledwindow), treeview);

    /* Secondary label */
    secondary_label = gtk_label_new (_("If you don't save, "
                                       "all your changes will be permanently lost."));

    gtk_box_pack_start (GTK_BOX (vbox2), secondary_label, FALSE, FALSE, 0);
    gtk_label_set_line_wrap (GTK_LABEL (secondary_label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (secondary_label), 0.0, 0.5);
    gtk_label_set_selectable (GTK_LABEL (secondary_label), TRUE);

    gtk_label_set_mnemonic_widget (GTK_LABEL (select_label), treeview);

    gtk_widget_show_all (hbox);
}

static void
set_unsaved_document (XedCloseConfirmationDialog *dlg,
                      const GList                *list)
{
    XedCloseConfirmationDialogPrivate *priv;

    g_return_if_fail (list != NULL);

    priv = dlg->priv;
    g_return_if_fail (priv->unsaved_documents == NULL);

    priv->unsaved_documents = g_list_copy ((GList *)list);

    if (GET_MODE (priv) == SINGLE_DOC_MODE)
    {
        build_single_doc_dialog (dlg);
    }
    else
    {
        build_multiple_docs_dialog (dlg);
    }
}

const GList *
xed_close_confirmation_dialog_get_unsaved_documents (XedCloseConfirmationDialog *dlg)
{
    g_return_val_if_fail (XED_IS_CLOSE_CONFIRMATION_DIALOG (dlg), NULL);

    return dlg->priv->unsaved_documents;
}

