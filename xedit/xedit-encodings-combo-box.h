/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xedit-encodings-combo-box.h
 * This file is part of xedit
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
 * Modified by the xedit Team, 2003-2005. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id: xedit-encodings-option-menu.h 4429 2005-12-12 17:28:04Z pborelli $
 */

#ifndef __XEDIT_ENCODINGS_COMBO_BOX_H__
#define __XEDIT_ENCODINGS_COMBO_BOX_H__

#include <gtk/gtk.h>
#include <xedit/xedit-encodings.h>

G_BEGIN_DECLS

#define XEDIT_TYPE_ENCODINGS_COMBO_BOX             (xedit_encodings_combo_box_get_type ())
#define XEDIT_ENCODINGS_COMBO_BOX(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_ENCODINGS_COMBO_BOX, XeditEncodingsComboBox))
#define XEDIT_ENCODINGS_COMBO_BOX_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_ENCODINGS_COMBO_BOX, XeditEncodingsComboBoxClass))
#define XEDIT_IS_ENCODINGS_COMBO_BOX(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XEDIT_TYPE_ENCODINGS_COMBO_BOX))
#define XEDIT_IS_ENCODINGS_COMBO_BOX_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_ENCODINGS_COMBO_BOX))
#define XEDIT_ENCODINGS_COMBO_BOX_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), XEDIT_TYPE_ENCODINGS_COMBO_BOX, XeditEncodingsComboBoxClass))


typedef struct _XeditEncodingsComboBox 	XeditEncodingsComboBox;
typedef struct _XeditEncodingsComboBoxClass 	XeditEncodingsComboBoxClass;

typedef struct _XeditEncodingsComboBoxPrivate	XeditEncodingsComboBoxPrivate;

struct _XeditEncodingsComboBox
{
	GtkComboBox			 parent;

	XeditEncodingsComboBoxPrivate	*priv;
};

struct _XeditEncodingsComboBoxClass
{
	GtkComboBoxClass		 parent_class;
};

GType		     xedit_encodings_combo_box_get_type		(void) G_GNUC_CONST;

/* Constructor */
GtkWidget 	    *xedit_encodings_combo_box_new 			(gboolean save_mode);

const XeditEncoding *xedit_encodings_combo_box_get_selected_encoding	(XeditEncodingsComboBox *menu);
void		     xedit_encodings_combo_box_set_selected_encoding	(XeditEncodingsComboBox *menu,
									 const XeditEncoding      *encoding);

G_END_DECLS

#endif /* __XEDIT_ENCODINGS_COMBO_BOX_H__ */


