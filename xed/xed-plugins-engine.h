/*
 * xed-plugins-engine.h
 * This file is part of xed
 *
 * Copyright (C) 2002-2005 - Paolo Maggi 
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
 * Modified by the xed Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the xed Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __XED_PLUGINS_ENGINE_H__
#define __XED_PLUGINS_ENGINE_H__

#include <glib.h>
#include "xed-window.h"
#include "xed-plugin-info.h"
#include "xed-plugin.h"

G_BEGIN_DECLS

#define XED_TYPE_PLUGINS_ENGINE              (xed_plugins_engine_get_type ())
#define XED_PLUGINS_ENGINE(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_PLUGINS_ENGINE, XedPluginsEngine))
#define XED_PLUGINS_ENGINE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_PLUGINS_ENGINE, XedPluginsEngineClass))
#define XED_IS_PLUGINS_ENGINE(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_PLUGINS_ENGINE))
#define XED_IS_PLUGINS_ENGINE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_PLUGINS_ENGINE))
#define XED_PLUGINS_ENGINE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_PLUGINS_ENGINE, XedPluginsEngineClass))

typedef struct _XedPluginsEngine		XedPluginsEngine;
typedef struct _XedPluginsEnginePrivate	XedPluginsEnginePrivate;

struct _XedPluginsEngine
{
	GObject parent;
	XedPluginsEnginePrivate *priv;
};

typedef struct _XedPluginsEngineClass		XedPluginsEngineClass;

struct _XedPluginsEngineClass
{
	GObjectClass parent_class;

	void	 (* activate_plugin)		(XedPluginsEngine *engine,
						 XedPluginInfo    *info);

	void	 (* deactivate_plugin)		(XedPluginsEngine *engine,
						 XedPluginInfo    *info);
};

GType			 xed_plugins_engine_get_type		(void) G_GNUC_CONST;

XedPluginsEngine	*xed_plugins_engine_get_default	(void);

void		 xed_plugins_engine_garbage_collect	(XedPluginsEngine *engine);

const GList	*xed_plugins_engine_get_plugin_list 	(XedPluginsEngine *engine);

XedPluginInfo	*xed_plugins_engine_get_plugin_info	(XedPluginsEngine *engine,
							 const gchar        *name);

/* plugin load and unloading (overall, for all windows) */
gboolean 	 xed_plugins_engine_activate_plugin 	(XedPluginsEngine *engine,
							 XedPluginInfo    *info);
gboolean 	 xed_plugins_engine_deactivate_plugin	(XedPluginsEngine *engine,
							 XedPluginInfo    *info);

void	 	 xed_plugins_engine_configure_plugin	(XedPluginsEngine *engine,
							 XedPluginInfo    *info,
							 GtkWindow          *parent);

/* plugin activation/deactivation per window, private to XedWindow */
void 		 xed_plugins_engine_activate_plugins   (XedPluginsEngine *engine,
							  XedWindow        *window);
void 		 xed_plugins_engine_deactivate_plugins (XedPluginsEngine *engine,
							  XedWindow        *window);
void		 xed_plugins_engine_update_plugins_ui  (XedPluginsEngine *engine,
							  XedWindow        *window);

/* private for GSettings notification */
void		 xed_plugins_engine_active_plugins_changed
							(XedPluginsEngine *engine);

void		 xed_plugins_engine_rescan_plugins	(XedPluginsEngine *engine);

G_END_DECLS

#endif  /* __XED_PLUGINS_ENGINE_H__ */
