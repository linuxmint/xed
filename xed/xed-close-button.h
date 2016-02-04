/*
 * xed-close-button.h
 * This file is part of xed
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

#ifndef __XED_CLOSE_BUTTON_H__
#define __XED_CLOSE_BUTTON_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XED_TYPE_CLOSE_BUTTON			(xed_close_button_get_type ())
#define XED_CLOSE_BUTTON(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_CLOSE_BUTTON, XedCloseButton))
#define XED_CLOSE_BUTTON_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_CLOSE_BUTTON, XedCloseButton const))
#define XED_CLOSE_BUTTON_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_CLOSE_BUTTON, XedCloseButtonClass))
#define XED_IS_CLOSE_BUTTON(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_CLOSE_BUTTON))
#define XED_IS_CLOSE_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_CLOSE_BUTTON))
#define XED_CLOSE_BUTTON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), XED_TYPE_CLOSE_BUTTON, XedCloseButtonClass))

typedef struct _XedCloseButton	XedCloseButton;
typedef struct _XedCloseButtonClass	XedCloseButtonClass;
typedef struct _XedCloseButtonPrivate	XedCloseButtonPrivate;

struct _XedCloseButton {
	GtkButton parent;
};

struct _XedCloseButtonClass {
	GtkButtonClass parent_class;
};

GType		  xed_close_button_get_type (void) G_GNUC_CONST;

GtkWidget	 *xed_close_button_new (void);

G_END_DECLS

#endif /* __XED_CLOSE_BUTTON_H__ */
