/*
 * xed-window-activatable.h
 * This file is part of xed
 *
 * Copyright (C) 2010 Steve Frécinaux
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xed-window-activatable.h"
#include "xed-window.h"

/**
 * SECTION:xed-window-activatable
 * @short_description: Interface for activatable extensions on windows
 * @see_also: #PeasExtensionSet
 *
 * #XedWindowActivatable is an interface which should be implemented by
 * extensions that should be activated on a xed main window.
 **/
G_DEFINE_INTERFACE(XedWindowActivatable, xed_window_activatable, G_TYPE_OBJECT)

void
xed_window_activatable_default_init (XedWindowActivatableInterface *iface)
{
    /**
     * XedWindowActivatable:window:
     *
     * The window property contains the xed window for this
     * #XedWindowActivatable instance.
     */
    g_object_interface_install_property (iface,
                                         g_param_spec_object ("window",
                                                              "Window",
                                                              "The xed window",
                                                              XED_TYPE_WINDOW,
                                                              G_PARAM_READWRITE |
                                                              G_PARAM_CONSTRUCT_ONLY |
                                                              G_PARAM_STATIC_STRINGS));
}

/**
 * xed_window_activatable_activate:
 * @activatable: A #XedWindowActivatable.
 *
 * Activates the extension on the window property.
 */
void
xed_window_activatable_activate (XedWindowActivatable *activatable)
{
    XedWindowActivatableInterface *iface;

    g_return_if_fail (XED_IS_WINDOW_ACTIVATABLE (activatable));

    iface = XED_WINDOW_ACTIVATABLE_GET_IFACE (activatable);
    if (iface->activate != NULL)
    {
        iface->activate (activatable);
    }
}

/**
 * xed_window_activatable_deactivate:
 * @activatable: A #XedWindowActivatable.
 *
 * Deactivates the extension on the window property.
 */
void
xed_window_activatable_deactivate (XedWindowActivatable *activatable)
{
    XedWindowActivatableInterface *iface;

    g_return_if_fail (XED_IS_WINDOW_ACTIVATABLE (activatable));

    iface = XED_WINDOW_ACTIVATABLE_GET_IFACE (activatable);
    if (iface->deactivate != NULL)
    {
        iface->deactivate (activatable);
    }
}

/**
 * xed_window_activatable_update_state:
 * @activatable: A #XedWindowActivatable.
 *
 * Triggers an update of the extension insternal state to take into account
 * state changes in the window state, due to some event or user action.
 */
void
xed_window_activatable_update_state (XedWindowActivatable *activatable)
{
    XedWindowActivatableInterface *iface;

    g_return_if_fail (XED_IS_WINDOW_ACTIVATABLE (activatable));

    iface = XED_WINDOW_ACTIVATABLE_GET_IFACE (activatable);
    if (iface->update_state != NULL)
    {
        iface->update_state (activatable);
    }
}
