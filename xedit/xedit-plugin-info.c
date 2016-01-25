/*
 * xedit-plugin-info.c
 * This file is part of xedit
 *
 * Copyright (C) 2002-2005 - Paolo Maggi 
 * Copyright (C) 2007 - Paolo Maggi, Steve Frécinaux
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
 * Modified by the xedit Team, 2002-2007. See the AUTHORS file for a
 * list of people on the xedit Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <glib/gi18n.h>
#include <glib.h>

#include "xedit-plugin-info.h"
#include "xedit-plugin-info-priv.h"
#include "xedit-debug.h"
#include "xedit-plugin.h"

void
_xedit_plugin_info_ref (XeditPluginInfo *info)
{
	g_atomic_int_inc (&info->refcount);
}

static XeditPluginInfo *
xedit_plugin_info_copy (XeditPluginInfo *info)
{
	_xedit_plugin_info_ref (info);
	return info;
}

void
_xedit_plugin_info_unref (XeditPluginInfo *info)
{
	if (!g_atomic_int_dec_and_test (&info->refcount))
		return;

	if (info->plugin != NULL)
	{
		xedit_debug_message (DEBUG_PLUGINS, "Unref plugin %s", info->name);

		g_object_unref (info->plugin);
	}

	g_free (info->file);
	g_free (info->module_name);
	g_strfreev (info->dependencies);
	g_free (info->name);
	g_free (info->desc);
	g_free (info->icon_name);
	g_free (info->website);
	g_free (info->copyright);
	g_free (info->loader);
	g_free (info->version);
	g_strfreev (info->authors);

	g_free (info);
}

/**
 * xedit_plugin_info_get_type:
 *
 * Retrieves the #GType object which is associated with the #XeditPluginInfo
 * class.
 *
 * Return value: the GType associated with #XeditPluginInfo.
 **/
GType
xedit_plugin_info_get_type (void)
{
	static GType the_type = 0;

	if (G_UNLIKELY (!the_type))
		the_type = g_boxed_type_register_static (
					"XeditPluginInfo",
					(GBoxedCopyFunc) xedit_plugin_info_copy,
					(GBoxedFreeFunc) _xedit_plugin_info_unref);

	return the_type;
} 

/**
 * xedit_plugin_info_new:
 * @filename: the filename where to read the plugin information
 *
 * Creates a new #XeditPluginInfo from a file on the disk.
 *
 * Return value: a newly created #XeditPluginInfo.
 */
