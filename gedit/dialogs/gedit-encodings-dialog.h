/*
 * gedit-encodings-dialog.h
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */

/*
 * Modified by the gedit Team, 2003-2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __GEDIT_ENCODINGS_DIALOG_H__
#define __GEDIT_ENCODINGS_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GEDIT_TYPE_ENCODINGS_DIALOG              (gedit_encodings_dialog_get_type())
#define GEDIT_ENCODINGS_DIALOG(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GEDIT_TYPE_ENCODINGS_DIALOG, GeditEncodingsDialog))
#define GEDIT_ENCODINGS_DIALOG_CONST(obj)        (G_TYPE_CHECK_INSTANCE_CAST((obj), GEDIT_TYPE_ENCODINGS_DIALOG, GeditEncodingsDialog const))
#define GEDIT_ENCODINGS_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GEDIT_TYPE_ENCODINGS_DIALOG, GeditEncodingsDialogClass))
#define GEDIT_IS_ENCODINGS_DIALOG(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GEDIT_TYPE_ENCODINGS_DIALOG))
#define GEDIT_IS_ENCODINGS_DIALOG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_ENCODINGS_DIALOG))
#define GEDIT_ENCODINGS_DIALOG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GEDIT_TYPE_ENCODINGS_DIALOG, GeditEncodingsDialogClass))


/* Private structure type */
typedef struct _GeditEncodingsDialogPrivate GeditEncodingsDialogPrivate;

/*
 * Main object structure
 */
typedef struct _GeditEncodingsDialog GeditEncodingsDialog;

struct _GeditEncodingsDialog 
{
	GtkDialog dialog;

	/*< private > */
	GeditEncodingsDialogPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GeditEncodingsDialogClass GeditEncodingsDialogClass;

struct _GeditEncodingsDialogClass 
{
	GtkDialogClass parent_class;
};

/*
 * Public methods
 */
GType		 gedit_encodings_dialog_get_type	(void) G_GNUC_CONST;

GtkWidget	*gedit_encodings_dialog_new		(void);

G_END_DECLS

#endif /* __GEDIT_ENCODINGS_DIALOG_H__ */

