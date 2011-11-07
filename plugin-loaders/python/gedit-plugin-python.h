/*
 * gedit-plugin-python.h
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */

#ifndef __GEDIT_PLUGIN_PYTHON_H__
#define __GEDIT_PLUGIN_PYTHON_H__

#define NO_IMPORT_PYGOBJECT

#include <glib-object.h>
#include <pygobject.h>

#include <gedit/gedit-plugin.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GEDIT_TYPE_PLUGIN_PYTHON		(gedit_plugin_python_get_type())
#define GEDIT_PLUGIN_PYTHON(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), GEDIT_TYPE_PLUGIN_PYTHON, GeditPluginPython))
#define GEDIT_PLUGIN_PYTHON_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), GEDIT_TYPE_PLUGIN_PYTHON, GeditPluginPythonClass))
#define GEDIT_IS_PLUGIN_PYTHON(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), GEDIT_TYPE_PLUGIN_PYTHON))
#define GEDIT_IS_PLUGIN_PYTHON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_PLUGIN_PYTHON))
#define GEDIT_PLUGIN_PYTHON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GEDIT_TYPE_PLUGIN_PYTHON, GeditPluginPythonClass))

/* Private structure type */
typedef struct _GeditPluginPythonPrivate GeditPluginPythonPrivate;

/*
 * Main object structure
 */
typedef struct _GeditPluginPython GeditPluginPython;

struct _GeditPluginPython 
{
	GeditPlugin parent;
	
	/*< private > */
	GeditPluginPythonPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GeditPluginPythonClass GeditPluginPythonClass;

struct _GeditPluginPythonClass 
{
	GeditPluginClass parent_class;
};

/*
 * Public methods
 */
GType	 gedit_plugin_python_get_type 		(void) G_GNUC_CONST;


/* 
 * Private methods
 */
void	  _gedit_plugin_python_set_instance	(GeditPluginPython *plugin, 
						 PyObject 	   *instance);
PyObject *_gedit_plugin_python_get_instance	(GeditPluginPython *plugin);

G_END_DECLS

#endif  /* __GEDIT_PLUGIN_PYTHON_H__ */

