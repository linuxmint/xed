/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gedit-encodings-combo-box.h
 * This file is part of gedit
 *
 * Copyright (C) 2003-2005 - Paolo Maggi
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
 
/*
 * Modified by the gedit Team, 2003-2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id: gedit-encodings-option-menu.h 4429 2005-12-12 17:28:04Z pborelli $
 */

#ifndef __GEDIT_ENCODINGS_COMBO_BOX_H__
#define __GEDIT_ENCODINGS_COMBO_BOX_H__

#include <gtk/gtkoptionmenu.h>
#include <gedit/gedit-encodings.h>

G_BEGIN_DECLS

#define GEDIT_TYPE_ENCODINGS_COMBO_BOX             (gedit_encodings_combo_box_get_type ())
#define GEDIT_ENCODINGS_COMBO_BOX(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_ENCODINGS_COMBO_BOX, GeditEncodingsComboBox))
#define GEDIT_ENCODINGS_COMBO_BOX_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_ENCODINGS_COMBO_BOX, GeditEncodingsComboBoxClass))
#define GEDIT_IS_ENCODINGS_COMBO_BOX(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_ENCODINGS_COMBO_BOX))
#define GEDIT_IS_ENCODINGS_COMBO_BOX_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_ENCODINGS_COMBO_BOX))
#define GEDIT_ENCODINGS_COMBO_BOX_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_TYPE_ENCODINGS_COMBO_BOX, GeditEncodingsComboBoxClass))


typedef struct _GeditEncodingsComboBox 	GeditEncodingsComboBox;
typedef struct _GeditEncodingsComboBoxClass 	GeditEncodingsComboBoxClass;

typedef struct _GeditEncodingsComboBoxPrivate	GeditEncodingsComboBoxPrivate;

struct _GeditEncodingsComboBox
{
	GtkComboBox			 parent;

	GeditEncodingsComboBoxPrivate	*priv;
};

struct _GeditEncodingsComboBoxClass
{
	GtkComboBoxClass		 parent_class;
};

GType		     gedit_encodings_combo_box_get_type		(void) G_GNUC_CONST;

/* Constructor */
GtkWidget 	    *gedit_encodings_combo_box_new 			(gboolean save_mode);

const GeditEncoding *gedit_encodings_combo_box_get_selected_encoding	(GeditEncodingsComboBox *menu);
void		     gedit_encodings_combo_box_set_selected_encoding	(GeditEncodingsComboBox *menu,
									 const GeditEncoding      *encoding);

G_END_DECLS

#endif /* __GEDIT_ENCODINGS_COMBO_BOX_H__ */


