/*
 * pluma-close-confirmation-dialog.h
 * This file is part of pluma
 *
 * Copyright (C) 2004-2005 GNOME Foundation
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
 * Modified by the pluma Team, 2004-2005. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifndef __PLUMA_CLOSE_CONFIRMATION_DIALOG_H__
#define __PLUMA_CLOSE_CONFIRMATION_DIALOG_H__

#include <glib.h>
#include <gtk/gtk.h>

#include <pluma/pluma-document.h>

#define PLUMA_TYPE_CLOSE_CONFIRMATION_DIALOG		(pluma_close_confirmation_dialog_get_type ())
#define PLUMA_CLOSE_CONFIRMATION_DIALOG(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_CLOSE_CONFIRMATION_DIALOG, PlumaCloseConfirmationDialog))
#define PLUMA_CLOSE_CONFIRMATION_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PLUMA_TYPE_CLOSE_CONFIRMATION_DIALOG, PlumaCloseConfirmationDialogClass))
#define PLUMA_IS_CLOSE_CONFIRMATION_DIALOG(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUMA_TYPE_CLOSE_CONFIRMATION_DIALOG))
#define PLUMA_IS_CLOSE_CONFIRMATION_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_CLOSE_CONFIRMATION_DIALOG))
#define PLUMA_CLOSE_CONFIRMATION_DIALOG_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj),PLUMA_TYPE_CLOSE_CONFIRMATION_DIALOG, PlumaCloseConfirmationDialogClass))

typedef struct _PlumaCloseConfirmationDialog 		PlumaCloseConfirmationDialog;
typedef struct _PlumaCloseConfirmationDialogClass 	PlumaCloseConfirmationDialogClass;
typedef struct _PlumaCloseConfirmationDialogPrivate 	PlumaCloseConfirmationDialogPrivate;

struct _PlumaCloseConfirmationDialog 
{
	GtkDialog parent;

	/*< private > */
	PlumaCloseConfirmationDialogPrivate *priv;
};

struct _PlumaCloseConfirmationDialogClass 
{
	GtkDialogClass parent_class;
};

GType 		 pluma_close_confirmation_dialog_get_type		(void) G_GNUC_CONST;

GtkWidget	*pluma_close_confirmation_dialog_new			(GtkWindow     *parent,
									 GList         *unsaved_documents,
									 gboolean       logout_mode);
GtkWidget 	*pluma_close_confirmation_dialog_new_single 		(GtkWindow     *parent, 
									 PlumaDocument *doc,
 									 gboolean       logout_mode);

const GList	*pluma_close_confirmation_dialog_get_unsaved_documents  (PlumaCloseConfirmationDialog *dlg);

GList		*pluma_close_confirmation_dialog_get_selected_documents	(PlumaCloseConfirmationDialog *dlg);

#endif /* __PLUMA_CLOSE_CONFIRMATION_DIALOG_H__ */

