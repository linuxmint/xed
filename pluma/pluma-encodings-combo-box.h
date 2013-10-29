/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pluma-encodings-combo-box.h
 * This file is part of pluma
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA 02110-1301, USA.
 */
 
/*
 * Modified by the pluma Team, 2003-2005. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id: pluma-encodings-option-menu.h 4429 2005-12-12 17:28:04Z pborelli $
 */

#ifndef __PLUMA_ENCODINGS_COMBO_BOX_H__
#define __PLUMA_ENCODINGS_COMBO_BOX_H__

#include <gtk/gtk.h>
#include <pluma/pluma-encodings.h>

G_BEGIN_DECLS

#define PLUMA_TYPE_ENCODINGS_COMBO_BOX             (pluma_encodings_combo_box_get_type ())
#define PLUMA_ENCODINGS_COMBO_BOX(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_ENCODINGS_COMBO_BOX, PlumaEncodingsComboBox))
#define PLUMA_ENCODINGS_COMBO_BOX_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), PLUMA_TYPE_ENCODINGS_COMBO_BOX, PlumaEncodingsComboBoxClass))
#define PLUMA_IS_ENCODINGS_COMBO_BOX(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUMA_TYPE_ENCODINGS_COMBO_BOX))
#define PLUMA_IS_ENCODINGS_COMBO_BOX_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_ENCODINGS_COMBO_BOX))
#define PLUMA_ENCODINGS_COMBO_BOX_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), PLUMA_TYPE_ENCODINGS_COMBO_BOX, PlumaEncodingsComboBoxClass))


typedef struct _PlumaEncodingsComboBox 	PlumaEncodingsComboBox;
typedef struct _PlumaEncodingsComboBoxClass 	PlumaEncodingsComboBoxClass;

typedef struct _PlumaEncodingsComboBoxPrivate	PlumaEncodingsComboBoxPrivate;

struct _PlumaEncodingsComboBox
{
	GtkComboBox			 parent;

	PlumaEncodingsComboBoxPrivate	*priv;
};

struct _PlumaEncodingsComboBoxClass
{
	GtkComboBoxClass		 parent_class;
};

GType		     pluma_encodings_combo_box_get_type		(void) G_GNUC_CONST;

/* Constructor */
GtkWidget 	    *pluma_encodings_combo_box_new 			(gboolean save_mode);

const PlumaEncoding *pluma_encodings_combo_box_get_selected_encoding	(PlumaEncodingsComboBox *menu);
void		     pluma_encodings_combo_box_set_selected_encoding	(PlumaEncodingsComboBox *menu,
									 const PlumaEncoding      *encoding);

G_END_DECLS

#endif /* __PLUMA_ENCODINGS_COMBO_BOX_H__ */


