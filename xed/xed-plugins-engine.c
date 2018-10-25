/*
 * xed-plugins-engine.c
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
#include <girepository.h>

#include "xed-plugins-engine.h"
#include "xed-debug.h"
#include "xed-app.h"
#include "xed-dirs.h"
#include "xed-settings.h"
#include "xed-utils.h"

G_DEFINE_TYPE (XedPluginsEngine, xed_plugins_engine, PEAS_TYPE_ENGINE)

struct _XedPluginsEnginePrivate
{
    GSettings *plugin_settings;
};

XedPluginsEngine *default_engine = NULL;

static void
xed_plugins_engine_init (XedPluginsEngine *engine)
{
    gchar *typelib_dir;
    GError *error = NULL;
    const GList *all_plugins, *l;

    xed_debug (DEBUG_PLUGINS);

    engine->priv = G_TYPE_INSTANCE_GET_PRIVATE (engine, XED_TYPE_PLUGINS_ENGINE, XedPluginsEnginePrivate);

    engine->priv->plugin_settings = g_settings_new ("org.x.editor.plugins");

    peas_engine_enable_loader (PEAS_ENGINE (engine), "python3");

    typelib_dir = g_build_filename (xed_dirs_get_xed_lib_dir (), "girepository-1.0", NULL);

    if (!g_irepository_require_private (g_irepository_get_default (), typelib_dir, "Xed", "1.0", 0, &error))
    {
        g_warning ("Could not load Xed repository: %s", error->message);
        g_error_free (error);
        error = NULL;
    }

    g_free (typelib_dir);

    /* This should be moved to libpeas */
    if (!g_irepository_require (g_irepository_get_default (), "Peas", "1.0", 0, &error))
    {
        g_warning ("Could not load Peas repository: %s", error->message);
        g_error_free (error);
        error = NULL;
    }

    if (!g_irepository_require (g_irepository_get_default (), "PeasGtk", "1.0", 0, &error))
    {
        g_warning ("Could not load PeasGtk repository: %s", error->message);
        g_error_free (error);
        error = NULL;
    }

    peas_engine_add_search_path (PEAS_ENGINE (engine),
                                 xed_dirs_get_user_plugins_dir (),
                                 xed_dirs_get_user_plugins_dir ());

    peas_engine_add_search_path (PEAS_ENGINE (engine),
                                 xed_dirs_get_xed_plugins_dir(),
                                 xed_dirs_get_xed_plugins_data_dir());

    g_settings_bind (engine->priv->plugin_settings,
                     XED_SETTINGS_ACTIVE_PLUGINS,
                     engine,
                     "loaded-plugins",
                     G_SETTINGS_BIND_DEFAULT);

    /* Load our builtin plugins */
    all_plugins = peas_engine_get_plugin_list (PEAS_ENGINE (engine));

    for (l = all_plugins; l != NULL; l = l->next)
    {
        if (peas_plugin_info_is_builtin (l->data))
        {
            gboolean loaded;

            loaded = peas_engine_load_plugin (PEAS_ENGINE (engine), l->data);
            if (!loaded)
            {
                g_warning ("Failed to load builtin plugin: %s", peas_plugin_info_get_name (l->data));
            }
        }
    }
}

static void
xed_plugins_engine_dispose (GObject *object)
{
    XedPluginsEngine *engine = XED_PLUGINS_ENGINE (object);

    if (engine->priv->plugin_settings != NULL)
    {
        g_object_unref (engine->priv->plugin_settings);
        engine->priv->plugin_settings = NULL;
    }

    G_OBJECT_CLASS (xed_plugins_engine_parent_class)->dispose (object);
}

static void
xed_plugins_engine_class_init (XedPluginsEngineClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = xed_plugins_engine_dispose;

    g_type_class_add_private (klass, sizeof (XedPluginsEnginePrivate));
}

XedPluginsEngine *
xed_plugins_engine_get_default (void)
{
    if (default_engine == NULL)
    {
        default_engine = XED_PLUGINS_ENGINE (g_object_new (XED_TYPE_PLUGINS_ENGINE, NULL));
        g_object_add_weak_pointer (G_OBJECT (default_engine), (gpointer) &default_engine);
    }

    return default_engine;
}
