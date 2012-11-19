/*
 * pluma-taglist-plugin-panel.h
 * This file is part of pluma
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
 * Modified by the pluma Team, 2005. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __PLUMA_TAGLIST_PLUGIN_PANEL_H__
#define __PLUMA_TAGLIST_PLUGIN_PANEL_H__

#include <gtk/gtk.h>

#include <pluma/pluma-window.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_TAGLIST_PLUGIN_PANEL              (pluma_taglist_plugin_panel_get_type())
#define PLUMA_TAGLIST_PLUGIN_PANEL(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_TAGLIST_PLUGIN_PANEL, PlumaTaglistPluginPanel))
#define PLUMA_TAGLIST_PLUGIN_PANEL_CONST(obj)        (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_TAGLIST_PLUGIN_PANEL, PlumaTaglistPluginPanel const))
#define PLUMA_TAGLIST_PLUGIN_PANEL_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), PLUMA_TYPE_TAGLIST_PLUGIN_PANEL, PlumaTaglistPluginPanelClass))
#define PLUMA_IS_TAGLIST_PLUGIN_PANEL(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), PLUMA_TYPE_TAGLIST_PLUGIN_PANEL))
#define PLUMA_IS_TAGLIST_PLUGIN_PANEL_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_TAGLIST_PLUGIN_PANEL))
#define PLUMA_TAGLIST_PLUGIN_PANEL_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), PLUMA_TYPE_TAGLIST_PLUGIN_PANEL, PlumaTaglistPluginPanelClass))

/* Private structure type */
typedef struct _PlumaTaglistPluginPanelPrivate PlumaTaglistPluginPanelPrivate;

/*
 * Main object structure
 */
typedef struct _PlumaTaglistPluginPanel PlumaTaglistPluginPanel;

struct _PlumaTaglistPluginPanel 
{
	GtkVBox vbox;

	/*< private > */
	PlumaTaglistPluginPanelPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _PlumaTaglistPluginPanelClass PlumaTaglistPluginPanelClass;

struct _PlumaTaglistPluginPanelClass 
{
	GtkVBoxClass parent_class;
};

/*
 * Public methods
 */
GType		 pluma_taglist_plugin_panel_register_type	(GTypeModule *module);
							
GType 		 pluma_taglist_plugin_panel_get_type		(void) G_GNUC_CONST;

GtkWidget	*pluma_taglist_plugin_panel_new 		(PlumaWindow *window,
								 const gchar *data_dir);

G_END_DECLS

#endif  /* __PLUMA_TAGLIST_PLUGIN_PANEL_H__  */
