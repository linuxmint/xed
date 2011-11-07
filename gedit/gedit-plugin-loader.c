/*
 * gedit-plugin-loader.c
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

#include "gedit-plugin-loader.h"

static void
gedit_plugin_loader_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (G_UNLIKELY (!initialized))
	{
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
gedit_plugin_loader_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY (type == 0))
	{
		static const GTypeInfo info =
		{
			sizeof (GeditPluginLoaderInterface),
			gedit_plugin_loader_base_init,   /* base_init */
			NULL,   /* base_finalize */
			NULL,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			0,
			0,      /* n_preallocs */
			NULL    /* instance_init */
		};
		
		type = g_type_register_static (G_TYPE_INTERFACE, "GeditPluginLoader", &info, 0);
	}

	return type;
}

const gchar *
gedit_plugin_loader_type_get_id (GType type)
{
	GTypeClass *klass;
	GeditPluginLoaderInterface *iface;
	
	klass = g_type_class_ref (type);
	
	if (klass == NULL)
	{
		g_warning ("Could not get class info for plugin loader");
		return NULL;
	}

	iface = g_type_interface_peek (klass, GEDIT_TYPE_PLUGIN_LOADER);
	
	if (iface == NULL)
	{
		g_warning ("Could not get plugin loader interface");
		g_type_class_unref (klass);
		
		return NULL;
	}
	
	g_return_val_if_fail (iface->get_id != NULL, NULL);
	return iface->get_id ();
}

GeditPlugin *
gedit_plugin_loader_load (GeditPluginLoader *loader,
			  GeditPluginInfo   *info,
			  const gchar       *path)
{
	GeditPluginLoaderInterface *iface;
	
	g_return_val_if_fail (GEDIT_IS_PLUGIN_LOADER (loader), NULL);
	
	iface = GEDIT_PLUGIN_LOADER_GET_INTERFACE (loader);
	g_return_val_if_fail (iface->load != NULL, NULL);
	
	return iface->load (loader, info, path);
}

void
gedit_plugin_loader_unload (GeditPluginLoader *loader,
			    GeditPluginInfo   *info)
{
	GeditPluginLoaderInterface *iface;
	
	g_return_if_fail (GEDIT_IS_PLUGIN_LOADER (loader));
	
	iface = GEDIT_PLUGIN_LOADER_GET_INTERFACE (loader);
	g_return_if_fail (iface->unload != NULL);
	
	iface->unload (loader, info);
}

void
gedit_plugin_loader_garbage_collect (GeditPluginLoader *loader)
{
	GeditPluginLoaderInterface *iface;
	
	g_return_if_fail (GEDIT_IS_PLUGIN_LOADER (loader));
	
	iface = GEDIT_PLUGIN_LOADER_GET_INTERFACE (loader);
	
	if (iface->garbage_collect != NULL)
		iface->garbage_collect (loader);
}
