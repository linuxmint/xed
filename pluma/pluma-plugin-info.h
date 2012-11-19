/*
 * pluma-plugin-info.h
 * This file is part of pluma
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
 * Modified by the pluma Team, 2002-2007. See the AUTHORS file for a
 * list of people on the pluma Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __PLUMA_PLUGIN_INFO_H__
#define __PLUMA_PLUGIN_INFO_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define PLUMA_TYPE_PLUGIN_INFO			(pluma_plugin_info_get_type ())
#define PLUMA_PLUGIN_INFO(obj)			((PlumaPluginInfo *) (obj))

typedef struct _PlumaPluginInfo			PlumaPluginInfo;

GType		 pluma_plugin_info_get_type		(void) G_GNUC_CONST;

gboolean 	 pluma_plugin_info_is_active		(PlumaPluginInfo *info);
gboolean 	 pluma_plugin_info_is_available		(PlumaPluginInfo *info);
gboolean	 pluma_plugin_info_is_configurable	(PlumaPluginInfo *info);

const gchar	*pluma_plugin_info_get_module_name	(PlumaPluginInfo *info);

const gchar	*pluma_plugin_info_get_name		(PlumaPluginInfo *info);
const gchar	*pluma_plugin_info_get_description	(PlumaPluginInfo *info);
const gchar	*pluma_plugin_info_get_icon_name	(PlumaPluginInfo *info);
const gchar    **pluma_plugin_info_get_authors		(PlumaPluginInfo *info);
const gchar	*pluma_plugin_info_get_website		(PlumaPluginInfo *info);
const gchar	*pluma_plugin_info_get_copyright	(PlumaPluginInfo *info);
const gchar	*pluma_plugin_info_get_version		(PlumaPluginInfo *info);

G_END_DECLS

#endif /* __PLUMA_PLUGIN_INFO_H__ */

