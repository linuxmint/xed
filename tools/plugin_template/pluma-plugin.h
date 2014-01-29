/*
 * ##(FILENAME) - ##(DESCRIPTION)
 *
 * Copyright (C) ##(DATE_YEAR) - ##(AUTHOR_FULLNAME)
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
 */

#ifndef __##(PLUGIN_ID.upper)_PLUGIN_H__
#define __##(PLUGIN_ID.upper)_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <pluma/pluma-plugin.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define TYPE_##(PLUGIN_ID.upper)_PLUGIN		(##(PLUGIN_ID.lower)_plugin_get_type ())
#define ##(PLUGIN_ID.upper)_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_##(PLUGIN_ID.upper)_PLUGIN, ##(PLUGIN_ID.camel)Plugin))
#define ##(PLUGIN_ID.upper)_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), TYPE_##(PLUGIN_ID.upper)_PLUGIN, ##(PLUGIN_ID.camel)PluginClass))
#define IS_##(PLUGIN_ID.upper)_PLUGIN(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_##(PLUGIN_ID.upper)_PLUGIN))
#define IS_##(PLUGIN_ID.upper)_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_##(PLUGIN_ID.upper)_PLUGIN))
#define ##(PLUGIN_ID.upper)_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_##(PLUGIN_ID.upper)_PLUGIN, ##(PLUGIN_ID.camel)PluginClass))

/* Private structure type */
typedef struct _##(PLUGIN_ID.camel)PluginPrivate	##(PLUGIN_ID.camel)PluginPrivate;

/*
 * Main object structure
 */
typedef struct _##(PLUGIN_ID.camel)Plugin		##(PLUGIN_ID.camel)Plugin;

struct _##(PLUGIN_ID.camel)Plugin
{
	PlumaPlugin parent_instance;

	/*< private >*/
	##(PLUGIN_ID.camel)PluginPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _##(PLUGIN_ID.camel)PluginClass	##(PLUGIN_ID.camel)PluginClass;

struct _##(PLUGIN_ID.camel)PluginClass
{
	PlumaPluginClass parent_class;
};

/*
 * Public methods
 */
GType	##(PLUGIN_ID.lower)_plugin_get_type	(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT GType register_pluma_plugin (GTypeModule *module);

G_END_DECLS

#endif /* __##(PLUGIN_ID.upper)_PLUGIN_H__ */
