/*
 * xed-panel.h
 * This file is part of xed
 *
 * Copyright (C) 2005 - Paolo Maggi
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
 * Modified by the xed Team, 2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __XED_PANEL_H__
#define __XED_PANEL_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XED_TYPE_PANEL            (xed_panel_get_type())
#define XED_PANEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_PANEL, XedPanel))
#define XED_PANEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_PANEL, XedPanelClass))
#define XED_IS_PANEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_PANEL))
#define XED_IS_PANEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_PANEL))
#define XED_PANEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_PANEL, XedPanelClass))

/* Private structure type */
typedef struct _XedPanelPrivate XedPanelPrivate;

/*
 * Main object structure
 */
typedef struct _XedPanel XedPanel;

struct _XedPanel
{
    GtkBin parent;

    /*< private > */
    XedPanelPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XedPanelClass XedPanelClass;

struct _XedPanelClass
{
    GtkBinClass parent_class;

    void (* item_added)     (XedPanel     *panel,
                             GtkWidget      *item);
    void (* item_removed)   (XedPanel     *panel,
                             GtkWidget      *item);

    /* Keybinding signals */
    void (* close)          (XedPanel     *panel);
    void (* focus_document) (XedPanel     *panel);

    /* Padding for future expansion */
    void (*_xed_reserved1) (void);
    void (*_xed_reserved2) (void);
    void (*_xed_reserved3) (void);
    void (*_xed_reserved4) (void);
};

/*
 * Public methods
 */
GType xed_panel_get_type (void) G_GNUC_CONST;

GtkWidget *xed_panel_new (GtkOrientation  orientation);

void xed_panel_add_item (XedPanel    *panel,
                         GtkWidget   *item,
                         const gchar *name,
                         const gchar *icon_name);

gboolean xed_panel_remove_item (XedPanel  *panel,
                                GtkWidget *item);

gboolean xed_panel_activate_item (XedPanel  *panel,
                                  GtkWidget *item);

gboolean xed_panel_item_is_active (XedPanel  *panel,
                                   GtkWidget *item);

GtkOrientation xed_panel_get_orientation (XedPanel   *panel);

gint xed_panel_get_n_items (XedPanel *panel);


/*
 * Non exported functions
 */
gint _xed_panel_get_active_item_id (XedPanel   *panel);

void _xed_panel_set_active_item_by_id (XedPanel *panel,
                                       gint      id);

G_END_DECLS

#endif  /* __XED_PANEL_H__  */
