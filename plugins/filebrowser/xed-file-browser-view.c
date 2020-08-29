/*
 * xed-file-browser-view.c - Xed plugin providing easy file access
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

#include <string.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "xed-file-browser-store.h"
#include "xed-file-bookmarks-store.h"
#include "xed-file-browser-view.h"
#include "xed-file-browser-marshal.h"
#include "xed-file-browser-enum-types.h"

struct _XedFileBrowserViewPrivate
{
    GtkTreeViewColumn *column;
    GtkCellRenderer *pixbuf_renderer;
    GtkCellRenderer *text_renderer;

    GtkTreeModel *model;
    GtkTreeRowReference *editable;

    GtkTreePath *double_click_path[2]; /* Both clicks in a double click need to be on the same row */
    GtkTreePath *hover_path;
    GdkCursor *hand_cursor;
    gboolean ignore_release;
    gboolean selected_on_button_down;
    gint drag_button;
    gboolean drag_started;

    gboolean restore_expand_state;
    gboolean is_refresh;
    GHashTable * expand_state;
};

/* Properties */
enum
{
    PROP_0,

    PROP_RESTORE_EXPAND_STATE
};

/* Signals */
enum
{
    ERROR,
    FILE_ACTIVATED,
    DIRECTORY_ACTIVATED,
    BOOKMARK_ACTIVATED,
    NUM_SIGNALS
};

static guint signals[NUM_SIGNALS] = { 0 };

static const GtkTargetEntry drag_source_targets[] = {
    { "text/uri-list", 0, 0 }
};

G_DEFINE_DYNAMIC_TYPE_EXTENDED (XedFileBrowserView,
                                xed_file_browser_view,
                                GTK_TYPE_TREE_VIEW,
                                0,
                                G_ADD_PRIVATE_DYNAMIC (XedFileBrowserView))

static void on_cell_edited (GtkCellRendererText *cell,
                            gchar               *path,
                            gchar               *new_text,
                            XedFileBrowserView  *tree_view);

static void on_begin_refresh (XedFileBrowserStore *model,
                              XedFileBrowserView  *view);
static void on_end_refresh (XedFileBrowserStore *model,
                            XedFileBrowserView  *view);

static void on_unload (XedFileBrowserStore *model,
                       GFile               *location,
                       XedFileBrowserView  *view);

static void on_row_inserted (XedFileBrowserStore *model,
                             GtkTreePath         *path,
                             GtkTreeIter         *iter,
                             XedFileBrowserView  *view);

static void
xed_file_browser_view_finalize (GObject *object)
{
    XedFileBrowserView *obj = XED_FILE_BROWSER_VIEW(object);

    if (obj->priv->hand_cursor)
    {
        g_object_unref (obj->priv->hand_cursor);
    }

    if (obj->priv->hover_path)
    {
        gtk_tree_path_free (obj->priv->hover_path);
    }

    if (obj->priv->expand_state)
    {
        g_hash_table_destroy (obj->priv->expand_state);
        obj->priv->expand_state = NULL;
    }

    G_OBJECT_CLASS (xed_file_browser_view_parent_class)->finalize (object);
}

static void
add_expand_state (XedFileBrowserView *view,
                  GFile              *location)
{
    if (!location)
    {
        return;
    }

    if (view->priv->expand_state)
    {
        g_hash_table_insert (view->priv->expand_state, location, g_object_ref (location));
    }
}

static void
remove_expand_state (XedFileBrowserView *view,
                     GFile              *location)
{
    if (!location)
    {
        return;
    }

    if (view->priv->expand_state)
    {
        g_hash_table_remove (view->priv->expand_state, location);
    }
}

static void
row_expanded (GtkTreeView *tree_view,
              GtkTreeIter *iter,
              GtkTreePath *path)
{
    XedFileBrowserView *view = XED_FILE_BROWSER_VIEW (tree_view);

    if (GTK_TREE_VIEW_CLASS (xed_file_browser_view_parent_class)->row_expanded)
    {
        GTK_TREE_VIEW_CLASS (xed_file_browser_view_parent_class)->row_expanded (tree_view, iter, path);
    }

    if (!XED_IS_FILE_BROWSER_STORE (view->priv->model))
    {
        return;
    }

    if (view->priv->restore_expand_state)
    {
        GFile *location;

        gtk_tree_model_get (view->priv->model, iter, XED_FILE_BROWSER_STORE_COLUMN_LOCATION, &location, -1);

        add_expand_state (view, location);

        if (location)
        {
            g_object_unref (location);
        }
    }

    _xed_file_browser_store_iter_expanded (XED_FILE_BROWSER_STORE (view->priv->model), iter);
}

