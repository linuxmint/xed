/*
 * xed-settings.c
 * This file is part of xed
 *
 * Copyright (C) 2002-2005 - Paolo Maggi
 *               2009 - Ignacio Casal Quinteiro
 *
 * xed is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xed is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xed; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include <string.h>

#include <gtksourceview/gtksource.h>

#include "xed-settings.h"
#include "xed-app.h"
#include "xed-debug.h"
#include "xed-view.h"
#include "xed-window.h"
#include "xed-notebook.h"
#include "xed-plugins-engine.h"
#include "xed-dirs.h"
#include "xed-utils.h"
#include "xed-window-private.h"

#define XED_SETTINGS_SYSTEM_FONT "monospace-font-name"

#define XED_SETTINGS_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), XED_TYPE_SETTINGS, XedSettingsPrivate))

struct _XedSettingsPrivate
{
    GSettings *interface;
    GSettings *editor;
    GSettings *ui;

    gchar *old_scheme;
};

G_DEFINE_TYPE (XedSettings, xed_settings, G_TYPE_OBJECT)

static void
xed_settings_finalize (GObject *object)
{
    XedSettings *xs = XED_SETTINGS (object);

    g_free (xs->priv->old_scheme);

    G_OBJECT_CLASS (xed_settings_parent_class)->finalize (object);
}

static void
xed_settings_dispose (GObject *object)
{
    XedSettings *xs = XED_SETTINGS (object);

    if (xs->priv->interface != NULL)
    {
        g_object_unref (xs->priv->interface);
        xs->priv->interface = NULL;
    }

    if (xs->priv->editor != NULL)
    {
        g_object_unref (xs->priv->editor);
        xs->priv->editor = NULL;
    }

    if (xs->priv->ui != NULL)
    {
        g_object_unref (xs->priv->ui);
        xs->priv->ui = NULL;
    }

    G_OBJECT_CLASS (xed_settings_parent_class)->dispose (object);
}

static void
set_font (XedSettings *xs,
          const gchar *font)
{
    GList *views, *l;
    guint ts;

    ts = g_settings_get_uint (xs->priv->editor, XED_SETTINGS_TABS_SIZE);

    views = xed_app_get_views (XED_APP (g_application_get_default ()));

    for (l = views; l != NULL; l = g_list_next (l))
    {
        /* Note: we use def=FALSE to avoid XedView to query dconf */
        xed_view_set_font (XED_VIEW (l->data), FALSE, font);

        gtk_source_view_set_tab_width (GTK_SOURCE_VIEW (l->data), ts);
    }

    g_list_free (views);
}

static void
on_system_font_changed (GSettings   *settings,
                        const gchar *key,
                        XedSettings *xs)
{

    gboolean use_default_font;
    gchar *font;

    use_default_font = g_settings_get_boolean (xs->priv->editor, XED_SETTINGS_USE_DEFAULT_FONT);
    if (!use_default_font)
    {
        return;
    }

    font = g_settings_get_string (settings, key);

    set_font (xs, font);

    g_free (font);
}

static void
on_use_default_font_changed (GSettings   *settings,
                             const gchar *key,
                             XedSettings *xs)
{
    gboolean def;
    gchar *font;

    def = g_settings_get_boolean (settings, key);

    if (def)
    {
        font = g_settings_get_string (xs->priv->interface, XED_SETTINGS_SYSTEM_FONT);
    }
    else
    {
        font = g_settings_get_string (xs->priv->editor, XED_SETTINGS_EDITOR_FONT);
    }

    set_font (xs, font);

    g_free (font);
}

static void
on_editor_font_changed (GSettings   *settings,
                        const gchar *key,
                        XedSettings *xs)
{
    gboolean use_default_font;
    gchar *font;

    use_default_font = g_settings_get_boolean (xs->priv->editor, XED_SETTINGS_USE_DEFAULT_FONT);
    if (use_default_font)
    {
        return;
    }

    font = g_settings_get_string (settings, key);

    set_font (xs, font);

    g_free (font);
}

