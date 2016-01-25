/*
 * xedit-plugin-manager.h
 * This file is part of xedit
 *
 * Copyright (C) 2002-2005 Paolo Maggi
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

#ifndef __XEDIT_PLUGIN_MANAGER_H__
#define __XEDIT_PLUGIN_MANAGER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XEDIT_TYPE_PLUGIN_MANAGER              (xedit_plugin_manager_get_type())
#define XEDIT_PLUGIN_MANAGER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XEDIT_TYPE_PLUGIN_MANAGER, XeditPluginManager))
#define XEDIT_PLUGIN_MANAGER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XEDIT_TYPE_PLUGIN_MANAGER, XeditPluginManagerClass))
#define XEDIT_IS_PLUGIN_MANAGER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XEDIT_TYPE_PLUGIN_MANAGER))
#define XEDIT_IS_PLUGIN_MANAGER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_PLUGIN_MANAGER))
#define XEDIT_PLUGIN_MANAGER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XEDIT_TYPE_PLUGIN_MANAGER, XeditPluginManagerClass))

/* Private structure type */
typedef struct _XeditPluginManagerPrivate XeditPluginManagerPrivate;

/*
 * Main object structure
 */
typedef struct _XeditPluginManager XeditPluginManager;

struct _XeditPluginManager 
{
#if GTK_CHECK_VERSION (3, 0, 0)
	GtkBox vbox;
#else
	GtkVBox vbox;
#endif

	/*< private > */
	XeditPluginManagerPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XeditPluginManagerClass XeditPluginManagerClass;

struct _XeditPluginManagerClass 
{
#if GTK_CHECK_VERSION (3, 0, 0)
	GtkBoxClass parent_class;
#else
	GtkVBoxClass parent_class;
#endif
};

/*
 * Public methods
 */
GType		 xedit_plugin_manager_get_type		(void) G_GNUC_CONST;

GtkWidget	*xedit_plugin_manager_new		(void);
   
G_END_DECLS

#endif  /* __XEDIT_PLUGIN_MANAGER_H__  */