static void
row_collapsed (GtkTreeView *tree_view,
               GtkTreeIter *iter,
               GtkTreePath *path)
{
    XedFileBrowserView *view = XED_FILE_BROWSER_VIEW (tree_view);

    if (GTK_TREE_VIEW_CLASS (xed_file_browser_view_parent_class)->row_collapsed)
    {
        GTK_TREE_VIEW_CLASS (xed_file_browser_view_parent_class)->row_collapsed (tree_view, iter, path);
    }

    if (!XED_IS_FILE_BROWSER_STORE (view->priv->model))
    {
        return;
    }

    if (view->priv->restore_expand_state)
    {
        GFile *location;

        gtk_tree_model_get (view->priv->model, iter, XED_FILE_BROWSER_STORE_COLUMN_LOCATION, &location, -1);

        remove_expand_state (view, location);

        if (location)
        {
            g_object_unref (location);
        }
    }

    _xed_file_browser_store_iter_collapsed (XED_FILE_BROWSER_STORE (view->priv->model), iter);
}

static void
directory_activated (XedFileBrowserView *view,
                     GtkTreeIter        *iter)
{
    xed_file_browser_store_set_virtual_root (XED_FILE_BROWSER_STORE (view->priv->model), iter);
}

static void
activate_selected_files (XedFileBrowserView *view)
{
    GtkTreeView *tree_view = GTK_TREE_VIEW (view);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (tree_view);
    GList *rows, *row;
    GtkTreePath *directory = NULL;
    GtkTreePath *path;
    GtkTreeIter iter;
    XedFileBrowserStoreFlag flags;

    rows = gtk_tree_selection_get_selected_rows (selection, &view->priv->model);

    for (row = rows; row; row = row->next)
    {
        path = (GtkTreePath *)(row->data);

        /* Get iter from path */
        if (!gtk_tree_model_get_iter (view->priv->model, &iter, path))
        {
            continue;
        }

        gtk_tree_model_get (view->priv->model, &iter, XED_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags, -1);

        if (FILE_IS_DIR (flags))
        {
            if (directory == NULL)
            {
                directory = path;
            }

        }
        else if (!FILE_IS_DUMMY (flags))
        {
            g_signal_emit (view, signals[FILE_ACTIVATED], 0, &iter);
        }
    }

    if (directory != NULL)
    {
        if (gtk_tree_model_get_iter (view->priv->model, &iter, directory))
        {
            g_signal_emit (view, signals[DIRECTORY_ACTIVATED], 0, &iter);
        }
    }

    g_list_foreach (rows, (GFunc)gtk_tree_path_free, NULL);
    g_list_free (rows);
}

static void
activate_selected_bookmark (XedFileBrowserView *view)
{
    GtkTreeView *tree_view = GTK_TREE_VIEW (view);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (tree_view);
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected (selection, &view->priv->model, &iter))
    {
        g_signal_emit (view, signals[BOOKMARK_ACTIVATED], 0, &iter);
    }
}

static void
activate_selected_items (XedFileBrowserView *view)
{
    if (XED_IS_FILE_BROWSER_STORE (view->priv->model))
    {
        activate_selected_files (view);
    }
    else if (XED_IS_FILE_BOOKMARKS_STORE (view->priv->model))
    {
        activate_selected_bookmark (view);
    }
}

static void
row_activated (GtkTreeView       *tree_view,
               GtkTreePath       *path,
               GtkTreeViewColumn *column)
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection (tree_view);

    /* Make sure the activated row is the only one selected */
    gtk_tree_selection_unselect_all (selection);
    gtk_tree_selection_select_path (selection, path);

    activate_selected_items (XED_FILE_BROWSER_VIEW (tree_view));
}

