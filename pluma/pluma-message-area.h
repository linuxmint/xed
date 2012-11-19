/*
 * pluma-message-area.h
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

#ifndef __PLUMA_MESSAGE_AREA_H__
#define __PLUMA_MESSAGE_AREA_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_MESSAGE_AREA              (pluma_message_area_get_type())
#define PLUMA_MESSAGE_AREA(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_MESSAGE_AREA, PlumaMessageArea))
#define PLUMA_MESSAGE_AREA_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), PLUMA_TYPE_MESSAGE_AREA, PlumaMessageAreaClass))
#define PLUMA_IS_MESSAGE_AREA(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), PLUMA_TYPE_MESSAGE_AREA))
#define PLUMA_IS_MESSAGE_AREA_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_MESSAGE_AREA))
#define PLUMA_MESSAGE_AREA_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), PLUMA_TYPE_MESSAGE_AREA, PlumaMessageAreaClass))

/* Private structure type */
typedef struct _PlumaMessageAreaPrivate PlumaMessageAreaPrivate;

/*
 * Main object structure
 */
typedef struct _PlumaMessageArea PlumaMessageArea;

struct _PlumaMessageArea 
{
	GtkHBox parent;

	/*< private > */
	PlumaMessageAreaPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _PlumaMessageAreaClass PlumaMessageAreaClass;

struct _PlumaMessageAreaClass 
{
	GtkHBoxClass parent_class;

	/* Signals */
	void (* response) (PlumaMessageArea *message_area, gint response_id);

	/* Keybinding signals */
	void (* close)    (PlumaMessageArea *message_area);

	/* Padding for future expansion */
	void (*_pluma_reserved1) (void);
	void (*_pluma_reserved2) (void);	
};

/*
 * Public methods
 */
GType 		 pluma_message_area_get_type 		(void) G_GNUC_CONST;

GtkWidget	*pluma_message_area_new      		(void);

GtkWidget	*pluma_message_area_new_with_buttons	(const gchar      *first_button_text,
                                        		 ...);

void		 pluma_message_area_set_contents	(PlumaMessageArea *message_area,
                                             		 GtkWidget        *contents);
                              		 
void		 pluma_message_area_add_action_widget	(PlumaMessageArea *message_area,
                                         		 GtkWidget        *child,
                                         		 gint              response_id);
                                         		 
GtkWidget	*pluma_message_area_add_button        	(PlumaMessageArea *message_area,
                                         		 const gchar      *button_text,
                                         		 gint              response_id);
             		 
GtkWidget	*pluma_message_area_add_stock_button_with_text 
							(PlumaMessageArea *message_area, 
				    			 const gchar      *text, 
				    			 const gchar      *stock_id, 
				    			 gint              response_id);

void       	 pluma_message_area_add_buttons 	(PlumaMessageArea *message_area,
                                         		 const gchar      *first_button_text,
                                         		 ...);

void		 pluma_message_area_set_response_sensitive 
							(PlumaMessageArea *message_area,
                                        		 gint              response_id,
                                        		 gboolean          setting);
void 		 pluma_message_area_set_default_response 
							(PlumaMessageArea *message_area,
                                        		 gint              response_id);

/* Emit response signal */
void		 pluma_message_area_response           	(PlumaMessageArea *message_area,
                                    			 gint              response_id);

G_END_DECLS

#endif  /* __PLUMA_MESSAGE_AREA_H__  */
