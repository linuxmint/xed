/*
 * pluma-plugin-loader-python.h
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

#ifndef __PLUMA_PLUGIN_LOADER_PYTHON_H__
#define __PLUMA_PLUGIN_LOADER_PYTHON_H__

#include <pluma/pluma-plugin-loader.h>

G_BEGIN_DECLS

#define PLUMA_TYPE_PLUGIN_LOADER_PYTHON		(pluma_plugin_loader_python_get_type ())
#define PLUMA_PLUGIN_LOADER_PYTHON(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_PLUGIN_LOADER_PYTHON, PlumaPluginLoaderPython))
#define PLUMA_PLUGIN_LOADER_PYTHON_CONST(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_PLUGIN_LOADER_PYTHON, PlumaPluginLoaderPython const))
#define PLUMA_PLUGIN_LOADER_PYTHON_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PLUMA_TYPE_PLUGIN_LOADER_PYTHON, PlumaPluginLoaderPythonClass))
#define PLUMA_IS_PLUGIN_LOADER_PYTHON(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUMA_TYPE_PLUGIN_LOADER_PYTHON))
#define PLUMA_IS_PLUGIN_LOADER_PYTHON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_PLUGIN_LOADER_PYTHON))
#define PLUMA_PLUGIN_LOADER_PYTHON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), PLUMA_TYPE_PLUGIN_LOADER_PYTHON, PlumaPluginLoaderPythonClass))

typedef struct _PlumaPluginLoaderPython		PlumaPluginLoaderPython;
typedef struct _PlumaPluginLoaderPythonClass		PlumaPluginLoaderPythonClass;
typedef struct _PlumaPluginLoaderPythonPrivate	PlumaPluginLoaderPythonPrivate;

struct _PlumaPluginLoaderPython {
	GObject parent;
	
	PlumaPluginLoaderPythonPrivate *priv;
};

struct _PlumaPluginLoaderPythonClass {
	GObjectClass parent_class;
};

GType pluma_plugin_loader_python_get_type (void) G_GNUC_CONST;
PlumaPluginLoaderPython *pluma_plugin_loader_python_new(void);

/* All the loaders must implement this function */
G_MODULE_EXPORT GType register_pluma_plugin_loader (GTypeModule * module);

G_END_DECLS

#endif /* __PLUMA_PLUGIN_LOADER_PYTHON_H__ */

