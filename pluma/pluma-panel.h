/*
 * pluma-panel.h
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

#ifndef __PLUMA_PANEL_H__
#define __PLUMA_PANEL_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_PANEL		(pluma_panel_get_type())
#define PLUMA_PANEL(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_PANEL, PlumaPanel))
#define PLUMA_PANEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), PLUMA_TYPE_PANEL, PlumaPanelClass))
#define PLUMA_IS_PANEL(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), PLUMA_TYPE_PANEL))
#define PLUMA_IS_PANEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_PANEL))
#define PLUMA_PANEL_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), PLUMA_TYPE_PANEL, PlumaPanelClass))

/* Private structure type */
typedef struct _PlumaPanelPrivate PlumaPanelPrivate;

/*
 * Main object structure
 */
typedef struct _PlumaPanel PlumaPanel;

struct _PlumaPanel 
{
	GtkVBox vbox;

	/*< private > */
	PlumaPanelPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _PlumaPanelClass PlumaPanelClass;

struct _PlumaPanelClass 
{
	GtkVBoxClass parent_class;

	void (* item_added)     (PlumaPanel     *panel,
				 GtkWidget      *item);
	void (* item_removed)   (PlumaPanel     *panel,
				 GtkWidget      *item);

	/* Keybinding signals */
	void (* close)          (PlumaPanel     *panel);
	void (* focus_document) (PlumaPanel     *panel);

	/* Padding for future expansion */
	void (*_pluma_reserved1) (void);
	void (*_pluma_reserved2) (void);
	void (*_pluma_reserved3) (void);
	void (*_pluma_reserved4) (void);	
};

/*
 * Public methods
 */
GType 		 pluma_panel_get_type 			(void) G_GNUC_CONST;

GtkWidget 	*pluma_panel_new 			(GtkOrientation	 orientation);

void		 pluma_panel_add_item			(PlumaPanel     *panel,
						      	 GtkWidget      *item,
						      	 const gchar    *name,
							 GtkWidget      *image);

void		 pluma_panel_add_item_with_stock_icon	(PlumaPanel     *panel,
							 GtkWidget      *item,
						      	 const gchar    *name,
						      	 const gchar    *stock_id);

gboolean	 pluma_panel_remove_item	(PlumaPanel     *panel,
					  	 GtkWidget      *item);

gboolean	 pluma_panel_activate_item 	(PlumaPanel     *panel,
					    	 GtkWidget      *item);

gboolean	 pluma_panel_item_is_active 	(PlumaPanel     *panel,
					    	 GtkWidget      *item);

GtkOrientation	 pluma_panel_get_orientation	(PlumaPanel	*panel);

gint		 pluma_panel_get_n_items	(PlumaPanel	*panel);


/*
 * Non exported functions
 */
gint		 _pluma_panel_get_active_item_id	(PlumaPanel	*panel);

void		 _pluma_panel_set_active_item_by_id	(PlumaPanel	*panel,
							 gint		 id);

G_END_DECLS

#endif  /* __PLUMA_PANEL_H__  */
