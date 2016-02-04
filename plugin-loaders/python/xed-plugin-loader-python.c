/*
 * xed-plugin-loader-python.c
 * This file is part of xed
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

#include "xed-plugin-loader-python.h"
#include "xed-plugin-python.h"
#include <xed/xed-object-module.h>

#define NO_IMPORT_PYGOBJECT
#define NO_IMPORT_PYGTK

#include <Python.h>
#include <pygobject.h>
#include <pygtk/pygtk.h>
#include <signal.h>
#include "config.h"

#if PY_VERSION_HEX < 0x02050000
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
#endif

#define XED_PLUGIN_LOADER_PYTHON_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), XED_TYPE_PLUGIN_LOADER_PYTHON, XedPluginLoaderPythonPrivate))

struct _XedPluginLoaderPythonPrivate
{
	GHashTable *loaded_plugins;
	guint idle_gc;
	gboolean init_failed;
};

typedef struct
{
	PyObject *type;
	PyObject *instance;
	gchar    *path;
} PythonInfo;

static void xed_plugin_loader_iface_init (gpointer g_iface, gpointer iface_data);

/* Exported by pyxed module */
void pyxed_register_classes (PyObject *d);
void pyxed_add_constants (PyObject *module, const gchar *strip_prefix);
extern PyMethodDef pyxed_functions[];

/* Exported by pyxedutils module */
void pyxedutils_register_classes (PyObject *d);
extern PyMethodDef pyxedutils_functions[];

/* Exported by pyxedcommands module */
void pyxedcommands_register_classes (PyObject *d);
extern PyMethodDef pyxedcommands_functions[];

/* We retreive this to check for correct class hierarchy */
static PyTypeObject *PyXedPlugin_Type;

XED_PLUGIN_LOADER_REGISTER_TYPE (XedPluginLoaderPython, xed_plugin_loader_python, G_TYPE_OBJECT, xed_plugin_loader_iface_init);


static PyObject *
find_python_plugin_type (XedPluginInfo *info,
			 PyObject        *pymodule)
{
	PyObject *locals, *key, *value;
	Py_ssize_t pos = 0;
	
	locals = PyModule_GetDict (pymodule);

	while (PyDict_Next (locals, &pos, &key, &value))
	{
		if (!PyType_Check(value))
			continue;

		if (PyObject_IsSubclass (value, (PyObject*) PyXedPlugin_Type))
			return value;
	}
	
	g_warning ("No XedPlugin derivative found in Python plugin '%s'",
		   xed_plugin_info_get_name (info));
	return NULL;
}

static XedPlugin *
new_plugin_from_info (XedPluginLoaderPython *loader,
		      XedPluginInfo         *info)
{
	PythonInfo *pyinfo;
	PyTypeObject *pytype;
	PyObject *pyobject;
	PyGObject *pygobject;
	XedPlugin *instance;
	PyObject *emptyarg;

	pyinfo = (PythonInfo *)g_hash_table_lookup (loader->priv->loaded_plugins, info);
	
	if (pyinfo == NULL)
		return NULL;
	
	pytype = (PyTypeObject *)pyinfo->type;
	
	if (pytype->tp_new == NULL)
		return NULL;

	emptyarg = PyTuple_New(0);
	pyobject = pytype->tp_new (pytype, emptyarg, NULL);
	Py_DECREF (emptyarg);
	
	if (pyobject == NULL)
	{
		g_error ("Could not create instance for %s.", xed_plugin_info_get_name (info));
		return NULL;
	}

	pygobject = (PyGObject *)pyobject;
	
	if (pygobject->obj != NULL)
	{
		Py_DECREF (pyobject);
		g_error ("Could not create instance for %s (GObject already initialized).", xed_plugin_info_get_name (info));
		return NULL;
	}
	
	pygobject_construct (pygobject,
			     "install-dir", pyinfo->path,
			     "data-dir-name", xed_plugin_info_get_module_name (info),
			     NULL);
	
	if (pygobject->obj == NULL)
	{
		g_error ("Could not create instance for %s (GObject not constructed).", xed_plugin_info_get_name (info));
		Py_DECREF (pyobject);

		return NULL;
	}

	/* now call tp_init manually */
	if (PyType_IsSubtype (pyobject->ob_type, pytype) && 
	    pyobject->ob_type->tp_init != NULL)
	{
		emptyarg = PyTuple_New(0);
		pyobject->ob_type->tp_init (pyobject, emptyarg, NULL);
		Py_DECREF (emptyarg);
	}

	instance = XED_PLUGIN (pygobject->obj);
	pyinfo->instance = (PyObject *)pygobject;

	/* make sure to register the python instance for the XedPluginPython
	   object to it can wrap the virtual xed plugin funcs back to python */
	_xed_plugin_python_set_instance (XED_PLUGIN_PYTHON (instance), (PyObject *)pygobject);
	
	/* we return a reference here because the other is owned by python */
	return XED_PLUGIN (g_object_ref (instance));
}

