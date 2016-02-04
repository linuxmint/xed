/*
 * xed-plugin-info-priv.h
 * This file is part of xed
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
 * Modified by the xed Team, 2002-2007. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __XED_PLUGIN_INFO_PRIV_H__
#define __XED_PLUGIN_INFO_PRIV_H__

#include "xed-plugin-info.h"
#include "xed-plugin.h"

struct _XedPluginInfo
{
	gint               refcount;

	XedPlugin       *plugin;
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

XedPluginInfo		*_xed_plugin_info_new		(const gchar *file);
void			 _xed_plugin_info_ref		(XedPluginInfo *info);
void			 _xed_plugin_info_unref	(XedPluginInfo *info);


#endif /* __XED_PLUGIN_INFO_PRIV_H__ */
