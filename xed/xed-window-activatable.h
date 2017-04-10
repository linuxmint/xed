/*
 * xed-window-activatable.h
 * This file is part of xed
 *
 * Copyright (C) 2010 - Steve Frécinaux
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __XED_WINDOW_ACTIVATABLE_H__
#define __XED_WINDOW_ACTIVATABLE_H__

#include <glib-object.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XED_TYPE_WINDOW_ACTIVATABLE       (xed_window_activatable_get_type ())
#define XED_WINDOW_ACTIVATABLE(obj)       (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_WINDOW_ACTIVATABLE, XedWindowActivatable))
#define XED_WINDOW_ACTIVATABLE_IFACE(obj) (G_TYPE_CHECK_CLASS_CAST ((obj), XED_TYPE_WINDOW_ACTIVATABLE, XedWindowActivatableInterface))
#define XED_IS_WINDOW_ACTIVATABLE(obj)    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_WINDOW_ACTIVATABLE))
#define XED_WINDOW_ACTIVATABLE_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), XED_TYPE_WINDOW_ACTIVATABLE, XedWindowActivatableInterface))

typedef struct _XedWindowActivatable           XedWindowActivatable; /* dummy typedef */
typedef struct _XedWindowActivatableInterface  XedWindowActivatableInterface;

struct _XedWindowActivatableInterface
{
    GTypeInterface g_iface;

    /* Virtual public methods */
    void    (*activate)     (XedWindowActivatable *activatable);
    void    (*deactivate)       (XedWindowActivatable *activatable);
    void    (*update_state)     (XedWindowActivatable *activatable);
};

/*
 * Public methods
 */
GType    xed_window_activatable_get_type  (void)  G_GNUC_CONST;

void     xed_window_activatable_activate  (XedWindowActivatable *activatable);
void     xed_window_activatable_deactivate    (XedWindowActivatable *activatable);
void     xed_window_activatable_update_state  (XedWindowActivatable *activatable);

G_END_DECLS

#endif /* __XED_WINDOW_ACTIVATABLE_H__ */
