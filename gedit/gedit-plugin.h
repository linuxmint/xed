/*
 * gedit-plugin.h
 * This file is part of gedit
 *
 * Copyright (C) 2002-2005 - Paolo Maggi 
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
 
/*
 * Modified by the gedit Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __GEDIT_PLUGIN_H__
#define __GEDIT_PLUGIN_H__

#include <glib-object.h>

#include <gedit/gedit-window.h>
#include <gedit/gedit-debug.h>

/* TODO: add a .h file that includes all the .h files normally needed to
 * develop a plugin */ 

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GEDIT_TYPE_PLUGIN              (gedit_plugin_get_type())
#define GEDIT_PLUGIN(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GEDIT_TYPE_PLUGIN, GeditPlugin))
#define GEDIT_PLUGIN_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GEDIT_TYPE_PLUGIN, GeditPluginClass))
#define GEDIT_IS_PLUGIN(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GEDIT_TYPE_PLUGIN))
#define GEDIT_IS_PLUGIN_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_PLUGIN))
#define GEDIT_PLUGIN_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GEDIT_TYPE_PLUGIN, GeditPluginClass))

/*
 * Main object structure
 */
typedef struct _GeditPlugin GeditPlugin;

struct _GeditPlugin 
{
	GObject parent;
};

/*
 * Class definition
 */
typedef struct _GeditPluginClass GeditPluginClass;

struct _GeditPluginClass 
{
	GObjectClass parent_class;

	/* Virtual public methods */
	
	void 		(*activate)		(GeditPlugin *plugin,
						 GeditWindow *window);
	void 		(*deactivate)		(GeditPlugin *plugin,
						 GeditWindow *window);

	void 		(*update_ui)		(GeditPlugin *plugin,
						 GeditWindow *window);

	GtkWidget 	*(*create_configure_dialog)
						(GeditPlugin *plugin);

	/* Plugins should not override this, it's handled automatically by
	   the GeditPluginClass */
	gboolean 	(*is_configurable)
						(GeditPlugin *plugin);

	/* Padding for future expansion */
	void		(*_gedit_reserved1)	(void);
	void		(*_gedit_reserved2)	(void);
	void		(*_gedit_reserved3)	(void);
	void		(*_gedit_reserved4)	(void);
};

/*
 * Public methods
 */
GType 		 gedit_plugin_get_type 		(void) G_GNUC_CONST;

gchar 		*gedit_plugin_get_install_dir	(GeditPlugin *plugin);
gchar 		*gedit_plugin_get_data_dir	(GeditPlugin *plugin);

void 		 gedit_plugin_activate		(GeditPlugin *plugin,
						 GeditWindow *window);
void 		 gedit_plugin_deactivate	(GeditPlugin *plugin,
						 GeditWindow *window);
				 
void 		 gedit_plugin_update_ui		(GeditPlugin *plugin,
						 GeditWindow *window);

gboolean	 gedit_plugin_is_configurable	(GeditPlugin *plugin);
GtkWidget	*gedit_plugin_create_configure_dialog		
						(GeditPlugin *plugin);

/**
 * GEDIT_PLUGIN_REGISTER_TYPE_WITH_CODE(PluginName, plugin_name, CODE):
 *
 * Utility macro used to register plugins with additional code.
 */
#define GEDIT_PLUGIN_REGISTER_TYPE_WITH_CODE(PluginName, plugin_name, CODE) 	\
	G_DEFINE_DYNAMIC_TYPE_EXTENDED (PluginName,				\
					plugin_name,				\
					GEDIT_TYPE_PLUGIN,			\
					0,					\
					GTypeModule *module G_GNUC_UNUSED = type_module; /* back compat */	\
					CODE)					\
										\
/* This is not very nice, but G_DEFINE_DYNAMIC wants it and our old macro	\
 * did not support it */							\
static void									\
plugin_name##_class_finalize (PluginName##Class *klass)				\
{										\
}										\
										\
										\
