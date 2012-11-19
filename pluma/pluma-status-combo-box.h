/*
 * pluma-status-combo-box.h
 * This file is part of pluma
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

#ifndef __PLUMA_STATUS_COMBO_BOX_H__
#define __PLUMA_STATUS_COMBO_BOX_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PLUMA_TYPE_STATUS_COMBO_BOX		(pluma_status_combo_box_get_type ())
#define PLUMA_STATUS_COMBO_BOX(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_STATUS_COMBO_BOX, PlumaStatusComboBox))
#define PLUMA_STATUS_COMBO_BOX_CONST(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_STATUS_COMBO_BOX, PlumaStatusComboBox const))
#define PLUMA_STATUS_COMBO_BOX_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PLUMA_TYPE_STATUS_COMBO_BOX, PlumaStatusComboBoxClass))
#define PLUMA_IS_STATUS_COMBO_BOX(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUMA_TYPE_STATUS_COMBO_BOX))
#define PLUMA_IS_STATUS_COMBO_BOX_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_STATUS_COMBO_BOX))
#define PLUMA_STATUS_COMBO_BOX_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), PLUMA_TYPE_STATUS_COMBO_BOX, PlumaStatusComboBoxClass))

typedef struct _PlumaStatusComboBox		PlumaStatusComboBox;
typedef struct _PlumaStatusComboBoxClass	PlumaStatusComboBoxClass;
typedef struct _PlumaStatusComboBoxPrivate	PlumaStatusComboBoxPrivate;

struct _PlumaStatusComboBox {
	GtkEventBox parent;
	
	PlumaStatusComboBoxPrivate *priv;
};

struct _PlumaStatusComboBoxClass {
	GtkEventBoxClass parent_class;
	
	void (*changed) (PlumaStatusComboBox *combo,
			 GtkMenuItem         *item);
};

GType pluma_status_combo_box_get_type 			(void) G_GNUC_CONST;
GtkWidget *pluma_status_combo_box_new			(const gchar 		*label);

const gchar *pluma_status_combo_box_get_label 		(PlumaStatusComboBox 	*combo);
void pluma_status_combo_box_set_label 			(PlumaStatusComboBox 	*combo,
							 const gchar         	*label);

void pluma_status_combo_box_add_item 			(PlumaStatusComboBox 	*combo,
							 GtkMenuItem         	*item,
							 const gchar         	*text);
void pluma_status_combo_box_remove_item			(PlumaStatusComboBox    *combo,
							 GtkMenuItem            *item);

GList *pluma_status_combo_box_get_items			(PlumaStatusComboBox    *combo);
const gchar *pluma_status_combo_box_get_item_text 	(PlumaStatusComboBox	*combo,
							 GtkMenuItem		*item);
void pluma_status_combo_box_set_item_text 		(PlumaStatusComboBox	*combo,
							 GtkMenuItem		*item,
							 const gchar            *text);

void pluma_status_combo_box_set_item			(PlumaStatusComboBox	*combo,
							 GtkMenuItem		*item);

GtkLabel *pluma_status_combo_box_get_item_label		(PlumaStatusComboBox	*combo);

G_END_DECLS

#endif /* __PLUMA_STATUS_COMBO_BOX_H__ */
