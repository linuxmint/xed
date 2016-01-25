/*
 * xedit-plugin-info.h
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

#ifndef __XEDIT_PLUGIN_INFO_H__
#define __XEDIT_PLUGIN_INFO_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define XEDIT_TYPE_PLUGIN_INFO			(xedit_plugin_info_get_type ())
#define XEDIT_PLUGIN_INFO(obj)			((XeditPluginInfo *) (obj))

typedef struct _XeditPluginInfo			XeditPluginInfo;

GType		 xedit_plugin_info_get_type		(void) G_GNUC_CONST;

gboolean 	 xedit_plugin_info_is_active		(XeditPluginInfo *info);
gboolean 	 xedit_plugin_info_is_available		(XeditPluginInfo *info);
gboolean	 xedit_plugin_info_is_configurable	(XeditPluginInfo *info);

const gchar	*xedit_plugin_info_get_module_name	(XeditPluginInfo *info);

const gchar	*xedit_plugin_info_get_name		(XeditPluginInfo *info);
const gchar	*xedit_plugin_info_get_description	(XeditPluginInfo *info);
const gchar	*xedit_plugin_info_get_icon_name	(XeditPluginInfo *info);
const gchar    **xedit_plugin_info_get_authors		(XeditPluginInfo *info);
const gchar	*xedit_plugin_info_get_website		(XeditPluginInfo *info);
const gchar	*xedit_plugin_info_get_copyright	(XeditPluginInfo *info);
const gchar	*xedit_plugin_info_get_version		(XeditPluginInfo *info);

G_END_DECLS

#endif /* __XEDIT_PLUGIN_INFO_H__ */

