/*
 * xedit-taglist-plugin.h
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
 * Modified by the xedit Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */
 
#ifndef __XEDIT_TAGLIST_PLUGIN_H__
#define __XEDIT_TAGLIST_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <xedit/xedit-plugin.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XEDIT_TYPE_TAGLIST_PLUGIN		(xedit_taglist_plugin_get_type ())
#define XEDIT_TAGLIST_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), XEDIT_TYPE_TAGLIST_PLUGIN, XeditTaglistPlugin))
#define XEDIT_TAGLIST_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), XEDIT_TYPE_TAGLIST_PLUGIN, XeditTaglistPluginClass))
#define XEDIT_IS_TAGLIST_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), XEDIT_TYPE_TAGLIST_PLUGIN))
#define XEDIT_IS_TAGLIST_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), XEDIT_TYPE_TAGLIST_PLUGIN))
#define XEDIT_TAGLIST_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), XEDIT_TYPE_TAGLIST_PLUGIN, XeditTaglistPluginClass))

/* Private structure type */
typedef struct _XeditTaglistPluginPrivate	XeditTaglistPluginPrivate;

/*
 * Main object structure
 */
typedef struct _XeditTaglistPlugin		XeditTaglistPlugin;

struct _XeditTaglistPlugin
{
	XeditPlugin parent_instance;

	/*< private >*/
	XeditTaglistPluginPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XeditTaglistPluginClass	XeditTaglistPluginClass;

struct _XeditTaglistPluginClass
{
	XeditPluginClass parent_class;
};

/*
 * Public methods
 */
GType	xedit_taglist_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT GType register_xedit_plugin (GTypeModule *module);

G_END_DECLS

#endif /* __XEDIT_TAGLIST_PLUGIN_H__ */
