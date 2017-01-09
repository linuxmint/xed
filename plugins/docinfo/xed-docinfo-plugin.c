/*
 * xed-docinfo-plugin.c
 *
 * Copyright (C) 2002-2005 Paolo Maggi
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
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xed-docinfo-plugin.h"

#include <string.h> /* For strlen (...) */

#include <glib/gi18n.h>
#include <pango/pango-break.h>
#include <gmodule.h>

#include <xed/xed-window.h>
#include <xed/xed-window-activatable.h>
#include <xed/xed-debug.h>
#include <xed/xed-utils.h>

#define MENU_PATH "/MenuBar/ToolsMenu/ToolsOps_2"

struct _XedDocInfoPluginPrivate
{
    XedWindow *window;

    GtkActionGroup *action_group;
    guint ui_id;

    GtkWidget *dialog;
    GtkWidget *file_name_label;
    GtkWidget *lines_label;
    GtkWidget *words_label;
    GtkWidget *chars_label;
    GtkWidget *chars_ns_label;
    GtkWidget *bytes_label;
    GtkWidget *selection_vbox;
    GtkWidget *selected_lines_label;
    GtkWidget *selected_words_label;
    GtkWidget *selected_chars_label;
    GtkWidget *selected_chars_ns_label;
    GtkWidget *selected_bytes_label;
};

enum
{
    PROP_0,
    PROP_WINDOW
};

static void xed_window_activatable_iface_init (XedWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (XedDocInfoPlugin,
                                xed_docinfo_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (XED_TYPE_WINDOW_ACTIVATABLE,
                                                               xed_window_activatable_iface_init))

static void
calculate_info (XedDocument *doc,
                GtkTextIter *start,
                GtkTextIter *end,
                gint        *chars,
                gint        *words,
                gint        *white_chars,
                gint        *bytes)
{
    gchar *text;

    xed_debug (DEBUG_PLUGINS);

    text = gtk_text_buffer_get_slice (GTK_TEXT_BUFFER (doc), start, end, TRUE);

    *chars = g_utf8_strlen (text, -1);
    *bytes = strlen (text);

    if (*chars > 0)
    {
        PangoLogAttr *attrs;
        gint i;

        attrs = g_new0 (PangoLogAttr, *chars + 1);

        pango_get_log_attrs (text, -1, 0, pango_language_from_string ("C"), attrs, *chars + 1);

        for (i = 0; i < (*chars); i++)
        {
            if (attrs[i].is_white)
            {
                ++(*white_chars);
            }

            if (attrs[i].is_word_start)
            {
                ++(*words);
            }
        }

        g_free (attrs);
    }
    else
    {
        *white_chars = 0;
        *words = 0;
    }

    g_free (text);
}

static void
update_document_info (XedDocInfoPlugin *plugin,
                      XedDocument *doc)
{
    XedDocInfoPluginPrivate *priv;
    GtkTextIter start, end;
    gint words = 0;
    gint chars = 0;
    gint white_chars = 0;
    gint lines = 0;
    gint bytes = 0;
    gchar *tmp_str;
    gchar *doc_name;

    xed_debug (DEBUG_PLUGINS);

    priv = plugin->priv;

    gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (doc), &start, &end);

    lines = gtk_text_buffer_get_line_count (GTK_TEXT_BUFFER (doc));

    calculate_info (doc, &start, &end, &chars, &words, &white_chars, &bytes);

    if (chars == 0)
    {
        lines = 0;
    }

    xed_debug_message (DEBUG_PLUGINS, "Chars: %d", chars);
    xed_debug_message (DEBUG_PLUGINS, "Lines: %d", lines);
    xed_debug_message (DEBUG_PLUGINS, "Words: %d", words);
    xed_debug_message (DEBUG_PLUGINS, "Chars non-space: %d", chars - white_chars);
    xed_debug_message (DEBUG_PLUGINS, "Bytes: %d", bytes);

    doc_name = xed_document_get_short_name_for_display (doc);
    tmp_str = g_strdup_printf ("<span weight=\"bold\">%s</span>", doc_name);
    gtk_label_set_markup (GTK_LABEL (priv->file_name_label), tmp_str);
    g_free (doc_name);
    g_free (tmp_str);

    tmp_str = g_strdup_printf("%d", lines);
    gtk_label_set_text (GTK_LABEL (priv->lines_label), tmp_str);
    g_free (tmp_str);

    tmp_str = g_strdup_printf("%d", words);
    gtk_label_set_text (GTK_LABEL (priv->words_label), tmp_str);
    g_free (tmp_str);

    tmp_str = g_strdup_printf("%d", chars);
    gtk_label_set_text (GTK_LABEL (priv->chars_label), tmp_str);
    g_free (tmp_str);

    tmp_str = g_strdup_printf("%d", chars - white_chars);
    gtk_label_set_text (GTK_LABEL (priv->chars_ns_label), tmp_str);
    g_free (tmp_str);

    tmp_str = g_strdup_printf("%d", bytes);
    gtk_label_set_text (GTK_LABEL (priv->bytes_label), tmp_str);
    g_free (tmp_str);
}