static XedPlugin *
add_python_info (XedPluginLoaderPython *loader,
		 XedPluginInfo         *info,
		 PyObject		 *module,
		 const gchar             *path,
		 PyObject                *type)
{
	PythonInfo *pyinfo;
	
	pyinfo = g_new (PythonInfo, 1);
	pyinfo->path = g_strdup (path);
	pyinfo->type = type;

	Py_INCREF (pyinfo->type);
	
	g_hash_table_insert (loader->priv->loaded_plugins, info, pyinfo);
	
	return new_plugin_from_info (loader, info);
}

static const gchar *
xed_plugin_loader_iface_get_id (void)
{
	return "Python";
}

static XedPlugin *
xed_plugin_loader_iface_load (XedPluginLoader *loader,
				XedPluginInfo   *info,
				const gchar       *path)
{
	XedPluginLoaderPython *pyloader = XED_PLUGIN_LOADER_PYTHON (loader);
	PyObject *main_module, *main_locals, *pytype;
	PyObject *pymodule, *fromlist;
	gchar *module_name;
	XedPlugin *result;
	
	if (pyloader->priv->init_failed)
	{
		g_warning ("Cannot load python plugin Python '%s' since xed was"
		           "not able to initialize the Python interpreter.",
		           xed_plugin_info_get_name (info));
		return NULL;
	}
	
	/* see if py definition for the plugin is already loaded */
	result = new_plugin_from_info (pyloader, info);
	
	if (result != NULL)
		return result;
	
	main_module = PyImport_AddModule ("xed.plugins");
	if (main_module == NULL)
	{
		g_warning ("Could not get xed.plugins.");
		return NULL;
	}
	
	/* If we have a special path, we register it */
	if (path != NULL)
	{
		PyObject *sys_path = PySys_GetObject ("path");
		PyObject *pypath = PyString_FromString (path);

		if (PySequence_Contains (sys_path, pypath) == 0)
			PyList_Insert (sys_path, 0, pypath);

		Py_DECREF (pypath);
	}
	
	main_locals = PyModule_GetDict (main_module);
	
	/* we need a fromlist to be able to import modules with a '.' in the
	   name. */
	fromlist = PyTuple_New(0);
	module_name = g_strdup (xed_plugin_info_get_module_name (info));
	
	pymodule = PyImport_ImportModuleEx (module_name, 
					    main_locals, 
					    main_locals, 
					    fromlist);
	
	Py_DECREF(fromlist);

	if (!pymodule)
	{
		g_free (module_name);
		PyErr_Print ();
		return NULL;
	}

	PyDict_SetItemString (main_locals, module_name, pymodule);
	g_free (module_name);
	
	pytype = find_python_plugin_type (info, pymodule);
	
	if (pytype)
		return add_python_info (pyloader, info, pymodule, path, pytype);

	return NULL;
}

static void
xed_plugin_loader_iface_unload (XedPluginLoader *loader,
				  XedPluginInfo   *info)
{
	XedPluginLoaderPython *pyloader = XED_PLUGIN_LOADER_PYTHON (loader);
	PythonInfo *pyinfo;
	PyGILState_STATE state;
	
	pyinfo = (PythonInfo *)g_hash_table_lookup (pyloader->priv->loaded_plugins, info);
	
	if (!pyinfo)
		return;
	
	state = pyg_gil_state_ensure ();
	Py_XDECREF (pyinfo->instance);
	pyg_gil_state_release (state);
	
	pyinfo->instance = NULL;
}

