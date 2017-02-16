/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xed-encodings-combo-box.h
 * This file is part of xed
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
 * Modified by the xed Team, 2003-2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id: xed-encodings-option-menu.h 4429 2005-12-12 17:28:04Z pborelli $
 */

#ifndef __XED_ENCODINGS_COMBO_BOX_H__
#define __XED_ENCODINGS_COMBO_BOX_H__

#include <gtksourceview/gtksource.h>

G_BEGIN_DECLS

#define XED_TYPE_ENCODINGS_COMBO_BOX             (xed_encodings_combo_box_get_type ())
#define XED_ENCODINGS_COMBO_BOX(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_ENCODINGS_COMBO_BOX, XedEncodingsComboBox))
#define XED_ENCODINGS_COMBO_BOX_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_ENCODINGS_COMBO_BOX, XedEncodingsComboBoxClass))
#define XED_IS_ENCODINGS_COMBO_BOX(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_ENCODINGS_COMBO_BOX))
#define XED_IS_ENCODINGS_COMBO_BOX_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_ENCODINGS_COMBO_BOX))
#define XED_ENCODINGS_COMBO_BOX_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), XED_TYPE_ENCODINGS_COMBO_BOX, XedEncodingsComboBoxClass))


typedef struct _XedEncodingsComboBox        XedEncodingsComboBox;
typedef struct _XedEncodingsComboBoxClass   XedEncodingsComboBoxClass;
typedef struct _XedEncodingsComboBoxPrivate XedEncodingsComboBoxPrivate;

struct _XedEncodingsComboBox
{
    GtkComboBox parent;

    XedEncodingsComboBoxPrivate *priv;
};

struct _XedEncodingsComboBoxClass
{
    GtkComboBoxClass parent_class;
};

GType xed_encodings_combo_box_get_type (void) G_GNUC_CONST;

/* Constructor */
GtkWidget *xed_encodings_combo_box_new (gboolean save_mode);

const GtkSourceEncoding *xed_encodings_combo_box_get_selected_encoding (XedEncodingsComboBox *menu);
void xed_encodings_combo_box_set_selected_encoding  (XedEncodingsComboBox    *menu,
                                                     const GtkSourceEncoding *encoding);

G_END_DECLS

#endif /* __XED_ENCODINGS_COMBO_BOX_H__ */