static void
on_prefer_dark_theme_changed (GSettings   *settings,
                              const gchar *key,
                              XedSettings *xs)
{
    GtkSettings *gtk_settings;
    gboolean prefer_dark_theme;

    prefer_dark_theme = g_settings_get_boolean (xs->priv->editor, XED_SETTINGS_PREFER_DARK_THEME);
    gtk_settings = gtk_settings_get_default ();

    g_object_set (G_OBJECT (gtk_settings), "gtk-application-prefer-dark-theme", prefer_dark_theme, NULL);
}

static void
on_scheme_changed (GSettings   *settings,
                   const gchar *key,
                   XedSettings *xs)
{
    GtkSourceStyleSchemeManager *manager;
    GtkSourceStyleScheme *style;
    gchar *scheme;
    GList *docs;
    GList *l;

    scheme = g_settings_get_string (settings, key);

    if (xs->priv->old_scheme != NULL && (strcmp (scheme, xs->priv->old_scheme) == 0))
    {
        return;
    }

    g_free (xs->priv->old_scheme);
    xs->priv->old_scheme = scheme;

    manager = gtk_source_style_scheme_manager_get_default ();
    style = gtk_source_style_scheme_manager_get_scheme (manager, scheme);

    if (style == NULL)
    {
        g_warning ("Default style scheme '%s' not found, falling back to 'classic'", scheme);

        style = gtk_source_style_scheme_manager_get_scheme (manager, "classic");

        if (style == NULL)
        {
            g_warning ("Style scheme 'classic' cannot be found, check your GtkSourceView installation.");
            return;
        }
    }

    docs = xed_app_get_documents (XED_APP (g_application_get_default ()));
    for (l = docs; l != NULL; l = g_list_next (l))
    {
        g_return_if_fail (GTK_SOURCE_IS_BUFFER (l->data));

        gtk_source_buffer_set_style_scheme (GTK_SOURCE_BUFFER (l->data), style);
    }

    g_list_free (docs);
}

static void
on_auto_save_changed (GSettings   *settings,
                      const gchar *key,
                      XedSettings *xs)
{
    GList *docs, *l;
    gboolean auto_save;

    auto_save = g_settings_get_boolean (settings, key);

    docs = xed_app_get_documents (XED_APP (g_application_get_default ()));

    for (l = docs; l != NULL; l = g_list_next (l))
    {
        XedTab *tab = xed_tab_get_from_document (XED_DOCUMENT (l->data));

        xed_tab_set_auto_save_enabled (tab, auto_save);
    }

    g_list_free (docs);
}

static void
on_auto_save_interval_changed (GSettings   *settings,
                               const gchar *key,
                               XedSettings *xs)
{
    GList *docs, *l;
    gint auto_save_interval;

    g_settings_get (settings, key, "u", &auto_save_interval);

    docs = xed_app_get_documents (XED_APP (g_application_get_default ()));

    for (l = docs; l != NULL; l = g_list_next (l))
    {
        XedTab *tab = xed_tab_get_from_document (XED_DOCUMENT (l->data));

        xed_tab_set_auto_save_interval (tab, auto_save_interval);
    }

    g_list_free (docs);
}

static void
on_syntax_highlighting_changed (GSettings   *settings,
                                const gchar *key,
                                XedSettings *xs)
{
    GList *docs, *l;
    gboolean enable;

    enable = g_settings_get_boolean (settings, key);

    docs = xed_app_get_documents (XED_APP (g_application_get_default ()));

    for (l = docs; l != NULL; l = g_list_next (l))
    {
        gtk_source_buffer_set_highlight_syntax (GTK_SOURCE_BUFFER (l->data), enable);
    }

    g_list_free (docs);
}

static void
on_enable_tab_scrolling_changed (GSettings   *settings,
                                 const gchar *key,
                                 XedSettings *xs)
{
    const GList *windows;
    gboolean enable;

    enable = g_settings_get_boolean (settings, key);

    windows = xed_app_get_main_windows (XED_APP (g_application_get_default ()));
    while (windows != NULL)
    {
        XedNotebook *notebook;

        notebook = XED_NOTEBOOK (_xed_window_get_notebook (windows->data));
        xed_notebook_set_tab_scrolling_enabled (notebook, enable);

        windows = g_list_next (windows);
    }
}

