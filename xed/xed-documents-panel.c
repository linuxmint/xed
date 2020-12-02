/*
 * xed-documents-panel.c
 * This file is part of xed
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/*
 * Modified by the xed Team, 2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#include <config.h>
#include <glib/gi18n.h>

#include "xed-documents-panel.h"
#include "xed-utils.h"
#include "xed-notebook.h"

struct _XedDocumentsPanelPrivate
{
    XedWindow  *window;

    GtkWidget    *treeview;
    GtkTreeModel *model;

    guint         adding_tab : 1;
    guint         is_reodering : 1;
};

G_DEFINE_TYPE_WITH_PRIVATE (XedDocumentsPanel, xed_documents_panel, GTK_TYPE_BOX)

enum
{
    PROP_0,
    PROP_WINDOW
};

enum
{
    PIXBUF_COLUMN,
    NAME_COLUMN,
    TAB_COLUMN,
    N_COLUMNS
};

#define MAX_DOC_NAME_LENGTH 60

static gchar *
tab_get_name (XedTab *tab)
{
    XedDocument *doc;
    gchar *name;
    gchar *docname;
    gchar *tab_name;

    g_return_val_if_fail (XED_IS_TAB (tab), NULL);

    doc = xed_tab_get_document (tab);

    name = xed_document_get_short_name_for_display (doc);

    /* Truncate the name so it doesn't get insanely wide. */
    docname = xed_utils_str_middle_truncate (name, MAX_DOC_NAME_LENGTH);

    if (gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (doc)))
    {
        if (xed_document_get_readonly (doc))
        {
            tab_name = g_markup_printf_escaped ("<i>%s</i> [<i>%s</i>]", docname, _("Read-Only"));
        }
        else
        {
            tab_name = g_markup_printf_escaped ("<i>%s</i>", docname);
        }
    }
    else
    {
        if (xed_document_get_readonly (doc))
        {
            tab_name = g_markup_printf_escaped ("%s [<i>%s</i>]", docname, _("Read-Only"));
        }
        else
        {
            tab_name = g_markup_escape_text (docname, -1);
        }
    }

    g_free (docname);
    g_free (name);

    return tab_name;
}

static void
get_iter_from_tab (XedDocumentsPanel *panel,
                   XedTab            *tab,
                   GtkTreeIter       *iter)
{
    gint num;
    GtkWidget *nb;
    GtkTreePath *path;

    nb = _xed_window_get_notebook (panel->priv->window);
    num = gtk_notebook_page_num (GTK_NOTEBOOK (nb), GTK_WIDGET (tab));

    path = gtk_tree_path_new_from_indices (num, -1);
    gtk_tree_model_get_iter (panel->priv->model, iter, path);
    gtk_tree_path_free (path);
}

static void
window_active_tab_changed (XedWindow         *window,
                           XedTab            *tab,
                           XedDocumentsPanel *panel)
{
    g_return_if_fail (tab != NULL);

    if (!_xed_window_is_removing_tabs (window))
    {
        GtkTreeIter iter;
        GtkTreeSelection *selection;

        get_iter_from_tab (panel, tab, &iter);

        if (gtk_list_store_iter_is_valid (GTK_LIST_STORE (panel->priv->model), &iter))
        {
            selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (panel->priv->treeview));

            gtk_tree_selection_select_iter (selection, &iter);
        }
    }
}

static void
refresh_list (XedDocumentsPanel *panel)
{
    /* TODO: refresh the list only if the panel is visible */

    GList *tabs;
    GList *l;
    GtkWidget *nb;
    GtkListStore *list_store;
    XedTab *active_tab;

    /* g_debug ("refresh_list"); */

    list_store = GTK_LIST_STORE (panel->priv->model);

    gtk_list_store_clear (list_store);

    active_tab = xed_window_get_active_tab (panel->priv->window);

    nb = _xed_window_get_notebook (panel->priv->window);

    tabs = gtk_container_get_children (GTK_CONTAINER (nb));
    l = tabs;

    panel->priv->adding_tab = TRUE;

    while (l != NULL)
    {
        GdkPixbuf *pixbuf;
        gchar *name;
        GtkTreeIter iter;

        name = tab_get_name (XED_TAB (l->data));
        pixbuf = _xed_tab_get_icon (XED_TAB (l->data));

        /* Add a new row to the model */
        gtk_list_store_append (list_store, &iter);
        gtk_list_store_set (list_store, &iter,
                            PIXBUF_COLUMN, pixbuf,
                            NAME_COLUMN, name,
                            TAB_COLUMN, l->data,
                            -1);

        g_free (name);
        if (pixbuf != NULL)
        {
            g_object_unref (pixbuf);
        }

        if (l->data == active_tab)
        {
            GtkTreeSelection *selection;

            selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (panel->priv->treeview));

            gtk_tree_selection_select_iter (selection, &iter);
        }

        l = g_list_next (l);
    }

    panel->priv->adding_tab = FALSE;

    g_list_free (tabs);
}

