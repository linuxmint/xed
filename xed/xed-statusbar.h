/*
 * xed-statusbar.h
 * This file is part of xed
 *
 * Copyright (C) 2005 - Paolo Borelli
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
 */

#ifndef XED_STATUSBAR_H
#define XED_STATUSBAR_H

#include <gtk/gtk.h>
#include <xed/xed-window.h>

G_BEGIN_DECLS

#define XED_TYPE_STATUSBAR          (xed_statusbar_get_type ())
#define XED_STATUSBAR(o)            (G_TYPE_CHECK_INSTANCE_CAST ((o), XED_TYPE_STATUSBAR, XedStatusbar))
#define XED_STATUSBAR_CLASS(k)      (G_TYPE_CHECK_CLASS_CAST((k), XED_TYPE_STATUSBAR, XedStatusbarClass))
#define XED_IS_STATUSBAR(o)         (G_TYPE_CHECK_INSTANCE_TYPE ((o), XED_TYPE_STATUSBAR))
#define XED_IS_STATUSBAR_CLASS(k)   (G_TYPE_CHECK_CLASS_TYPE ((k), XED_TYPE_STATUSBAR))
#define XED_STATUSBAR_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), XED_TYPE_STATUSBAR, XedStatusbarClass))

typedef struct _XedStatusbar        XedStatusbar;
typedef struct _XedStatusbarPrivate XedStatusbarPrivate;
typedef struct _XedStatusbarClass   XedStatusbarClass;

struct _XedStatusbar
{
        GtkStatusbar parent;

    /* <private/> */
        XedStatusbarPrivate *priv;
};

struct _XedStatusbarClass
{
        GtkStatusbarClass parent_class;
};

GType xed_statusbar_get_type (void) G_GNUC_CONST;

GtkWidget *xed_statusbar_new (void);

void xed_statusbar_set_window_state (XedStatusbar   *statusbar,
                                     XedWindowState  state,
                                     gint            num_of_errors);

void xed_statusbar_set_overwrite (XedStatusbar *statusbar,
                                  gboolean      overwrite);

void xed_statusbar_set_cursor_position (XedStatusbar *statusbar,
                                        gint          line,
                                        gint          col);

void xed_statusbar_clear_overwrite (XedStatusbar *statusbar);

void xed_statusbar_flash_message (XedStatusbar *statusbar,
                                  guint         context_id,
                                  const gchar  *format,
                                  ...) G_GNUC_PRINTF(3, 4);

G_END_DECLS

#endif
