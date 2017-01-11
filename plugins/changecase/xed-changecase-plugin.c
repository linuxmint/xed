/*
 * xed-changecase-plugin.c
 *
 * Copyright (C) 2004-2005 - Paolo Borelli
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

#include "xed-changecase-plugin.h"

#include <glib/gi18n.h>
#include <gmodule.h>

#include <xed/xed-window.h>
#include <xed/xed-window-activatable.h>
#include <xed/xed-debug.h>

static void xed_window_activatable_iface_init (XedWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (XedChangecasePlugin,
                                xed_changecase_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (XED_TYPE_WINDOW_ACTIVATABLE,
                                                               xed_window_activatable_iface_init))

struct _XedChangecasePluginPrivate
{
    XedWindow *window;

    GtkActionGroup *action_group;
    guint           ui_id;
};

enum
{
    PROP_0,
    PROP_WINDOW
};

typedef enum {
    TO_UPPER_CASE,
    TO_LOWER_CASE,
    INVERT_CASE,
    TO_TITLE_CASE,
} ChangeCaseChoice;

static void
do_upper_case (GtkTextBuffer *buffer,
               GtkTextIter   *start,
               GtkTextIter   *end)
{
    GString *s = g_string_new (NULL);

    while (!gtk_text_iter_is_end (start) && !gtk_text_iter_equal (start, end))
    {
        gunichar c, nc;

        c = gtk_text_iter_get_char (start);
        nc = g_unichar_toupper (c);
        g_string_append_unichar (s, nc);

        gtk_text_iter_forward_char (start);
    }

    gtk_text_buffer_delete_selection (buffer, TRUE, TRUE);
    gtk_text_buffer_insert_at_cursor (buffer, s->str, s->len);

    g_string_free (s, TRUE);
}

static void
do_lower_case (GtkTextBuffer *buffer,
               GtkTextIter   *start,
               GtkTextIter   *end)
{
    GString *s = g_string_new (NULL);

    while (!gtk_text_iter_is_end (start) && !gtk_text_iter_equal (start, end))
    {
        gunichar c, nc;

        c = gtk_text_iter_get_char (start);
        nc = g_unichar_tolower (c);
        g_string_append_unichar (s, nc);

        gtk_text_iter_forward_char (start);
    }

    gtk_text_buffer_delete_selection (buffer, TRUE, TRUE);
    gtk_text_buffer_insert_at_cursor (buffer, s->str, s->len);

    g_string_free (s, TRUE);
}

static void
do_invert_case (GtkTextBuffer *buffer,
                GtkTextIter   *start,
                GtkTextIter   *end)
{
    GString *s = g_string_new (NULL);

    while (!gtk_text_iter_is_end (start) && !gtk_text_iter_equal (start, end))
    {
        gunichar c, nc;

        c = gtk_text_iter_get_char (start);
        if (g_unichar_islower (c))
        {
            nc = g_unichar_toupper (c);
        }
        else
        {
            nc = g_unichar_tolower (c);
        }
        g_string_append_unichar (s, nc);

        gtk_text_iter_forward_char (start);
    }

    gtk_text_buffer_delete_selection (buffer, TRUE, TRUE);
    gtk_text_buffer_insert_at_cursor (buffer, s->str, s->len);

    g_string_free (s, TRUE);
}

static void
do_title_case (GtkTextBuffer *buffer,
               GtkTextIter   *start,
               GtkTextIter   *end)
{
    GString *s = g_string_new (NULL);

    while (!gtk_text_iter_is_end (start) && !gtk_text_iter_equal (start, end))
    {
        gunichar c, nc;

        c = gtk_text_iter_get_char (start);
        if (gtk_text_iter_starts_word (start))
        {
            nc = g_unichar_totitle (c);
        }
        else
        {
            nc = g_unichar_tolower (c);
        }
        g_string_append_unichar (s, nc);

        gtk_text_iter_forward_char (start);
    }

    gtk_text_buffer_delete_selection (buffer, TRUE, TRUE);
    gtk_text_buffer_insert_at_cursor (buffer, s->str, s->len);

    g_string_free (s, TRUE);
}

static void
change_case (XedWindow        *window,
             ChangeCaseChoice  choice)
{
    XedDocument *doc;
    GtkTextIter start, end;

    xed_debug (DEBUG_PLUGINS);

    doc = xed_window_get_active_document (window);
    g_return_if_fail (doc != NULL);

    if (!gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (doc), &start, &end))
    {
        return;
    }

    gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER (doc));

    switch (choice)
    {
        case TO_UPPER_CASE:
            do_upper_case (GTK_TEXT_BUFFER (doc), &start, &end);
            break;
        case TO_LOWER_CASE:
            do_lower_case (GTK_TEXT_BUFFER (doc), &start, &end);
            break;
        case INVERT_CASE:
            do_invert_case (GTK_TEXT_BUFFER (doc), &start, &end);
            break;
        case TO_TITLE_CASE:
            do_title_case (GTK_TEXT_BUFFER (doc), &start, &end);
            break;
        default:
            g_return_if_reached ();
    }

    gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (doc));
}

static void
upper_case_cb (GtkAction *action,
               XedWindow *window)
{
    change_case (window, TO_UPPER_CASE);
}

static void
lower_case_cb (GtkAction *action,
               XedWindow *window)
{
    change_case (window, TO_LOWER_CASE);
}

static void
invert_case_cb (GtkAction *action,
                XedWindow *window)
{
    change_case (window, INVERT_CASE);
}

static void
title_case_cb (GtkAction *action,
               XedWindow *window)
{
    change_case (window, TO_TITLE_CASE);
}

static const GtkActionEntry action_entries[] =
{
    { "ChangeCase", NULL, N_("C_hange Case") },
    { "UpperCase", NULL, N_("All _Upper Case"), NULL,
      N_("Change selected text to upper case"),
      G_CALLBACK (upper_case_cb) },
    { "LowerCase", NULL, N_("All _Lower Case"), NULL,
      N_("Change selected text to lower case"),
      G_CALLBACK (lower_case_cb) },
    { "InvertCase", NULL, N_("_Invert Case"), NULL,
      N_("Invert the case of selected text"),
      G_CALLBACK (invert_case_cb) },
    { "TitleCase", NULL, N_("_Title Case"), NULL,
      N_("Capitalize the first letter of each selected word"),
      G_CALLBACK (title_case_cb) }
};

const gchar submenu[] =
"<ui>"
"  <menubar name='MenuBar'>"
"    <menu name='EditMenu' action='Edit'>"
"      <placeholder name='EditOps_6'>"
"        <menu action='ChangeCase'>"
"          <menuitem action='UpperCase'/>"
"          <menuitem action='LowerCase'/>"
"          <menuitem action='InvertCase'/>"
"          <menuitem action='TitleCase'/>"
"        </menu>"
"      </placeholder>"
"    </menu>"
"  </menubar>"
"</ui>";

static void
xed_changecase_plugin_init (XedChangecasePlugin *plugin)
{
    xed_debug_message (DEBUG_PLUGINS, "XedChangecasePlugin initializing");

    plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin, XED_TYPE_CHANGECASE_PLUGIN, XedChangecasePluginPrivate);
}

static void
xed_changecase_plugin_dispose (GObject *object)
{
    XedChangecasePlugin *plugin = XED_CHANGECASE_PLUGIN (object);

    xed_debug_message (DEBUG_PLUGINS, "XedChangecasePlugin disposing");

    g_clear_object (&plugin->priv->window);
    g_clear_object (&plugin->priv->action_group);

    G_OBJECT_CLASS (xed_changecase_plugin_parent_class)->dispose (object);
}

static void
xed_changecase_plugin_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
    XedChangecasePlugin *plugin = XED_CHANGECASE_PLUGIN (object);

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
xed_changecase_plugin_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
    XedChangecasePlugin *plugin = XED_CHANGECASE_PLUGIN (object);

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
update_ui (XedChangecasePlugin *plugin)
{
    GtkTextView *view;
    GtkAction *action;
    gboolean sensitive = FALSE;

    xed_debug (DEBUG_PLUGINS);

    view = GTK_TEXT_VIEW (xed_window_get_active_view (plugin->priv->window));

    if (view != NULL)
    {
        GtkTextBuffer *buffer;

        buffer = gtk_text_view_get_buffer (view);
        sensitive = (gtk_text_view_get_editable (view) && gtk_text_buffer_get_has_selection (buffer));
    }

    action = gtk_action_group_get_action (plugin->priv->action_group, "ChangeCase");
    gtk_action_set_sensitive (action, sensitive);
}

static void
xed_changecase_plugin_activate (XedWindowActivatable *activatable)
{
    XedChangecasePluginPrivate *priv;
    GtkUIManager *manager;
    GError *error = NULL;

    xed_debug (DEBUG_PLUGINS);

    priv = XED_CHANGECASE_PLUGIN (activatable)->priv;

    manager = xed_window_get_ui_manager (priv->window);

    priv->action_group = gtk_action_group_new ("XedChangecasePluginActions");
    gtk_action_group_set_translation_domain (priv->action_group, GETTEXT_PACKAGE);
    gtk_action_group_add_actions (priv->action_group, action_entries, G_N_ELEMENTS (action_entries), priv->window);

    gtk_ui_manager_insert_action_group (manager, priv->action_group, -1);

    priv->ui_id = gtk_ui_manager_add_ui_from_string (manager, submenu, -1, &error);
    if (priv->ui_id == 0)
    {
        g_warning ("%s", error->message);
        return;
    }

    update_ui (XED_CHANGECASE_PLUGIN (activatable));
}

static void
xed_changecase_plugin_deactivate (XedWindowActivatable *activatable)
{
    XedChangecasePluginPrivate *priv;
    GtkUIManager *manager;

    xed_debug (DEBUG_PLUGINS);

    priv = XED_CHANGECASE_PLUGIN (activatable)->priv;

    manager = xed_window_get_ui_manager (priv->window);

    gtk_ui_manager_remove_ui (manager, priv->ui_id);
    gtk_ui_manager_remove_action_group (manager, priv->action_group);
}

static void
xed_changecase_plugin_update_state (XedWindowActivatable *activatable)
{
    xed_debug (DEBUG_PLUGINS);

    update_ui (XED_CHANGECASE_PLUGIN (activatable));
}

static void
xed_changecase_plugin_class_init (XedChangecasePluginClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = xed_changecase_plugin_dispose;
    object_class->set_property = xed_changecase_plugin_set_property;
    object_class->get_property = xed_changecase_plugin_get_property;

    g_object_class_override_property (object_class, PROP_WINDOW, "window");

    g_type_class_add_private (klass, sizeof (XedChangecasePluginPrivate));
}

static void
xed_changecase_plugin_class_finalize (XedChangecasePluginClass *klass)
{
    /* dummy function - used by G_DEFINE_DYNAMIC_TYPE_EXTENDED */
}

static void
xed_window_activatable_iface_init (XedWindowActivatableInterface *iface)
{
    iface->activate = xed_changecase_plugin_activate;
    iface->deactivate = xed_changecase_plugin_deactivate;
    iface->update_state = xed_changecase_plugin_update_state;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
    xed_changecase_plugin_register_type (G_TYPE_MODULE (module));

    peas_object_module_register_extension_type (module,
                                                XED_TYPE_WINDOW_ACTIVATABLE,
                                                XED_TYPE_CHANGECASE_PLUGIN);
}
