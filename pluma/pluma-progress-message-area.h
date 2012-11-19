/*
 * pluma-progress-message-area.h
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

#ifndef __PLUMA_PROGRESS_MESSAGE_AREA_H__
#define __PLUMA_PROGRESS_MESSAGE_AREA_H__

#if !GTK_CHECK_VERSION (2, 17, 1)
#include <pluma/pluma-message-area.h>
#else
#include <gtk/gtk.h>
#endif

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_PROGRESS_MESSAGE_AREA              (pluma_progress_message_area_get_type())
#define PLUMA_PROGRESS_MESSAGE_AREA(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_PROGRESS_MESSAGE_AREA, PlumaProgressMessageArea))
#define PLUMA_PROGRESS_MESSAGE_AREA_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), PLUMA_TYPE_PROGRESS_MESSAGE_AREA, PlumaProgressMessageAreaClass))
#define PLUMA_IS_PROGRESS_MESSAGE_AREA(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), PLUMA_TYPE_PROGRESS_MESSAGE_AREA))
#define PLUMA_IS_PROGRESS_MESSAGE_AREA_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_PROGRESS_MESSAGE_AREA))
#define PLUMA_PROGRESS_MESSAGE_AREA_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), PLUMA_TYPE_PROGRESS_MESSAGE_AREA, PlumaProgressMessageAreaClass))

/* Private structure type */
typedef struct _PlumaProgressMessageAreaPrivate PlumaProgressMessageAreaPrivate;

/*
 * Main object structure
 */
typedef struct _PlumaProgressMessageArea PlumaProgressMessageArea;

struct _PlumaProgressMessageArea 
{
#if !GTK_CHECK_VERSION (2, 17, 1)
	PlumaMessageArea parent;
#else
	GtkInfoBar parent;
#endif

	/*< private > */
	PlumaProgressMessageAreaPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _PlumaProgressMessageAreaClass PlumaProgressMessageAreaClass;

struct _PlumaProgressMessageAreaClass 
{
#if !GTK_CHECK_VERSION (2, 17, 1)
	PlumaMessageAreaClass parent_class;
#else
	GtkInfoBarClass parent_class;
#endif
};

/*
 * Public methods
 */
GType 		 pluma_progress_message_area_get_type 		(void) G_GNUC_CONST;

GtkWidget	*pluma_progress_message_area_new      		(const gchar              *stock_id,
								 const gchar              *markup,
								 gboolean                  has_cancel);

void		 pluma_progress_message_area_set_stock_image	(PlumaProgressMessageArea *area,
								 const gchar              *stock_id);

void		 pluma_progress_message_area_set_markup		(PlumaProgressMessageArea *area,
								 const gchar              *markup);

void		 pluma_progress_message_area_set_text		(PlumaProgressMessageArea *area,
								 const gchar              *text);

void		 pluma_progress_message_area_set_fraction	(PlumaProgressMessageArea *area,
								 gdouble                   fraction);

void		 pluma_progress_message_area_pulse		(PlumaProgressMessageArea *area);
								 

G_END_DECLS

#endif  /* __PLUMA_PROGRESS_MESSAGE_AREA_H__  */
