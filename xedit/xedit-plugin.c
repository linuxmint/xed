/*
 * xedit-plugin.h
 * This file is part of xedit
 *
 * Copyright (C) 2002-2005 Paolo Maggi
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
 * Modified by the xedit Team, 2002-2005. See the AUTHORS file for a
 * list of people on the xedit Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xedit-plugin.h"
#include "xedit-dirs.h"

/* properties */
enum {
	PROP_0,
	PROP_INSTALL_DIR,
	PROP_DATA_DIR_NAME,
	PROP_DATA_DIR
};

typedef struct _XeditPluginPrivate XeditPluginPrivate;

struct _XeditPluginPrivate
{
	gchar *install_dir;
	gchar *data_dir_name;
};

#define XEDIT_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), XEDIT_TYPE_PLUGIN, XeditPluginPrivate))

G_DEFINE_TYPE(XeditPlugin, xedit_plugin, G_TYPE_OBJECT)

static void
dummy (XeditPlugin *plugin, XeditWindow *window)
{
	/* Empty */
}

static GtkWidget *
create_configure_dialog	(XeditPlugin *plugin)
{
	return NULL;
}

static gboolean
is_configurable (XeditPlugin *plugin)
{
	return (XEDIT_PLUGIN_GET_CLASS (plugin)->create_configure_dialog !=
		create_configure_dialog);
}

static void
xedit_plugin_get_property (GObject    *object,
			   guint       prop_id,
			   GValue     *value,
			   GParamSpec *pspec)
{
	switch (prop_id)
	{
		case PROP_INSTALL_DIR:
			g_value_take_string (value, xedit_plugin_get_install_dir (XEDIT_PLUGIN (object)));
			break;
		case PROP_DATA_DIR:
			g_value_take_string (value, xedit_plugin_get_data_dir (XEDIT_PLUGIN (object)));
			break;
		default:
			g_return_if_reached ();
	}
}

static void
xedit_plugin_set_property (GObject      *object,
			   guint         prop_id,
			   const GValue *value,
			   GParamSpec   *pspec)
{
	XeditPluginPrivate *priv = XEDIT_PLUGIN_GET_PRIVATE (object);

	switch (prop_id)
	{
		case PROP_INSTALL_DIR:
			priv->install_dir = g_value_dup_string (value);
			break;
		case PROP_DATA_DIR_NAME:
			priv->data_dir_name = g_value_dup_string (value);
			break;
		default:
			g_return_if_reached ();
	}
}

static void
xedit_plugin_finalize (GObject *object)
{
	XeditPluginPrivate *priv = XEDIT_PLUGIN_GET_PRIVATE (object);

	g_free (priv->install_dir);
	g_free (priv->data_dir_name);

	G_OBJECT_CLASS (xedit_plugin_parent_class)->finalize (object);
}

