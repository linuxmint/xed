/*
 * gedit-status-combo-box.h
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GEDIT_STATUS_COMBO_BOX_H__
#define __GEDIT_STATUS_COMBO_BOX_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GEDIT_TYPE_STATUS_COMBO_BOX		(gedit_status_combo_box_get_type ())
#define GEDIT_STATUS_COMBO_BOX(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_STATUS_COMBO_BOX, GeditStatusComboBox))
#define GEDIT_STATUS_COMBO_BOX_CONST(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_STATUS_COMBO_BOX, GeditStatusComboBox const))
#define GEDIT_STATUS_COMBO_BOX_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_STATUS_COMBO_BOX, GeditStatusComboBoxClass))
#define GEDIT_IS_STATUS_COMBO_BOX(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_STATUS_COMBO_BOX))
#define GEDIT_IS_STATUS_COMBO_BOX_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_STATUS_COMBO_BOX))
#define GEDIT_STATUS_COMBO_BOX_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_TYPE_STATUS_COMBO_BOX, GeditStatusComboBoxClass))

typedef struct _GeditStatusComboBox		GeditStatusComboBox;
typedef struct _GeditStatusComboBoxClass	GeditStatusComboBoxClass;
typedef struct _GeditStatusComboBoxPrivate	GeditStatusComboBoxPrivate;

struct _GeditStatusComboBox {
	GtkEventBox parent;
	
	GeditStatusComboBoxPrivate *priv;
};

struct _GeditStatusComboBoxClass {
	GtkEventBoxClass parent_class;
	
	void (*changed) (GeditStatusComboBox *combo,
			 GtkMenuItem         *item);
};

GType gedit_status_combo_box_get_type 			(void) G_GNUC_CONST;
GtkWidget *gedit_status_combo_box_new			(const gchar 		*label);

const gchar *gedit_status_combo_box_get_label 		(GeditStatusComboBox 	*combo);
void gedit_status_combo_box_set_label 			(GeditStatusComboBox 	*combo,
							 const gchar         	*label);

void gedit_status_combo_box_add_item 			(GeditStatusComboBox 	*combo,
							 GtkMenuItem         	*item,
							 const gchar         	*text);
void gedit_status_combo_box_remove_item			(GeditStatusComboBox    *combo,
							 GtkMenuItem            *item);

GList *gedit_status_combo_box_get_items			(GeditStatusComboBox    *combo);
const gchar *gedit_status_combo_box_get_item_text 	(GeditStatusComboBox	*combo,
							 GtkMenuItem		*item);
void gedit_status_combo_box_set_item_text 		(GeditStatusComboBox	*combo,
							 GtkMenuItem		*item,
							 const gchar            *text);

void gedit_status_combo_box_set_item			(GeditStatusComboBox	*combo,
							 GtkMenuItem		*item);

GtkLabel *gedit_status_combo_box_get_item_label		(GeditStatusComboBox	*combo);

G_END_DECLS

#endif /* __GEDIT_STATUS_COMBO_BOX_H__ */
