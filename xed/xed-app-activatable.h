/*
 * xed-app-activatable.h
 * This file is part of xed
 *
 * Copyright (C) 2010 - Steve Frécinaux
 * Copyright (C) 2010 - Jesse van den Kieboom
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

#ifndef __XED_APP_ACTIVATABLE_H__
#define __XED_APP_ACTIVATABLE_H__

#include <glib-object.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XED_TYPE_APP_ACTIVATABLE      (xed_app_activatable_get_type ())
#define XED_APP_ACTIVATABLE(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_APP_ACTIVATABLE, XedAppActivatable))
#define XED_APP_ACTIVATABLE_IFACE(obj)    (G_TYPE_CHECK_CLASS_CAST ((obj), XED_TYPE_APP_ACTIVATABLE, XedAppActivatableInterface))
#define XED_IS_APP_ACTIVATABLE(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_APP_ACTIVATABLE))
#define XED_APP_ACTIVATABLE_GET_IFACE(obj)    (G_TYPE_INSTANCE_GET_INTERFACE ((obj), XED_TYPE_APP_ACTIVATABLE, XedAppActivatableInterface))

typedef struct _XedAppActivatable           XedAppActivatable; /* dummy typedef */
typedef struct _XedAppActivatableInterface  XedAppActivatableInterface;

struct _XedAppActivatableInterface
{
    GTypeInterface g_iface;

    /* Virtual public methods */
    void    (*activate)     (XedAppActivatable *activatable);
    void    (*deactivate)       (XedAppActivatable *activatable);
};

/*
 * Public methods
 */
GType    xed_app_activatable_get_type (void)  G_GNUC_CONST;

void     xed_app_activatable_activate (XedAppActivatable *activatable);
void     xed_app_activatable_deactivate   (XedAppActivatable *activatable);

G_END_DECLS

#endif /* __XED_APP_ACTIVATABLE_H__ */
