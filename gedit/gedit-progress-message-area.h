/*
 * gedit-progress-message-area.h
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

#ifndef __GEDIT_PROGRESS_MESSAGE_AREA_H__
#define __GEDIT_PROGRESS_MESSAGE_AREA_H__

#if !GTK_CHECK_VERSION (2, 17, 1)
#include <gedit/gedit-message-area.h>
#else
#include <gtk/gtk.h>
#endif

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GEDIT_TYPE_PROGRESS_MESSAGE_AREA              (gedit_progress_message_area_get_type())
#define GEDIT_PROGRESS_MESSAGE_AREA(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GEDIT_TYPE_PROGRESS_MESSAGE_AREA, GeditProgressMessageArea))
#define GEDIT_PROGRESS_MESSAGE_AREA_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GEDIT_TYPE_PROGRESS_MESSAGE_AREA, GeditProgressMessageAreaClass))
#define GEDIT_IS_PROGRESS_MESSAGE_AREA(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GEDIT_TYPE_PROGRESS_MESSAGE_AREA))
#define GEDIT_IS_PROGRESS_MESSAGE_AREA_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_PROGRESS_MESSAGE_AREA))
#define GEDIT_PROGRESS_MESSAGE_AREA_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GEDIT_TYPE_PROGRESS_MESSAGE_AREA, GeditProgressMessageAreaClass))

/* Private structure type */
typedef struct _GeditProgressMessageAreaPrivate GeditProgressMessageAreaPrivate;

/*
 * Main object structure
 */
typedef struct _GeditProgressMessageArea GeditProgressMessageArea;

struct _GeditProgressMessageArea 
{
#if !GTK_CHECK_VERSION (2, 17, 1)
	GeditMessageArea parent;
#else
	GtkInfoBar parent;
#endif

	/*< private > */
	GeditProgressMessageAreaPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GeditProgressMessageAreaClass GeditProgressMessageAreaClass;

struct _GeditProgressMessageAreaClass 
{
#if !GTK_CHECK_VERSION (2, 17, 1)
	GeditMessageAreaClass parent_class;
#else
	GtkInfoBarClass parent_class;
#endif
};

/*
 * Public methods
 */
GType 		 gedit_progress_message_area_get_type 		(void) G_GNUC_CONST;

GtkWidget	*gedit_progress_message_area_new      		(const gchar              *stock_id,
								 const gchar              *markup,
								 gboolean                  has_cancel);

void		 gedit_progress_message_area_set_stock_image	(GeditProgressMessageArea *area,
								 const gchar              *stock_id);

void		 gedit_progress_message_area_set_markup		(GeditProgressMessageArea *area,
								 const gchar              *markup);

void		 gedit_progress_message_area_set_text		(GeditProgressMessageArea *area,
								 const gchar              *text);

void		 gedit_progress_message_area_set_fraction	(GeditProgressMessageArea *area,
								 gdouble                   fraction);

void		 gedit_progress_message_area_pulse		(GeditProgressMessageArea *area);
								 

G_END_DECLS

#endif  /* __GEDIT_PROGRESS_MESSAGE_AREA_H__  */