static gboolean
run_gc (XedPluginLoaderPython *loader)
{
	while (PyGC_Collect ())
		;

	loader->priv->idle_gc = 0;
	return FALSE;
}

static void
xed_plugin_loader_iface_garbage_collect (XedPluginLoader *loader)
{
	XedPluginLoaderPython *pyloader;
	
	if (!Py_IsInitialized())
		return;

	pyloader = XED_PLUGIN_LOADER_PYTHON (loader);

	/*
	 * We both run the GC right now and we schedule
	 * a further collection in the main loop.
	 */

	while (PyGC_Collect ())
		;

	if (pyloader->priv->idle_gc == 0)
		pyloader->priv->idle_gc = g_idle_add ((GSourceFunc)run_gc, pyloader);
}

static void
xed_plugin_loader_iface_init (gpointer g_iface, 
				gpointer iface_data)
{
	XedPluginLoaderInterface *iface = (XedPluginLoaderInterface *)g_iface;
	
	iface->get_id = xed_plugin_loader_iface_get_id;
	iface->load = xed_plugin_loader_iface_load;
	iface->unload = xed_plugin_loader_iface_unload;
	iface->garbage_collect = xed_plugin_loader_iface_garbage_collect;
}

static void
xed_python_shutdown (XedPluginLoaderPython *loader)
{
	if (!Py_IsInitialized ())
		return;

	if (loader->priv->idle_gc != 0)
	{
		g_source_remove (loader->priv->idle_gc);
		loader->priv->idle_gc = 0;
	}

	while (PyGC_Collect ())
		;	

	Py_Finalize ();
}


/* C equivalent of
 *    import pygtk
 *    pygtk.require ("2.0")
 */
static gboolean
xed_check_pygtk2 (void)
{
	PyObject *pygtk, *mdict, *require;

	/* pygtk.require("2.0") */
	pygtk = PyImport_ImportModule ("pygtk");
	if (pygtk == NULL)
	{
		g_warning ("Error initializing Python interpreter: could not import pygtk.");
		return FALSE;
	}

	mdict = PyModule_GetDict (pygtk);
	require = PyDict_GetItemString (mdict, "require");
	PyObject_CallObject (require,
			     Py_BuildValue ("(S)", PyString_FromString ("2.0")));
	if (PyErr_Occurred())
	{
		g_warning ("Error initializing Python interpreter: pygtk 2 is required.");
		return FALSE;
	}

	return TRUE;
}

/* Note: the following two functions are needed because
 * init_pyobject and init_pygtk which are *macros* which in case
 * case of error set the PyErr and then make the calling
 * function return behind our back.
 * It's up to the caller to check the result with PyErr_Occurred()
 */
static void
xed_init_pygobject (void)
{
	init_pygobject_check (2, 11, 5); /* FIXME: get from config */
}

static void
xed_init_pygtk (void)
{
	PyObject *gtk, *mdict, *version, *required_version;

	init_pygtk ();

	/* there isn't init_pygtk_check(), do the version
	 * check ourselves */
	gtk = PyImport_ImportModule("gtk");
	mdict = PyModule_GetDict(gtk);
	version = PyDict_GetItemString (mdict, "pygtk_version");
	if (!version)
	{
		PyErr_SetString (PyExc_ImportError,
				 "PyGObject version too old");
		return;
	}

	required_version = Py_BuildValue ("(iii)", 2, 4, 0); /* FIXME */

	if (PyObject_Compare (version, required_version) == -1)
	{
		PyErr_SetString (PyExc_ImportError,
				 "PyGObject version too old");
		Py_DECREF (required_version);
		return;
	}

	Py_DECREF (required_version);
}

static void
old_gtksourceview_init (void)
{
	PyErr_SetString(PyExc_ImportError,
			"gtksourceview module not allowed, use gtksourceview2");
}

