/*
 * xedit-status-combo-box.h
 * This file is part of xedit
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

#ifndef __XEDIT_STATUS_COMBO_BOX_H__
#define __XEDIT_STATUS_COMBO_BOX_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XEDIT_TYPE_STATUS_COMBO_BOX             (xedit_status_combo_box_get_type ())
#define XEDIT_STATUS_COMBO_BOX(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_STATUS_COMBO_BOX, XeditStatusComboBox))
#define XEDIT_STATUS_COMBO_BOX_CONST(obj)       (G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_STATUS_COMBO_BOX, XeditStatusComboBox const))
#define XEDIT_STATUS_COMBO_BOX_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_STATUS_COMBO_BOX, XeditStatusComboBoxClass))
#define XEDIT_IS_STATUS_COMBO_BOX(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XEDIT_TYPE_STATUS_COMBO_BOX))
#define XEDIT_IS_STATUS_COMBO_BOX_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_STATUS_COMBO_BOX))
#define XEDIT_STATUS_COMBO_BOX_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), XEDIT_TYPE_STATUS_COMBO_BOX, XeditStatusComboBoxClass))

typedef struct _XeditStatusComboBox             XeditStatusComboBox;
typedef struct _XeditStatusComboBoxPrivate      XeditStatusComboBoxPrivate;
typedef struct _XeditStatusComboBoxClass        XeditStatusComboBoxClass;
typedef struct _XeditStatusComboBoxClassPrivate XeditStatusComboBoxClassPrivate;

struct _XeditStatusComboBox
{
    GtkEventBox parent;
    
    XeditStatusComboBoxPrivate *priv;
};

struct _XeditStatusComboBoxClass
{
    GtkEventBoxClass parent_class;

    XeditStatusComboBoxClassPrivate *priv;
    
    void (*changed) (XeditStatusComboBox *combo,
                     GtkMenuItem         *item);
};

GType xedit_status_combo_box_get_type           (void) G_GNUC_CONST;
GtkWidget *xedit_status_combo_box_new           (const gchar        *label);

const gchar *xedit_status_combo_box_get_label       (XeditStatusComboBox    *combo);
void xedit_status_combo_box_set_label               (XeditStatusComboBox    *combo,
                                                     const gchar            *label);

void xedit_status_combo_box_add_item                (XeditStatusComboBox    *combo,
                                                     GtkMenuItem            *item,
                                                     const gchar            *text);
void xedit_status_combo_box_remove_item             (XeditStatusComboBox    *combo,
                                                     GtkMenuItem            *item);

GList *xedit_status_combo_box_get_items             (XeditStatusComboBox    *combo);
const gchar *xedit_status_combo_box_get_item_text   (XeditStatusComboBox    *combo,
                                                     GtkMenuItem        *item);
void xedit_status_combo_box_set_item_text           (XeditStatusComboBox    *combo,
                                                     GtkMenuItem        *item,
                                                     const gchar            *text);

void xedit_status_combo_box_set_item                (XeditStatusComboBox    *combo,
                                                     GtkMenuItem        *item);

GtkLabel *xedit_status_combo_box_get_item_label     (XeditStatusComboBox    *combo);

G_END_DECLS

#endif /* __XEDIT_STATUS_COMBO_BOX_H__ */
