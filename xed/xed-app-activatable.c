/*
 * xed-app-activatable.h
 * This file is part of xed
 *
 * Copyright (C) 2010 Steve Frécinaux
 * Copyright (C) 2010 Jesse van den Kieboom
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

#include <config.h>

#include "xed-app-activatable.h"
#include "xed-app.h"

/**
 * SECTION:xed-app-activatable
 * @short_description: Interface for activatable extensions on apps
 * @see_also: #PeasExtensionSet
 *
 * #XedAppActivatable is an interface which should be implemented by
 * extensions that should be activated on a xed application.
 **/

G_DEFINE_INTERFACE(XedAppActivatable, xed_app_activatable, G_TYPE_OBJECT)

void
xed_app_activatable_default_init (XedAppActivatableInterface *iface)
{
    static gboolean initialized = FALSE;

    if (!initialized)
    {
        /**
         * XedAppActivatable:app:
         *
         * The app property contains the xed app for this
         * #XedAppActivatable instance.
         */
        g_object_interface_install_property (iface,
                                             g_param_spec_object ("app",
                                                                  "App",
                                                                  "The xed app",
                                                                  XED_TYPE_APP,
                                                                  G_PARAM_READWRITE |
                                                                  G_PARAM_CONSTRUCT_ONLY |
                                                                  G_PARAM_STATIC_STRINGS));

        initialized = TRUE;
    }
}

/**
 * xed_app_activatable_activate:
 * @activatable: A #XedAppActivatable.
 *
 * Activates the extension on the application.
 */
void
xed_app_activatable_activate (XedAppActivatable *activatable)
{
    XedAppActivatableInterface *iface;

    g_return_if_fail (XED_IS_APP_ACTIVATABLE (activatable));

    iface = XED_APP_ACTIVATABLE_GET_IFACE (activatable);

    if (iface->activate != NULL)
    {
        iface->activate (activatable);
    }
}

/**
 * xed_app_activatable_deactivate:
 * @activatable: A #XedAppActivatable.
 *
 * Deactivates the extension from the application.
 *
 */
void
xed_app_activatable_deactivate (XedAppActivatable *activatable)
{
    XedAppActivatableInterface *iface;

    g_return_if_fail (XED_IS_APP_ACTIVATABLE (activatable));

    iface = XED_APP_ACTIVATABLE_GET_IFACE (activatable);

    if (iface->deactivate != NULL)
    {
        iface->deactivate (activatable);
    }
}
