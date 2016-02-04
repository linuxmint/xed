/*
 * xed-taglist-plugin.h
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
 */

/*
 * Modified by the xed Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the xed Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */
 
#ifndef __XED_TAGLIST_PLUGIN_H__
#define __XED_TAGLIST_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <xed/xed-plugin.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XED_TYPE_TAGLIST_PLUGIN		(xed_taglist_plugin_get_type ())
#define XED_TAGLIST_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), XED_TYPE_TAGLIST_PLUGIN, XedTaglistPlugin))
#define XED_TAGLIST_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), XED_TYPE_TAGLIST_PLUGIN, XedTaglistPluginClass))
#define XED_IS_TAGLIST_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), XED_TYPE_TAGLIST_PLUGIN))
#define XED_IS_TAGLIST_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), XED_TYPE_TAGLIST_PLUGIN))
#define XED_TAGLIST_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), XED_TYPE_TAGLIST_PLUGIN, XedTaglistPluginClass))

/* Private structure type */
typedef struct _XedTaglistPluginPrivate	XedTaglistPluginPrivate;

/*
 * Main object structure
 */
typedef struct _XedTaglistPlugin		XedTaglistPlugin;

struct _XedTaglistPlugin
{
	XedPlugin parent_instance;

	/*< private >*/
	XedTaglistPluginPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XedTaglistPluginClass	XedTaglistPluginClass;

struct _XedTaglistPluginClass
{
	XedPluginClass parent_class;
};

/*
 * Public methods
 */
GType	xed_taglist_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT GType register_xed_plugin (GTypeModule *module);

G_END_DECLS

#endif /* __XED_TAGLIST_PLUGIN_H__ */