static void
update_selection_info (XedDocInfoPlugin *plugin,
                       XedDocument *doc)
{
    XedDocInfoPluginPrivate *priv;
    gboolean sel;
    GtkTextIter start, end;
    gint words = 0;
    gint chars = 0;
    gint white_chars = 0;
    gint lines = 0;
    gint bytes = 0;
    gchar *tmp_str;

    xed_debug (DEBUG_PLUGINS);

    priv = plugin->priv;

    sel = gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (doc), &start, &end);

    if (sel)
    {
        lines = gtk_text_iter_get_line (&end) - gtk_text_iter_get_line (&start) + 1;

        calculate_info (doc, &start, &end, &chars, &words, &white_chars, &bytes);

        xed_debug_message (DEBUG_PLUGINS, "Selected chars: %d", chars);
        xed_debug_message (DEBUG_PLUGINS, "Selected lines: %d", lines);
        xed_debug_message (DEBUG_PLUGINS, "Selected words: %d", words);
        xed_debug_message (DEBUG_PLUGINS, "Selected chars non-space: %d", chars - white_chars);
        xed_debug_message (DEBUG_PLUGINS, "Selected bytes: %d", bytes);

        gtk_widget_set_sensitive (priv->selection_vbox, TRUE);
    }
    else
    {
        gtk_widget_set_sensitive (priv->selection_vbox, FALSE);

        xed_debug_message (DEBUG_PLUGINS, "Selection empty");
    }

    if (chars == 0)
    {
        lines = 0;
    }

    tmp_str = g_strdup_printf("%d", lines);
    gtk_label_set_text (GTK_LABEL (priv->selected_lines_label), tmp_str);
    g_free (tmp_str);

    tmp_str = g_strdup_printf("%d", words);
    gtk_label_set_text (GTK_LABEL (priv->selected_words_label), tmp_str);
    g_free (tmp_str);

    tmp_str = g_strdup_printf("%d", chars);
    gtk_label_set_text (GTK_LABEL (priv->selected_chars_label), tmp_str);
    g_free (tmp_str);

    tmp_str = g_strdup_printf("%d", chars - white_chars);
    gtk_label_set_text (GTK_LABEL (priv->selected_chars_ns_label), tmp_str);
    g_free (tmp_str);

    tmp_str = g_strdup_printf("%d", bytes);
    gtk_label_set_text (GTK_LABEL (priv->selected_bytes_label), tmp_str);
    g_free (tmp_str);
}

static void
docinfo_dialog_response_cb (GtkDialog        *widget,
                            gint              res_id,
                            XedDocInfoPlugin *plugin)
{
    XedDocInfoPluginPrivate *priv;

    priv = plugin->priv;

    switch (res_id)
    {
        case GTK_RESPONSE_CLOSE:
        {
            xed_debug_message (DEBUG_PLUGINS, "GTK_RESPONSE_CLOSE");
            gtk_widget_destroy (priv->dialog);
            break;
        }

        case GTK_RESPONSE_OK:
        {
            XedDocument *doc;

            xed_debug_message (DEBUG_PLUGINS, "GTK_RESPONSE_OK");

            doc = xed_window_get_active_document (priv->window);

            update_document_info (plugin, doc);
            update_selection_info (plugin, doc);
            break;
        }
    }
}