XeditPluginInfo *
_xedit_plugin_info_new (const gchar *file)
{
	XeditPluginInfo *info;
	GKeyFile *plugin_file = NULL;
	gchar *str;

	g_return_val_if_fail (file != NULL, NULL);

	xedit_debug_message (DEBUG_PLUGINS, "Loading plugin: %s", file);

	info = g_new0 (XeditPluginInfo, 1);
	info->refcount = 1;
	info->file = g_strdup (file);

	plugin_file = g_key_file_new ();
	if (!g_key_file_load_from_file (plugin_file, file, G_KEY_FILE_NONE, NULL))
	{
		g_warning ("Bad plugin file: %s", file);
		goto error;
	}

	if (!g_key_file_has_key (plugin_file,
			   	 "Xedit Plugin",
				 "IAge",
				 NULL))
	{
		xedit_debug_message (DEBUG_PLUGINS,
				     "IAge key does not exist in file: %s", file);
		goto error;
	}
	
	/* Check IAge=2 */
	if (g_key_file_get_integer (plugin_file,
				    "Xedit Plugin",
				    "IAge",
				    NULL) != 2)
	{
		xedit_debug_message (DEBUG_PLUGINS,
				     "Wrong IAge in file: %s", file);
		goto error;
	}
				    
	/* Get module name */
	str = g_key_file_get_string (plugin_file,
				     "Xedit Plugin",
				     "Module",
				     NULL);

	if ((str != NULL) && (*str != '\0'))
	{
		info->module_name = str;
	}
	else
	{
		g_warning ("Could not find 'Module' in %s", file);
		g_free (str);
		goto error;
	}

	/* Get the dependency list */
	info->dependencies = g_key_file_get_string_list (plugin_file,
							 "Xedit Plugin",
							 "Depends",
							 NULL,
							 NULL);
	if (info->dependencies == NULL)
	{
		xedit_debug_message (DEBUG_PLUGINS, "Could not find 'Depends' in %s", file);
		info->dependencies = g_new0 (gchar *, 1);
	}

	/* Get the loader for this plugin */
	str = g_key_file_get_string (plugin_file,
				     "Xedit Plugin",
				     "Loader",
				     NULL);
	
	if ((str != NULL) && (*str != '\0'))
	{
		info->loader = str;
	}
	else
	{
		/* default to the C loader */
		info->loader = g_strdup("c");
		g_free (str);
	}

	/* Get Name */
	str = g_key_file_get_locale_string (plugin_file,
					    "Xedit Plugin",
					    "Name",
					    NULL, NULL);
	if (str)
		info->name = str;
	else
	{
		g_warning ("Could not find 'Name' in %s", file);
		goto error;
	}

	/* Get Description */
	str = g_key_file_get_locale_string (plugin_file,
					    "Xedit Plugin",
					    "Description",
					    NULL, NULL);
	if (str)
		info->desc = str;
	else
		xedit_debug_message (DEBUG_PLUGINS, "Could not find 'Description' in %s", file);

	/* Get Icon */
	str = g_key_file_get_locale_string (plugin_file,
					    "Xedit Plugin",
					    "Icon",
					    NULL, NULL);
	if (str)
		info->icon_name = str;
	else
		xedit_debug_message (DEBUG_PLUGINS, "Could not find 'Icon' in %s, using 'xedit-plugin'", file);
	

	/* Get Authors */
	info->authors = g_key_file_get_string_list (plugin_file,
						    "Xedit Plugin",
						    "Authors",
						    NULL,
						    NULL);
	if (info->authors == NULL)
		xedit_debug_message (DEBUG_PLUGINS, "Could not find 'Authors' in %s", file);


	/* Get Copyright */
	str = g_key_file_get_string (plugin_file,
				     "Xedit Plugin",
				     "Copyright",
				     NULL);
	if (str)
		info->copyright = str;
	else
		xedit_debug_message (DEBUG_PLUGINS, "Could not find 'Copyright' in %s", file);

	/* Get Website */
	str = g_key_file_get_string (plugin_file,
				     "Xedit Plugin",
				     "Website",
				     NULL);
	if (str)
		info->website = str;
	else
		xedit_debug_message (DEBUG_PLUGINS, "Could not find 'Website' in %s", file);
	
	/* Get Version */
	str = g_key_file_get_string (plugin_file,
				     "Xedit Plugin",
				     "Version",
				     NULL);
	if (str)
		info->version = str;
	else
		xedit_debug_message (DEBUG_PLUGINS, "Could not find 'Version' in %s", file);
	
	g_key_file_free (plugin_file);
	
	/* If we know nothing about the availability of the plugin,
	   set it as available */
	info->available = TRUE;
	
	return info;

error:
	g_free (info->file);
	g_free (info->module_name);
	g_free (info->name);
	g_free (info->loader);
	g_free (info);
	g_key_file_free (plugin_file);

	return NULL;
}

gboolean
xedit_plugin_info_is_active (XeditPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, FALSE);

	return info->available && info->plugin != NULL;
}

gboolean
xedit_plugin_info_is_available (XeditPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, FALSE);

	return info->available != FALSE;
}

gboolean
xedit_plugin_info_is_configurable (XeditPluginInfo *info)
{
	xedit_debug_message (DEBUG_PLUGINS, "Is '%s' configurable?", info->name);

	g_return_val_if_fail (info != NULL, FALSE);

	if (info->plugin == NULL || !info->available)
		return FALSE;

	return xedit_plugin_is_configurable (info->plugin);
}

const gchar *
xedit_plugin_info_get_module_name (XeditPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->module_name;
}

const gchar *
xedit_plugin_info_get_name (XeditPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->name;
}

const gchar *
xedit_plugin_info_get_description (XeditPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->desc;
}

const gchar *
xedit_plugin_info_get_icon_name (XeditPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	/* use the xedit-plugin icon as a default if the plugin does not
	   have its own */
	if (info->icon_name != NULL && 
	    gtk_icon_theme_has_icon (gtk_icon_theme_get_default (),
				     info->icon_name))
		return info->icon_name;
	else
		return "xedit-plugin";
}

const gchar **
xedit_plugin_info_get_authors (XeditPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, (const gchar **)NULL);

	return (const gchar **) info->authors;
}

const gchar *
xedit_plugin_info_get_website (XeditPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->website;
}

const gchar *
xedit_plugin_info_get_copyright (XeditPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->copyright;
}

const gchar *
xedit_plugin_info_get_version (XeditPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->version;
}
