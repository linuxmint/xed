/*
 * pluma-statusbar.h
 * This file is part of pluma
 *
 * Copyright (C) 2005 - Paolo Borelli
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
 */

#ifndef PLUMA_STATUSBAR_H
#define PLUMA_STATUSBAR_H

#include <gtk/gtk.h>
#include <pluma/pluma-window.h>

G_BEGIN_DECLS

#define PLUMA_TYPE_STATUSBAR		(pluma_statusbar_get_type ())
#define PLUMA_STATUSBAR(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), PLUMA_TYPE_STATUSBAR, PlumaStatusbar))
#define PLUMA_STATUSBAR_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), PLUMA_TYPE_STATUSBAR, PlumaStatusbarClass))
#define PLUMA_IS_STATUSBAR(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), PLUMA_TYPE_STATUSBAR))
#define PLUMA_IS_STATUSBAR_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), PLUMA_TYPE_STATUSBAR))
#define PLUMA_STATUSBAR_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), PLUMA_TYPE_STATUSBAR, PlumaStatusbarClass))

typedef struct _PlumaStatusbar		PlumaStatusbar;
typedef struct _PlumaStatusbarPrivate	PlumaStatusbarPrivate;
typedef struct _PlumaStatusbarClass	PlumaStatusbarClass;

struct _PlumaStatusbar
{
        GtkStatusbar parent;

	/* <private/> */
        PlumaStatusbarPrivate *priv;
};

struct _PlumaStatusbarClass
{
        GtkStatusbarClass parent_class;
};

GType		 pluma_statusbar_get_type		(void) G_GNUC_CONST;

GtkWidget	*pluma_statusbar_new			(void);

/* FIXME: status is not defined in any .h */
#define PlumaStatus gint
void		 pluma_statusbar_set_window_state	(PlumaStatusbar   *statusbar,
							 PlumaWindowState  state,
							 gint              num_of_errors);

void		 pluma_statusbar_set_overwrite		(PlumaStatusbar   *statusbar,
							 gboolean          overwrite);

void		 pluma_statusbar_set_cursor_position	(PlumaStatusbar   *statusbar,
							 gint              line,
							 gint              col);

void		 pluma_statusbar_clear_overwrite 	(PlumaStatusbar   *statusbar);

void		 pluma_statusbar_flash_message		(PlumaStatusbar   *statusbar,
							 guint             context_id,
							 const gchar      *format,
							 ...) G_GNUC_PRINTF(3, 4);
/* FIXME: these would be nice for plugins...
void		 pluma_statusbar_add_widget		(PlumaStatusbar   *statusbar,
							 GtkWidget        *widget);
void		 pluma_statusbar_remove_widget		(PlumaStatusbar   *statusbar,
							 GtkWidget        *widget);
*/

/*
 * Non exported functions
 */
void		_pluma_statusbar_set_has_resize_grip	(PlumaStatusbar   *statusbar,
							 gboolean          show);

G_END_DECLS

#endif