static void
sync_name_and_icon (XedTab            *tab,
                    GParamSpec        *pspec,
                    XedDocumentsPanel *panel)
{
    GdkPixbuf *pixbuf;
    gchar *name;
    GtkTreeIter iter;

    get_iter_from_tab (panel, tab, &iter);

    name = tab_get_name (tab);
    pixbuf = _xed_tab_get_icon (tab);

    gtk_list_store_set (GTK_LIST_STORE (panel->priv->model), &iter,
                        PIXBUF_COLUMN, pixbuf,
                        NAME_COLUMN, name,
                        TAB_COLUMN, tab,
                        -1);

    g_free (name);
    if (pixbuf != NULL)
    {
        g_object_unref (pixbuf);
    }
}

static void
window_tab_removed (XedWindow         *window,
                    XedTab            *tab,
                    XedDocumentsPanel *panel)
{
    g_signal_handlers_disconnect_by_func (tab, G_CALLBACK (sync_name_and_icon), panel);

    if (_xed_window_is_removing_tabs (window))
    {
        gtk_list_store_clear (GTK_LIST_STORE (panel->priv->model));
    }
    else
    {
        refresh_list (panel);
    }
}

static void
window_tab_added (XedWindow         *window,
                  XedTab            *tab,
                  XedDocumentsPanel *panel)
{
    GtkTreeIter iter;
    GtkTreeIter sibling;
    GdkPixbuf *pixbuf;
    gchar *name;

    g_signal_connect (tab, "notify::name", G_CALLBACK (sync_name_and_icon), panel);
    g_signal_connect (tab, "notify::state", G_CALLBACK (sync_name_and_icon), panel);

    get_iter_from_tab (panel, tab, &sibling);

    panel->priv->adding_tab = TRUE;

    if (gtk_list_store_iter_is_valid (GTK_LIST_STORE (panel->priv->model), &sibling))
    {
        gtk_list_store_insert_after (GTK_LIST_STORE (panel->priv->model), &iter, &sibling);
    }
    else
    {
        XedTab *active_tab;

        gtk_list_store_append (GTK_LIST_STORE (panel->priv->model), &iter);

        active_tab = xed_window_get_active_tab (panel->priv->window);

        if (tab == active_tab)
        {
            GtkTreeSelection *selection;

            selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (panel->priv->treeview));
            gtk_tree_selection_select_iter (selection, &iter);
        }
    }

    name = tab_get_name (tab);
    pixbuf = _xed_tab_get_icon (tab);

    gtk_list_store_set (GTK_LIST_STORE (panel->priv->model), &iter,
                        PIXBUF_COLUMN, pixbuf,
                        NAME_COLUMN, name,
                        TAB_COLUMN, tab,
                        -1);

    g_free (name);
    if (pixbuf != NULL)
    {
        g_object_unref (pixbuf);
    }

    panel->priv->adding_tab = FALSE;
}

static void
window_tabs_reordered (XedWindow         *window,
                       XedDocumentsPanel *panel)
{
    if (panel->priv->is_reodering)
    {
        return;
    }

    refresh_list (panel);
}

static void
set_window (XedDocumentsPanel *panel,
            XedWindow         *window)
{
    g_return_if_fail (panel->priv->window == NULL);
    g_return_if_fail (XED_IS_WINDOW (window));

    panel->priv->window = g_object_ref (window);

    g_signal_connect (window, "tab_added", G_CALLBACK (window_tab_added), panel);
    g_signal_connect (window, "tab_removed", G_CALLBACK (window_tab_removed), panel);
    g_signal_connect (window, "tabs_reordered", G_CALLBACK (window_tabs_reordered), panel);
    g_signal_connect (window, "active_tab_changed", G_CALLBACK (window_active_tab_changed), panel);
}

static void
treeview_cursor_changed (GtkTreeView       *view,
                         XedDocumentsPanel *panel)
{
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    gpointer tab;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (panel->priv->treeview));

    if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
        gtk_tree_model_get (panel->priv->model, &iter,
                            TAB_COLUMN, &tab,
                            -1);

        if (xed_window_get_active_tab (panel->priv->window) != tab)
        {
            xed_window_set_active_tab (panel->priv->window, XED_TAB (tab));
        }
    }
}