static void
toggle_hidden_filter (XedFileBrowserView *view)
{
    XedFileBrowserStoreFilterMode mode;

    if (XED_IS_FILE_BROWSER_STORE (view->priv->model))
    {
        mode = xed_file_browser_store_get_filter_mode (XED_FILE_BROWSER_STORE (view->priv->model));
        mode ^= XED_FILE_BROWSER_STORE_FILTER_MODE_HIDE_HIDDEN;
        xed_file_browser_store_set_filter_mode (XED_FILE_BROWSER_STORE (view->priv->model), mode);
    }
}

static gboolean
button_event_modifies_selection (GdkEventButton *event)
{
    return (event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) != 0;
}

static void
drag_begin (GtkWidget      *widget,
            GdkDragContext *context)
{
    XedFileBrowserView *view = XED_FILE_BROWSER_VIEW (widget);

    view->priv->drag_button = 0;
    view->priv->drag_started = TRUE;

    /* Chain up */
    GTK_WIDGET_CLASS (xed_file_browser_view_parent_class)->drag_begin (widget, context);
}

static void
did_not_drag (XedFileBrowserView *view,
              GdkEventButton     *event)
{
    GtkTreeView *tree_view;
    GtkTreeSelection *selection;
    GtkTreePath *path;

    tree_view = GTK_TREE_VIEW (view);
    selection = gtk_tree_view_get_selection (tree_view);

    if (gtk_tree_view_get_path_at_pos (tree_view, event->x, event->y, &path, NULL, NULL, NULL))
    {
        if ((event->button == 1 || event->button == 2)
                 && ((event->state & GDK_CONTROL_MASK) != 0 ||
                 (event->state & GDK_SHIFT_MASK) == 0)
                 && view->priv->selected_on_button_down)
        {
            if (!button_event_modifies_selection (event))
            {
                gtk_tree_selection_unselect_all (selection);
                gtk_tree_selection_select_path (selection, path);
            }
            else
            {
                gtk_tree_selection_unselect_path (selection, path);
            }
        }

        gtk_tree_path_free (path);
    }
}

static gboolean
button_release_event (GtkWidget      *widget,
                      GdkEventButton *event)
{
    XedFileBrowserView *view = XED_FILE_BROWSER_VIEW (widget);

    if (event->button == view->priv->drag_button)
    {
        view->priv->drag_button = 0;

        if (!view->priv->drag_started && !view->priv->ignore_release)
        {
            did_not_drag (view, event);
        }
    }

    /* Chain up */
    return GTK_WIDGET_CLASS (xed_file_browser_view_parent_class)->button_release_event (widget, event);
}

