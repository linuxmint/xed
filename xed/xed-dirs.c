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

static gchar *user_config_dir      = NULL;
static gchar *user_cache_dir       = NULL;
static gchar *user_styles_dir      = NULL;
static gchar *user_plugins_dir     = NULL;
static gchar *xed_data_dir         = NULL;
static gchar *xed_locale_dir       = NULL;
static gchar *xed_lib_dir          = NULL;
static gchar *xed_plugins_dir      = NULL;
static gchar *xed_plugins_data_dir = NULL;

void
xed_dirs_init ()
{
    if (xed_data_dir == NULL)
    {
        xed_data_dir = g_build_filename (DATADIR, "xed", NULL);
        xed_locale_dir = g_build_filename (DATADIR, "locale", NULL);
        xed_lib_dir = g_build_filename (LIBDIR, "xed", NULL);
    }

    user_cache_dir = g_build_filename (g_get_user_cache_dir (), "xed", NULL);
    user_config_dir = g_build_filename (g_get_user_config_dir (), "xed", NULL);
    user_styles_dir = g_build_filename (g_get_user_data_dir (), "xed", "styles", NULL);
    user_plugins_dir = g_build_filename (g_get_user_data_dir (), "xed", "plugins", NULL);
    xed_plugins_dir = g_build_filename (xed_lib_dir, "plugins", NULL);
    xed_plugins_data_dir = g_build_filename (xed_data_dir, "plugins", NULL);
}

void
xed_dirs_shutdown ()
{
    g_free (user_config_dir);
    g_free (user_cache_dir);
    g_free (user_plugins_dir);
    g_free (xed_data_dir);
    g_free (xed_locale_dir);
    g_free (xed_lib_dir);
    g_free (xed_plugins_dir);
    g_free (xed_plugins_data_dir);
}

const gchar *
xed_dirs_get_user_config_dir (void)
{
    return user_config_dir;
}

const gchar *
xed_dirs_get_user_cache_dir (void)
{
    return user_cache_dir;
}

const gchar *
xed_dirs_get_user_styles_dir (void)
{
    return user_styles_dir;
}

const gchar *
xed_dirs_get_user_plugins_dir (void)
{
    return user_plugins_dir;
}

const gchar *
xed_dirs_get_xed_data_dir (void)
{
    return xed_data_dir;
}

const gchar *
xed_dirs_get_xed_locale_dir (void)
{
    return xed_locale_dir;
}

const gchar *
xed_dirs_get_xed_lib_dir (void)
{
    return xed_lib_dir;
}

const gchar *
xed_dirs_get_xed_plugins_dir (void)
{
    return xed_plugins_dir;
}

const gchar *
xed_dirs_get_xed_plugins_data_dir (void)
{
    return xed_plugins_data_dir;
}

const gchar *
xed_dirs_get_binding_modules_dir (void)
{
    return xed_lib_dir;
}

gchar *
xed_dirs_get_ui_file (const gchar *file)
{
    gchar *ui_file;

    g_return_val_if_fail (file != NULL, NULL);

    ui_file = g_build_filename (xed_dirs_get_xed_data_dir (), "ui", file, NULL);

    return ui_file;
}