static void
xed_init_pygtksourceview (void)
{
	PyObject *gtksourceview, *mdict, *version, *required_version;

	gtksourceview = PyImport_ImportModule("gtksourceview2");
	if (gtksourceview == NULL)
	{
		PyErr_SetString (PyExc_ImportError,
				 "could not import gtksourceview");
		return;
	}

	mdict = PyModule_GetDict (gtksourceview);
	version = PyDict_GetItemString (mdict, "pygtksourceview2_version");
	if (!version)
	{
		PyErr_SetString (PyExc_ImportError,
				 "PyGtkSourceView version too old");
		return;
	}

	required_version = Py_BuildValue ("(iii)", 0, 8, 0); /* FIXME */

	if (PyObject_Compare (version, required_version) == -1)
	{
		PyErr_SetString (PyExc_ImportError,
				 "PyGtkSourceView version too old");
		Py_DECREF (required_version);
		return;
	}

	Py_DECREF (required_version);

	/* Create a dummy 'gtksourceview' module to prevent
	 * loading of the old 'gtksourceview' modules that
	 * has conflicting symbols with the gtksourceview2 module.
	 * Raise an exception when trying to import it.
	 */
	PyImport_AppendInittab ("gtksourceview", old_gtksourceview_init);
}

static gboolean
xed_python_init (XedPluginLoaderPython *loader)
{
	PyObject *mdict, *tuple;
	PyObject *xed, *xedutils, *xedcommands, *xedplugins;
	PyObject *gettext, *install, *gettext_args;
	//char *argv[] = { "xed", NULL };
	char *argv[] = { XED_PLUGINS_LIBS_DIR, NULL };
#ifdef HAVE_SIGACTION
	gint res;
	struct sigaction old_sigint;
#endif

	if (loader->priv->init_failed)
	{
		/* We already failed to initialized Python, don't need to
		 * retry again */
		return FALSE;
	}
	
	if (Py_IsInitialized ())
	{
		/* Python has already been successfully initialized */
		return TRUE;
	}

	/* We are trying to initialize Python for the first time,
	   set init_failed to FALSE only if the entire initialization process
	   ends with success */
	loader->priv->init_failed = TRUE;

	/* Hack to make python not overwrite SIGINT: this is needed to avoid
	 * the crash reported on bug #326191 */

	/* CHECK: can't we use Py_InitializeEx instead of Py_Initialize in order
          to avoid to manage signal handlers ? - Paolo (Dec. 31, 2006) */

#ifdef HAVE_SIGACTION
	/* Save old handler */
	res = sigaction (SIGINT, NULL, &old_sigint);  
	if (res != 0)
	{
		g_warning ("Error initializing Python interpreter: cannot get "
		           "handler to SIGINT signal (%s)",
		           g_strerror (errno));

		return FALSE;
	}
#endif

	/* Python initialization */
	Py_Initialize ();

#ifdef HAVE_SIGACTION
	/* Restore old handler */
	res = sigaction (SIGINT, &old_sigint, NULL);
	if (res != 0)
	{
		g_warning ("Error initializing Python interpreter: cannot restore "
		           "handler to SIGINT signal (%s).",
		           g_strerror (errno));

		goto python_init_error;
	}
#endif

	PySys_SetArgv (1, argv);

	if (!xed_check_pygtk2 ())
	{
		/* Warning message already printed in check_pygtk2 */
		goto python_init_error;
	}

	/* import gobject */	
	xed_init_pygobject ();
	if (PyErr_Occurred ())
	{
		g_warning ("Error initializing Python interpreter: could not import pygobject.");

		goto python_init_error;		
	}

	/* import gtk */
	xed_init_pygtk ();
	if (PyErr_Occurred ())
	{
		g_warning ("Error initializing Python interpreter: could not import pygtk.");

		goto python_init_error;
	}
	
	/* import gtksourceview */
	xed_init_pygtksourceview ();
	if (PyErr_Occurred ())
	{
		PyErr_Print ();

		g_warning ("Error initializing Python interpreter: could not import pygtksourceview.");

		goto python_init_error;
	}	
	
	/* import xed */
	xed = Py_InitModule ("xed", pyxed_functions);
	mdict = PyModule_GetDict (xed);

	pyxed_register_classes (mdict);
	pyxed_add_constants (xed, "XED_");

	/* xed version */
	tuple = Py_BuildValue("(iii)", 
			      XED_MAJOR_VERSION,
			      XED_MINOR_VERSION,
			      XED_MICRO_VERSION);
	PyDict_SetItemString(mdict, "version", tuple);
	Py_DECREF(tuple);
	
	/* Retrieve the Python type for xed.Plugin */
	PyXedPlugin_Type = (PyTypeObject *) PyDict_GetItemString (mdict, "Plugin"); 
	if (PyXedPlugin_Type == NULL)
	{
		PyErr_Print ();

		goto python_init_error;
	}

	/* import xed.utils */
	xedutils = Py_InitModule ("xed.utils", pyxedutils_functions);
	PyDict_SetItemString (mdict, "utils", xedutils);

	/* import xed.commands */
	xedcommands = Py_InitModule ("xed.commands", pyxedcommands_functions);
	PyDict_SetItemString (mdict, "commands", xedcommands);

	/* initialize empty xed.plugins module */
	xedplugins = Py_InitModule ("xed.plugins", NULL);
	PyDict_SetItemString (mdict, "plugins", xedplugins);

	mdict = PyModule_GetDict (xedutils);
	pyxedutils_register_classes (mdict);
	
	mdict = PyModule_GetDict (xedcommands);
	pyxedcommands_register_classes (mdict);

	/* i18n support */
	gettext = PyImport_ImportModule ("gettext");
	if (gettext == NULL)
	{
		g_warning ("Error initializing Python interpreter: could not import gettext.");

		goto python_init_error;
	}

	mdict = PyModule_GetDict (gettext);
	install = PyDict_GetItemString (mdict, "install");
	gettext_args = Py_BuildValue ("ss", GETTEXT_PACKAGE, XED_LOCALEDIR);
	PyObject_CallObject (install, gettext_args);
	Py_DECREF (gettext_args);
	
	/* Python has been successfully initialized */
	loader->priv->init_failed = FALSE;
	
	return TRUE;
	
python_init_error:

	g_warning ("Please check the installation of all the Python related packages required "
	           "by xed and try again.");

	PyErr_Clear ();

	xed_python_shutdown (loader);

	return FALSE;
}

