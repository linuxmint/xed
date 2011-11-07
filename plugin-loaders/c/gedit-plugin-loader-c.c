/*
 * gedit-plugin-loader-c.c
 * This file is part of gedit
 *
 * Copyright (C) 2008 - Jesse van den Kieboom
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */

#include "gedit-plugin-loader-c.h"
#include <gedit/gedit-object-module.h>

#define GEDIT_PLUGIN_LOADER_C_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_TYPE_PLUGIN_LOADER_C, GeditPluginLoaderCPrivate))

struct _GeditPluginLoaderCPrivate
{
	GHashTable *loaded_plugins;
};

static void gedit_plugin_loader_iface_init (gpointer g_iface, gpointer iface_data);

GEDIT_PLUGIN_LOADER_REGISTER_TYPE (GeditPluginLoaderC, gedit_plugin_loader_c, G_TYPE_OBJECT, gedit_plugin_loader_iface_init);


static const gchar *
gedit_plugin_loader_iface_get_id (void)
{
	return "C";
}

static GeditPlugin *
gedit_plugin_loader_iface_load (GeditPluginLoader *loader,
				GeditPluginInfo   *info,
				const gchar       *path)
{
	GeditPluginLoaderC *cloader = GEDIT_PLUGIN_LOADER_C (loader);
	GeditObjectModule *module;
	const gchar *module_name;
	GeditPlugin *result;

	module = (GeditObjectModule *)g_hash_table_lookup (cloader->priv->loaded_plugins, info);
	module_name = gedit_plugin_info_get_module_name (info);

	if (module == NULL)
	{
		/* For now we force all modules to be resident */
		module = gedit_object_module_new (module_name,
						  path,
						  "register_gedit_plugin",
						  TRUE);

		/* Infos are available for all the lifetime of the loader.
		 * If this changes, we should use weak refs or something */

		g_hash_table_insert (cloader->priv->loaded_plugins, info, module);
	}

	if (!g_type_module_use (G_TYPE_MODULE (module)))
	{
		g_warning ("Could not load plugin module: %s", gedit_plugin_info_get_name (info));

		return NULL;
	}

	/* TODO: for now we force data-dir-name = module-name... if needed we can
	 * add a datadir field to the plugin descriptor file.
	 */
	result = (GeditPlugin *)gedit_object_module_new_object (module,
								"install-dir", path,
								"data-dir-name", module_name,
								NULL);

	if (!result)
	{
		g_warning ("Could not create plugin object: %s", gedit_plugin_info_get_name (info));
		g_type_module_unuse (G_TYPE_MODULE (module));
		
		return NULL;
	}

	g_type_module_unuse (G_TYPE_MODULE (module));
	
	return result;
}

static void
gedit_plugin_loader_iface_unload (GeditPluginLoader *loader,
				  GeditPluginInfo   *info)
{
	//GeditPluginLoaderC *cloader = GEDIT_PLUGIN_LOADER_C (loader);
	
	/* this is a no-op, since the type module will be properly unused as
	   the last reference to the plugin dies. When the plugin is activated
	   again, the library will be reloaded */
}

static void
gedit_plugin_loader_iface_init (gpointer g_iface, 
				gpointer iface_data)
{
	GeditPluginLoaderInterface *iface = (GeditPluginLoaderInterface *)g_iface;
	
	iface->get_id = gedit_plugin_loader_iface_get_id;
	iface->load = gedit_plugin_loader_iface_load;
	iface->unload = gedit_plugin_loader_iface_unload;
}

static void
gedit_plugin_loader_c_finalize (GObject *object)
{
	GeditPluginLoaderC *cloader = GEDIT_PLUGIN_LOADER_C (object);
	GList *infos;
	GList *item;

	/* FIXME: this sanity check it's not efficient. Let's remove it
	 * once we are confident with the code */

	infos = g_hash_table_get_keys (cloader->priv->loaded_plugins);
	
	for (item = infos; item; item = item->next)
	{
		GeditPluginInfo *info = (GeditPluginInfo *)item->data;

		if (gedit_plugin_info_is_active (info))
		{
			g_warning ("There are still C plugins loaded during destruction");
			break;
		}
	}

	g_list_free (infos);	

	g_hash_table_destroy (cloader->priv->loaded_plugins);
	
	G_OBJECT_CLASS (gedit_plugin_loader_c_parent_class)->finalize (object);
}

static void
gedit_plugin_loader_c_class_init (GeditPluginLoaderCClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	object_class->finalize = gedit_plugin_loader_c_finalize;

	g_type_class_add_private (object_class, sizeof (GeditPluginLoaderCPrivate));
}

static void
gedit_plugin_loader_c_class_finalize (GeditPluginLoaderCClass *klass)
{
}

static void
gedit_plugin_loader_c_init (GeditPluginLoaderC *self)
{
	self->priv = GEDIT_PLUGIN_LOADER_C_GET_PRIVATE (self);
	
	/* loaded_plugins maps GeditPluginInfo to a GeditObjectModule */
	self->priv->loaded_plugins = g_hash_table_new (g_direct_hash,
						       g_direct_equal);
}

GeditPluginLoaderC *
gedit_plugin_loader_c_new ()
{
	GObject *loader = g_object_new (GEDIT_TYPE_PLUGIN_LOADER_C, NULL);

	return GEDIT_PLUGIN_LOADER_C (loader);
}