static void
create_docinfo_dialog (XedDocInfoPlugin *plugin)
{
    XedDocInfoPluginPrivate *priv;
    gchar *data_dir;
    gchar *ui_file;
    GtkWidget *content;
    GtkWidget *error_widget;
    gboolean ret;

    xed_debug (DEBUG_PLUGINS);

    priv = plugin->priv;

    data_dir = peas_extension_base_get_data_dir (PEAS_EXTENSION_BASE (plugin));
    ui_file = g_build_filename (data_dir, "docinfo.ui", NULL);
    ret = xed_utils_get_ui_objects (ui_file,
                                    NULL,
                                    &error_widget,
                                    "dialog", &priv->dialog,
                                    "docinfo_dialog_content", &content,
                                    "file_name_label", &priv->file_name_label,
                                    "words_label", &priv->words_label,
                                    "bytes_label", &priv->bytes_label,
                                    "lines_label", &priv->lines_label,
                                    "chars_label", &priv->chars_label,
                                    "chars_ns_label", &priv->chars_ns_label,
                                    "selection_vbox", &priv->selection_vbox,
                                    "selected_words_label", &priv->selected_words_label,
                                    "selected_bytes_label", &priv->selected_bytes_label,
                                    "selected_lines_label", &priv->selected_lines_label,
                                    "selected_chars_label", &priv->selected_chars_label,
                                    "selected_chars_ns_label", &priv->selected_chars_ns_label,
                                    NULL);

    g_free (data_dir);
    g_free (ui_file);

    if (!ret)
    {
        const gchar *err_message;

        err_message = gtk_label_get_label (GTK_LABEL (error_widget));
        xed_warning (GTK_WINDOW (priv->window), "%s", err_message);

        gtk_widget_destroy (error_widget);

        return;
    }

    gtk_dialog_set_default_response (GTK_DIALOG (priv->dialog), GTK_RESPONSE_OK);
    gtk_window_set_transient_for (GTK_WINDOW (priv->dialog), GTK_WINDOW (priv->window));

    g_signal_connect (priv->dialog, "destroy",
                      G_CALLBACK (gtk_widget_destroyed), &priv->dialog);
    g_signal_connect (priv->dialog, "response",
                      G_CALLBACK (docinfo_dialog_response_cb), plugin);
}

static void
docinfo_cb (GtkAction        *action,
            XedDocInfoPlugin *plugin)
{
    XedDocInfoPluginPrivate *priv;
    XedDocument *doc;

    xed_debug (DEBUG_PLUGINS);

    priv = plugin->priv;

    doc = xed_window_get_active_document (priv->window);

    if (priv->dialog != NULL)
    {
        gtk_window_present (GTK_WINDOW (priv->dialog));
        gtk_widget_grab_focus (GTK_WIDGET (priv->dialog));
    }
    else
    {
        create_docinfo_dialog (plugin);
        gtk_widget_show (GTK_WIDGET (priv->dialog));
    }

    update_document_info (plugin, doc);
    update_selection_info (plugin, doc);
}

static const GtkActionEntry action_entries[] =
{
    { "DocumentStatistics",
      NULL,
      N_("_Document Statistics"),
      NULL,
      N_("Get statistical information on the current document"),
      G_CALLBACK (docinfo_cb) }
};

static void
xed_docinfo_plugin_init (XedDocInfoPlugin *plugin)
{
    xed_debug_message (DEBUG_PLUGINS, "XedDocInfoPlugin initializing");
    plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin, XED_TYPE_DOCINFO_PLUGIN, XedDocInfoPluginPrivate);
}

static void
xed_docinfo_plugin_dispose (GObject *object)
{
    XedDocInfoPlugin *plugin = XED_DOCINFO_PLUGIN (object);

    xed_debug_message (DEBUG_PLUGINS, "XedDocInfoPlugin dispose");

    g_clear_object (&plugin->priv->action_group);
    g_clear_object (&plugin->priv->window);

    G_OBJECT_CLASS (xed_docinfo_plugin_parent_class)->dispose (object);
}

