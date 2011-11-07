/*
 * gedit-taglist-plugin.h
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

/*
 * Modified by the gedit Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */
 
#ifndef __GEDIT_TAGLIST_PLUGIN_H__
#define __GEDIT_TAGLIST_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <gedit/gedit-plugin.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GEDIT_TYPE_TAGLIST_PLUGIN		(gedit_taglist_plugin_get_type ())
#define GEDIT_TAGLIST_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), GEDIT_TYPE_TAGLIST_PLUGIN, GeditTaglistPlugin))
#define GEDIT_TAGLIST_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), GEDIT_TYPE_TAGLIST_PLUGIN, GeditTaglistPluginClass))
#define GEDIT_IS_TAGLIST_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GEDIT_TYPE_TAGLIST_PLUGIN))
#define GEDIT_IS_TAGLIST_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), GEDIT_TYPE_TAGLIST_PLUGIN))
#define GEDIT_TAGLIST_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), GEDIT_TYPE_TAGLIST_PLUGIN, GeditTaglistPluginClass))

/* Private structure type */
typedef struct _GeditTaglistPluginPrivate	GeditTaglistPluginPrivate;

/*
 * Main object structure
 */
typedef struct _GeditTaglistPlugin		GeditTaglistPlugin;

struct _GeditTaglistPlugin
{
	GeditPlugin parent_instance;

	/*< private >*/
	GeditTaglistPluginPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GeditTaglistPluginClass	GeditTaglistPluginClass;

struct _GeditTaglistPluginClass
{
	GeditPluginClass parent_class;
};

/*
 * Public methods
 */
GType	gedit_taglist_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT GType register_gedit_plugin (GTypeModule *module);

G_END_DECLS

#endif /* __GEDIT_TAGLIST_PLUGIN_H__ */
