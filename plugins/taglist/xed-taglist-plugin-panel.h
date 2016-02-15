/*
 * xed-taglist-plugin-panel.h
 * This file is part of xed
 *
 * Copyright (C) 2005 - Paolo Maggi 
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
 * Modified by the xed Team, 2005. See the AUTHORS file for a 
 * list of people on the xed Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __XED_TAGLIST_PLUGIN_PANEL_H__
#define __XED_TAGLIST_PLUGIN_PANEL_H__

#include <gtk/gtk.h>

#include <xed/xed-window.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XED_TYPE_TAGLIST_PLUGIN_PANEL              (xed_taglist_plugin_panel_get_type())
#define XED_TAGLIST_PLUGIN_PANEL(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_TAGLIST_PLUGIN_PANEL, XedTaglistPluginPanel))
#define XED_TAGLIST_PLUGIN_PANEL_CONST(obj)        (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_TAGLIST_PLUGIN_PANEL, XedTaglistPluginPanel const))
#define XED_TAGLIST_PLUGIN_PANEL_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_TAGLIST_PLUGIN_PANEL, XedTaglistPluginPanelClass))
#define XED_IS_TAGLIST_PLUGIN_PANEL(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_TAGLIST_PLUGIN_PANEL))
#define XED_IS_TAGLIST_PLUGIN_PANEL_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_TAGLIST_PLUGIN_PANEL))
#define XED_TAGLIST_PLUGIN_PANEL_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_TAGLIST_PLUGIN_PANEL, XedTaglistPluginPanelClass))

/* Private structure type */
typedef struct _XedTaglistPluginPanelPrivate XedTaglistPluginPanelPrivate;

/*
 * Main object structure
 */
typedef struct _XedTaglistPluginPanel XedTaglistPluginPanel;

struct _XedTaglistPluginPanel 
{
	GtkBox vbox;

	/*< private > */
	XedTaglistPluginPanelPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XedTaglistPluginPanelClass XedTaglistPluginPanelClass;

struct _XedTaglistPluginPanelClass 
{
	GtkBoxClass parent_class;
};

/*
 * Public methods
 */
GType		 xed_taglist_plugin_panel_register_type	(GTypeModule *module);
							
GType 		 xed_taglist_plugin_panel_get_type		(void) G_GNUC_CONST;

GtkWidget	*xed_taglist_plugin_panel_new 		(XedWindow *window,
								 const gchar *data_dir);

G_END_DECLS

#endif  /* __XED_TAGLIST_PLUGIN_PANEL_H__  */