static gboolean
button_press_event (GtkWidget      *widget,
                    GdkEventButton *event)
{
    int double_click_time;
    static int click_count = 0;
    static guint32 last_click_time = 0;
    XedFileBrowserView *view;
    GtkTreeView *tree_view;
    GtkTreeSelection *selection;
    GtkTreePath *path;
    int expander_size;
    int horizontal_separator;
    gboolean on_expander;
    gboolean call_parent;
    gboolean selected;
    GtkWidgetClass *widget_parent = GTK_WIDGET_CLASS(xed_file_browser_view_parent_class);

    tree_view = GTK_TREE_VIEW (widget);
    view = XED_FILE_BROWSER_VIEW (widget);
    selection = gtk_tree_view_get_selection (tree_view);

    /* Get double click time */
    g_object_get (G_OBJECT (gtk_widget_get_settings (widget)),
                  "gtk-double-click-time", &double_click_time,
                  NULL);

    /* Determine click count */
    if (event->time - last_click_time < double_click_time)
    {
        click_count++;
    }
    else
    {
        click_count = 0;
    }

    last_click_time = event->time;

    view->priv->ignore_release = FALSE;
    call_parent = TRUE;

    if (gtk_tree_view_get_path_at_pos (tree_view, event->x, event->y, &path, NULL, NULL, NULL))
    {
        /* Keep track of path of last click so double clicks only happen
         * on the same item */
        if ((event->button == 1 || event->button == 2) && event->type == GDK_BUTTON_PRESS)
        {
            if (view->priv->double_click_path[1])
            {
                gtk_tree_path_free (view->priv->double_click_path[1]);
            }

            view->priv->double_click_path[1] = view->priv->double_click_path[0];
            view->priv->double_click_path[0] = gtk_tree_path_copy (path);
        }

        if (event->type == GDK_2BUTTON_PRESS)
        {
            /* Chain up, must be before activating the selected
               items because this will cause the view to grab focus */
            widget_parent->button_press_event (widget, event);

            if (view->priv->double_click_path[1] &&
                gtk_tree_path_compare (view->priv->double_click_path[0], view->priv->double_click_path[1]) == 0)
            {
                activate_selected_items (view);
            }
        }
        else
        {
            /* We're going to filter out some situations where
             * we can't let the default code run because all
             * but one row would be would be deselected. We don't
             * want that; we want the right click menu or single
             * click to apply to everything that's currently selected. */
            selected = gtk_tree_selection_path_is_selected (selection, path);

            if (event->button == 3 && selected)
            {
                call_parent = FALSE;
            }

            if ((event->button == 1 || event->button == 2) &&
                ((event->state & GDK_CONTROL_MASK) != 0 ||
                 (event->state & GDK_SHIFT_MASK) == 0))
            {
                gtk_widget_style_get (widget,
                                      "expander-size", &expander_size,
                                      "horizontal-separator", &horizontal_separator,
                                      NULL);
                on_expander = (event->x <= horizontal_separator / 2 + gtk_tree_path_get_depth (path) * expander_size);

                view->priv->selected_on_button_down = selected;

                if (selected)
                {
                    call_parent = on_expander || gtk_tree_selection_count_selected_rows (selection) == 1;
                    view->priv->ignore_release = call_parent;
                }
                else if  ((event->state & GDK_CONTROL_MASK) != 0)
                {
                    call_parent = FALSE;
                    gtk_tree_selection_select_path (selection, path);
                }
                else
                {
                    view->priv->ignore_release = on_expander;
                }
            }

            if (call_parent)
            {
                /* Chain up */
                widget_parent->button_press_event (widget, event);
            }
            else if (selected)
            {
                gtk_widget_grab_focus (widget);
            }

            if ((event->button == 1 || event->button == 2) &&
                event->type == GDK_BUTTON_PRESS)
            {
                view->priv->drag_started = FALSE;
                view->priv->drag_button = event->button;
            }
        }

        gtk_tree_path_free (path);
    }
    else
    {
        if ((event->button == 1 || event->button == 2) && event->type == GDK_BUTTON_PRESS)
        {
            if (view->priv->double_click_path[1])
            {
                gtk_tree_path_free (view->priv->double_click_path[1]);
            }

            view->priv->double_click_path[1] = view->priv->double_click_path[0];
            view->priv->double_click_path[0] = NULL;
        }

        gtk_tree_selection_unselect_all (selection);
        /* Chain up */
        widget_parent->button_press_event (widget, event);
    }

    /* We already chained up if nescessary, so just return TRUE */
    return TRUE;
}

static gboolean
key_press_event (GtkWidget   *widget,
                 GdkEventKey *event)
{
    XedFileBrowserView *view;
    guint modifiers;
    gboolean handled;

    view = XED_FILE_BROWSER_VIEW (widget);
    handled = FALSE;

    modifiers = gtk_accelerator_get_default_mod_mask ();

    switch (event->keyval)
    {
        case GDK_KEY_space:
            if (event->state & GDK_CONTROL_MASK)
            {
                handled = FALSE;
                break;
            }
            if (!gtk_widget_has_focus (widget))
            {
                handled = FALSE;
                break;
            }

            activate_selected_items (view);
            handled = TRUE;
            break;

        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            activate_selected_items (view);
            handled = TRUE;
            break;

        case GDK_KEY_h:
            if ((event->state & modifiers) == GDK_CONTROL_MASK)
            {
                toggle_hidden_filter (view);
                handled = TRUE;
                break;
            }

        default:
            handled = FALSE;
    }

    /* Chain up */
    if (!handled)
    {
        return GTK_WIDGET_CLASS (xed_file_browser_view_parent_class)->key_press_event (widget, event);
    }

    return TRUE;
}

