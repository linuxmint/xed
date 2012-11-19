/*
 * pluma-modeline-plugin.h
 * Emacs, Kate and Vim-style modelines support for pluma.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __PLUMA_MODELINE_PLUGIN_H__
#define __PLUMA_MODELINE_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <pluma/pluma-plugin.h>

G_BEGIN_DECLS

#define PLUMA_TYPE_MODELINE_PLUGIN		(pluma_modeline_plugin_get_type ())
#define PLUMA_MODELINE_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), PLUMA_TYPE_MODELINE_PLUGIN, PlumaModelinePlugin))
#define PLUMA_MODELINE_PLUGIN_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), PLUMA_TYPE_MODELINE_PLUGIN, PlumaModelinePluginClass))
#define PLUMA_IS_MODELINE_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), PLUMA_TYPE_MODELINE_PLUGIN))
#define PLUMA_IS_MODELINE_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), PLUMA_TYPE_MODELINE_PLUGIN))
#define PLUMA_MODELINE_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), PLUMA_TYPE_MODELINE_PLUGIN, PlumaModelinePluginClass))

/* Private structure type */
typedef PlumaPluginClass	PlumaModelinePluginClass;
typedef PlumaPlugin		PlumaModelinePlugin;

GType	pluma_modeline_plugin_get_type		(void) G_GNUC_CONST;

G_MODULE_EXPORT GType register_pluma_plugin (GTypeModule *module);

G_END_DECLS

#endif /* __PLUMA_MODELINE_PLUGIN_H__ */
