/*
 * xedit-time-plugin.h
 * 
 * Copyright (C) 2002-2005 - Paolo Maggi
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

#ifndef __XEDIT_TIME_PLUGIN_H__
#define __XEDIT_TIME_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <xedit/xedit-plugin.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XEDIT_TYPE_TIME_PLUGIN		(xedit_time_plugin_get_type ())
#define XEDIT_TIME_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), XEDIT_TYPE_TIME_PLUGIN, XeditTimePlugin))
#define XEDIT_TIME_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), XEDIT_TYPE_TIME_PLUGIN, XeditTimePluginClass))
#define XEDIT_IS_TIME_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), XEDIT_TYPE_TIME_PLUGIN))
#define XEDIT_IS_TIME_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), XEDIT_TYPE_TIME_PLUGIN))
#define XEDIT_TIME_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), XEDIT_TYPE_TIME_PLUGIN, XeditTimePluginClass))

/* Private structure type */
typedef struct _XeditTimePluginPrivate	XeditTimePluginPrivate;

/*
 * Main object structure
 */
typedef struct _XeditTimePlugin		XeditTimePlugin;

struct _XeditTimePlugin
{
	XeditPlugin parent_instance;

	/*< private >*/
	XeditTimePluginPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XeditTimePluginClass	XeditTimePluginClass;

struct _XeditTimePluginClass
{
	XeditPluginClass parent_class;
};

/*
 * Public methods
 */
GType	xedit_time_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT GType register_xedit_plugin (GTypeModule *module);

G_END_DECLS

#endif /* __XEDIT_TIME_PLUGIN_H__ */