static void
xed_docinfo_plugin_finalize (GObject *object)
{
    xed_debug_message (DEBUG_PLUGINS, "XedDocInfoPlugin finalizing");

    G_OBJECT_CLASS (xed_docinfo_plugin_parent_class)->finalize (object);
}

static void
xed_docinfo_plugin_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
    XedDocInfoPlugin *plugin = XED_DOCINFO_PLUGIN (object);

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
xed_docinfo_plugin_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
    XedDocInfoPlugin *plugin = XED_DOCINFO_PLUGIN (object);

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
update_ui (XedDocInfoPlugin *plugin)
{
    XedDocInfoPluginPrivate *priv;
    XedView *view;

    xed_debug (DEBUG_PLUGINS);

    priv = plugin->priv;

    view = xed_window_get_active_view (priv->window);

    gtk_action_group_set_sensitive (priv->action_group, (view != NULL));

    if (priv->dialog != NULL)
    {
        gtk_dialog_set_response_sensitive (GTK_DIALOG (priv->dialog), GTK_RESPONSE_OK, (view != NULL));
    }
}

static void
xed_docinfo_plugin_activate (XedWindowActivatable *activatable)
{
    XedDocInfoPluginPrivate *priv;
    GtkUIManager *manager;

    xed_debug (DEBUG_PLUGINS);

    priv = XED_DOCINFO_PLUGIN (activatable)->priv;
    manager = xed_window_get_ui_manager (priv->window);

    priv->action_group = gtk_action_group_new ("XedDocinfoPluginActions");
    gtk_action_group_set_translation_domain (priv->action_group, GETTEXT_PACKAGE);
    gtk_action_group_add_actions (priv->action_group, action_entries, G_N_ELEMENTS (action_entries), activatable);

    gtk_ui_manager_insert_action_group (manager, priv->action_group, -1);

    priv->ui_id = gtk_ui_manager_new_merge_id (manager);

    gtk_ui_manager_add_ui (manager,
                           priv->ui_id,
                           MENU_PATH,
                           "DocumentStatistics",
                           "DocumentStatistics",
                           GTK_UI_MANAGER_MENUITEM,
                           FALSE);

    update_ui (XED_DOCINFO_PLUGIN (activatable));
}

static void
xed_docinfo_plugin_deactivate (XedWindowActivatable *activatable)
{
    XedDocInfoPluginPrivate *priv;
    GtkUIManager *manager;

    xed_debug (DEBUG_PLUGINS);

    priv = XED_DOCINFO_PLUGIN (activatable)->priv;
    manager = xed_window_get_ui_manager (priv->window);

    gtk_ui_manager_remove_ui (manager, priv->ui_id);
    gtk_ui_manager_remove_action_group (manager, priv->action_group);
}

static void
xed_docinfo_plugin_update_state (XedWindowActivatable *activatable)
{
    xed_debug (DEBUG_PLUGINS);

    update_ui (XED_DOCINFO_PLUGIN (activatable));
}

static void
xed_docinfo_plugin_class_init (XedDocInfoPluginClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = xed_docinfo_plugin_dispose;
    object_class->finalize = xed_docinfo_plugin_finalize;
    object_class->set_property = xed_docinfo_plugin_set_property;
    object_class->get_property = xed_docinfo_plugin_get_property;

    g_object_class_override_property (object_class, PROP_WINDOW, "window");

    g_type_class_add_private (klass, sizeof (XedDocInfoPluginPrivate));
}

static void
xed_window_activatable_iface_init (XedWindowActivatableInterface *iface)
{
    iface->activate = xed_docinfo_plugin_activate;
    iface->deactivate = xed_docinfo_plugin_deactivate;
    iface->update_state = xed_docinfo_plugin_update_state;
}

static void
xed_docinfo_plugin_class_finalize (XedDocInfoPluginClass *klass)
{
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
    xed_docinfo_plugin_register_type (G_TYPE_MODULE (module));

    peas_object_module_register_extension_type (module,
                                                XED_TYPE_WINDOW_ACTIVATABLE,
                                                XED_TYPE_DOCINFO_PLUGIN);
}
