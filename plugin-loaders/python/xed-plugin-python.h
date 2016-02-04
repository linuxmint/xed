/*
 * xed-plugin-python.h
 * This file is part of xed
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

#ifndef __XED_PLUGIN_PYTHON_H__
#define __XED_PLUGIN_PYTHON_H__

#define NO_IMPORT_PYGOBJECT

#include <glib-object.h>
#include <pygobject.h>

#include <xed/xed-plugin.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XED_TYPE_PLUGIN_PYTHON		(xed_plugin_python_get_type())
#define XED_PLUGIN_PYTHON(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_PLUGIN_PYTHON, XedPluginPython))
#define XED_PLUGIN_PYTHON_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_PLUGIN_PYTHON, XedPluginPythonClass))
#define XED_IS_PLUGIN_PYTHON(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_PLUGIN_PYTHON))
#define XED_IS_PLUGIN_PYTHON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_PLUGIN_PYTHON))
#define XED_PLUGIN_PYTHON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_PLUGIN_PYTHON, XedPluginPythonClass))

/* Private structure type */
typedef struct _XedPluginPythonPrivate XedPluginPythonPrivate;

/*
 * Main object structure
 */
typedef struct _XedPluginPython XedPluginPython;

struct _XedPluginPython 
{
	XedPlugin parent;
	
	/*< private > */
	XedPluginPythonPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XedPluginPythonClass XedPluginPythonClass;

struct _XedPluginPythonClass 
{
	XedPluginClass parent_class;
};

/*
 * Public methods
 */
GType	 xed_plugin_python_get_type 		(void) G_GNUC_CONST;


/* 
 * Private methods
 */
void	  _xed_plugin_python_set_instance	(XedPluginPython *plugin, 
						 PyObject 	   *instance);
PyObject *_xed_plugin_python_get_instance	(XedPluginPython *plugin);

G_END_DECLS

#endif  /* __XED_PLUGIN_PYTHON_H__ */

