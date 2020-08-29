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

#include <config.h>
#include <string.h>
#include <stdlib.h>
#include <glib/gi18n.h>
#include <xed/xed-window.h>
#include <xed/xed-window-activatable.h>
#include <xed/xed-debug.h>
#include <xed/xed-utils.h>
#include <xed/xed-app.h>

#include "xed-sort-plugin.h"

#define MENU_PATH "/MenuBar/EditMenu/EditOps_6"

static void xed_window_activatable_iface_init (XedWindowActivatableInterface *iface);

struct _XedSortPluginPrivate
{
    XedWindow *window;

    GtkActionGroup *ui_action_group;
    guint ui_id;

    GtkTextIter start, end; /* selection */
};

enum
{
    PROP_0,
    PROP_WINDOW
};

G_DEFINE_DYNAMIC_TYPE_EXTENDED (XedSortPlugin,
                                xed_sort_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (XED_TYPE_WINDOW_ACTIVATABLE,
                                                               xed_window_activatable_iface_init)
                                G_ADD_PRIVATE_DYNAMIC (XedSortPlugin))

static void sort_cb (GtkAction     *action,
                     XedSortPlugin *plugin);

static void buffer_sort_lines (GtkSourceBuffer *buffer,
                               GtkTextIter     *start,
                               GtkTextIter     *end);

static const GtkActionEntry action_entries[] =
{
    { "Sort",
      NULL,
      N_("S_ort lines"),
      "F10",
      N_("Sort the current document or selection"),
      G_CALLBACK (sort_cb)
    }
};

/* NOTE: we store the current selection in the dialog since focusing
 * the text field (like the combo box) looses the documnent selection.
 * Storing the selection ONLY works because the dialog is modal */
static void
get_current_selection (XedSortPlugin *plugin)
{
    XedSortPluginPrivate *priv;
    XedDocument *doc;

    xed_debug (DEBUG_PLUGINS);

    priv = plugin->priv;

    doc = xed_window_get_active_document (priv->window);

    if (!gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (doc), &priv->start, &priv->end))
    {
        /* No selection, get the whole document. */
        gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (doc), &priv->start, &priv->end);
    }
}

static void
sort_cb (GtkAction     *action,
         XedSortPlugin *plugin)
{
    XedSortPluginPrivate *priv;
    XedDocument *doc;

    xed_debug (DEBUG_PLUGINS);

    priv = plugin->priv;

    doc = xed_window_get_active_document (priv->window);
    g_return_if_fail (doc != NULL);

    get_current_selection (plugin);

    buffer_sort_lines (GTK_SOURCE_BUFFER (doc),
                       &priv->start,
                       &priv->end);
}

static gchar *
get_line_slice (GtkTextBuffer *buf,
                gint           line)
{
    GtkTextIter start, end;

    gtk_text_buffer_get_iter_at_line (buf, &start, line);
    end = start;

    if (!gtk_text_iter_ends_line (&start))
    {
        gtk_text_iter_forward_to_line_end (&end);
    }

    return gtk_text_buffer_get_slice (buf, &start, &end, TRUE);
}

typedef struct {
    gchar *line; /* the original text to re-insert */
    gchar *key;  /* the key to use for the comparison */
} SortLine;

static gint
compare_line (gconstpointer aptr,
              gconstpointer bptr)
{
    const SortLine *a = aptr;
    const SortLine *b = bptr;

    return g_strcmp0 (a->key, b->key);
}

static void
buffer_sort_lines (GtkSourceBuffer    *buffer,
                   GtkTextIter        *start,
                   GtkTextIter        *end)
{
    GtkTextBuffer *text_buffer;
    gint start_line;
    gint end_line;
    gint num_lines;
    SortLine *lines;
    gint i;

    g_return_if_fail (GTK_SOURCE_IS_BUFFER (buffer));
    g_return_if_fail (start != NULL);
    g_return_if_fail (end != NULL);

    text_buffer = GTK_TEXT_BUFFER (buffer);

    gtk_text_iter_order (start, end);

    start_line = gtk_text_iter_get_line (start);
    end_line = gtk_text_iter_get_line (end);

    /* Required for gtk_text_buffer_delete() */
    if (!gtk_text_iter_starts_line (start))
    {
        gtk_text_iter_set_line_offset (start, 0);
    }

    /* if we are at line start our last line is the previus one.
     * Otherwise the last line is the current one but we try to
     * move the iter after the line terminator */
    if (gtk_text_iter_starts_line (end))
    {
        end_line = MAX (start_line, end_line - 1);
    }
    else
    {
        gtk_text_iter_forward_line (end);
    }

    if (start_line == end_line)
    {
        return;
    }

    num_lines = end_line - start_line + 1;
    lines = g_new0 (SortLine, num_lines);

    for (i = 0; i < num_lines; i++)
    {
        gchar *line;

        lines[i].line = get_line_slice (text_buffer, start_line + i);
        line = g_utf8_casefold (lines[i].line, -1);
        lines[i].key = g_utf8_collate_key (line, -1);

        g_free (line);
    }

    qsort (lines, num_lines, sizeof (SortLine), compare_line);

    gtk_text_buffer_begin_user_action (text_buffer);

    gtk_text_buffer_delete (text_buffer, start, end);

    for (i = 0; i < num_lines; i++)
    {
        gtk_text_buffer_insert (text_buffer, start, lines[i].line, -1);
        gtk_text_buffer_insert (text_buffer, start, "\n", -1);
    }

    gtk_text_buffer_end_user_action (text_buffer);

    for (i = 0; i < num_lines; i++)
    {
        g_free (lines[i].line);
        g_free (lines[i].key);
    }

    g_free (lines);
}

