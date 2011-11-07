/*
 * gedit-plugin-loader-python.h
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

#ifndef __GEDIT_PLUGIN_LOADER_PYTHON_H__
#define __GEDIT_PLUGIN_LOADER_PYTHON_H__

#include <gedit/gedit-plugin-loader.h>

G_BEGIN_DECLS

#define GEDIT_TYPE_PLUGIN_LOADER_PYTHON		(gedit_plugin_loader_python_get_type ())
#define GEDIT_PLUGIN_LOADER_PYTHON(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_PLUGIN_LOADER_PYTHON, GeditPluginLoaderPython))
#define GEDIT_PLUGIN_LOADER_PYTHON_CONST(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_PLUGIN_LOADER_PYTHON, GeditPluginLoaderPython const))
#define GEDIT_PLUGIN_LOADER_PYTHON_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_PLUGIN_LOADER_PYTHON, GeditPluginLoaderPythonClass))
#define GEDIT_IS_PLUGIN_LOADER_PYTHON(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_PLUGIN_LOADER_PYTHON))
#define GEDIT_IS_PLUGIN_LOADER_PYTHON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_PLUGIN_LOADER_PYTHON))
#define GEDIT_PLUGIN_LOADER_PYTHON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_TYPE_PLUGIN_LOADER_PYTHON, GeditPluginLoaderPythonClass))

typedef struct _GeditPluginLoaderPython		GeditPluginLoaderPython;
typedef struct _GeditPluginLoaderPythonClass		GeditPluginLoaderPythonClass;
typedef struct _GeditPluginLoaderPythonPrivate	GeditPluginLoaderPythonPrivate;

struct _GeditPluginLoaderPython {
	GObject parent;
	
	GeditPluginLoaderPythonPrivate *priv;
};

struct _GeditPluginLoaderPythonClass {
	GObjectClass parent_class;
};

GType gedit_plugin_loader_python_get_type (void) G_GNUC_CONST;
GeditPluginLoaderPython *gedit_plugin_loader_python_new(void);

/* All the loaders must implement this function */
G_MODULE_EXPORT GType register_gedit_plugin_loader (GTypeModule * module);

G_END_DECLS

#endif /* __GEDIT_PLUGIN_LOADER_PYTHON_H__ */

