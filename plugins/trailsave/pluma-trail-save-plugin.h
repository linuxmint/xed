/*
 * pluma-trail-save-plugin.h
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

#ifndef __PLUMA_TRAIL_SAVE_PLUGIN_H__
#define __PLUMA_TRAIL_SAVE_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <pluma/pluma-plugin.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_TRAIL_SAVE_PLUGIN		(pluma_trail_save_plugin_get_type ())
#define PLUMA_TRAIL_SAVE_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), PLUMA_TYPE_TRAIL_SAVE_PLUGIN, PlumaTrailSavePlugin))
#define PLUMA_TRAIL_SAVE_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), PLUMA_TYPE_TRAIL_SAVE_PLUGIN, PlumaTrailSavePluginClass))
#define PLUMA_IS_TRAIL_SAVE_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), PLUMA_TYPE_TRAIL_SAVE_PLUGIN))
#define PLUMA_IS_TRAIL_SAVE_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), PLUMA_TYPE_TRAIL_SAVE_PLUGIN))
#define PLUMA_TRAIL_SAVE_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), PLUMA_TYPE_TRAIL_SAVE_PLUGIN, PlumaTrailSavePluginClass))

/*
 * Main object structure
 */
typedef struct _PlumaTrailSavePlugin		PlumaTrailSavePlugin;

struct _PlumaTrailSavePlugin
{
	PlumaPlugin parent_instance;
};

/*
 * Class definition
 */
typedef struct _PlumaTrailSavePluginClass	PlumaTrailSavePluginClass;

struct _PlumaTrailSavePluginClass
{
	PlumaPluginClass parent_class;
};

/*
 * Public methods
 */
GType	pluma_trail_save_plugin_get_type	(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT GType register_pluma_plugin (GTypeModule *module);

G_END_DECLS

#endif /* __PLUMA_TRAIL_SAVE_PLUGIN_H__ */
