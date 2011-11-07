/*
 * gedit-modeline-plugin.h
 * Emacs, Kate and Vim-style modelines support for gedit.
 * 
 * Copyright (C) 2005-2007 - Steve Frécinaux <code@istique.net>
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

#ifndef __GEDIT_MODELINE_PLUGIN_H__
#define __GEDIT_MODELINE_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <gedit/gedit-plugin.h>

G_BEGIN_DECLS

#define GEDIT_TYPE_MODELINE_PLUGIN		(gedit_modeline_plugin_get_type ())
#define GEDIT_MODELINE_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), GEDIT_TYPE_MODELINE_PLUGIN, GeditModelinePlugin))
#define GEDIT_MODELINE_PLUGIN_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), GEDIT_TYPE_MODELINE_PLUGIN, GeditModelinePluginClass))
#define GEDIT_IS_MODELINE_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GEDIT_TYPE_MODELINE_PLUGIN))
#define GEDIT_IS_MODELINE_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), GEDIT_TYPE_MODELINE_PLUGIN))
#define GEDIT_MODELINE_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), GEDIT_TYPE_MODELINE_PLUGIN, GeditModelinePluginClass))

/* Private structure type */
typedef GeditPluginClass	GeditModelinePluginClass;
typedef GeditPlugin		GeditModelinePlugin;

GType	gedit_modeline_plugin_get_type		(void) G_GNUC_CONST;

G_MODULE_EXPORT GType register_gedit_plugin (GTypeModule *module);

G_END_DECLS

#endif /* __GEDIT_MODELINE_PLUGIN_H__ */
