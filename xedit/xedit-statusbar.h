/*
 * xedit-statusbar.h
 * This file is part of xedit
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
 * Modified by the xedit Team, 2005. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifndef XEDIT_STATUSBAR_H
#define XEDIT_STATUSBAR_H

#include <gtk/gtk.h>
#include <xedit/xedit-window.h>

G_BEGIN_DECLS

#define XEDIT_TYPE_STATUSBAR		(xedit_statusbar_get_type ())
#define XEDIT_STATUSBAR(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), XEDIT_TYPE_STATUSBAR, XeditStatusbar))
#define XEDIT_STATUSBAR_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), XEDIT_TYPE_STATUSBAR, XeditStatusbarClass))
#define XEDIT_IS_STATUSBAR(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), XEDIT_TYPE_STATUSBAR))
#define XEDIT_IS_STATUSBAR_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), XEDIT_TYPE_STATUSBAR))
#define XEDIT_STATUSBAR_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), XEDIT_TYPE_STATUSBAR, XeditStatusbarClass))

typedef struct _XeditStatusbar		XeditStatusbar;
typedef struct _XeditStatusbarPrivate	XeditStatusbarPrivate;
typedef struct _XeditStatusbarClass	XeditStatusbarClass;

struct _XeditStatusbar
{
        GtkStatusbar parent;

	/* <private/> */
        XeditStatusbarPrivate *priv;
};

struct _XeditStatusbarClass
{
        GtkStatusbarClass parent_class;
};

GType		 xedit_statusbar_get_type		(void) G_GNUC_CONST;

GtkWidget	*xedit_statusbar_new			(void);

/* FIXME: status is not defined in any .h */
#define XeditStatus gint
void		 xedit_statusbar_set_window_state	(XeditStatusbar   *statusbar,
							 XeditWindowState  state,
							 gint              num_of_errors);

void		 xedit_statusbar_set_overwrite		(XeditStatusbar   *statusbar,
							 gboolean          overwrite);

void		 xedit_statusbar_set_cursor_position	(XeditStatusbar   *statusbar,
							 gint              line,
							 gint              col);

void		 xedit_statusbar_clear_overwrite 	(XeditStatusbar   *statusbar);

void		 xedit_statusbar_flash_message		(XeditStatusbar   *statusbar,
							 guint             context_id,
							 const gchar      *format,
							 ...) G_GNUC_PRINTF(3, 4);

G_END_DECLS

#endif
