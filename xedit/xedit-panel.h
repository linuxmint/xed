/*
 * xedit-panel.h
 * This file is part of xedit
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
 * Modified by the xedit Team, 2005. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __XEDIT_PANEL_H__
#define __XEDIT_PANEL_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XEDIT_TYPE_PANEL		(xedit_panel_get_type())
#define XEDIT_PANEL(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), XEDIT_TYPE_PANEL, XeditPanel))
#define XEDIT_PANEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), XEDIT_TYPE_PANEL, XeditPanelClass))
#define XEDIT_IS_PANEL(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), XEDIT_TYPE_PANEL))
#define XEDIT_IS_PANEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_PANEL))
#define XEDIT_PANEL_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XEDIT_TYPE_PANEL, XeditPanelClass))

/* Private structure type */
typedef struct _XeditPanelPrivate XeditPanelPrivate;

/*
 * Main object structure
 */
typedef struct _XeditPanel XeditPanel;

struct _XeditPanel 
{
#if GTK_CHECK_VERSION (3, 0, 0)
	GtkBox vbox;
#else
	GtkVBox vbox;
#endif

	/*< private > */
	XeditPanelPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XeditPanelClass XeditPanelClass;

struct _XeditPanelClass 
{
#if GTK_CHECK_VERSION (3, 0, 0)
	GtkBoxClass parent_class;
#else
	GtkVBoxClass parent_class;
#endif

	void (* item_added)     (XeditPanel     *panel,
				 GtkWidget      *item);
	void (* item_removed)   (XeditPanel     *panel,
				 GtkWidget      *item);

	/* Keybinding signals */
	void (* close)          (XeditPanel     *panel);
	void (* focus_document) (XeditPanel     *panel);

	/* Padding for future expansion */
	void (*_xedit_reserved1) (void);
	void (*_xedit_reserved2) (void);
	void (*_xedit_reserved3) (void);
	void (*_xedit_reserved4) (void);	
};

/*
 * Public methods
 */
GType 		 xedit_panel_get_type 			(void) G_GNUC_CONST;

GtkWidget 	*xedit_panel_new 			(GtkOrientation	 orientation);

void		 xedit_panel_add_item			(XeditPanel     *panel,
						      	 GtkWidget      *item,
						      	 const gchar    *name,
							 GtkWidget      *image);

void		 xedit_panel_add_item_with_stock_icon	(XeditPanel     *panel,
							 GtkWidget      *item,
						      	 const gchar    *name,
						      	 const gchar    *stock_id);

gboolean	 xedit_panel_remove_item	(XeditPanel     *panel,
					  	 GtkWidget      *item);

gboolean	 xedit_panel_activate_item 	(XeditPanel     *panel,
					    	 GtkWidget      *item);

gboolean	 xedit_panel_item_is_active 	(XeditPanel     *panel,
					    	 GtkWidget      *item);

GtkOrientation	 xedit_panel_get_orientation	(XeditPanel	*panel);

gint		 xedit_panel_get_n_items	(XeditPanel	*panel);


/*
 * Non exported functions
 */
gint		 _xedit_panel_get_active_item_id	(XeditPanel	*panel);

void		 _xedit_panel_set_active_item_by_id	(XeditPanel	*panel,
							 gint		 id);

G_END_DECLS

#endif  /* __XEDIT_PANEL_H__  */
