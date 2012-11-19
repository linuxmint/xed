/*
 * pluma-close-button.h
 * This file is part of pluma
 *
 * Copyright (C) 2010 - Paolo Borelli
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

#ifndef __PLUMA_CLOSE_BUTTON_H__
#define __PLUMA_CLOSE_BUTTON_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PLUMA_TYPE_CLOSE_BUTTON			(pluma_close_button_get_type ())
#define PLUMA_CLOSE_BUTTON(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_CLOSE_BUTTON, PlumaCloseButton))
#define PLUMA_CLOSE_BUTTON_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_CLOSE_BUTTON, PlumaCloseButton const))
#define PLUMA_CLOSE_BUTTON_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), PLUMA_TYPE_CLOSE_BUTTON, PlumaCloseButtonClass))
#define PLUMA_IS_CLOSE_BUTTON(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUMA_TYPE_CLOSE_BUTTON))
#define PLUMA_IS_CLOSE_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_CLOSE_BUTTON))
#define PLUMA_CLOSE_BUTTON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), PLUMA_TYPE_CLOSE_BUTTON, PlumaCloseButtonClass))

typedef struct _PlumaCloseButton	PlumaCloseButton;
typedef struct _PlumaCloseButtonClass	PlumaCloseButtonClass;
typedef struct _PlumaCloseButtonPrivate	PlumaCloseButtonPrivate;

struct _PlumaCloseButton {
	GtkButton parent;
};

struct _PlumaCloseButtonClass {
	GtkButtonClass parent_class;
};

GType		  pluma_close_button_get_type (void) G_GNUC_CONST;

GtkWidget	 *pluma_close_button_new (void);

G_END_DECLS

#endif /* __PLUMA_CLOSE_BUTTON_H__ */
