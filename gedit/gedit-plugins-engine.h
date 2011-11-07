/*
 * gedit-plugins-engine.h
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */
 
/*
 * Modified by the gedit Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __GEDIT_PLUGINS_ENGINE_H__
#define __GEDIT_PLUGINS_ENGINE_H__

#include <glib.h>
#include "gedit-window.h"
#include "gedit-plugin-info.h"
#include "gedit-plugin.h"

G_BEGIN_DECLS

#define GEDIT_TYPE_PLUGINS_ENGINE              (gedit_plugins_engine_get_type ())
#define GEDIT_PLUGINS_ENGINE(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GEDIT_TYPE_PLUGINS_ENGINE, GeditPluginsEngine))
#define GEDIT_PLUGINS_ENGINE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GEDIT_TYPE_PLUGINS_ENGINE, GeditPluginsEngineClass))
#define GEDIT_IS_PLUGINS_ENGINE(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GEDIT_TYPE_PLUGINS_ENGINE))
#define GEDIT_IS_PLUGINS_ENGINE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_PLUGINS_ENGINE))
#define GEDIT_PLUGINS_ENGINE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GEDIT_TYPE_PLUGINS_ENGINE, GeditPluginsEngineClass))

typedef struct _GeditPluginsEngine		GeditPluginsEngine;
typedef struct _GeditPluginsEnginePrivate	GeditPluginsEnginePrivate;

struct _GeditPluginsEngine
{
	GObject parent;
	GeditPluginsEnginePrivate *priv;
};

typedef struct _GeditPluginsEngineClass		GeditPluginsEngineClass;

struct _GeditPluginsEngineClass
{
	GObjectClass parent_class;

	void	 (* activate_plugin)		(GeditPluginsEngine *engine,
						 GeditPluginInfo    *info);

	void	 (* deactivate_plugin)		(GeditPluginsEngine *engine,
						 GeditPluginInfo    *info);
};

GType			 gedit_plugins_engine_get_type		(void) G_GNUC_CONST;

GeditPluginsEngine	*gedit_plugins_engine_get_default	(void);

void		 gedit_plugins_engine_garbage_collect	(GeditPluginsEngine *engine);

const GList	*gedit_plugins_engine_get_plugin_list 	(GeditPluginsEngine *engine);

GeditPluginInfo	*gedit_plugins_engine_get_plugin_info	(GeditPluginsEngine *engine,
							 const gchar        *name);

/* plugin load and unloading (overall, for all windows) */
gboolean 	 gedit_plugins_engine_activate_plugin 	(GeditPluginsEngine *engine,
							 GeditPluginInfo    *info);
gboolean 	 gedit_plugins_engine_deactivate_plugin	(GeditPluginsEngine *engine,
							 GeditPluginInfo    *info);

void	 	 gedit_plugins_engine_configure_plugin	(GeditPluginsEngine *engine,
							 GeditPluginInfo    *info,
							 GtkWindow          *parent);

/* plugin activation/deactivation per window, private to GeditWindow */
void 		 gedit_plugins_engine_activate_plugins   (GeditPluginsEngine *engine,
							  GeditWindow        *window);
void 		 gedit_plugins_engine_deactivate_plugins (GeditPluginsEngine *engine,
							  GeditWindow        *window);
void		 gedit_plugins_engine_update_plugins_ui  (GeditPluginsEngine *engine,
							  GeditWindow        *window);

/* private for mateconf notification */
void		 gedit_plugins_engine_active_plugins_changed
							(GeditPluginsEngine *engine);

void		 gedit_plugins_engine_rescan_plugins	(GeditPluginsEngine *engine);

G_END_DECLS

#endif  /* __GEDIT_PLUGINS_ENGINE_H__ */
