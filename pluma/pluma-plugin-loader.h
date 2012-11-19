/*
 * pluma-plugin-loader.h
 * This file is part of pluma
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

#ifndef __PLUMA_PLUGIN_LOADER_H__
#define __PLUMA_PLUGIN_LOADER_H__

#include <glib-object.h>
#include <pluma/pluma-plugin.h>
#include <pluma/pluma-plugin-info.h>

G_BEGIN_DECLS

#define PLUMA_TYPE_PLUGIN_LOADER                (pluma_plugin_loader_get_type ())
#define PLUMA_PLUGIN_LOADER(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_PLUGIN_LOADER, PlumaPluginLoader))
#define PLUMA_IS_PLUGIN_LOADER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUMA_TYPE_PLUGIN_LOADER))
#define PLUMA_PLUGIN_LOADER_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE ((inst), PLUMA_TYPE_PLUGIN_LOADER, PlumaPluginLoaderInterface))

typedef struct _PlumaPluginLoader PlumaPluginLoader; /* dummy object */
typedef struct _PlumaPluginLoaderInterface PlumaPluginLoaderInterface;

struct _PlumaPluginLoaderInterface {
	GTypeInterface parent;

	const gchar *(*get_id)		(void);

	PlumaPlugin *(*load) 		(PlumaPluginLoader 	*loader,
			     		 PlumaPluginInfo	*info,
			      		 const gchar       	*path);

	void 	     (*unload)		(PlumaPluginLoader 	*loader,
					 PlumaPluginInfo       	*info);

	void         (*garbage_collect) 	(PlumaPluginLoader	*loader);
};

GType pluma_plugin_loader_get_type (void);

const gchar *pluma_plugin_loader_type_get_id	(GType 			 type);
PlumaPlugin *pluma_plugin_loader_load		(PlumaPluginLoader 	*loader,
						 PlumaPluginInfo 	*info,
						 const gchar		*path);
void pluma_plugin_loader_unload			(PlumaPluginLoader 	*loader,
						 PlumaPluginInfo	*info);
void pluma_plugin_loader_garbage_collect	(PlumaPluginLoader 	*loader);

/**
 * PLUMA_PLUGIN_LOADER_IMPLEMENT_INTERFACE(TYPE_IFACE, iface_init):
 *
 * Utility macro used to register interfaces for gobject types in plugin loaders.
 */
#define PLUMA_PLUGIN_LOADER_IMPLEMENT_INTERFACE(TYPE_IFACE, iface_init)		\
	const GInterfaceInfo g_implement_interface_info = 			\
	{ 									\
		(GInterfaceInitFunc) iface_init,				\
		NULL, 								\
		NULL								\
	};									\
										\
	g_type_module_add_interface (type_module,				\
				     g_define_type_id, 				\
				     TYPE_IFACE, 				\
				     &g_implement_interface_info);

/**
 * PLUMA_PLUGIN_LOADER_REGISTER_TYPE(PluginLoaderName, plugin_loader_name, PARENT_TYPE, loader_interface_init):
 *
 * Utility macro used to register plugin loaders.
 */
#define PLUMA_PLUGIN_LOADER_REGISTER_TYPE(PluginLoaderName, plugin_loader_name, PARENT_TYPE, loader_iface_init) 	\
	G_DEFINE_DYNAMIC_TYPE_EXTENDED (PluginLoaderName,			\
					plugin_loader_name,			\
					PARENT_TYPE,			\
					0,					\
					PLUMA_PLUGIN_LOADER_IMPLEMENT_INTERFACE(PLUMA_TYPE_PLUGIN_LOADER, loader_iface_init));	\
										\
										\
G_MODULE_EXPORT GType								\
register_pluma_plugin_loader (GTypeModule *type_module)				\
{										\
	plugin_loader_name##_register_type (type_module);			\
										\
	return plugin_loader_name##_get_type();					\
}

G_END_DECLS

#endif /* __PLUMA_PLUGIN_LOADER_H__ */
