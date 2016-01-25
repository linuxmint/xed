/*
 * xedit-plugin-python.h
 * This file is part of xedit
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

#ifndef __XEDIT_PLUGIN_PYTHON_H__
#define __XEDIT_PLUGIN_PYTHON_H__

#define NO_IMPORT_PYGOBJECT

#include <glib-object.h>
#include <pygobject.h>

#include <xedit/xedit-plugin.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XEDIT_TYPE_PLUGIN_PYTHON		(xedit_plugin_python_get_type())
#define XEDIT_PLUGIN_PYTHON(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), XEDIT_TYPE_PLUGIN_PYTHON, XeditPluginPython))
#define XEDIT_PLUGIN_PYTHON_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), XEDIT_TYPE_PLUGIN_PYTHON, XeditPluginPythonClass))
#define XEDIT_IS_PLUGIN_PYTHON(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), XEDIT_TYPE_PLUGIN_PYTHON))
#define XEDIT_IS_PLUGIN_PYTHON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_PLUGIN_PYTHON))
#define XEDIT_PLUGIN_PYTHON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XEDIT_TYPE_PLUGIN_PYTHON, XeditPluginPythonClass))

/* Private structure type */
typedef struct _XeditPluginPythonPrivate XeditPluginPythonPrivate;

/*
 * Main object structure
 */
typedef struct _XeditPluginPython XeditPluginPython;

struct _XeditPluginPython 
{
	XeditPlugin parent;
	
	/*< private > */
	XeditPluginPythonPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XeditPluginPythonClass XeditPluginPythonClass;

struct _XeditPluginPythonClass 
{
	XeditPluginClass parent_class;
};

/*
 * Public methods
 */
GType	 xedit_plugin_python_get_type 		(void) G_GNUC_CONST;


/* 
 * Private methods
 */
void	  _xedit_plugin_python_set_instance	(XeditPluginPython *plugin, 
						 PyObject 	   *instance);
PyObject *_xedit_plugin_python_get_instance	(XeditPluginPython *plugin);

G_END_DECLS

#endif  /* __XEDIT_PLUGIN_PYTHON_H__ */