static void
xed_plugin_loader_python_finalize (GObject *object)
{
	XedPluginLoaderPython *pyloader = XED_PLUGIN_LOADER_PYTHON (object);
	
	g_hash_table_destroy (pyloader->priv->loaded_plugins);
	xed_python_shutdown (pyloader);

	G_OBJECT_CLASS (xed_plugin_loader_python_parent_class)->finalize (object);
}

static void
xed_plugin_loader_python_class_init (XedPluginLoaderPythonClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	object_class->finalize = xed_plugin_loader_python_finalize;

	g_type_class_add_private (object_class, sizeof (XedPluginLoaderPythonPrivate));
}

static void
xed_plugin_loader_python_class_finalize (XedPluginLoaderPythonClass *klass)
{
}

static void
destroy_python_info (PythonInfo *info)
{
	PyGILState_STATE state = pyg_gil_state_ensure ();
	Py_XDECREF (info->type);	
	pyg_gil_state_release (state);
	
	g_free (info->path);
	g_free (info);
}

static void
xed_plugin_loader_python_init (XedPluginLoaderPython *self)
{
	self->priv = XED_PLUGIN_LOADER_PYTHON_GET_PRIVATE (self);
	
	/* initialize python interpreter */
	xed_python_init (self);

	/* loaded_plugins maps XedPluginInfo to a PythonInfo */
	self->priv->loaded_plugins = g_hash_table_new_full (g_direct_hash,
						            g_direct_equal,
						            NULL,
						            (GDestroyNotify)destroy_python_info);
}

XedPluginLoaderPython *
xed_plugin_loader_python_new ()
{
	GObject *loader = g_object_new (XED_TYPE_PLUGIN_LOADER_PYTHON, NULL);

	return XED_PLUGIN_LOADER_PYTHON (loader);
}

