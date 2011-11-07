/*
 * gedit-message-area.h
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

#ifndef __GEDIT_MESSAGE_AREA_H__
#define __GEDIT_MESSAGE_AREA_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GEDIT_TYPE_MESSAGE_AREA              (gedit_message_area_get_type())
#define GEDIT_MESSAGE_AREA(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GEDIT_TYPE_MESSAGE_AREA, GeditMessageArea))
#define GEDIT_MESSAGE_AREA_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GEDIT_TYPE_MESSAGE_AREA, GeditMessageAreaClass))
#define GEDIT_IS_MESSAGE_AREA(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GEDIT_TYPE_MESSAGE_AREA))
#define GEDIT_IS_MESSAGE_AREA_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_MESSAGE_AREA))
#define GEDIT_MESSAGE_AREA_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GEDIT_TYPE_MESSAGE_AREA, GeditMessageAreaClass))

/* Private structure type */
typedef struct _GeditMessageAreaPrivate GeditMessageAreaPrivate;

/*
 * Main object structure
 */
typedef struct _GeditMessageArea GeditMessageArea;

struct _GeditMessageArea 
{
	GtkHBox parent;

	/*< private > */
	GeditMessageAreaPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GeditMessageAreaClass GeditMessageAreaClass;

struct _GeditMessageAreaClass 
{
	GtkHBoxClass parent_class;

	/* Signals */
	void (* response) (GeditMessageArea *message_area, gint response_id);

	/* Keybinding signals */
	void (* close)    (GeditMessageArea *message_area);

	/* Padding for future expansion */
	void (*_gedit_reserved1) (void);
	void (*_gedit_reserved2) (void);	
};

/*
 * Public methods
 */
GType 		 gedit_message_area_get_type 		(void) G_GNUC_CONST;

GtkWidget	*gedit_message_area_new      		(void);

GtkWidget	*gedit_message_area_new_with_buttons	(const gchar      *first_button_text,
                                        		 ...);

void		 gedit_message_area_set_contents	(GeditMessageArea *message_area,
                                             		 GtkWidget        *contents);
                              		 
void		 gedit_message_area_add_action_widget	(GeditMessageArea *message_area,
                                         		 GtkWidget        *child,
                                         		 gint              response_id);
                                         		 
GtkWidget	*gedit_message_area_add_button        	(GeditMessageArea *message_area,
                                         		 const gchar      *button_text,
                                         		 gint              response_id);
             		 
GtkWidget	*gedit_message_area_add_stock_button_with_text 
							(GeditMessageArea *message_area, 
				    			 const gchar      *text, 
				    			 const gchar      *stock_id, 
				    			 gint              response_id);

void       	 gedit_message_area_add_buttons 	(GeditMessageArea *message_area,
                                         		 const gchar      *first_button_text,
                                         		 ...);

void		 gedit_message_area_set_response_sensitive 
							(GeditMessageArea *message_area,
                                        		 gint              response_id,
                                        		 gboolean          setting);
void 		 gedit_message_area_set_default_response 
							(GeditMessageArea *message_area,
                                        		 gint              response_id);

/* Emit response signal */
void		 gedit_message_area_response           	(GeditMessageArea *message_area,
                                    			 gint              response_id);

G_END_DECLS

#endif  /* __GEDIT_MESSAGE_AREA_H__  */
