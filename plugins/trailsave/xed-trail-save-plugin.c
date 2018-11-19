/*
 * xed-trail-save-plugin.c
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

#include <config.h>
#include <string.h>
#include <xed/xed-window.h>
#include <xed/xed-window-activatable.h>
#include <xed/xed-debug.h>

#include "xed-trail-save-plugin.h"

#define XED_TRAIL_SAVE_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
                                                  XED_TYPE_TRAIL_SAVE_PLUGIN, \
                                                  XedTrailSavePluginPrivate))

static void xed_window_activatable_iface_init (XedWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (XedTrailSavePlugin,
                                xed_trail_save_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (XED_TYPE_WINDOW_ACTIVATABLE,
                                                               xed_window_activatable_iface_init))

struct _XedTrailSavePluginPrivate
{
   XedWindow *window;
};

enum
{
   PROP_0,
   PROP_WINDOW
};

static void
strip_trailing_spaces (GtkTextBuffer *text_buffer)
{
    gint line_count, line_num;
    GtkTextIter line_start, line_end;
    gchar *slice;
    gchar byte;
    gint byte_index;
    gint strip_start_index, strip_end_index;
    gint empty_lines_start = -1;
    gboolean should_strip;
    GtkTextIter strip_start, strip_end;

    g_assert (text_buffer != NULL);

    line_count = gtk_text_buffer_get_line_count (text_buffer);

    for (line_num = 0; line_num < line_count; ++line_num)
    {
        /* Get line text */
        gtk_text_buffer_get_iter_at_line (text_buffer, &line_start, line_num);

        if (line_num == line_count - 1)
        {
            gtk_text_buffer_get_end_iter (text_buffer, &line_end);
        }
        else
        {
            gtk_text_buffer_get_iter_at_line (text_buffer, &line_end, line_num + 1);
        }

        slice = gtk_text_buffer_get_slice (text_buffer, &line_start, &line_end, TRUE);

        if (slice == NULL)
        {
            continue;
        }

        /* Find indices of bytes that should be stripped */
        should_strip = FALSE;
        empty_lines_start = (empty_lines_start < 0) ? line_num : empty_lines_start;

        for (byte_index = 0; slice [byte_index] != 0; ++byte_index)
        {
            byte = slice [byte_index];

            if ((byte == ' ') || (byte == '\t'))
            {
                if (!should_strip)
                {
                    strip_start_index = byte_index;
                    should_strip = TRUE;
                }

                strip_end_index = byte_index + 1;
            }
            else if ((byte == '\r') || (byte == '\n'))
            {
                break;
            }
            else
            {
                should_strip = FALSE;
                empty_lines_start = -1;
            }
        }

        g_free (slice);

        /* Strip trailing spaces */
        if (should_strip)
        {
            gtk_text_buffer_get_iter_at_line_index (text_buffer, &strip_start, line_num, strip_start_index);
            gtk_text_buffer_get_iter_at_line_index (text_buffer, &strip_end, line_num, strip_end_index);
            gtk_text_buffer_delete (text_buffer, &strip_start, &strip_end);
        }
    }

    /* Strip trailing lines (except for one) */
    if (empty_lines_start != -1 && empty_lines_start != (line_count - 1))
    {
        gtk_text_buffer_get_iter_at_line (text_buffer, &strip_start, empty_lines_start);
        gtk_text_buffer_get_end_iter (text_buffer, &strip_end);
        gtk_text_buffer_delete (text_buffer, &strip_start, &strip_end);
    }
}

static void
on_save (XedDocument          *document,
         XedTrailSavePlugin   *plugin)
{
    GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (document);

    strip_trailing_spaces (text_buffer);
}

static void
on_tab_added (XedWindow          *window,
              XedTab             *tab,
              XedTrailSavePlugin *plugin)
{
    XedDocument *document;

    document = xed_tab_get_document (tab);
    g_signal_connect (document, "save",
                      G_CALLBACK (on_save), plugin);
}

static void
on_tab_removed (XedWindow          *window,
                XedTab             *tab,
                XedTrailSavePlugin *plugin)
{
    XedDocument *document;

    document = xed_tab_get_document (tab);
    g_signal_handlers_disconnect_by_data (document, plugin);
}