static void
fill_expand_state (XedFileBrowserView *view,
                   GtkTreeIter        *iter)
{
    GtkTreePath * path;
    GtkTreeIter child;

    if (!gtk_tree_model_iter_has_child (view->priv->model, iter))
    {
        return;
    }

    path = gtk_tree_model_get_path (view->priv->model, iter);

    if (gtk_tree_view_row_expanded (GTK_TREE_VIEW (view), path))
    {
        GFile *location;

        gtk_tree_model_get (view->priv->model,
                            iter,
                            XED_FILE_BROWSER_STORE_COLUMN_LOCATION,
                            &location,
                            -1);

        add_expand_state (view, location);

        if (location)
        {
            g_object_unref (location);
        }
    }

    if (gtk_tree_model_iter_children (view->priv->model, &child, iter))
    {
        do
        {
            fill_expand_state (view, &child);
        } while (gtk_tree_model_iter_next (view->priv->model, &child));
    }

    gtk_tree_path_free (path);
}

static void
uninstall_restore_signals (XedFileBrowserView *tree_view,
                           GtkTreeModel       *model)
{
    g_signal_handlers_disconnect_by_func (model, on_begin_refresh, tree_view);
    g_signal_handlers_disconnect_by_func (model, on_end_refresh, tree_view);
    g_signal_handlers_disconnect_by_func (model, on_unload, tree_view);
    g_signal_handlers_disconnect_by_func (model, on_row_inserted, tree_view);
}

static void
install_restore_signals (XedFileBrowserView *tree_view,
                         GtkTreeModel       *model)
{
    g_signal_connect (model, "begin-refresh",
                      G_CALLBACK (on_begin_refresh), tree_view);
    g_signal_connect (model, "end-refresh",
                      G_CALLBACK (on_end_refresh), tree_view);
    g_signal_connect (model, "unload",
                      G_CALLBACK (on_unload), tree_view);
    g_signal_connect_after (model, "row-inserted",
                            G_CALLBACK (on_row_inserted), tree_view);
}

static void
set_restore_expand_state (XedFileBrowserView *view,
                          gboolean            state)
{
    if (state == view->priv->restore_expand_state)
    {
        return;
    }

    if (view->priv->expand_state)
    {
        g_hash_table_destroy (view->priv->expand_state);
        view->priv->expand_state = NULL;
    }

    if (state)
    {
        view->priv->expand_state = g_hash_table_new_full (g_file_hash,
                                                          (GEqualFunc)g_file_equal,
                                                          g_object_unref,
                                                          NULL);

        if (view->priv->model && XED_IS_FILE_BROWSER_STORE (view->priv->model))
        {
            fill_expand_state (view, NULL);

            install_restore_signals (view, view->priv->model);
        }
    }
    else if (view->priv->model && XED_IS_FILE_BROWSER_STORE (view->priv->model))
    {
        uninstall_restore_signals (view, view->priv->model);
    }

    view->priv->restore_expand_state = state;
}

