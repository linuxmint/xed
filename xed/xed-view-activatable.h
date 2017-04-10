/*
 * xed-view-activatable.h
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

#ifndef __XED_VIEW_ACTIVATABLE_H__
#define __XED_VIEW_ACTIVATABLE_H__

#include <glib-object.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XED_TYPE_VIEW_ACTIVATABLE     (xed_view_activatable_get_type ())
#define XED_VIEW_ACTIVATABLE(obj)     (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_VIEW_ACTIVATABLE, XedViewActivatable))
#define XED_VIEW_ACTIVATABLE_IFACE(obj)   (G_TYPE_CHECK_CLASS_CAST ((obj), XED_TYPE_VIEW_ACTIVATABLE, XedViewActivatableInterface))
#define XED_IS_VIEW_ACTIVATABLE(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_VIEW_ACTIVATABLE))
#define XED_VIEW_ACTIVATABLE_GET_IFACE(obj)   (G_TYPE_INSTANCE_GET_INTERFACE ((obj), XED_TYPE_VIEW_ACTIVATABLE, XedViewActivatableInterface))

typedef struct _XedViewActivatable           XedViewActivatable; /* dummy typedef */
typedef struct _XedViewActivatableInterface  XedViewActivatableInterface;

struct _XedViewActivatableInterface
{
    GTypeInterface g_iface;

    /* Virtual public methods */
    void    (*activate)     (XedViewActivatable *activatable);
    void    (*deactivate)       (XedViewActivatable   *activatable);
};

/*
 * Public methods
 */
GType    xed_view_activatable_get_type    (void)  G_GNUC_CONST;

void     xed_view_activatable_activate    (XedViewActivatable *activatable);
void     xed_view_activatable_deactivate  (XedViewActivatable *activatable);

G_END_DECLS

#endif /* __XED_VIEW_ACTIVATABLE_H__ */
