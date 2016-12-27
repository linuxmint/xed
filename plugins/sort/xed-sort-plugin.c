/*
 * xed-sort-plugin.c
 *
 * Original author: Carlo Borreo <borreo@softhome.net>
 * Ported to Xed2 by Lee Mallabone <mate@fonicmonkey.net>
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
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xed-sort-plugin.h"

#include <string.h>
#include <glib/gi18n-lib.h>
#include <gmodule.h>
#include <libpeas/peas-activatable.h>

#include <xed/xed-window.h>
#include <xed/xed-debug.h>
#include <xed/xed-utils.h>
#include <xed/xed-help.h>

#define XED_SORT_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), XED_TYPE_SORT_PLUGIN, XedSortPluginPrivate))

#define MENU_PATH "/MenuBar/EditMenu/EditOps_6"

static void peas_activatable_iface_init (PeasActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (XedSortPlugin,
                                xed_sort_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_TYPE_ACTIVATABLE,
                                                               peas_activatable_iface_init))

enum
{
    PROP_0,
    PROP_OBJECT
};

typedef struct
{
    GtkWidget *dialog;
    GtkWidget *col_num_spinbutton;
    GtkWidget *reverse_order_checkbutton;
    GtkWidget *ignore_case_checkbutton;
    GtkWidget *remove_dups_checkbutton;

    XedDocument *doc;

    GtkTextIter start, end; /* selection */
} SortDialog;

struct _XedSortPluginPrivate
{
    GtkWidget *window;

    GtkActionGroup *ui_action_group;
    guint ui_id;
};

typedef struct
{
    gboolean ignore_case;
    gboolean reverse_order;
    gboolean remove_duplicates;
    gint starting_column;
} SortInfo;

static void sort_cb (GtkAction     *action,
                     XedSortPlugin *plugin);
static void sort_real (SortDialog *dialog);

static const GtkActionEntry action_entries[] =
{
    { "Sort",
      "view-sort-ascending-symbolic",
      N_("S_ort..."),
      NULL,
      N_("Sort the current document or selection"),
      G_CALLBACK (sort_cb)
    }
};

static void
sort_dialog_destroy (GObject  *obj,
                     gpointer  dialog_pointer)
{
    xed_debug (DEBUG_PLUGINS);

    g_slice_free (SortDialog, dialog_pointer);
}

static void
sort_dialog_response_handler (GtkDialog  *widget,
                              gint        res_id,
                              SortDialog *dialog)
{
    xed_debug (DEBUG_PLUGINS);

    switch (res_id)
    {
        case GTK_RESPONSE_OK:
            sort_real (dialog);
            gtk_widget_destroy (dialog->dialog);
            break;

        case GTK_RESPONSE_HELP:
            xed_help_display (GTK_WINDOW (widget), NULL, "xed-sort-plugin");
            break;

        case GTK_RESPONSE_CANCEL:
            gtk_widget_destroy (dialog->dialog);
            break;
    }
}

/* NOTE: we store the current selection in the dialog since focusing
 * the text field (like the combo box) looses the documnent selection.
 * Storing the selection ONLY works because the dialog is modal */
static void
get_current_selection (XedWindow  *window,
                       SortDialog *dialog)
{
    XedDocument *doc;

    xed_debug (DEBUG_PLUGINS);

    doc = xed_window_get_active_document (window);

    if (!gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (doc), &dialog->start, &dialog->end))
    {
        /* No selection, get the whole document. */
        gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (doc), &dialog->start, &dialog->end);
    }
}

