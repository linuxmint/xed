/*
 * xed-dirs.c
 * This file is part of xed
 *
 * Copyright (C) 2008 Ignacio Casal Quinteiro
 * Copyright (C) 2011 Perberos
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xed-dirs.h"

gchar *
xed_dirs_get_user_config_dir (void)
{
    gchar* config_dir = NULL;

    config_dir = g_build_filename(g_get_user_config_dir(), "xed", NULL);

    return config_dir;
}

gchar *
xed_dirs_get_user_cache_dir (void)
{
    const gchar* cache_dir;

    cache_dir = g_get_user_cache_dir();

    return g_build_filename(cache_dir, "xed", NULL);
}

gchar *
xed_dirs_get_user_styles_dir (void)
{
    gchar *user_style_dir;

    user_style_dir = g_build_filename (g_get_user_data_dir (), "xed", "styles", NULL);
}

gchar *
xed_dirs_get_user_plugins_dir (void)
{
    gchar* plugin_dir;

    plugin_dir = g_build_filename(g_get_user_data_dir(), "xed", "plugins", NULL);

    return plugin_dir;
}

gchar *
xed_dirs_get_user_accels_file (void)
{
    gchar* accels = NULL;
    gchar *config_dir = NULL;

    config_dir = xed_dirs_get_user_config_dir();
    accels = g_build_filename(config_dir, "accels", NULL);

    g_free(config_dir);

    return accels;
}

gchar *
xed_dirs_get_xed_data_dir (void)
{
    return g_build_filename(DATADIR, "xed", NULL);
}

gchar *
xed_dirs_get_xed_locale_dir (void)
{
    return g_build_filename(DATADIR, "locale", NULL);
}

gchar *
xed_dirs_get_xed_lib_dir (void)
{
    return g_build_filename(LIBDIR, "xed", NULL);
}

gchar *
xed_dirs_get_xed_plugins_dir (void)
{
    gchar* lib_dir;
    gchar* plugin_dir;

    lib_dir = xed_dirs_get_xed_lib_dir();

    plugin_dir = g_build_filename(lib_dir, "plugins", NULL);
    g_free(lib_dir);

    return plugin_dir;
}

gchar *
xed_dirs_get_xed_plugins_data_dir (void)
{
    gchar* data_dir;
    gchar* plugin_data_dir;

    data_dir = xed_dirs_get_xed_data_dir ();

    plugin_data_dir = g_build_filename (data_dir, "plugins", NULL);
    g_free (data_dir);

    return plugin_data_dir;
}

gchar *
xed_dirs_get_ui_file (const gchar* file)
{
    gchar* datadir;
    gchar* ui_file;

    g_return_val_if_fail(file != NULL, NULL);

    datadir = xed_dirs_get_xed_data_dir();
    ui_file = g_build_filename(datadir, "ui", file, NULL);
    g_free(datadir);

    return ui_file;
}