static void
get_property (GObject    *object,
              guint       prop_id,
              GValue     *value,
              GParamSpec *pspec)
{
    XedFileBrowserView *obj = XED_FILE_BROWSER_VIEW (object);

    switch (prop_id)
    {
        case PROP_RESTORE_EXPAND_STATE:
            g_value_set_boolean (value, obj->priv->restore_expand_state);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
set_property (GObject      *object,
              guint         prop_id,
              const GValue *value,
              GParamSpec   *pspec)
{
    XedFileBrowserView *obj = XED_FILE_BROWSER_VIEW (object);

    switch (prop_id)
    {
        case PROP_RESTORE_EXPAND_STATE:
            set_restore_expand_state (obj, g_value_get_boolean (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_file_browser_view_class_init (XedFileBrowserViewClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkTreeViewClass *tree_view_class = GTK_TREE_VIEW_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->finalize = xed_file_browser_view_finalize;
    object_class->get_property = get_property;
    object_class->set_property = set_property;

    /* Event handlers */
    widget_class->button_press_event = button_press_event;
    widget_class->button_release_event = button_release_event;
    widget_class->drag_begin = drag_begin;
    widget_class->key_press_event = key_press_event;

    /* Tree view handlers */
    tree_view_class->row_activated = row_activated;
    tree_view_class->row_expanded = row_expanded;
    tree_view_class->row_collapsed = row_collapsed;

    /* Default handlers */
    klass->directory_activated = directory_activated;

    g_object_class_install_property (object_class, PROP_RESTORE_EXPAND_STATE,
                                     g_param_spec_boolean ("restore-expand-state",
                                                           "Restore Expand State",
                                                           "Restore expanded state of loaded directories",
                                                           FALSE,
                                                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    signals[ERROR] =
        g_signal_new ("error",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedFileBrowserViewClass,
                               error), NULL, NULL,
                      xed_file_browser_marshal_VOID__UINT_STRING,
                      G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_STRING);
    signals[FILE_ACTIVATED] =
        g_signal_new ("file-activated",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedFileBrowserViewClass,
                               file_activated), NULL, NULL,
                      g_cclosure_marshal_VOID__BOXED,
                      G_TYPE_NONE, 1, GTK_TYPE_TREE_ITER);
    signals[DIRECTORY_ACTIVATED] =
        g_signal_new ("directory-activated",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedFileBrowserViewClass,
                               directory_activated), NULL, NULL,
                      g_cclosure_marshal_VOID__BOXED,
                      G_TYPE_NONE, 1, GTK_TYPE_TREE_ITER);
    signals[BOOKMARK_ACTIVATED] =
        g_signal_new ("bookmark-activated",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedFileBrowserViewClass,
                               bookmark_activated), NULL, NULL,
                      g_cclosure_marshal_VOID__BOXED,
                      G_TYPE_NONE, 1, GTK_TYPE_TREE_ITER);
}

static void
xed_file_browser_view_class_finalize (XedFileBrowserViewClass *klass)
{
    /* dummy function - used by G_DEFINE_DYNAMIC_TYPE */
}

static void
cell_data_cb (GtkTreeViewColumn  *tree_column,
              GtkCellRenderer    *cell,
              GtkTreeModel       *tree_model,
              GtkTreeIter        *iter,
              XedFileBrowserView *obj)
{
    GtkTreePath *path;
    PangoUnderline underline = PANGO_UNDERLINE_NONE;
    gboolean editable = FALSE;

    path = gtk_tree_model_get_path (tree_model, iter);

    if (XED_IS_FILE_BROWSER_STORE (tree_model))
    {
        if (obj->priv->editable != NULL && gtk_tree_row_reference_valid (obj->priv->editable))
        {
            GtkTreePath *edpath = gtk_tree_row_reference_get_path (obj->priv->editable);

            editable = edpath && gtk_tree_path_compare (path, edpath) == 0;
        }
    }

    gtk_tree_path_free (path);
    g_object_set (cell, "editable", editable, "underline", underline, NULL);
}

static void
xed_file_browser_view_init (XedFileBrowserView *obj)
{
    obj->priv = xed_file_browser_view_get_instance_private (obj);

    obj->priv->column = gtk_tree_view_column_new ();

    obj->priv->pixbuf_renderer = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start (obj->priv->column, obj->priv->pixbuf_renderer, FALSE);
    gtk_tree_view_column_add_attribute (obj->priv->column,
                                        obj->priv->pixbuf_renderer,
                                        "pixbuf",
                                        XED_FILE_BROWSER_STORE_COLUMN_ICON);

    obj->priv->text_renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (obj->priv->column, obj->priv->text_renderer, TRUE);
    gtk_tree_view_column_add_attribute (obj->priv->column,
                                        obj->priv->text_renderer,
                                        "text",
                                        XED_FILE_BROWSER_STORE_COLUMN_NAME);

    g_signal_connect (obj->priv->text_renderer, "edited",
                      G_CALLBACK (on_cell_edited), obj);

    gtk_tree_view_append_column (GTK_TREE_VIEW (obj), obj->priv->column);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (obj), FALSE);

    gtk_tree_view_enable_model_drag_source (GTK_TREE_VIEW (obj),
                                            GDK_BUTTON1_MASK,
                                            drag_source_targets,
                                            G_N_ELEMENTS (drag_source_targets),
                                            GDK_ACTION_COPY);
}

static gboolean
bookmarks_separator_func (GtkTreeModel *model,
                          GtkTreeIter  *iter,
                          gpointer      user_data)
{
    guint flags;

    gtk_tree_model_get (model, iter, XED_FILE_BOOKMARKS_STORE_COLUMN_FLAGS, &flags, -1);

    return (flags & XED_FILE_BOOKMARKS_STORE_IS_SEPARATOR);
}

/* Public */
GtkWidget *
xed_file_browser_view_new (void)
{
    XedFileBrowserView *obj = XED_FILE_BROWSER_VIEW (g_object_new (XED_TYPE_FILE_BROWSER_VIEW, NULL));

    return GTK_WIDGET (obj);
}

void
xed_file_browser_view_set_model (XedFileBrowserView *tree_view,
                                 GtkTreeModel       *model)
{
    GtkTreeSelection *selection;

    if (tree_view->priv->model == model)
    {
        return;
    }

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

    if (XED_IS_FILE_BOOKMARKS_STORE (model))
    {
        gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);
        gtk_tree_view_set_row_separator_func (GTK_TREE_VIEW
                                              (tree_view),
                                              bookmarks_separator_func,
                                              NULL, NULL);
        gtk_tree_view_column_set_cell_data_func (tree_view->priv->
                                                 column,
                                                 tree_view->priv->
                                                 text_renderer,
                                                 (GtkTreeCellDataFunc)
                                                 cell_data_cb,
                                                 tree_view, NULL);
    }
    else
    {
        gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
        gtk_tree_view_set_row_separator_func (GTK_TREE_VIEW
                                              (tree_view), NULL,
                                              NULL, NULL);
        gtk_tree_view_column_set_cell_data_func (tree_view->priv->
                                                 column,
                                                 tree_view->priv->
                                                 text_renderer,
                                                 (GtkTreeCellDataFunc)
                                                 cell_data_cb,
                                                 tree_view, NULL);

        if (tree_view->priv->restore_expand_state)
        {
            install_restore_signals (tree_view, model);
        }

    }

    if (tree_view->priv->hover_path != NULL)
    {
        gtk_tree_path_free (tree_view->priv->hover_path);
        tree_view->priv->hover_path = NULL;
    }

    if (XED_IS_FILE_BROWSER_STORE (tree_view->priv->model))
    {
        if (tree_view->priv->restore_expand_state)
        {
            uninstall_restore_signals (tree_view, tree_view->priv->model);
        }
    }

    tree_view->priv->model = model;
    gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), model);
}

