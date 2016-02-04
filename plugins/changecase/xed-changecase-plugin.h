/*
 * xed-changecase-plugin.h
 * 
 * Copyright (C) 2004-2005 - Paolo Borelli
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Id$
 */

#ifndef __XED_CHANGECASE_PLUGIN_H__
#define __XED_CHANGECASE_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <xed/xed-plugin.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XED_TYPE_CHANGECASE_PLUGIN		(xed_changecase_plugin_get_type ())
#define XED_CHANGECASE_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), XED_TYPE_CHANGECASE_PLUGIN, XedChangecasePlugin))
#define XED_CHANGECASE_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), XED_TYPE_CHANGECASE_PLUGIN, XedChangecasePluginClass))
#define XED_IS_CHANGECASE_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), XED_TYPE_CHANGECASE_PLUGIN))
#define XED_IS_CHANGECASE_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), XED_TYPE_CHANGECASE_PLUGIN))
#define XED_CHANGECASE_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), XED_TYPE_CHANGECASE_PLUGIN, XedChangecasePluginClass))

/*
 * Main object structure
 */
typedef struct _XedChangecasePlugin		XedChangecasePlugin;

struct _XedChangecasePlugin
{
	XedPlugin parent_instance;
};

/*
 * Class definition
 */
typedef struct _XedChangecasePluginClass	XedChangecasePluginClass;

struct _XedChangecasePluginClass
{
	XedPluginClass parent_class;
};

/*
 * Public methods
 */
GType	xed_changecase_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT GType register_xed_plugin (GTypeModule *module);

G_END_DECLS

#endif /* __XED_CHANGECASE_PLUGIN_H__ */