static SortDialog *
get_sort_dialog (XedSortPlugin *plugin)
{
    XedWindow *window;
    SortDialog *dialog;
    GtkWidget *error_widget;
    gboolean ret;
    gchar *data_dir;
    gchar *ui_file;

    xed_debug (DEBUG_PLUGINS);

    window = XED_WINDOW (plugin->priv->window);

    dialog = g_slice_new (SortDialog);

    data_dir = peas_extension_base_get_data_dir (PEAS_EXTENSION_BASE (plugin));
    ui_file = g_build_filename (data_dir, "sort.ui", NULL);
    g_free (data_dir);
    ret = xed_utils_get_ui_objects (ui_file,
                                    NULL,
                                    &error_widget,
                                    "sort_dialog", &dialog->dialog,
                                    "reverse_order_checkbutton", &dialog->reverse_order_checkbutton,
                                    "col_num_spinbutton", &dialog->col_num_spinbutton,
                                    "ignore_case_checkbutton", &dialog->ignore_case_checkbutton,
                                    "remove_dups_checkbutton", &dialog->remove_dups_checkbutton,
                                    NULL);
    g_free (ui_file);

    if (!ret)
    {
        const gchar *err_message;

        err_message = gtk_label_get_label (GTK_LABEL (error_widget));
        xed_warning (GTK_WINDOW (window), "%s", err_message);

        g_slice_free (SortDialog, dialog);
        gtk_widget_destroy (error_widget);

        return NULL;
    }

    gtk_dialog_set_default_response (GTK_DIALOG (dialog->dialog), GTK_RESPONSE_OK);

    g_signal_connect (dialog->dialog, "destroy", G_CALLBACK (sort_dialog_destroy), dialog);
    g_signal_connect (dialog->dialog, "response", G_CALLBACK (sort_dialog_response_handler), dialog);

    get_current_selection (window, dialog);

    return dialog;
}

static void
sort_cb (GtkAction     *action,
         XedSortPlugin *plugin)
{
    XedWindow *window;
    XedDocument *doc;
    GtkWindowGroup *wg;
    SortDialog *dialog;

    xed_debug (DEBUG_PLUGINS);

    window = XED_WINDOW (plugin->priv->window);

    doc = xed_window_get_active_document (window);
    g_return_if_fail (doc != NULL);

    dialog = get_sort_dialog (plugin);
    g_return_if_fail (dialog != NULL);

    wg = xed_window_get_group (window);
    gtk_window_group_add_window (wg, GTK_WINDOW (dialog->dialog));

    dialog->doc = doc;

    gtk_window_set_transient_for (GTK_WINDOW (dialog->dialog), GTK_WINDOW (window));
    gtk_window_set_modal (GTK_WINDOW (dialog->dialog), TRUE);

    gtk_widget_show (GTK_WIDGET (dialog->dialog));
}

/* Compares two strings for the sorting algorithm. Uses the UTF-8 processing
 * functions in GLib to be as correct as possible.*/
static gint
compare_algorithm (gconstpointer s1,
                   gconstpointer s2,
                   gpointer      data)
{
    gint length1, length2;
    gint ret;
    gchar *string1, *string2;
    gchar *substring1, *substring2;
    gchar *key1, *key2;
    SortInfo *sort_info;

    xed_debug (DEBUG_PLUGINS);

    sort_info = (SortInfo *) data;
    g_return_val_if_fail (sort_info != NULL, -1);

    if (!sort_info->ignore_case)
    {
        string1 = *((gchar **) s1);
        string2 = *((gchar **) s2);
    }
    else
    {
        string1 = g_utf8_casefold (*((gchar **) s1), -1);
        string2 = g_utf8_casefold (*((gchar **) s2), -1);
    }

    length1 = g_utf8_strlen (string1, -1);
    length2 = g_utf8_strlen (string2, -1);

    if ((length1 < sort_info->starting_column) &&
        (length2 < sort_info->starting_column))
    {
        ret = 0;
    }
    else if (length1 < sort_info->starting_column)
    {
        ret = -1;
    }
    else if (length2 < sort_info->starting_column)
    {
        ret = 1;
    }
    else if (sort_info->starting_column < 1)
    {
        key1 = g_utf8_collate_key (string1, -1);
        key2 = g_utf8_collate_key (string2, -1);
        ret = strcmp (key1, key2);

        g_free (key1);
        g_free (key2);
    }
    else
    {
        /* A character column offset is required, so figure out
         * the correct offset into the UTF-8 string. */
        substring1 = g_utf8_offset_to_pointer (string1, sort_info->starting_column);
        substring2 = g_utf8_offset_to_pointer (string2, sort_info->starting_column);

        key1 = g_utf8_collate_key (substring1, -1);
        key2 = g_utf8_collate_key (substring2, -1);
        ret = strcmp (key1, key2);

        g_free (key1);
        g_free (key2);
    }

    /* Do the necessary cleanup. */
    if (sort_info->ignore_case)
    {
        g_free (string1);
        g_free (string2);
    }

    if (sort_info->reverse_order)
    {
        ret = -1 * ret;
    }

    return ret;
}