void
xed_file_browser_view_start_rename (XedFileBrowserView *tree_view,
                                    GtkTreeIter        *iter)
{
    guint flags;
    GtkTreeRowReference *rowref;
    GtkTreePath *path;

    g_return_if_fail (XED_IS_FILE_BROWSER_VIEW (tree_view));
    g_return_if_fail (XED_IS_FILE_BROWSER_STORE (tree_view->priv->model));
    g_return_if_fail (iter != NULL);

    gtk_tree_model_get (tree_view->priv->model, iter, XED_FILE_BROWSER_STORE_COLUMN_FLAGS, &flags, -1);

    if (!(FILE_IS_DIR (flags) || !FILE_IS_DUMMY (flags)))
    {
        return;
    }

    path = gtk_tree_model_get_path (tree_view->priv->model, iter);
    rowref = gtk_tree_row_reference_new (tree_view->priv->model, path);

    /* Start editing */
    gtk_widget_grab_focus (GTK_WIDGET (tree_view));

    if (gtk_tree_path_up (path))
    {
        gtk_tree_view_expand_to_path (GTK_TREE_VIEW (tree_view), path);
    }

    gtk_tree_path_free (path);
    tree_view->priv->editable = rowref;

    gtk_tree_view_set_cursor (GTK_TREE_VIEW (tree_view),
                              gtk_tree_row_reference_get_path (tree_view->priv->editable),
                              tree_view->priv->column, TRUE);

    gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view),
                                  gtk_tree_row_reference_get_path (tree_view->priv->editable),
                                  tree_view->priv->column,
                                  FALSE, 0.0, 0.0);
}