static void
xed_trail_save_plugin_activate (XedWindowActivatable *activatable)
{
    XedTrailSavePluginPrivate *priv;
    GList *documents;
    GList *documents_iter;
    XedDocument *document;

    xed_debug (DEBUG_PLUGINS);

    priv = XED_TRAIL_SAVE_PLUGIN (activatable)->priv;

    g_signal_connect (priv->window, "tab_added",
                      G_CALLBACK (on_tab_added), XED_TRAIL_SAVE_PLUGIN (activatable));
    g_signal_connect (priv->window, "tab_removed",
                      G_CALLBACK (on_tab_removed), XED_TRAIL_SAVE_PLUGIN (activatable));

    documents = xed_window_get_documents (priv->window);

    for (documents_iter = documents;
         documents_iter && documents_iter->data;
         documents_iter = documents_iter->next)
    {
        document = (XedDocument *) documents_iter->data;
        g_signal_connect (document, "save",
                          G_CALLBACK (on_save), XED_TRAIL_SAVE_PLUGIN (activatable));
    }

    g_list_free (documents);
}

static void
xed_trail_save_plugin_deactivate (XedWindowActivatable *activatable)
{
    XedTrailSavePluginPrivate *priv;
    GList *documents;
    GList *documents_iter;
    XedDocument *document;

    xed_debug (DEBUG_PLUGINS);

    priv = XED_TRAIL_SAVE_PLUGIN (activatable)->priv;

    g_signal_handlers_disconnect_by_data (priv->window, XED_TRAIL_SAVE_PLUGIN (activatable));

    documents = xed_window_get_documents (priv->window);

    for (documents_iter = documents;
         documents_iter && documents_iter->data;
         documents_iter = documents_iter->next)
    {
        document = (XedDocument *) documents_iter->data;
        g_signal_handlers_disconnect_by_data (document, XED_TRAIL_SAVE_PLUGIN (activatable));
    }

    g_list_free (documents);
}

static void
xed_trail_save_plugin_init (XedTrailSavePlugin *plugin)
{
    xed_debug_message (DEBUG_PLUGINS, "XedTrailSavePlugin initializing");

    plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin, XED_TYPE_TRAIL_SAVE_PLUGIN, XedTrailSavePluginPrivate);
}

static void
xed_trail_save_plugin_dispose (GObject *object)
{
    XedTrailSavePlugin *plugin = XED_TRAIL_SAVE_PLUGIN (object);

    xed_debug_message (DEBUG_PLUGINS, "XedTrailSavePlugin disposing");

    g_clear_object (&plugin->priv->window);

    G_OBJECT_CLASS (xed_trail_save_plugin_parent_class)->dispose (object);
}

static void
xed_trail_save_plugin_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
    XedTrailSavePlugin *plugin = XED_TRAIL_SAVE_PLUGIN (object);

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
xed_trail_save_plugin_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
    XedTrailSavePlugin *plugin = XED_TRAIL_SAVE_PLUGIN (object);

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
xed_trail_save_plugin_class_init (XedTrailSavePluginClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = xed_trail_save_plugin_dispose;
    object_class->set_property = xed_trail_save_plugin_set_property;
    object_class->get_property = xed_trail_save_plugin_get_property;

    g_object_class_override_property (object_class, PROP_WINDOW, "window");

    g_type_class_add_private (object_class, sizeof (XedTrailSavePluginPrivate));
}

static void
xed_trail_save_plugin_class_finalize (XedTrailSavePluginClass *klass)
{
    /* dummy function - used by G_DEFINE_DYNAMIC_TYPE_EXTENDED */
}

static void
xed_window_activatable_iface_init (XedWindowActivatableInterface *iface)
{
    iface->activate = xed_trail_save_plugin_activate;
    iface->deactivate = xed_trail_save_plugin_deactivate;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
    xed_trail_save_plugin_register_type (G_TYPE_MODULE (module));

    peas_object_module_register_extension_type (module,
                                                XED_TYPE_WINDOW_ACTIVATABLE,
                                                XED_TYPE_TRAIL_SAVE_PLUGIN);
}
