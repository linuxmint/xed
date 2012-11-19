/*
 * Copyright (C) 2009 - Ignacio Casal Quinteiro <icq@gnome.org>
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
 */

#ifndef __PLUMA_CHECK_UPDATE_PLUGIN_H__
#define __PLUMA_CHECK_UPDATE_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <pluma/pluma-plugin.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_CHECK_UPDATE_PLUGIN		(pluma_check_update_plugin_get_type ())
#define PLUMA_CHECK_UPDATE_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), PLUMA_TYPE_CHECK_UPDATE_PLUGIN, PlumaCheckUpdatePlugin))
#define PLUMA_CHECK_UPDATE_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), PLUMA_TYPE_CHECK_UPDATE_PLUGIN, PlumaCheckUpdatePluginClass))
#define IS_PLUMA_CHECK_UPDATE_PLUGIN(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), PLUMA_TYPE_CHECK_UPDATE_PLUGIN))
#define IS_PLUMA_CHECK_UPDATE_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), PLUMA_TYPE_CHECK_UPDATE_PLUGIN))
#define PLUMA_CHECK_UPDATE_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), PLUMA_TYPE_CHECK_UPDATE_PLUGIN, PlumaCheckUpdatePluginClass))

/* Private structure type */
typedef struct _PlumaCheckUpdatePluginPrivate	PlumaCheckUpdatePluginPrivate;

/*
 * Main object structure
 */
typedef struct _PlumaCheckUpdatePlugin		PlumaCheckUpdatePlugin;

struct _PlumaCheckUpdatePlugin
{
	PlumaPlugin parent_instance;

	/*< private >*/
	PlumaCheckUpdatePluginPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _PlumaCheckUpdatePluginClass	PlumaCheckUpdatePluginClass;

struct _PlumaCheckUpdatePluginClass
{
	PlumaPluginClass parent_class;
};

/*
 * Public methods
 */
GType	pluma_check_update_plugin_get_type	(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT GType register_pluma_plugin (GTypeModule *module);

G_END_DECLS

#endif /* __PLUMA_CHECK_UPDATE_PLUGIN_H__ */
