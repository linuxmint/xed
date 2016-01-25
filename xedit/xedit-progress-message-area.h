/*
 * xedit-progress-message-area.h
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

#ifndef __XEDIT_PROGRESS_MESSAGE_AREA_H__
#define __XEDIT_PROGRESS_MESSAGE_AREA_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XEDIT_TYPE_PROGRESS_MESSAGE_AREA              (xedit_progress_message_area_get_type())
#define XEDIT_PROGRESS_MESSAGE_AREA(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XEDIT_TYPE_PROGRESS_MESSAGE_AREA, XeditProgressMessageArea))
#define XEDIT_PROGRESS_MESSAGE_AREA_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XEDIT_TYPE_PROGRESS_MESSAGE_AREA, XeditProgressMessageAreaClass))
#define XEDIT_IS_PROGRESS_MESSAGE_AREA(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XEDIT_TYPE_PROGRESS_MESSAGE_AREA))
#define XEDIT_IS_PROGRESS_MESSAGE_AREA_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_PROGRESS_MESSAGE_AREA))
#define XEDIT_PROGRESS_MESSAGE_AREA_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XEDIT_TYPE_PROGRESS_MESSAGE_AREA, XeditProgressMessageAreaClass))

/* Private structure type */
typedef struct _XeditProgressMessageAreaPrivate XeditProgressMessageAreaPrivate;

/*
 * Main object structure
 */
typedef struct _XeditProgressMessageArea XeditProgressMessageArea;

struct _XeditProgressMessageArea 
{
	GtkInfoBar parent;

	/*< private > */
	XeditProgressMessageAreaPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XeditProgressMessageAreaClass XeditProgressMessageAreaClass;

struct _XeditProgressMessageAreaClass 
{
	GtkInfoBarClass parent_class;
};

/*
 * Public methods
 */
GType 		 xedit_progress_message_area_get_type 		(void) G_GNUC_CONST;

GtkWidget	*xedit_progress_message_area_new      		(const gchar              *stock_id,
								 const gchar              *markup,
								 gboolean                  has_cancel);

void		 xedit_progress_message_area_set_stock_image	(XeditProgressMessageArea *area,
								 const gchar              *stock_id);

void		 xedit_progress_message_area_set_markup		(XeditProgressMessageArea *area,
								 const gchar              *markup);

void		 xedit_progress_message_area_set_text		(XeditProgressMessageArea *area,
								 const gchar              *text);

void		 xedit_progress_message_area_set_fraction	(XeditProgressMessageArea *area,
								 gdouble                   fraction);

void		 xedit_progress_message_area_pulse		(XeditProgressMessageArea *area);
								 

G_END_DECLS

#endif  /* __XEDIT_PROGRESS_MESSAGE_AREA_H__  */
