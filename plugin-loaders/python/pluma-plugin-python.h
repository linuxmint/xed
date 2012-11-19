/*
 * pluma-plugin-python.h
 * This file is part of pluma
 *
 * Copyright (C) 2005 - Raphael Slinckx
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

#ifndef __PLUMA_PLUGIN_PYTHON_H__
#define __PLUMA_PLUGIN_PYTHON_H__

#define NO_IMPORT_PYGOBJECT

#include <glib-object.h>
#include <pygobject.h>

#include <pluma/pluma-plugin.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_PLUGIN_PYTHON		(pluma_plugin_python_get_type())
#define PLUMA_PLUGIN_PYTHON(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_PLUGIN_PYTHON, PlumaPluginPython))
#define PLUMA_PLUGIN_PYTHON_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), PLUMA_TYPE_PLUGIN_PYTHON, PlumaPluginPythonClass))
#define PLUMA_IS_PLUGIN_PYTHON(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), PLUMA_TYPE_PLUGIN_PYTHON))
#define PLUMA_IS_PLUGIN_PYTHON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_PLUGIN_PYTHON))
#define PLUMA_PLUGIN_PYTHON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), PLUMA_TYPE_PLUGIN_PYTHON, PlumaPluginPythonClass))

/* Private structure type */
typedef struct _PlumaPluginPythonPrivate PlumaPluginPythonPrivate;

/*
 * Main object structure
 */
typedef struct _PlumaPluginPython PlumaPluginPython;

struct _PlumaPluginPython 
{
	PlumaPlugin parent;
	
	/*< private > */
	PlumaPluginPythonPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _PlumaPluginPythonClass PlumaPluginPythonClass;

struct _PlumaPluginPythonClass 
{
	PlumaPluginClass parent_class;
};

/*
 * Public methods
 */
GType	 pluma_plugin_python_get_type 		(void) G_GNUC_CONST;


/* 
 * Private methods
 */
void	  _pluma_plugin_python_set_instance	(PlumaPluginPython *plugin, 
						 PyObject 	   *instance);
PyObject *_pluma_plugin_python_get_instance	(PlumaPluginPython *plugin);

G_END_DECLS

#endif  /* __PLUMA_PLUGIN_PYTHON_H__ */

