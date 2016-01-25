/*
 * xedit-plugin-loader-c.h
 * This file is part of xedit
 *
 * Copyright (C) 2008 - Jesse van den Kieboom
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

#ifndef __XEDIT_PLUGIN_LOADER_C_H__
#define __XEDIT_PLUGIN_LOADER_C_H__

#include <xedit/xedit-plugin-loader.h>

G_BEGIN_DECLS

#define XEDIT_TYPE_PLUGIN_LOADER_C		(xedit_plugin_loader_c_get_type ())
#define XEDIT_PLUGIN_LOADER_C(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_PLUGIN_LOADER_C, XeditPluginLoaderC))
#define XEDIT_PLUGIN_LOADER_C_CONST(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_PLUGIN_LOADER_C, XeditPluginLoaderC const))
#define XEDIT_PLUGIN_LOADER_C_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_PLUGIN_LOADER_C, XeditPluginLoaderCClass))
#define XEDIT_IS_PLUGIN_LOADER_C(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XEDIT_TYPE_PLUGIN_LOADER_C))
#define XEDIT_IS_PLUGIN_LOADER_C_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_PLUGIN_LOADER_C))
#define XEDIT_PLUGIN_LOADER_C_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), XEDIT_TYPE_PLUGIN_LOADER_C, XeditPluginLoaderCClass))

typedef struct _XeditPluginLoaderC		XeditPluginLoaderC;
typedef struct _XeditPluginLoaderCClass		XeditPluginLoaderCClass;
typedef struct _XeditPluginLoaderCPrivate	XeditPluginLoaderCPrivate;

struct _XeditPluginLoaderC {
	GObject parent;
	
	XeditPluginLoaderCPrivate *priv;
};

struct _XeditPluginLoaderCClass {
	GObjectClass parent_class;
};

GType xedit_plugin_loader_c_get_type (void) G_GNUC_CONST;
XeditPluginLoaderC *xedit_plugin_loader_c_new(void);

/* All the loaders must implement this function */
G_MODULE_EXPORT GType register_xedit_plugin_loader (GTypeModule * module);

G_END_DECLS

#endif /* __XEDIT_PLUGIN_LOADER_C_H__ */
