/*
 * xedit-plugin-loader-c.c
 * This file is part of xedit
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA 02110-1301, USA. 
 */

#include "xedit-plugin-loader-c.h"
#include <xedit/xedit-object-module.h>

#define XEDIT_PLUGIN_LOADER_C_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), XEDIT_TYPE_PLUGIN_LOADER_C, XeditPluginLoaderCPrivate))

struct _XeditPluginLoaderCPrivate
{
	GHashTable *loaded_plugins;
};

static void xedit_plugin_loader_iface_init (gpointer g_iface, gpointer iface_data);

XEDIT_PLUGIN_LOADER_REGISTER_TYPE (XeditPluginLoaderC, xedit_plugin_loader_c, G_TYPE_OBJECT, xedit_plugin_loader_iface_init);


static const gchar *
xedit_plugin_loader_iface_get_id (void)
{
	return "C";
}

static XeditPlugin *
xedit_plugin_loader_iface_load (XeditPluginLoader *loader,
				XeditPluginInfo   *info,
				const gchar       *path)
{
	XeditPluginLoaderC *cloader = XEDIT_PLUGIN_LOADER_C (loader);
	XeditObjectModule *module;
	const gchar *module_name;
	XeditPlugin *result;

	module = (XeditObjectModule *)g_hash_table_lookup (cloader->priv->loaded_plugins, info);
	module_name = xedit_plugin_info_get_module_name (info);

	if (module == NULL)
	{
		/* For now we force all modules to be resident */
		module = xedit_object_module_new (module_name,
						  path,
						  "register_xedit_plugin",
						  TRUE);

		/* Infos are available for all the lifetime of the loader.
		 * If this changes, we should use weak refs or something */

		g_hash_table_insert (cloader->priv->loaded_plugins, info, module);
	}

	if (!g_type_module_use (G_TYPE_MODULE (module)))
	{
		g_warning ("Could not load plugin module: %s", xedit_plugin_info_get_name (info));

		return NULL;
	}

	/* TODO: for now we force data-dir-name = module-name... if needed we can
	 * add a datadir field to the plugin descriptor file.
	 */
	result = (XeditPlugin *)xedit_object_module_new_object (module,
								"install-dir", path,
								"data-dir-name", module_name,
								NULL);

	if (!result)
	{
		g_warning ("Could not create plugin object: %s", xedit_plugin_info_get_name (info));
		g_type_module_unuse (G_TYPE_MODULE (module));
		
		return NULL;
	}

	g_type_module_unuse (G_TYPE_MODULE (module));
	
	return result;
}

static void
xedit_plugin_loader_iface_unload (XeditPluginLoader *loader,
				  XeditPluginInfo   *info)
{
	//XeditPluginLoaderC *cloader = XEDIT_PLUGIN_LOADER_C (loader);
	
	/* this is a no-op, since the type module will be properly unused as
	   the last reference to the plugin dies. When the plugin is activated
	   again, the library will be reloaded */
}

static void
xedit_plugin_loader_iface_init (gpointer g_iface, 
				gpointer iface_data)
{
	XeditPluginLoaderInterface *iface = (XeditPluginLoaderInterface *)g_iface;
	
	iface->get_id = xedit_plugin_loader_iface_get_id;
	iface->load = xedit_plugin_loader_iface_load;
	iface->unload = xedit_plugin_loader_iface_unload;
}

static void
xedit_plugin_loader_c_finalize (GObject *object)
{
	XeditPluginLoaderC *cloader = XEDIT_PLUGIN_LOADER_C (object);
	GList *infos;
	GList *item;

	/* FIXME: this sanity check it's not efficient. Let's remove it
	 * once we are confident with the code */

	infos = g_hash_table_get_keys (cloader->priv->loaded_plugins);
	
	for (item = infos; item; item = item->next)
	{
		XeditPluginInfo *info = (XeditPluginInfo *)item->data;

		if (xedit_plugin_info_is_active (info))
		{
			g_warning ("There are still C plugins loaded during destruction");
			break;
		}
	}

	g_list_free (infos);	

	g_hash_table_destroy (cloader->priv->loaded_plugins);
	
	G_OBJECT_CLASS (xedit_plugin_loader_c_parent_class)->finalize (object);
}

static void
xedit_plugin_loader_c_class_init (XeditPluginLoaderCClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	object_class->finalize = xedit_plugin_loader_c_finalize;

	g_type_class_add_private (object_class, sizeof (XeditPluginLoaderCPrivate));
}

static void
xedit_plugin_loader_c_class_finalize (XeditPluginLoaderCClass *klass)
{
}

static void
xedit_plugin_loader_c_init (XeditPluginLoaderC *self)
{
	self->priv = XEDIT_PLUGIN_LOADER_C_GET_PRIVATE (self);
	
	/* loaded_plugins maps XeditPluginInfo to a XeditObjectModule */
	self->priv->loaded_plugins = g_hash_table_new (g_direct_hash,
						       g_direct_equal);
}

XeditPluginLoaderC *
xedit_plugin_loader_c_new ()
{
	GObject *loader = g_object_new (XEDIT_TYPE_PLUGIN_LOADER_C, NULL);

	return XEDIT_PLUGIN_LOADER_C (loader);
}
