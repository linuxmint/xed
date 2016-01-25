/*
 * xedit-close-button.h
 * This file is part of xedit
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

#ifndef __XEDIT_CLOSE_BUTTON_H__
#define __XEDIT_CLOSE_BUTTON_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XEDIT_TYPE_CLOSE_BUTTON			(xedit_close_button_get_type ())
#define XEDIT_CLOSE_BUTTON(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_CLOSE_BUTTON, XeditCloseButton))
#define XEDIT_CLOSE_BUTTON_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_CLOSE_BUTTON, XeditCloseButton const))
#define XEDIT_CLOSE_BUTTON_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_CLOSE_BUTTON, XeditCloseButtonClass))
#define XEDIT_IS_CLOSE_BUTTON(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XEDIT_TYPE_CLOSE_BUTTON))
#define XEDIT_IS_CLOSE_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_CLOSE_BUTTON))
#define XEDIT_CLOSE_BUTTON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), XEDIT_TYPE_CLOSE_BUTTON, XeditCloseButtonClass))

typedef struct _XeditCloseButton	XeditCloseButton;
typedef struct _XeditCloseButtonClass	XeditCloseButtonClass;
typedef struct _XeditCloseButtonPrivate	XeditCloseButtonPrivate;

struct _XeditCloseButton {
	GtkButton parent;
};

struct _XeditCloseButtonClass {
	GtkButtonClass parent_class;
};

GType		  xedit_close_button_get_type (void) G_GNUC_CONST;

GtkWidget	 *xedit_close_button_new (void);

G_END_DECLS

#endif /* __XEDIT_CLOSE_BUTTON_H__ */
