/*
 * xed-status-combo-box.h
 * This file is part of xed
 *
 * Copyright (C) 2008 - Jesse van den Kieboom
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

#ifndef __XED_STATUS_COMBO_BOX_H__
#define __XED_STATUS_COMBO_BOX_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XED_TYPE_STATUS_COMBO_BOX             (xed_status_combo_box_get_type ())
#define XED_STATUS_COMBO_BOX(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_STATUS_COMBO_BOX, XedStatusComboBox))
#define XED_STATUS_COMBO_BOX_CONST(obj)       (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_STATUS_COMBO_BOX, XedStatusComboBox const))
#define XED_STATUS_COMBO_BOX_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_STATUS_COMBO_BOX, XedStatusComboBoxClass))
#define XED_IS_STATUS_COMBO_BOX(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_STATUS_COMBO_BOX))
#define XED_IS_STATUS_COMBO_BOX_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_STATUS_COMBO_BOX))
#define XED_STATUS_COMBO_BOX_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), XED_TYPE_STATUS_COMBO_BOX, XedStatusComboBoxClass))

typedef struct _XedStatusComboBox             XedStatusComboBox;
typedef struct _XedStatusComboBoxPrivate      XedStatusComboBoxPrivate;
typedef struct _XedStatusComboBoxClass        XedStatusComboBoxClass;
typedef struct _XedStatusComboBoxClassPrivate XedStatusComboBoxClassPrivate;

struct _XedStatusComboBox
{
    GtkEventBox parent;
    
    XedStatusComboBoxPrivate *priv;
};

struct _XedStatusComboBoxClass
{
    GtkEventBoxClass parent_class;

    XedStatusComboBoxClassPrivate *priv;
    
    void (*changed) (XedStatusComboBox *combo,
                     GtkMenuItem         *item);
};

GType xed_status_combo_box_get_type           (void) G_GNUC_CONST;
GtkWidget *xed_status_combo_box_new           (const gchar        *label);

const gchar *xed_status_combo_box_get_label       (XedStatusComboBox    *combo);
void xed_status_combo_box_set_label               (XedStatusComboBox    *combo,
                                                     const gchar            *label);

void xed_status_combo_box_add_item                (XedStatusComboBox    *combo,
                                                     GtkMenuItem            *item,
                                                     const gchar            *text);
void xed_status_combo_box_remove_item             (XedStatusComboBox    *combo,
                                                     GtkMenuItem            *item);

GList *xed_status_combo_box_get_items             (XedStatusComboBox    *combo);
const gchar *xed_status_combo_box_get_item_text   (XedStatusComboBox    *combo,
                                                     GtkMenuItem        *item);
void xed_status_combo_box_set_item_text           (XedStatusComboBox    *combo,
                                                     GtkMenuItem        *item,
                                                     const gchar            *text);

void xed_status_combo_box_set_item                (XedStatusComboBox    *combo,
                                                     GtkMenuItem        *item);

GtkLabel *xed_status_combo_box_get_item_label     (XedStatusComboBox    *combo);

G_END_DECLS

#endif /* __XED_STATUS_COMBO_BOX_H__ */