static gchar *
get_line_slice (GtkTextBuffer *buf,
                gint           line)
{
    GtkTextIter start, end;
    char *ret;

    gtk_text_buffer_get_iter_at_line (buf, &start, line);
    end = start;

    if (!gtk_text_iter_ends_line (&start))
    {
        gtk_text_iter_forward_to_line_end (&end);
    }

    ret= gtk_text_buffer_get_slice (buf, &start, &end, TRUE);

    g_assert (ret != NULL);

    return ret;
}

static void
sort_real (SortDialog *dialog)
{
    XedDocument *doc;
    GtkTextIter start, end;
    gint start_line, end_line;
    gint i;
    gchar *last_row = NULL;
    gint num_lines;
    gchar **lines;
    SortInfo *sort_info;

    xed_debug (DEBUG_PLUGINS);

    doc = dialog->doc;
    g_return_if_fail (doc != NULL);

    sort_info = g_new0 (SortInfo, 1);
    sort_info->ignore_case = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->ignore_case_checkbutton));
    sort_info->reverse_order = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->reverse_order_checkbutton));
    sort_info->remove_duplicates = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->remove_dups_checkbutton));
    sort_info->starting_column = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->col_num_spinbutton)) - 1;

    start = dialog->start;
    end = dialog->end;
    start_line = gtk_text_iter_get_line (&start);
    end_line = gtk_text_iter_get_line (&end);

    /* if we are at line start our last line is the previus one.
     * Otherwise the last line is the current one but we try to
     * move the iter after the line terminator */
    if (gtk_text_iter_get_line_offset (&end) == 0)
    {
        end_line = MAX (start_line, end_line - 1);
    }
    else
    {
        gtk_text_iter_forward_line (&end);
    }

    num_lines = end_line - start_line + 1;
    lines = g_new0 (gchar *, num_lines + 1);

    xed_debug_message (DEBUG_PLUGINS, "Building list...");

    for (i = 0; i < num_lines; i++)
    {
        lines[i] = get_line_slice (GTK_TEXT_BUFFER (doc), start_line + i);
    }

    lines[num_lines] = NULL;

    xed_debug_message (DEBUG_PLUGINS, "Sort list...");

    g_qsort_with_data (lines, num_lines, sizeof (gpointer), compare_algorithm, sort_info);

    xed_debug_message (DEBUG_PLUGINS, "Rebuilding document...");

    gtk_source_buffer_begin_not_undoable_action (GTK_SOURCE_BUFFER (doc));

    gtk_text_buffer_delete (GTK_TEXT_BUFFER (doc), &start, &end);

    for (i = 0; i < num_lines; i++)
    {
        if (sort_info->remove_duplicates && last_row != NULL && (strcmp (last_row, lines[i]) == 0))
        {
            continue;
        }

        gtk_text_buffer_insert (GTK_TEXT_BUFFER (doc), &start, lines[i], -1);
        gtk_text_buffer_insert (GTK_TEXT_BUFFER (doc), &start, "\n", -1);

        last_row = lines[i];
    }

    gtk_source_buffer_end_not_undoable_action (GTK_SOURCE_BUFFER (doc));

    g_strfreev (lines);
    g_free (sort_info);

    xed_debug_message (DEBUG_PLUGINS, "Done.");
}

