/*
 * gedit-statusbar.h
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */
 
/*
 * Modified by the gedit Team, 2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifndef GEDIT_STATUSBAR_H
#define GEDIT_STATUSBAR_H

#include <gtk/gtk.h>
#include <gedit/gedit-window.h>

G_BEGIN_DECLS

#define GEDIT_TYPE_STATUSBAR		(gedit_statusbar_get_type ())
#define GEDIT_STATUSBAR(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), GEDIT_TYPE_STATUSBAR, GeditStatusbar))
#define GEDIT_STATUSBAR_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), GEDIT_TYPE_STATUSBAR, GeditStatusbarClass))
#define GEDIT_IS_STATUSBAR(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GEDIT_TYPE_STATUSBAR))
#define GEDIT_IS_STATUSBAR_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), GEDIT_TYPE_STATUSBAR))
#define GEDIT_STATUSBAR_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), GEDIT_TYPE_STATUSBAR, GeditStatusbarClass))

typedef struct _GeditStatusbar		GeditStatusbar;
typedef struct _GeditStatusbarPrivate	GeditStatusbarPrivate;
typedef struct _GeditStatusbarClass	GeditStatusbarClass;

struct _GeditStatusbar
{
        GtkStatusbar parent;

	/* <private/> */
        GeditStatusbarPrivate *priv;
};

struct _GeditStatusbarClass
{
        GtkStatusbarClass parent_class;
};

GType		 gedit_statusbar_get_type		(void) G_GNUC_CONST;

GtkWidget	*gedit_statusbar_new			(void);

/* FIXME: status is not defined in any .h */
#define GeditStatus gint
void		 gedit_statusbar_set_window_state	(GeditStatusbar   *statusbar,
							 GeditWindowState  state,
							 gint              num_of_errors);

void		 gedit_statusbar_set_overwrite		(GeditStatusbar   *statusbar,
							 gboolean          overwrite);

void		 gedit_statusbar_set_cursor_position	(GeditStatusbar   *statusbar,
							 gint              line,
							 gint              col);

void		 gedit_statusbar_clear_overwrite 	(GeditStatusbar   *statusbar);

void		 gedit_statusbar_flash_message		(GeditStatusbar   *statusbar,
							 guint             context_id,
							 const gchar      *format,
							 ...) G_GNUC_PRINTF(3, 4);
/* FIXME: these would be nice for plugins...
void		 gedit_statusbar_add_widget		(GeditStatusbar   *statusbar,
							 GtkWidget        *widget);
void		 gedit_statusbar_remove_widget		(GeditStatusbar   *statusbar,
							 GtkWidget        *widget);
*/

/*
 * Non exported functions
 */
void		_gedit_statusbar_set_has_resize_grip	(GeditStatusbar   *statusbar,
							 gboolean          show);

G_END_DECLS

#endif
