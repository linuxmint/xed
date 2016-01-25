/*
 * xedit-plugins-engine.h
 * This file is part of xedit
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
 * Modified by the xedit Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __XEDIT_PLUGINS_ENGINE_H__
#define __XEDIT_PLUGINS_ENGINE_H__

#include <glib.h>
#include "xedit-window.h"
#include "xedit-plugin-info.h"
#include "xedit-plugin.h"

G_BEGIN_DECLS

#define XEDIT_TYPE_PLUGINS_ENGINE              (xedit_plugins_engine_get_type ())
#define XEDIT_PLUGINS_ENGINE(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XEDIT_TYPE_PLUGINS_ENGINE, XeditPluginsEngine))
#define XEDIT_PLUGINS_ENGINE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XEDIT_TYPE_PLUGINS_ENGINE, XeditPluginsEngineClass))
#define XEDIT_IS_PLUGINS_ENGINE(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XEDIT_TYPE_PLUGINS_ENGINE))
#define XEDIT_IS_PLUGINS_ENGINE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_PLUGINS_ENGINE))
#define XEDIT_PLUGINS_ENGINE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XEDIT_TYPE_PLUGINS_ENGINE, XeditPluginsEngineClass))

typedef struct _XeditPluginsEngine		XeditPluginsEngine;
typedef struct _XeditPluginsEnginePrivate	XeditPluginsEnginePrivate;

struct _XeditPluginsEngine
{
	GObject parent;
	XeditPluginsEnginePrivate *priv;
};

typedef struct _XeditPluginsEngineClass		XeditPluginsEngineClass;

struct _XeditPluginsEngineClass
{
	GObjectClass parent_class;

	void	 (* activate_plugin)		(XeditPluginsEngine *engine,
						 XeditPluginInfo    *info);

	void	 (* deactivate_plugin)		(XeditPluginsEngine *engine,
						 XeditPluginInfo    *info);
};

GType			 xedit_plugins_engine_get_type		(void) G_GNUC_CONST;

XeditPluginsEngine	*xedit_plugins_engine_get_default	(void);

void		 xedit_plugins_engine_garbage_collect	(XeditPluginsEngine *engine);

const GList	*xedit_plugins_engine_get_plugin_list 	(XeditPluginsEngine *engine);

XeditPluginInfo	*xedit_plugins_engine_get_plugin_info	(XeditPluginsEngine *engine,
							 const gchar        *name);

/* plugin load and unloading (overall, for all windows) */
gboolean 	 xedit_plugins_engine_activate_plugin 	(XeditPluginsEngine *engine,
							 XeditPluginInfo    *info);
gboolean 	 xedit_plugins_engine_deactivate_plugin	(XeditPluginsEngine *engine,
							 XeditPluginInfo    *info);

void	 	 xedit_plugins_engine_configure_plugin	(XeditPluginsEngine *engine,
							 XeditPluginInfo    *info,
							 GtkWindow          *parent);

/* plugin activation/deactivation per window, private to XeditWindow */
void 		 xedit_plugins_engine_activate_plugins   (XeditPluginsEngine *engine,
							  XeditWindow        *window);
void 		 xedit_plugins_engine_deactivate_plugins (XeditPluginsEngine *engine,
							  XeditWindow        *window);
void		 xedit_plugins_engine_update_plugins_ui  (XeditPluginsEngine *engine,
							  XeditWindow        *window);

/* private for GSettings notification */
void		 xedit_plugins_engine_active_plugins_changed
							(XeditPluginsEngine *engine);

void		 xedit_plugins_engine_rescan_plugins	(XeditPluginsEngine *engine);

G_END_DECLS

#endif  /* __XEDIT_PLUGINS_ENGINE_H__ */