static void
xed_documents_panel_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
    XedDocumentsPanel *panel = XED_DOCUMENTS_PANEL (object);

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
xed_documents_panel_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
    XedDocumentsPanel *panel = XED_DOCUMENTS_PANEL (object);

    switch (prop_id)
    {
        case PROP_WINDOW:
            g_value_set_object (value, panel->priv->window);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_documents_panel_finalize (GObject *object)
{
    /* XedDocumentsPanel *tab = XED_DOCUMENTS_PANEL (object); */

    /* TODO: disconnect signal with window */

    G_OBJECT_CLASS (xed_documents_panel_parent_class)->finalize (object);
}

static void
xed_documents_panel_dispose (GObject *object)
{
    XedDocumentsPanel *panel = XED_DOCUMENTS_PANEL (object);

    if (panel->priv->window != NULL)
    {
        g_object_unref (panel->priv->window);
        panel->priv->window = NULL;
    }

    G_OBJECT_CLASS (xed_documents_panel_parent_class)->dispose (object);
}

static void
xed_documents_panel_class_init (XedDocumentsPanelClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = xed_documents_panel_finalize;
    object_class->dispose = xed_documents_panel_dispose;
    object_class->get_property = xed_documents_panel_get_property;
    object_class->set_property = xed_documents_panel_set_property;

    g_object_class_install_property (object_class,
                                     PROP_WINDOW,
                                     g_param_spec_object ("window",
                                                          "Window",
                                                          "The XedWindow this XedDocumentsPanel is associated with",
                                                          XED_TYPE_WINDOW,
                                                          G_PARAM_READWRITE |
                                                          G_PARAM_CONSTRUCT_ONLY |
                                                          G_PARAM_STATIC_STRINGS));
}

static GtkTreePath *
get_current_path (XedDocumentsPanel *panel)
{
    gint num;
    GtkWidget *nb;
    GtkTreePath *path;

    nb = _xed_window_get_notebook (panel->priv->window);
    num = gtk_notebook_get_current_page (GTK_NOTEBOOK (nb));

    path = gtk_tree_path_new_from_indices (num, -1);

    return path;
}

static gboolean
show_popup_menu (XedDocumentsPanel *panel,
                 GdkEventButton    *event)
{
    GtkWidget *menu;

    menu = gtk_ui_manager_get_widget (xed_window_get_ui_manager (panel->priv->window), "/NotebookPopup");
    g_return_val_if_fail (menu != NULL, FALSE);

    gtk_menu_popup_at_pointer (GTK_MENU (menu), (GdkEvent *) event);

    return TRUE;
}

static gboolean
panel_button_press_event (GtkTreeView       *treeview,
                          GdkEventButton    *event,
                          XedDocumentsPanel *panel)
{
    if ((GDK_BUTTON_PRESS == event->type) && (3 == event->button))
    {
        GtkTreePath* path = NULL;

        if (event->window == gtk_tree_view_get_bin_window (treeview))
        {
            /* Change the cursor position */
            if (gtk_tree_view_get_path_at_pos (treeview,
                                               event->x, event->y,
                                               &path, NULL,
                                               NULL, NULL))
            {

                gtk_tree_view_set_cursor (treeview, path, NULL, FALSE);

                gtk_tree_path_free (path);

                /* A row exists at mouse position */
                return show_popup_menu (panel, event);
            }
        }
    }

    return FALSE;
}

static gboolean
panel_popup_menu (GtkWidget         *treeview,
                  XedDocumentsPanel *panel)
{
    /* Only respond if the treeview is the actual focus */
    if (gtk_window_get_focus (GTK_WINDOW (panel->priv->window)) == treeview)
    {
        return show_popup_menu (panel, NULL);
    }

    return FALSE;
}

static gboolean
treeview_query_tooltip (GtkWidget  *widget,
                        gint        x,
                        gint        y,
                        gboolean    keyboard_tip,
                        GtkTooltip *tooltip,
                        gpointer    data)
{
    GtkTreeIter iter;
    GtkTreeView *tree_view = GTK_TREE_VIEW (widget);
    GtkTreeModel *model = gtk_tree_view_get_model (tree_view);
    GtkTreePath *path = NULL;
    gpointer *tab;
    gchar *tip;

    if (keyboard_tip)
    {
        gtk_tree_view_get_cursor (tree_view, &path, NULL);

        if (path == NULL)
        {
            return FALSE;
        }
    }
    else
    {
        gint bin_x, bin_y;

        gtk_tree_view_convert_widget_to_bin_window_coords (tree_view, x, y, &bin_x, &bin_y);

        if (!gtk_tree_view_get_path_at_pos (tree_view,
                                            bin_x, bin_y,
                                            &path, NULL,
                                            NULL, NULL))
        {
            return FALSE;
        }
    }

    gtk_tree_model_get_iter (model, &iter, path);
    gtk_tree_model_get (model, &iter, TAB_COLUMN, &tab, -1);

    tip = _xed_tab_get_tooltips (XED_TAB (tab));
    gtk_tooltip_set_markup (tooltip, tip);

    g_free (tip);
    gtk_tree_path_free (path);

    return TRUE;
}

static void
treeview_row_inserted (GtkTreeModel      *tree_model,
                       GtkTreePath       *path,
                       GtkTreeIter       *iter,
                       XedDocumentsPanel *panel)
{
    XedTab *tab;
    gint *indeces;
    GtkWidget *nb;
    gint old_position;
    gint new_position;

    if (panel->priv->adding_tab)
    {
        return;
    }

    tab = xed_window_get_active_tab (panel->priv->window);
    g_return_if_fail (tab != NULL);

    panel->priv->is_reodering = TRUE;

    indeces = gtk_tree_path_get_indices (path);

    /* g_debug ("New Index: %d (path: %s)", indeces[0], gtk_tree_path_to_string (path));*/

    nb = _xed_window_get_notebook (panel->priv->window);

    new_position = indeces[0];
    old_position = gtk_notebook_page_num (GTK_NOTEBOOK (nb), GTK_WIDGET (tab));
    if (new_position > old_position)
    {
        new_position = MAX (0, new_position - 1);
    }

    xed_notebook_reorder_tab (XED_NOTEBOOK (nb), tab, new_position);

    panel->priv->is_reodering = FALSE;
}

static void
xed_documents_panel_init (XedDocumentsPanel *panel)
{
    GtkWidget *sw;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeSelection *selection;

    panel->priv = xed_documents_panel_get_instance_private (panel);

    panel->priv->adding_tab = FALSE;
    panel->priv->is_reodering = FALSE;

    gtk_orientable_set_orientation (GTK_ORIENTABLE (panel), GTK_ORIENTATION_VERTICAL);

    /* Create the scrolled window */
    sw = gtk_scrolled_window_new (NULL, NULL);
    g_return_if_fail (sw != NULL);

    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_show (sw);
    gtk_box_pack_start (GTK_BOX (panel), sw, TRUE, TRUE, 0);

    /* Create the empty model */
    panel->priv->model = GTK_TREE_MODEL (gtk_list_store_new (N_COLUMNS,
                                         GDK_TYPE_PIXBUF,
                                         G_TYPE_STRING,
                                         G_TYPE_POINTER));

    /* Create the treeview */
    panel->priv->treeview = gtk_tree_view_new_with_model (panel->priv->model);
    g_object_unref (G_OBJECT (panel->priv->model));
    gtk_container_add (GTK_CONTAINER (sw), panel->priv->treeview);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (panel->priv->treeview), FALSE);
    gtk_tree_view_set_reorderable (GTK_TREE_VIEW (panel->priv->treeview), TRUE);

    /* Disable search because each time the selection is changed, the
       active tab is changed which focusses the view, and thus would remove
       the search entry, rendering it useless */
    gtk_tree_view_set_enable_search (GTK_TREE_VIEW (panel->priv->treeview), FALSE);

    /* Disable focus so it doesn't steel focus each time from the view */
    gtk_widget_set_can_focus (panel->priv->treeview, FALSE);

    g_object_set (panel->priv->treeview, "has-tooltip", TRUE, NULL);

    gtk_widget_show (panel->priv->treeview);

    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_title (column, _("Documents"));

    cell = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_add_attribute (column, cell, "pixbuf", PIXBUF_COLUMN);
    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, cell, TRUE);
    gtk_tree_view_column_add_attribute (column, cell, "markup", NAME_COLUMN);

    gtk_tree_view_append_column (GTK_TREE_VIEW (panel->priv->treeview), column);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (panel->priv->treeview));

    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

    g_signal_connect (panel->priv->treeview, "cursor_changed", G_CALLBACK (treeview_cursor_changed), panel);
    g_signal_connect (panel->priv->treeview, "button-press-event", G_CALLBACK (panel_button_press_event), panel);
    g_signal_connect (panel->priv->treeview, "popup-menu", G_CALLBACK (panel_popup_menu), panel);
    g_signal_connect (panel->priv->treeview, "query-tooltip", G_CALLBACK (treeview_query_tooltip), NULL);
    g_signal_connect (panel->priv->model, "row-inserted", G_CALLBACK (treeview_row_inserted), panel);
}

GtkWidget *
xed_documents_panel_new (XedWindow *window)
{
    g_return_val_if_fail (XED_IS_WINDOW (window), NULL);

    return GTK_WIDGET (g_object_new (XED_TYPE_DOCUMENTS_PANEL,
                     "window", window,
                     NULL));
}