static void
xed_sort_plugin_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    XedSortPlugin *plugin = XED_SORT_PLUGIN (object);

    switch (prop_id)
    {
        case PROP_OBJECT:
            plugin->priv->window = GTK_WIDGET (g_value_dup_object (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_sort_plugin_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    XedSortPlugin *plugin = XED_SORT_PLUGIN (object);

    switch (prop_id)
    {
        case PROP_OBJECT:
            g_value_set_object (value, plugin->priv->window);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
update_ui (XedSortPluginPrivate *data)
{
    XedWindow *window;
    XedView *view;

    xed_debug (DEBUG_PLUGINS);

    window = XED_WINDOW (data->window);
    view = xed_window_get_active_view (window);

    gtk_action_group_set_sensitive (data->ui_action_group,
                                    (view != NULL) &&
                                    gtk_text_view_get_editable (GTK_TEXT_VIEW (view)));
}

static void
xed_sort_plugin_activate (PeasActivatable *activatable)
{
    XedSortPlugin *plugin;
    XedSortPluginPrivate *data;
    XedWindow *window;
    GtkUIManager *manager;

    xed_debug (DEBUG_PLUGINS);

    plugin = XED_SORT_PLUGIN (activatable);
    data = plugin->priv;
    window = XED_WINDOW (data->window);

    manager = xed_window_get_ui_manager (window);

    data->ui_action_group = gtk_action_group_new ("XedSortPluginActions");
    gtk_action_group_set_translation_domain (data->ui_action_group, GETTEXT_PACKAGE);
    gtk_action_group_add_actions (data->ui_action_group,
                                  action_entries,
                                  G_N_ELEMENTS (action_entries),
                                  plugin);

    gtk_ui_manager_insert_action_group (manager, data->ui_action_group, -1);

    data->ui_id = gtk_ui_manager_new_merge_id (manager);

    gtk_ui_manager_add_ui (manager,
                           data->ui_id,
                           MENU_PATH,
                           "Sort",
                           "Sort",
                           GTK_UI_MANAGER_MENUITEM,
                           FALSE);

    update_ui (data);
}

static void
xed_sort_plugin_deactivate (PeasActivatable *activatable)
{
    XedSortPluginPrivate *data;
    XedWindow *window;
    GtkUIManager *manager;

    xed_debug (DEBUG_PLUGINS);

    data = XED_SORT_PLUGIN (activatable)->priv;
    window = XED_WINDOW (data->window);

    manager = xed_window_get_ui_manager (window);

    gtk_ui_manager_remove_ui (manager, data->ui_id);
    gtk_ui_manager_remove_action_group (manager, data->ui_action_group);
}

static void
xed_sort_plugin_update_state (PeasActivatable *activatable)
{
    xed_debug (DEBUG_PLUGINS);

    update_ui (XED_SORT_PLUGIN (activatable)->priv);
}

static void
xed_sort_plugin_init (XedSortPlugin *plugin)
{
    xed_debug_message (DEBUG_PLUGINS, "XedSortPlugin initializing");

    plugin->priv = XED_SORT_PLUGIN_GET_PRIVATE (plugin);
}

static void
xed_sort_plugin_dispose (GObject *object)
{
    XedSortPlugin *plugin = XED_SORT_PLUGIN (object);

    xed_debug_message (DEBUG_PLUGINS, "XedSortPlugin disposing");

    if (plugin->priv->window != NULL)
    {
        g_object_unref (plugin->priv->window);
        plugin->priv->window = NULL;
    }

    if (plugin->priv->ui_action_group)
    {
        g_object_unref (plugin->priv->ui_action_group);
        plugin->priv->ui_action_group = NULL;
    }

    G_OBJECT_CLASS (xed_sort_plugin_parent_class)->dispose (object);
}

static void
xed_sort_plugin_class_init (XedSortPluginClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = xed_sort_plugin_dispose;
    object_class->set_property = xed_sort_plugin_set_property;
    object_class->get_property = xed_sort_plugin_get_property;

    g_object_class_override_property (object_class, PROP_OBJECT, "object");

    g_type_class_add_private (klass, sizeof (XedSortPluginPrivate));
}

static void
xed_sort_plugin_class_finalize (XedSortPluginClass *klass)
{
    /* dummy function - used by G_DEFINE_DYNAMIC_TYPE_EXTENDED */
}

static void
peas_activatable_iface_init (PeasActivatableInterface *iface)
{
    iface->activate = xed_sort_plugin_activate;
    iface->deactivate = xed_sort_plugin_deactivate;
    iface->update_state = xed_sort_plugin_update_state;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
    xed_sort_plugin_register_type (G_TYPE_MODULE (module));

    peas_object_module_register_extension_type (module,
                                                PEAS_TYPE_ACTIVATABLE,
                                                XED_TYPE_SORT_PLUGIN);
}