void
xed_file_browser_view_set_restore_expand_state (XedFileBrowserView *tree_view,
                                                gboolean            restore_expand_state)
{
    g_return_if_fail (XED_IS_FILE_BROWSER_VIEW (tree_view));

    set_restore_expand_state (tree_view, restore_expand_state);
    g_object_notify (G_OBJECT (tree_view), "restore-expand-state");
}

/* Signal handlers */
static void
on_cell_edited (GtkCellRendererText *cell,
                gchar               *path,
                gchar               *new_text,
                XedFileBrowserView  *tree_view)
{
    GtkTreePath * treepath;
    GtkTreeIter iter;
    gboolean ret;
    GError * error = NULL;

    gtk_tree_row_reference_free (tree_view->priv->editable);
    tree_view->priv->editable = NULL;

    if (new_text == NULL || *new_text == '\0')
    {
        return;
    }

    treepath = gtk_tree_path_new_from_string (path);
    ret = gtk_tree_model_get_iter (GTK_TREE_MODEL (tree_view->priv->model), &iter, treepath);
    gtk_tree_path_free (treepath);

    if (ret)
    {
        if (xed_file_browser_store_rename (XED_FILE_BROWSER_STORE (tree_view->priv->model), &iter, new_text, &error))
        {
            treepath = gtk_tree_model_get_path (GTK_TREE_MODEL (tree_view->priv->model), &iter);
            gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), treepath, NULL, FALSE, 0.0, 0.0);
            gtk_tree_path_free (treepath);
        }
        else
        {
            if (error)
            {
                g_signal_emit (tree_view, signals[ERROR], 0, error->code, error->message);
                g_error_free (error);
            }
        }
    }
}

static void
on_begin_refresh (XedFileBrowserStore *model,
                  XedFileBrowserView  *view)
{
    /* Store the refresh state, so we can handle unloading of nodes while
       refreshing properly */
    view->priv->is_refresh = TRUE;
}

static void
on_end_refresh (XedFileBrowserStore *model,
                XedFileBrowserView  *view)
{
    /* Store the refresh state, so we can handle unloading of nodes while
       refreshing properly */
    view->priv->is_refresh = FALSE;
}

static void
on_unload (XedFileBrowserStore *model,
           GFile               *location,
           XedFileBrowserView  *view)
{
    /* Don't remove the expand state if we are refreshing */
    if (!view->priv->restore_expand_state || view->priv->is_refresh)
    {
        return;
    }

    remove_expand_state (view, location);
}

static void
restore_expand_state (XedFileBrowserView  *view,
                      XedFileBrowserStore *model,
                      GtkTreeIter         *iter)
{
    GFile *location;

    gtk_tree_model_get (GTK_TREE_MODEL (model),
                        iter,
                        XED_FILE_BROWSER_STORE_COLUMN_LOCATION,
                        &location,
                        -1);

    if (location)
    {
        GtkTreePath *path;

        path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), iter);

        if (g_hash_table_lookup (view->priv->expand_state, location))
        {
            gtk_tree_view_expand_row (GTK_TREE_VIEW (view), path, FALSE);
        }

        gtk_tree_path_free (path);
        g_object_unref (location);
    }
}

static void
on_row_inserted (XedFileBrowserStore *model,
                 GtkTreePath         *path,
                 GtkTreeIter         *iter,
                 XedFileBrowserView  *view)
{
    GtkTreeIter parent;
    GtkTreePath * copy;

    if (gtk_tree_model_iter_has_child (GTK_TREE_MODEL (model), iter))
    {
        restore_expand_state (view, model, iter);
    }

    copy = gtk_tree_path_copy (path);

    if (gtk_tree_path_up (copy) &&
        (gtk_tree_path_get_depth (copy) != 0) &&
        gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &parent, copy))
    {
        restore_expand_state (view, model, &parent);
    }

    gtk_tree_path_free (copy);
}

void
_xed_file_browser_view_register_type (GTypeModule *type_module)
{
    xed_file_browser_view_register_type (type_module);
}

// ex:ts=8:noet:
