/*
 * gedit-plugin-loader.h
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

#ifndef __GEDIT_PLUGIN_LOADER_H__
#define __GEDIT_PLUGIN_LOADER_H__

#include <glib-object.h>
#include <gedit/gedit-plugin.h>
#include <gedit/gedit-plugin-info.h>

G_BEGIN_DECLS

#define GEDIT_TYPE_PLUGIN_LOADER                (gedit_plugin_loader_get_type ())
#define GEDIT_PLUGIN_LOADER(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_PLUGIN_LOADER, GeditPluginLoader))
#define GEDIT_IS_PLUGIN_LOADER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_PLUGIN_LOADER))
#define GEDIT_PLUGIN_LOADER_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE ((inst), GEDIT_TYPE_PLUGIN_LOADER, GeditPluginLoaderInterface))

typedef struct _GeditPluginLoader GeditPluginLoader; /* dummy object */
typedef struct _GeditPluginLoaderInterface GeditPluginLoaderInterface;

struct _GeditPluginLoaderInterface {
	GTypeInterface parent;

	const gchar *(*get_id)		(void);

	GeditPlugin *(*load) 		(GeditPluginLoader 	*loader,
			     		 GeditPluginInfo	*info,
			      		 const gchar       	*path);

	void 	     (*unload)		(GeditPluginLoader 	*loader,
					 GeditPluginInfo       	*info);

	void         (*garbage_collect) 	(GeditPluginLoader	*loader);
};

GType gedit_plugin_loader_get_type (void);

const gchar *gedit_plugin_loader_type_get_id	(GType 			 type);
GeditPlugin *gedit_plugin_loader_load		(GeditPluginLoader 	*loader,
						 GeditPluginInfo 	*info,
						 const gchar		*path);
void gedit_plugin_loader_unload			(GeditPluginLoader 	*loader,
						 GeditPluginInfo	*info);
void gedit_plugin_loader_garbage_collect	(GeditPluginLoader 	*loader);

/**
 * GEDIT_PLUGIN_LOADER_IMPLEMENT_INTERFACE(TYPE_IFACE, iface_init):
 *
 * Utility macro used to register interfaces for gobject types in plugin loaders.
 */
#define GEDIT_PLUGIN_LOADER_IMPLEMENT_INTERFACE(TYPE_IFACE, iface_init)		\
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
 * GEDIT_PLUGIN_LOADER_REGISTER_TYPE(PluginLoaderName, plugin_loader_name, PARENT_TYPE, loader_interface_init):
 *
 * Utility macro used to register plugin loaders.
 */
#define GEDIT_PLUGIN_LOADER_REGISTER_TYPE(PluginLoaderName, plugin_loader_name, PARENT_TYPE, loader_iface_init) 	\
	G_DEFINE_DYNAMIC_TYPE_EXTENDED (PluginLoaderName,			\
					plugin_loader_name,			\
					PARENT_TYPE,			\
					0,					\
					GEDIT_PLUGIN_LOADER_IMPLEMENT_INTERFACE(GEDIT_TYPE_PLUGIN_LOADER, loader_iface_init));	\
										\
										\
G_MODULE_EXPORT GType								\
register_gedit_plugin_loader (GTypeModule *type_module)				\
{										\
	plugin_loader_name##_register_type (type_module);			\
										\
	return plugin_loader_name##_get_type();					\
}

G_END_DECLS

#endif /* __GEDIT_PLUGIN_LOADER_H__ */
