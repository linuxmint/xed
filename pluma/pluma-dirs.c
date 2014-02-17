/*
 * pluma-dirs.c
 * This file is part of pluma
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

#include "pluma-dirs.h"

#ifdef OS_OSX
	#include <ige-mac-bundle.h>
#endif

gchar* pluma_dirs_get_user_config_dir(void)
{
	gchar* config_dir = NULL;

	config_dir = g_build_filename(g_get_user_config_dir(), "pluma", NULL);

	return config_dir;
}

gchar* pluma_dirs_get_user_cache_dir(void)
{
	const gchar* cache_dir;

	cache_dir = g_get_user_cache_dir();

	return g_build_filename(cache_dir, "pluma", NULL);
}

gchar* pluma_dirs_get_user_plugins_dir(void)
{
	gchar* plugin_dir;

	plugin_dir = g_build_filename(g_get_user_data_dir(), "pluma", "plugins", NULL);

	return plugin_dir;
}

gchar* pluma_dirs_get_user_accels_file(void)
{
	gchar* accels = NULL;
	gchar *config_dir = NULL;

	config_dir = pluma_dirs_get_user_config_dir();
	accels = g_build_filename(config_dir, "accels", NULL);

	g_free(config_dir);

	return accels;
}

gchar* pluma_dirs_get_pluma_data_dir(void)
{
	gchar* data_dir;

	#ifdef G_OS_WIN32
		gchar* win32_dir;

		win32_dir = g_win32_get_package_installation_directory_of_module(NULL);

		data_dir = g_build_filename(win32_dir, "share", "pluma", NULL);

		g_free(win32_dir);

	#elif defined(OS_OSX)

		IgeMacBundle* bundle = ige_mac_bundle_get_default();

		if (ige_mac_bundle_get_is_app_bundle(bundle))
		{
			const gchar* bundle_data_dir = ige_mac_bundle_get_datadir(bundle);

			data_dir = g_build_filename(bundle_data_dir, "pluma", NULL);
		}
		else
		{
			data_dir = g_build_filename(DATADIR, "pluma", NULL);
		}
	#else
		data_dir = g_build_filename(DATADIR, "pluma", NULL);
	#endif

	return data_dir;
}

gchar* pluma_dirs_get_pluma_locale_dir(void)
{
	gchar* locale_dir;

	#ifdef G_OS_WIN32

		gchar* win32_dir;

		win32_dir = g_win32_get_package_installation_directory_of_module(NULL);

		locale_dir = g_build_filename(win32_dir, "share", "locale", NULL);

		g_free(win32_dir);

	#elif defined(OS_OSX)

		IgeMacBundle *bundle = ige_mac_bundle_get_default();

		if (ige_mac_bundle_get_is_app_bundle(bundle))
		{
			locale_dir = g_strdup(ige_mac_bundle_get_localedir(bundle));
		}
		else
		{
			locale_dir = g_build_filename(DATADIR, "locale", NULL);
		}
	#else
		locale_dir = g_build_filename(DATADIR, "locale", NULL);
	#endif

	return locale_dir;
}

gchar* pluma_dirs_get_pluma_lib_dir(void)
{
	gchar* lib_dir;

	#ifdef G_OS_WIN32

		gchar* win32_dir;

		win32_dir = g_win32_get_package_installation_directory_of_module(NULL);

		lib_dir = g_build_filename(win32_dir, "lib", "pluma", NULL);

		g_free(win32_dir);

	#elif defined(OS_OSX)
		IgeMacBundle* bundle = ige_mac_bundle_get_default();

		if (ige_mac_bundle_get_is_app_bundle(bundle))
		{
			const gchar* path = ige_mac_bundle_get_resourcesdir(bundle);
			lib_dir = g_build_filename(path, "lib", "pluma", NULL);
		}
		else
		{
			lib_dir = g_build_filename(LIBDIR, "pluma", NULL);
		}
	#else
		lib_dir = g_build_filename(LIBDIR, "pluma", NULL);
	#endif

	return lib_dir;
}

gchar* pluma_dirs_get_pluma_plugins_dir(void)
{
	gchar* lib_dir;
	gchar* plugin_dir;

	lib_dir = pluma_dirs_get_pluma_lib_dir();

	plugin_dir = g_build_filename(lib_dir, "plugins", NULL);
	g_free(lib_dir);

	return plugin_dir;
}

gchar* pluma_dirs_get_pluma_plugin_loaders_dir(void)
{
	gchar* lib_dir;
	gchar* loader_dir;

	lib_dir = pluma_dirs_get_pluma_lib_dir();

	loader_dir = g_build_filename(lib_dir, "plugin-loaders", NULL);
	g_free(lib_dir);

	return loader_dir;
}

gchar* pluma_dirs_get_ui_file(const gchar* file)
{
	gchar* datadir;
	gchar* ui_file;

	g_return_val_if_fail(file != NULL, NULL);

	datadir = pluma_dirs_get_pluma_data_dir();
	ui_file = g_build_filename(datadir, "ui", file, NULL);
	g_free(datadir);

	return ui_file;
}
