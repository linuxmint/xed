/*
 * xed-progress-info-bar.h
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

#ifndef __XED_PROGRESS_INFO_BAR_H__
#define __XED_PROGRESS_INFO_BAR_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XED_TYPE_PROGRESS_INFO_BAR              (xed_progress_info_bar_get_type())
#define XED_PROGRESS_INFO_BAR(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_PROGRESS_INFO_BAR, XedProgressInfoBar))
#define XED_PROGRESS_INFO_BAR_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_PROGRESS_INFO_BAR, XedProgressInfoBarClass))
#define XED_IS_PROGRESS_INFO_BAR(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_PROGRESS_INFO_BAR))
#define XED_IS_PROGRESS_INFO_BAR_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_PROGRESS_INFO_BAR))
#define XED_PROGRESS_INFO_BAR_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_PROGRESS_INFO_BAR, XedProgressInfoBarClass))

typedef struct _XedProgressInfoBar          XedProgressInfoBar;
typedef struct _XedProgressInfoBarClass     XedProgressInfoBarClass;
typedef struct _XedProgressInfoBarPrivate   XedProgressInfoBarPrivate;

struct _XedProgressInfoBar
{
    GtkInfoBar parent;

    /*< private > */
    XedProgressInfoBarPrivate *priv;
};

struct _XedProgressInfoBarClass
{
    GtkInfoBarClass parent_class;
};

GType xed_progress_info_bar_get_type (void) G_GNUC_CONST;

GtkWidget *xed_progress_info_bar_new (const gchar *icon_name,
                                      const gchar *markup,
                                      gboolean     has_cancel);

void xed_progress_info_bar_set_icon_name (XedProgressInfoBar *area,
                                          const gchar        *icon_name);

void xed_progress_info_bar_set_markup (XedProgressInfoBar *area,
                                       const gchar        *markup);

void xed_progress_info_bar_set_text (XedProgressInfoBar *area,
                                     const gchar        *text);

void xed_progress_info_bar_set_fraction (XedProgressInfoBar *area,
                                         gdouble             fraction);

void xed_progress_info_bar_pulse (XedProgressInfoBar *area);


G_END_DECLS

#endif  /* __XED_PROGRESS_INFO_BAR_H__  */