G_MODULE_EXPORT GType								\
register_gedit_plugin (GTypeModule *type_module)				\
{										\
	plugin_name##_register_type (type_module);				\
										\
	return plugin_name##_get_type();					\
}

/**
 * GEDIT_PLUGIN_REGISTER_TYPE(PluginName, plugin_name):
 * 
 * Utility macro used to register plugins.
 */
#define GEDIT_PLUGIN_REGISTER_TYPE(PluginName, plugin_name)			\
	GEDIT_PLUGIN_REGISTER_TYPE_WITH_CODE(PluginName, plugin_name, ;)

/**
 * GEDIT_PLUGIN_DEFINE_TYPE_WITH_CODE(ObjectName, object_name, PARENT_TYPE, CODE):
 *
 * Utility macro used to register gobject types in plugins with additional code.
 *
 * Deprecated: use G_DEFINE_DYNAMIC_TYPE_EXTENDED instead
 */
#define GEDIT_PLUGIN_DEFINE_TYPE_WITH_CODE(ObjectName, object_name, PARENT_TYPE, CODE) \
										\
static GType g_define_type_id = 0;						\
										\
GType										\
object_name##_get_type (void)							\
{										\
	return g_define_type_id;						\
}										\
										\
static void     object_name##_init              (ObjectName        *self);	\
static void     object_name##_class_init        (ObjectName##Class *klass);	\
static gpointer object_name##_parent_class = NULL;				\
static void     object_name##_class_intern_init (gpointer klass)		\
{										\
	object_name##_parent_class = g_type_class_peek_parent (klass);		\
	object_name##_class_init ((ObjectName##Class *) klass);			\
}										\
										\
GType										\
object_name##_register_type (GTypeModule *type_module)				\
{										\
	GTypeModule *module G_GNUC_UNUSED = type_module; /* back compat */			\
	static const GTypeInfo our_info =					\
	{									\
		sizeof (ObjectName##Class),					\
		NULL, /* base_init */						\
		NULL, /* base_finalize */					\
		(GClassInitFunc) object_name##_class_intern_init,		\
		NULL,								\
		NULL, /* class_data */						\
		sizeof (ObjectName),						\
		0, /* n_preallocs */						\
		(GInstanceInitFunc) object_name##_init				\
	};									\
										\
	g_define_type_id = g_type_module_register_type (type_module,		\
					   	        PARENT_TYPE,		\
					                #ObjectName,		\
					                &our_info,		\
					                0);			\
										\
	CODE									\
										\
	return g_define_type_id;						\
}


/**
 * GEDIT_PLUGIN_DEFINE_TYPE(ObjectName, object_name, PARENT_TYPE):
 *
 * Utility macro used to register gobject types in plugins.
 *
 * Deprecated: use G_DEFINE_DYNAMIC instead
 */
#define GEDIT_PLUGIN_DEFINE_TYPE(ObjectName, object_name, PARENT_TYPE)		\
	GEDIT_PLUGIN_DEFINE_TYPE_WITH_CODE(ObjectName, object_name, PARENT_TYPE, ;)

/**
 * GEDIT_PLUGIN_IMPLEMENT_INTERFACE(TYPE_IFACE, iface_init):
 *
 * Utility macro used to register interfaces for gobject types in plugins.
 */
#define GEDIT_PLUGIN_IMPLEMENT_INTERFACE(object_name, TYPE_IFACE, iface_init)	\
	const GInterfaceInfo object_name##_interface_info = 			\
	{ 									\
		(GInterfaceInitFunc) iface_init,				\
		NULL, 								\
		NULL								\
	};									\
										\
	g_type_module_add_interface (type_module, 					\
				     g_define_type_id, 				\
				     TYPE_IFACE, 				\
				     &object_name##_interface_info);

G_END_DECLS

#endif  /* __GEDIT_PLUGIN_H__ */
