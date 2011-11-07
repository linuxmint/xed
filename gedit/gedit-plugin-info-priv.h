/*
 * gedit-plugin-info-priv.h
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */
 
/*
 * Modified by the gedit Team, 2002-2007. See the AUTHORS file for a
 * list of people on the gedit Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __GEDIT_PLUGIN_INFO_PRIV_H__
#define __GEDIT_PLUGIN_INFO_PRIV_H__

#include "gedit-plugin-info.h"
#include "gedit-plugin.h"

struct _GeditPluginInfo
{
	gint               refcount;

	GeditPlugin       *plugin;
	gchar             *file;

	gchar             *module_name;
	gchar		  *loader;
	gchar            **dependencies;

	gchar             *name;
	gchar             *desc;
	gchar             *icon_name;
	gchar            **authors;
	gchar             *copyright;
	gchar             *website;
	gchar             *version;

	/* A plugin is unavailable if it is not possible to activate it
	   due to an error loading the plugin module (e.g. for Python plugins
	   when the interpreter has not been correctly initializated) */
	gint               available : 1;
};

GeditPluginInfo		*_gedit_plugin_info_new		(const gchar *file);
void			 _gedit_plugin_info_ref		(GeditPluginInfo *info);
void			 _gedit_plugin_info_unref	(GeditPluginInfo *info);


#endif /* __GEDIT_PLUGIN_INFO_PRIV_H__ */
