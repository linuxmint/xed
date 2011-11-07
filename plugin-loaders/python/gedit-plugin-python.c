/*
 * gedit-plugin-python.c
 * This file is part of gedit
 *
 * Copyright (C) 2005 Raphael Slinckx
 * Copyright (C) 2008 Jesse van den Kieboom
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

#include <config.h>

#include "gedit-plugin-python.h"

#include <gedit/gedit-plugin.h>
#include <gedit/gedit-debug.h>
#include <pygobject.h>
#include <string.h>

#define GEDIT_PLUGIN_PYTHON_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GEDIT_TYPE_PLUGIN_PYTHON, GeditPluginPythonPrivate))

static GObjectClass *parent_class;

struct _GeditPluginPythonPrivate 
{
	PyObject *instance;
};

static void	 gedit_plugin_python_class_init		(GeditPluginPythonClass *klass);
static void	 gedit_plugin_python_init 		(GeditPluginPython      *plugin);

G_DEFINE_TYPE (GeditPluginPython, gedit_plugin_python, GEDIT_TYPE_PLUGIN)

static PyObject *
call_python_method (GeditPluginPythonPrivate *priv,
		    GeditWindow		     *window,
		    gchar		     *method)
{
	PyObject *py_ret = NULL;

	g_return_val_if_fail (PyObject_HasAttrString (priv->instance, method), NULL);

	if (window == NULL)
	{
		py_ret = PyObject_CallMethod (priv->instance,
					      method,
					      NULL);
	}
	else
	{
		py_ret = PyObject_CallMethod (priv->instance,
					      method,
					      "(N)",
					      pygobject_new (G_OBJECT (window)));
	}
	
	if (!py_ret)
		PyErr_Print ();

	return py_ret;
}

static gboolean
check_py_object_is_gtk_widget (PyObject *py_obj)
{
	static PyTypeObject *_PyGtkWidget_Type = NULL;

	if (_PyGtkWidget_Type == NULL)
	{
		PyObject *module;

	    	if ((module = PyImport_ImportModule ("gtk")))
	    	{
			PyObject *moddict = PyModule_GetDict (module);
			_PyGtkWidget_Type = (PyTypeObject *) PyDict_GetItemString (moddict, "Widget");
	    	}

		if (_PyGtkWidget_Type == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "could not find Python gtk widget type");
			PyErr_Print();

			return FALSE;
		}
	}

	return PyObject_TypeCheck (py_obj, _PyGtkWidget_Type) ? TRUE : FALSE;
}

static void
impl_update_ui (GeditPlugin *plugin,
		GeditWindow *window)
{
	PyGILState_STATE state = pyg_gil_state_ensure ();
	GeditPluginPythonPrivate *priv = GEDIT_PLUGIN_PYTHON(plugin)->priv;
	
	if (PyObject_HasAttrString (priv->instance, "update_ui"))
	{		
		PyObject *py_ret = call_python_method (priv, window, "update_ui");
		
		if (py_ret)
		{
			Py_XDECREF (py_ret);
		}
	}
	else
		GEDIT_PLUGIN_CLASS (parent_class)->update_ui (plugin, window);

	pyg_gil_state_release (state);
}

static void
impl_deactivate (GeditPlugin *plugin,
		 GeditWindow *window)
{
	PyGILState_STATE state = pyg_gil_state_ensure ();
	GeditPluginPythonPrivate *priv = GEDIT_PLUGIN_PYTHON(plugin)->priv;
	
	if (PyObject_HasAttrString (priv->instance, "deactivate"))
	{		
		PyObject *py_ret = call_python_method (priv, window, "deactivate");
		
		if (py_ret)
		{
			Py_XDECREF (py_ret);
		}
	}
	else
		GEDIT_PLUGIN_CLASS (parent_class)->deactivate (plugin, window);

	pyg_gil_state_release (state);
}

static void
impl_activate (GeditPlugin *plugin,
	       GeditWindow *window)
{
	PyGILState_STATE state = pyg_gil_state_ensure ();
	GeditPluginPythonPrivate *priv = GEDIT_PLUGIN_PYTHON(plugin)->priv;
		
	if (PyObject_HasAttrString (priv->instance, "activate"))
	{
		PyObject *py_ret = call_python_method (priv, window, "activate");

		if (py_ret)
		{
			Py_XDECREF (py_ret);
		}
	}
	else
		GEDIT_PLUGIN_CLASS (parent_class)->activate (plugin, window);
	
	pyg_gil_state_release (state);
}

static GtkWidget *
impl_create_configure_dialog (GeditPlugin *plugin)
{
	PyGILState_STATE state = pyg_gil_state_ensure ();
	GeditPluginPythonPrivate *priv = GEDIT_PLUGIN_PYTHON(plugin)->priv;
	GtkWidget *ret = NULL;
	
	if (PyObject_HasAttrString (priv->instance, "create_configure_dialog"))
	{
		PyObject *py_ret = call_python_method (priv, NULL, "create_configure_dialog");
	
		if (py_ret)
		{
			if (check_py_object_is_gtk_widget (py_ret))
			{
				ret = GTK_WIDGET (pygobject_get (py_ret));
				g_object_ref (ret);
			}
			else
			{
				PyErr_SetString(PyExc_TypeError, "return value for create_configure_dialog is not a GtkWidget");
				PyErr_Print();
			}
			
			Py_DECREF (py_ret);
		}
	}
	else
		ret = GEDIT_PLUGIN_CLASS (parent_class)->create_configure_dialog (plugin);
 
	pyg_gil_state_release (state);
	
	return ret;
}

static gboolean
impl_is_configurable (GeditPlugin *plugin)
{
	PyGILState_STATE state = pyg_gil_state_ensure ();
	GeditPluginPythonPrivate *priv = GEDIT_PLUGIN_PYTHON(plugin)->priv;
	PyObject *dict = priv->instance->ob_type->tp_dict;	
	gboolean result;
	
	if (dict == NULL)
		result = FALSE;
	else if (!PyDict_Check(dict))
		result = FALSE;
	else 
		result = PyDict_GetItemString(dict, "create_configure_dialog") != NULL;

	pyg_gil_state_release (state);
	
	return result;
}

void
_gedit_plugin_python_set_instance (GeditPluginPython *plugin, 
				  PyObject 	    *instance)
{
	PyGILState_STATE state = pyg_gil_state_ensure ();
	
	/* we don't increment the instance here because we are the instance,
	   when it dies, we also die */
	plugin->priv->instance = instance;
	pyg_gil_state_release (state);
}

