/*
 * pluma-plugin.h
 * This file is part of pluma
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA 02110-1301, USA. 
 */
 
/*
 * Modified by the pluma Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __PLUMA_PLUGIN_H__
#define __PLUMA_PLUGIN_H__

#include <glib-object.h>

#include <pluma/pluma-window.h>
#include <pluma/pluma-debug.h>

/* TODO: add a .h file that includes all the .h files normally needed to
 * develop a plugin */ 

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_PLUGIN              (pluma_plugin_get_type())
#define PLUMA_PLUGIN(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_PLUGIN, PlumaPlugin))
#define PLUMA_PLUGIN_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), PLUMA_TYPE_PLUGIN, PlumaPluginClass))
#define PLUMA_IS_PLUGIN(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), PLUMA_TYPE_PLUGIN))
#define PLUMA_IS_PLUGIN_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_PLUGIN))
#define PLUMA_PLUGIN_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), PLUMA_TYPE_PLUGIN, PlumaPluginClass))

/*
 * Main object structure
 */
typedef struct _PlumaPlugin PlumaPlugin;

struct _PlumaPlugin 
{
	GObject parent;
};

/*
 * Class definition
 */
typedef struct _PlumaPluginClass PlumaPluginClass;

struct _PlumaPluginClass 
{
	GObjectClass parent_class;

	/* Virtual public methods */
	
	void 		(*activate)		(PlumaPlugin *plugin,
						 PlumaWindow *window);
	void 		(*deactivate)		(PlumaPlugin *plugin,
						 PlumaWindow *window);

	void 		(*update_ui)		(PlumaPlugin *plugin,
						 PlumaWindow *window);

	GtkWidget 	*(*create_configure_dialog)
						(PlumaPlugin *plugin);

	/* Plugins should not override this, it's handled automatically by
	   the PlumaPluginClass */
	gboolean 	(*is_configurable)
						(PlumaPlugin *plugin);

	/* Padding for future expansion */
	void		(*_pluma_reserved1)	(void);
	void		(*_pluma_reserved2)	(void);
	void		(*_pluma_reserved3)	(void);
	void		(*_pluma_reserved4)	(void);
};

/*
 * Public methods
 */
GType 		 pluma_plugin_get_type 		(void) G_GNUC_CONST;

gchar 		*pluma_plugin_get_install_dir	(PlumaPlugin *plugin);
gchar 		*pluma_plugin_get_data_dir	(PlumaPlugin *plugin);

void 		 pluma_plugin_activate		(PlumaPlugin *plugin,
						 PlumaWindow *window);
void 		 pluma_plugin_deactivate	(PlumaPlugin *plugin,
						 PlumaWindow *window);
				 
void 		 pluma_plugin_update_ui		(PlumaPlugin *plugin,
						 PlumaWindow *window);

gboolean	 pluma_plugin_is_configurable	(PlumaPlugin *plugin);
GtkWidget	*pluma_plugin_create_configure_dialog		
						(PlumaPlugin *plugin);

/**
 * PLUMA_PLUGIN_REGISTER_TYPE_WITH_CODE(PluginName, plugin_name, CODE):
 *
 * Utility macro used to register plugins with additional code.
 */
#define PLUMA_PLUGIN_REGISTER_TYPE_WITH_CODE(PluginName, plugin_name, CODE) 	\
	G_DEFINE_DYNAMIC_TYPE_EXTENDED (PluginName,				\
					plugin_name,				\
					PLUMA_TYPE_PLUGIN,			\
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
register_pluma_plugin (GTypeModule *type_module)				\
{										\
	plugin_name##_register_type (type_module);				\
										\
	return plugin_name##_get_type();					\
}

/**
 * PLUMA_PLUGIN_REGISTER_TYPE(PluginName, plugin_name):
 * 
 * Utility macro used to register plugins.
 */
#define PLUMA_PLUGIN_REGISTER_TYPE(PluginName, plugin_name)			\
	PLUMA_PLUGIN_REGISTER_TYPE_WITH_CODE(PluginName, plugin_name, ;)

/**
 * PLUMA_PLUGIN_DEFINE_TYPE_WITH_CODE(ObjectName, object_name, PARENT_TYPE, CODE):
 *
 * Utility macro used to register gobject types in plugins with additional code.
 *
 * Deprecated: use G_DEFINE_DYNAMIC_TYPE_EXTENDED instead
 */
#define PLUMA_PLUGIN_DEFINE_TYPE_WITH_CODE(ObjectName, object_name, PARENT_TYPE, CODE) \
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
 * PLUMA_PLUGIN_DEFINE_TYPE(ObjectName, object_name, PARENT_TYPE):
 *
 * Utility macro used to register gobject types in plugins.
 *
 * Deprecated: use G_DEFINE_DYNAMIC instead
 */
#define PLUMA_PLUGIN_DEFINE_TYPE(ObjectName, object_name, PARENT_TYPE)		\
	PLUMA_PLUGIN_DEFINE_TYPE_WITH_CODE(ObjectName, object_name, PARENT_TYPE, ;)

/**
 * PLUMA_PLUGIN_IMPLEMENT_INTERFACE(TYPE_IFACE, iface_init):
 *
 * Utility macro used to register interfaces for gobject types in plugins.
 */
#define PLUMA_PLUGIN_IMPLEMENT_INTERFACE(object_name, TYPE_IFACE, iface_init)	\
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

#endif  /* __PLUMA_PLUGIN_H__ */
