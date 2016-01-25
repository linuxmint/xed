/*
 * xedit-trail-save-plugin.h
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

#ifndef __XEDIT_TRAIL_SAVE_PLUGIN_H__
#define __XEDIT_TRAIL_SAVE_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <xedit/xedit-plugin.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XEDIT_TYPE_TRAIL_SAVE_PLUGIN		(xedit_trail_save_plugin_get_type ())
#define XEDIT_TRAIL_SAVE_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), XEDIT_TYPE_TRAIL_SAVE_PLUGIN, XeditTrailSavePlugin))
#define XEDIT_TRAIL_SAVE_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), XEDIT_TYPE_TRAIL_SAVE_PLUGIN, XeditTrailSavePluginClass))
#define XEDIT_IS_TRAIL_SAVE_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), XEDIT_TYPE_TRAIL_SAVE_PLUGIN))
#define XEDIT_IS_TRAIL_SAVE_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), XEDIT_TYPE_TRAIL_SAVE_PLUGIN))
#define XEDIT_TRAIL_SAVE_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), XEDIT_TYPE_TRAIL_SAVE_PLUGIN, XeditTrailSavePluginClass))

/*
 * Main object structure
 */
typedef struct _XeditTrailSavePlugin		XeditTrailSavePlugin;

struct _XeditTrailSavePlugin
{
	XeditPlugin parent_instance;
};

/*
 * Class definition
 */
typedef struct _XeditTrailSavePluginClass	XeditTrailSavePluginClass;

struct _XeditTrailSavePluginClass
{
	XeditPluginClass parent_class;
};

/*
 * Public methods
 */
GType	xedit_trail_save_plugin_get_type	(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT GType register_xedit_plugin (GTypeModule *module);

G_END_DECLS

#endif /* __XEDIT_TRAIL_SAVE_PLUGIN_H__ */