PyObject *
_gedit_plugin_python_get_instance (GeditPluginPython *plugin)
{
	return plugin->priv->instance;
}

static void
gedit_plugin_python_init (GeditPluginPython *plugin)
{
	plugin->priv = GEDIT_PLUGIN_PYTHON_GET_PRIVATE(plugin);

	gedit_debug_message (DEBUG_PLUGINS, "Creating Python plugin instance");
	plugin->priv->instance = 0;
}

static void
gedit_plugin_python_finalize (GObject *object)
{
	PyGILState_STATE state;
	
	gedit_debug_message (DEBUG_PLUGINS, "Finalizing Python plugin instance");

	state = pyg_gil_state_ensure ();
	Py_XDECREF (GEDIT_PLUGIN_PYTHON(object)->priv->instance);
	pyg_gil_state_release (state);
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gedit_plugin_python_class_init (GeditPluginPythonClass *klass)
{
	GeditPluginClass *plugin_class = GEDIT_PLUGIN_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	g_type_class_add_private (klass, sizeof (GeditPluginPythonPrivate));
	G_OBJECT_CLASS (klass)->finalize = gedit_plugin_python_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
	plugin_class->update_ui = impl_update_ui;
	plugin_class->create_configure_dialog = impl_create_configure_dialog;
	plugin_class->is_configurable = impl_is_configurable;
}

