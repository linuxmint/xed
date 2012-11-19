/*
 * pluma-encodings-dialog.h
 * This file is part of pluma
 *
 * Copyright (C) 2003-2005 Paolo Maggi 
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
 * $Id$
 */

#ifndef __PLUMA_ENCODINGS_DIALOG_H__
#define __PLUMA_ENCODINGS_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_ENCODINGS_DIALOG              (pluma_encodings_dialog_get_type())
#define PLUMA_ENCODINGS_DIALOG(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_ENCODINGS_DIALOG, PlumaEncodingsDialog))
#define PLUMA_ENCODINGS_DIALOG_CONST(obj)        (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_ENCODINGS_DIALOG, PlumaEncodingsDialog const))
#define PLUMA_ENCODINGS_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), PLUMA_TYPE_ENCODINGS_DIALOG, PlumaEncodingsDialogClass))
#define PLUMA_IS_ENCODINGS_DIALOG(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), PLUMA_TYPE_ENCODINGS_DIALOG))
#define PLUMA_IS_ENCODINGS_DIALOG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_ENCODINGS_DIALOG))
#define PLUMA_ENCODINGS_DIALOG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), PLUMA_TYPE_ENCODINGS_DIALOG, PlumaEncodingsDialogClass))


/* Private structure type */
typedef struct _PlumaEncodingsDialogPrivate PlumaEncodingsDialogPrivate;

/*
 * Main object structure
 */
typedef struct _PlumaEncodingsDialog PlumaEncodingsDialog;

struct _PlumaEncodingsDialog 
{
	GtkDialog dialog;

	/*< private > */
	PlumaEncodingsDialogPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _PlumaEncodingsDialogClass PlumaEncodingsDialogClass;

struct _PlumaEncodingsDialogClass 
{
	GtkDialogClass parent_class;
};

/*
 * Public methods
 */
GType		 pluma_encodings_dialog_get_type	(void) G_GNUC_CONST;

GtkWidget	*pluma_encodings_dialog_new		(void);

G_END_DECLS

#endif /* __PLUMA_ENCODINGS_DIALOG_H__ */

