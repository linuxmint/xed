/*
 * gedit-close-confirmation-dialog.h
 * This file is part of gedit
 *
 * Copyright (C) 2004-2005 MATE Foundation 
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
 * Modified by the gedit Team, 2004-2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifndef __GEDIT_CLOSE_CONFIRMATION_DIALOG_H__
#define __GEDIT_CLOSE_CONFIRMATION_DIALOG_H__

#include <glib.h>
#include <gtk/gtk.h>

#include <gedit/gedit-document.h>

#define GEDIT_TYPE_CLOSE_CONFIRMATION_DIALOG		(gedit_close_confirmation_dialog_get_type ())
#define GEDIT_CLOSE_CONFIRMATION_DIALOG(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_CLOSE_CONFIRMATION_DIALOG, GeditCloseConfirmationDialog))
#define GEDIT_CLOSE_CONFIRMATION_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_CLOSE_CONFIRMATION_DIALOG, GeditCloseConfirmationDialogClass))
#define GEDIT_IS_CLOSE_CONFIRMATION_DIALOG(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_CLOSE_CONFIRMATION_DIALOG))
#define GEDIT_IS_CLOSE_CONFIRMATION_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_CLOSE_CONFIRMATION_DIALOG))
#define GEDIT_CLOSE_CONFIRMATION_DIALOG_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj),GEDIT_TYPE_CLOSE_CONFIRMATION_DIALOG, GeditCloseConfirmationDialogClass))

typedef struct _GeditCloseConfirmationDialog 		GeditCloseConfirmationDialog;
typedef struct _GeditCloseConfirmationDialogClass 	GeditCloseConfirmationDialogClass;
typedef struct _GeditCloseConfirmationDialogPrivate 	GeditCloseConfirmationDialogPrivate;

struct _GeditCloseConfirmationDialog 
{
	GtkDialog parent;

	/*< private > */
	GeditCloseConfirmationDialogPrivate *priv;
};

struct _GeditCloseConfirmationDialogClass 
{
	GtkDialogClass parent_class;
};

GType 		 gedit_close_confirmation_dialog_get_type		(void) G_GNUC_CONST;

GtkWidget	*gedit_close_confirmation_dialog_new			(GtkWindow     *parent,
									 GList         *unsaved_documents,
									 gboolean       logout_mode);
GtkWidget 	*gedit_close_confirmation_dialog_new_single 		(GtkWindow     *parent, 
									 GeditDocument *doc,
 									 gboolean       logout_mode);

const GList	*gedit_close_confirmation_dialog_get_unsaved_documents  (GeditCloseConfirmationDialog *dlg);

GList		*gedit_close_confirmation_dialog_get_selected_documents	(GeditCloseConfirmationDialog *dlg);

#endif /* __GEDIT_CLOSE_CONFIRMATION_DIALOG_H__ */

