/*
 * xed-view-activatable.h
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

#include "xed-view-activatable.h"
#include "xed-view.h"

/**
 * SECTION:xed-view-activatable
 * @short_description: Interface for activatable extensions on views
 * @see_also: #PeasExtensionSet
 *
 * #XedViewActivatable is an interface which should be implemented by
 * extensions that should be activated on a xed view.
 **/
G_DEFINE_INTERFACE(XedViewActivatable, xed_view_activatable, G_TYPE_OBJECT)

void
xed_view_activatable_default_init (XedViewActivatableInterface *iface)
{
    /**
     * XedViewActivatable:view:
     *
     * The window property contains the xed window for this
     * #XedViewActivatable instance.
     */
    g_object_interface_install_property (iface,
                                         g_param_spec_object ("view",
                                                              "view",
                                                              "A xed view",
                                                              XED_TYPE_VIEW,
                                                              G_PARAM_READWRITE |
                                                              G_PARAM_CONSTRUCT_ONLY |
                                                              G_PARAM_STATIC_STRINGS));
}

/**
 * xed_view_activatable_activate:
 * @activatable: A #XedViewActivatable.
 *
 * Activates the extension on the window property.
 */
void
xed_view_activatable_activate (XedViewActivatable *activatable)
{
    XedViewActivatableInterface *iface;

    g_return_if_fail (XED_IS_VIEW_ACTIVATABLE (activatable));

    iface = XED_VIEW_ACTIVATABLE_GET_IFACE (activatable);
    if (iface->activate != NULL)
    {
        iface->activate (activatable);
    }
}

/**
 * xed_view_activatable_deactivate:
 * @activatable: A #XedViewActivatable.
 *
 * Deactivates the extension on the window property.
 */
void
xed_view_activatable_deactivate (XedViewActivatable *activatable)
{
    XedViewActivatableInterface *iface;

    g_return_if_fail (XED_IS_VIEW_ACTIVATABLE (activatable));

    iface = XED_VIEW_ACTIVATABLE_GET_IFACE (activatable);
    if (iface->deactivate != NULL)
    {
        iface->deactivate (activatable);
    }
}