static void
update_ui (XedSortPlugin *plugin)
{
    XedView *view;

    xed_debug (DEBUG_PLUGINS);

    view = xed_window_get_active_view (plugin->priv->window);

    gtk_action_group_set_sensitive (plugin->priv->ui_action_group,
                                    (view != NULL) &&
                                    gtk_text_view_get_editable (GTK_TEXT_VIEW (view)));
}

static void
xed_sort_plugin_activate (XedWindowActivatable *activatable)
{
    XedSortPluginPrivate *priv;
    GtkUIManager *manager;

    xed_debug (DEBUG_PLUGINS);

    priv = XED_SORT_PLUGIN (activatable)->priv;
    manager = xed_window_get_ui_manager (priv->window);

    priv->ui_action_group = gtk_action_group_new ("XedSortPluginActions");
    gtk_action_group_set_translation_domain (priv->ui_action_group, GETTEXT_PACKAGE);
    gtk_action_group_add_actions (priv->ui_action_group, action_entries, G_N_ELEMENTS (action_entries), activatable);

    gtk_ui_manager_insert_action_group (manager, priv->ui_action_group, -1);

    priv->ui_id = gtk_ui_manager_new_merge_id (manager);

    gtk_ui_manager_add_ui (manager,
                           priv->ui_id,
                           MENU_PATH,
                           "Sort",
                           "Sort",
                           GTK_UI_MANAGER_MENUITEM,
                           FALSE);

    update_ui (XED_SORT_PLUGIN (activatable));
}

static void
xed_sort_plugin_deactivate (XedWindowActivatable *activatable)
{
    XedSortPluginPrivate *priv;
    GtkUIManager *manager;

    xed_debug (DEBUG_PLUGINS);

    priv = XED_SORT_PLUGIN (activatable)->priv;
    manager = xed_window_get_ui_manager (priv->window);

    gtk_ui_manager_remove_ui (manager, priv->ui_id);
    gtk_ui_manager_remove_action_group (manager, priv->ui_action_group);
}

static void
xed_sort_plugin_update_state (XedWindowActivatable *activatable)
{
    xed_debug (DEBUG_PLUGINS);

    update_ui (XED_SORT_PLUGIN (activatable));
}

static void
xed_sort_plugin_init (XedSortPlugin *plugin)
{
    xed_debug_message (DEBUG_PLUGINS, "XedSortPlugin initializing");

    plugin->priv = xed_sort_plugin_get_instance_private (plugin);
}

static void
xed_sort_plugin_dispose (GObject *object)
{
    XedSortPlugin *plugin = XED_SORT_PLUGIN (object);

    xed_debug_message (DEBUG_PLUGINS, "XedSortPlugin disposing");

    g_clear_object (&plugin->priv->ui_action_group);
    g_clear_object (&plugin->priv->window);

    G_OBJECT_CLASS (xed_sort_plugin_parent_class)->dispose (object);
}

static void
xed_sort_plugin_finalize (GObject *object)
{
    xed_debug_message (DEBUG_PLUGINS, "XedSortPlugin finalizing");

    G_OBJECT_CLASS (xed_sort_plugin_parent_class)->finalize (object);
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
        case PROP_WINDOW:
            plugin->priv->window = XED_WINDOW (g_value_dup_object (value));
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
        case PROP_WINDOW:
            g_value_set_object (value, plugin->priv->window);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_sort_plugin_class_init (XedSortPluginClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = xed_sort_plugin_dispose;
    object_class->finalize = xed_sort_plugin_finalize;
    object_class->set_property = xed_sort_plugin_set_property;
    object_class->get_property = xed_sort_plugin_get_property;

    g_object_class_override_property (object_class, PROP_WINDOW, "window");
}

static void
xed_sort_plugin_class_finalize (XedSortPluginClass *klass)
{
    /* dummy function - used by G_DEFINE_DYNAMIC_TYPE_EXTENDED */
}

static void
xed_window_activatable_iface_init (XedWindowActivatableInterface *iface)
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
                                                XED_TYPE_WINDOW_ACTIVATABLE,
                                                XED_TYPE_SORT_PLUGIN);
}
