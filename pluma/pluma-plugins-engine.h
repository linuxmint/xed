/*
 * pluma-plugins-engine.h
 * This file is part of pluma
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
 * Modified by the pluma Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __PLUMA_PLUGINS_ENGINE_H__
#define __PLUMA_PLUGINS_ENGINE_H__

#include <glib.h>
#include "pluma-window.h"
#include "pluma-plugin-info.h"
#include "pluma-plugin.h"

G_BEGIN_DECLS

#define PLUMA_TYPE_PLUGINS_ENGINE              (pluma_plugins_engine_get_type ())
#define PLUMA_PLUGINS_ENGINE(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_PLUGINS_ENGINE, PlumaPluginsEngine))
#define PLUMA_PLUGINS_ENGINE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), PLUMA_TYPE_PLUGINS_ENGINE, PlumaPluginsEngineClass))
#define PLUMA_IS_PLUGINS_ENGINE(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), PLUMA_TYPE_PLUGINS_ENGINE))
#define PLUMA_IS_PLUGINS_ENGINE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_PLUGINS_ENGINE))
#define PLUMA_PLUGINS_ENGINE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), PLUMA_TYPE_PLUGINS_ENGINE, PlumaPluginsEngineClass))

typedef struct _PlumaPluginsEngine		PlumaPluginsEngine;
typedef struct _PlumaPluginsEnginePrivate	PlumaPluginsEnginePrivate;

struct _PlumaPluginsEngine
{
	GObject parent;
	PlumaPluginsEnginePrivate *priv;
};

typedef struct _PlumaPluginsEngineClass		PlumaPluginsEngineClass;

struct _PlumaPluginsEngineClass
{
	GObjectClass parent_class;

	void	 (* activate_plugin)		(PlumaPluginsEngine *engine,
						 PlumaPluginInfo    *info);

	void	 (* deactivate_plugin)		(PlumaPluginsEngine *engine,
						 PlumaPluginInfo    *info);
};

GType			 pluma_plugins_engine_get_type		(void) G_GNUC_CONST;

PlumaPluginsEngine	*pluma_plugins_engine_get_default	(void);

void		 pluma_plugins_engine_garbage_collect	(PlumaPluginsEngine *engine);

const GList	*pluma_plugins_engine_get_plugin_list 	(PlumaPluginsEngine *engine);

PlumaPluginInfo	*pluma_plugins_engine_get_plugin_info	(PlumaPluginsEngine *engine,
							 const gchar        *name);

/* plugin load and unloading (overall, for all windows) */
gboolean 	 pluma_plugins_engine_activate_plugin 	(PlumaPluginsEngine *engine,
							 PlumaPluginInfo    *info);
gboolean 	 pluma_plugins_engine_deactivate_plugin	(PlumaPluginsEngine *engine,
							 PlumaPluginInfo    *info);

void	 	 pluma_plugins_engine_configure_plugin	(PlumaPluginsEngine *engine,
							 PlumaPluginInfo    *info,
							 GtkWindow          *parent);

/* plugin activation/deactivation per window, private to PlumaWindow */
void 		 pluma_plugins_engine_activate_plugins   (PlumaPluginsEngine *engine,
							  PlumaWindow        *window);
void 		 pluma_plugins_engine_deactivate_plugins (PlumaPluginsEngine *engine,
							  PlumaWindow        *window);
void		 pluma_plugins_engine_update_plugins_ui  (PlumaPluginsEngine *engine,
							  PlumaWindow        *window);

/* private for GSettings notification */
void		 pluma_plugins_engine_active_plugins_changed
							(PlumaPluginsEngine *engine);

void		 pluma_plugins_engine_rescan_plugins	(PlumaPluginsEngine *engine);

G_END_DECLS

#endif  /* __PLUMA_PLUGINS_ENGINE_H__ */
