/*
 * xed-plugin-manager.h
 * This file is part of xed
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
 * Modified by the xed Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the xed Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __XED_PLUGIN_MANAGER_H__
#define __XED_PLUGIN_MANAGER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XED_TYPE_PLUGIN_MANAGER              (xed_plugin_manager_get_type())
#define XED_PLUGIN_MANAGER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_PLUGIN_MANAGER, XedPluginManager))
#define XED_PLUGIN_MANAGER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_PLUGIN_MANAGER, XedPluginManagerClass))
#define XED_IS_PLUGIN_MANAGER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_PLUGIN_MANAGER))
#define XED_IS_PLUGIN_MANAGER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_PLUGIN_MANAGER))
#define XED_PLUGIN_MANAGER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_PLUGIN_MANAGER, XedPluginManagerClass))

/* Private structure type */
typedef struct _XedPluginManagerPrivate XedPluginManagerPrivate;

/*
 * Main object structure
 */
typedef struct _XedPluginManager XedPluginManager;

struct _XedPluginManager 
{
#if GTK_CHECK_VERSION (3, 0, 0)
	GtkBox vbox;
#else
	GtkVBox vbox;
#endif

	/*< private > */
	XedPluginManagerPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XedPluginManagerClass XedPluginManagerClass;

struct _XedPluginManagerClass 
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
GType		 xed_plugin_manager_get_type		(void) G_GNUC_CONST;

GtkWidget	*xed_plugin_manager_new		(void);
   
G_END_DECLS

#endif  /* __XED_PLUGIN_MANAGER_H__  */
