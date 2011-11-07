/*
 * gedit-panel.h
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */
 
/*
 * Modified by the gedit Team, 2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __GEDIT_PANEL_H__
#define __GEDIT_PANEL_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GEDIT_TYPE_PANEL		(gedit_panel_get_type())
#define GEDIT_PANEL(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), GEDIT_TYPE_PANEL, GeditPanel))
#define GEDIT_PANEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), GEDIT_TYPE_PANEL, GeditPanelClass))
#define GEDIT_IS_PANEL(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), GEDIT_TYPE_PANEL))
#define GEDIT_IS_PANEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_PANEL))
#define GEDIT_PANEL_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GEDIT_TYPE_PANEL, GeditPanelClass))

/* Private structure type */
typedef struct _GeditPanelPrivate GeditPanelPrivate;

/*
 * Main object structure
 */
typedef struct _GeditPanel GeditPanel;

struct _GeditPanel 
{
	GtkVBox vbox;

	/*< private > */
	GeditPanelPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GeditPanelClass GeditPanelClass;

struct _GeditPanelClass 
{
	GtkVBoxClass parent_class;

	void (* item_added)     (GeditPanel     *panel,
				 GtkWidget      *item);
	void (* item_removed)   (GeditPanel     *panel,
				 GtkWidget      *item);

	/* Keybinding signals */
	void (* close)          (GeditPanel     *panel);
	void (* focus_document) (GeditPanel     *panel);

	/* Padding for future expansion */
	void (*_gedit_reserved1) (void);
	void (*_gedit_reserved2) (void);
	void (*_gedit_reserved3) (void);
	void (*_gedit_reserved4) (void);	
};

/*
 * Public methods
 */
GType 		 gedit_panel_get_type 			(void) G_GNUC_CONST;

GtkWidget 	*gedit_panel_new 			(GtkOrientation	 orientation);

void		 gedit_panel_add_item			(GeditPanel     *panel,
						      	 GtkWidget      *item,
						      	 const gchar    *name,
							 GtkWidget      *image);

void		 gedit_panel_add_item_with_stock_icon	(GeditPanel     *panel,
							 GtkWidget      *item,
						      	 const gchar    *name,
						      	 const gchar    *stock_id);

gboolean	 gedit_panel_remove_item	(GeditPanel     *panel,
					  	 GtkWidget      *item);

gboolean	 gedit_panel_activate_item 	(GeditPanel     *panel,
					    	 GtkWidget      *item);

gboolean	 gedit_panel_item_is_active 	(GeditPanel     *panel,
					    	 GtkWidget      *item);

GtkOrientation	 gedit_panel_get_orientation	(GeditPanel	*panel);

gint		 gedit_panel_get_n_items	(GeditPanel	*panel);


/*
 * Non exported functions
 */
gint		 _gedit_panel_get_active_item_id	(GeditPanel	*panel);

void		 _gedit_panel_set_active_item_by_id	(GeditPanel	*panel,
							 gint		 id);

G_END_DECLS

#endif  /* __GEDIT_PANEL_H__  */