static void
xedit_plugin_class_init (XeditPluginClass *klass)
{
    	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	klass->activate = dummy;
	klass->deactivate = dummy;
	klass->update_ui = dummy;

	klass->create_configure_dialog = create_configure_dialog;
	klass->is_configurable = is_configurable;

	object_class->get_property = xedit_plugin_get_property;
	object_class->set_property = xedit_plugin_set_property;
	object_class->finalize = xedit_plugin_finalize;

	g_object_class_install_property (object_class,
					 PROP_INSTALL_DIR,
					 g_param_spec_string ("install-dir",
							      "Install Directory",
							      "The directory where the plugin is installed",
							      NULL,
							      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	/* the basename of the data dir is set at construction time by the plugin loader
	 * while the full path is constructed on the fly to take into account relocability
	 * that's why we have a writeonly prop and a readonly prop */
	g_object_class_install_property (object_class,
					 PROP_DATA_DIR_NAME,
					 g_param_spec_string ("data-dir-name",
							      "Basename of the data directory",
							      "The basename of the directory where the plugin should look for its data files",
							      NULL,
							      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (object_class,
					 PROP_DATA_DIR,
					 g_param_spec_string ("data-dir",
							      "Data Directory",
							      "The full path of the directory where the plugin should look for its data files",
							      NULL,
							      G_PARAM_READABLE));

	g_type_class_add_private (klass, sizeof (XeditPluginPrivate));
}

static void
xedit_plugin_init (XeditPlugin *plugin)
{
	/* Empty */
}

/**
 * xedit_plugin_get_install_dir:
 * @plugin: a #XeditPlugin
 *
 * Get the path of the directory where the plugin is installed.
 *
 * Return value: a newly allocated string with the path of the
 * directory where the plugin is installed
 */
gchar *
xedit_plugin_get_install_dir (XeditPlugin *plugin)
{
	g_return_val_if_fail (XEDIT_IS_PLUGIN (plugin), NULL);

	return g_strdup (XEDIT_PLUGIN_GET_PRIVATE (plugin)->install_dir);
}

/**
 * xedit_plugin_get_data_dir:
 * @plugin: a #XeditPlugin
 *
 * Get the path of the directory where the plugin should look for
 * its data files.
 *
 * Return value: a newly allocated string with the path of the
 * directory where the plugin should look for its data files
 */
gchar *
xedit_plugin_get_data_dir (XeditPlugin *plugin)
{
	XeditPluginPrivate *priv;
	gchar *xedit_lib_dir;
	gchar *data_dir;

	g_return_val_if_fail (XEDIT_IS_PLUGIN (plugin), NULL);

	priv = XEDIT_PLUGIN_GET_PRIVATE (plugin);

	/* If it's a "user" plugin the data dir is
	 * install_dir/data_dir_name if instead it's a
	 * "system" plugin the data dir is under xedit_data_dir,
	 * so it's under $prefix/share/xedit/plugins/data_dir_name
	 * where data_dir_name usually it's the name of the plugin
	 */
	xedit_lib_dir = xedit_dirs_get_xedit_lib_dir ();

	/* CHECK: is checking the prefix enough or should we be more
	 * careful about normalizing paths etc? */
	if (g_str_has_prefix (priv->install_dir, xedit_lib_dir))
	{
		gchar *xedit_data_dir;

		xedit_data_dir = xedit_dirs_get_xedit_data_dir ();

		data_dir = g_build_filename (xedit_data_dir,
					     "plugins",
					     priv->data_dir_name,
					     NULL);

		g_free (xedit_data_dir);
	}
	else
	{
		data_dir = g_build_filename (priv->install_dir,
					     priv->data_dir_name,
					     NULL);
	}

	g_free (xedit_lib_dir);

	return data_dir;
}

/**
 * xedit_plugin_activate:
 * @plugin: a #XeditPlugin
 * @window: a #XeditWindow
 *
 * Activates the plugin.
 */
void
xedit_plugin_activate (XeditPlugin *plugin,
		       XeditWindow *window)
{
	g_return_if_fail (XEDIT_IS_PLUGIN (plugin));
	g_return_if_fail (XEDIT_IS_WINDOW (window));

	XEDIT_PLUGIN_GET_CLASS (plugin)->activate (plugin, window);
}

/**
 * xedit_plugin_deactivate:
 * @plugin: a #XeditPlugin
 * @window: a #XeditWindow
 *
 * Deactivates the plugin.
 */
void
xedit_plugin_deactivate	(XeditPlugin *plugin,
			 XeditWindow *window)
{
	g_return_if_fail (XEDIT_IS_PLUGIN (plugin));
	g_return_if_fail (XEDIT_IS_WINDOW (window));

	XEDIT_PLUGIN_GET_CLASS (plugin)->deactivate (plugin, window);
}

/**
 * xedit_plugin_update_ui:
 * @plugin: a #XeditPlugin
 * @window: a #XeditWindow
 *
 * Triggers an update of the user interface to take into account state changes
 * caused by the plugin.
 */
void
xedit_plugin_update_ui	(XeditPlugin *plugin,
			 XeditWindow *window)
{
	g_return_if_fail (XEDIT_IS_PLUGIN (plugin));
	g_return_if_fail (XEDIT_IS_WINDOW (window));

	XEDIT_PLUGIN_GET_CLASS (plugin)->update_ui (plugin, window);
}

/**
 * xedit_plugin_is_configurable:
 * @plugin: a #XeditPlugin
 *
 * Whether the plugin is configurable.
 *
 * Returns: TRUE if the plugin is configurable:
 */
gboolean
xedit_plugin_is_configurable (XeditPlugin *plugin)
{
	g_return_val_if_fail (XEDIT_IS_PLUGIN (plugin), FALSE);

	return XEDIT_PLUGIN_GET_CLASS (plugin)->is_configurable (plugin);
}

/**
 * xedit_plugin_create_configure_dialog:
 * @plugin: a #XeditPlugin
 *
 * Creates the configure dialog widget for the plugin.
 *
 * Returns: the configure dialog widget for the plugin.
 */
GtkWidget *
xedit_plugin_create_configure_dialog (XeditPlugin *plugin)
{
	g_return_val_if_fail (XEDIT_IS_PLUGIN (plugin), NULL);

	return XEDIT_PLUGIN_GET_CLASS (plugin)->create_configure_dialog (plugin);
}