static void
on_max_recents_changed (GSettings   *settings,
                        const gchar *key,
                        XedSettings *xs)
{
    /* FIXME: we have no way at the moment to trigger the
     * update of the inline recents in the File menu */
}

static void
xed_settings_init (XedSettings *xs)
{
    xs->priv = XED_SETTINGS_GET_PRIVATE (xs);

    xs->priv->old_scheme = NULL;
    xs->priv->editor = g_settings_new ("org.x.editor.preferences.editor");
    xs->priv->ui = g_settings_new ("org.x.editor.preferences.ui");

    /* Load settings */
    xs->priv->interface = g_settings_new ("org.gnome.desktop.interface");

    g_signal_connect (xs->priv->interface, "changed::monospace-font-name",
                      G_CALLBACK (on_system_font_changed), xs);

    /* editor changes */
    g_signal_connect (xs->priv->editor, "changed::use-default-font",
                      G_CALLBACK (on_use_default_font_changed), xs);
    g_signal_connect (xs->priv->editor, "changed::editor-font",
                      G_CALLBACK (on_editor_font_changed), xs);
    g_signal_connect (xs->priv->editor, "changed::prefer-dark-theme",
                      G_CALLBACK (on_prefer_dark_theme_changed), xs);
    g_signal_connect (xs->priv->editor, "changed::scheme",
                      G_CALLBACK (on_scheme_changed), xs);
    g_signal_connect (xs->priv->editor, "changed::auto-save",
                      G_CALLBACK (on_auto_save_changed), xs);
    g_signal_connect (xs->priv->editor, "changed::auto-save-interval",
                      G_CALLBACK (on_auto_save_interval_changed), xs);
    g_signal_connect (xs->priv->editor, "changed::syntax-highlighting",
                      G_CALLBACK (on_syntax_highlighting_changed), xs);
    g_signal_connect (xs->priv->ui, "changed::enable-tab-scrolling",
                      G_CALLBACK (on_enable_tab_scrolling_changed), xs);

    /* ui changes */
    g_signal_connect (xs->priv->ui, "changed::max-recents",
                      G_CALLBACK (on_max_recents_changed), xs);
}

static void
xed_settings_class_init (XedSettingsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = xed_settings_finalize;
    object_class->dispose = xed_settings_dispose;

    g_type_class_add_private (object_class, sizeof (XedSettingsPrivate));
}

GObject *
xed_settings_new ()
{
    return g_object_new (XED_TYPE_SETTINGS, NULL);
}

gchar *
xed_settings_get_system_font (XedSettings *xs)
{
    gchar *system_font;

    g_return_val_if_fail (XED_IS_SETTINGS (xs), NULL);

    system_font = g_settings_get_string (xs->priv->interface, "monospace-font-name");

    return system_font;
}

GSList *
xed_settings_get_list (GSettings   *settings,
                       const gchar *key)
{
    GSList *list = NULL;
    gchar **values;
    gsize i;

    g_return_val_if_fail (G_IS_SETTINGS (settings), NULL);
    g_return_val_if_fail (key != NULL, NULL);

    values = g_settings_get_strv (settings, key);
    i = 0;

    while (values[i] != NULL)
    {
        list = g_slist_prepend (list, values[i]);
        i++;
    }

    g_free (values);

    return g_slist_reverse (list);
}

void
xed_settings_set_list (GSettings    *settings,
                       const gchar  *key,
                       const GSList *list)
{
    gchar **values = NULL;
    const GSList *l;

    g_return_if_fail (G_IS_SETTINGS (settings));
    g_return_if_fail (key != NULL);

    if (list != NULL)
    {
        gint i, len;

        len = g_slist_length ((GSList *)list);
        values = g_new (gchar *, len + 1);

        for (l = list, i = 0; l != NULL; l = g_slist_next (l), i++)
        {
            values[i] = l->data;
        }
        values [i] = NULL;
    }

    g_settings_set_strv (settings, key, (const gchar * const *)values);
    g_free (values);
}
