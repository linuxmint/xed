/*
 * xed-close-confirmation-dialog.h
 * This file is part of xed
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
 * Modified by the xed Team, 2004-2005. See the AUTHORS file for a 
 * list of people on the xed Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifndef __XED_CLOSE_CONFIRMATION_DIALOG_H__
#define __XED_CLOSE_CONFIRMATION_DIALOG_H__

#include <glib.h>
#include <gtk/gtk.h>

#include <xed/xed-document.h>

#define XED_TYPE_CLOSE_CONFIRMATION_DIALOG		(xed_close_confirmation_dialog_get_type ())
#define XED_CLOSE_CONFIRMATION_DIALOG(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_CLOSE_CONFIRMATION_DIALOG, XedCloseConfirmationDialog))
#define XED_CLOSE_CONFIRMATION_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_CLOSE_CONFIRMATION_DIALOG, XedCloseConfirmationDialogClass))
#define XED_IS_CLOSE_CONFIRMATION_DIALOG(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_CLOSE_CONFIRMATION_DIALOG))
#define XED_IS_CLOSE_CONFIRMATION_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_CLOSE_CONFIRMATION_DIALOG))
#define XED_CLOSE_CONFIRMATION_DIALOG_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj),XED_TYPE_CLOSE_CONFIRMATION_DIALOG, XedCloseConfirmationDialogClass))

typedef struct _XedCloseConfirmationDialog 		XedCloseConfirmationDialog;
typedef struct _XedCloseConfirmationDialogClass 	XedCloseConfirmationDialogClass;
typedef struct _XedCloseConfirmationDialogPrivate 	XedCloseConfirmationDialogPrivate;

struct _XedCloseConfirmationDialog 
{
	GtkDialog parent;

	/*< private > */
	XedCloseConfirmationDialogPrivate *priv;
};

struct _XedCloseConfirmationDialogClass 
{
	GtkDialogClass parent_class;
};

GType 		 xed_close_confirmation_dialog_get_type		(void) G_GNUC_CONST;

GtkWidget	*xed_close_confirmation_dialog_new			(GtkWindow     *parent,
									 GList         *unsaved_documents,
									 gboolean       logout_mode);
GtkWidget 	*xed_close_confirmation_dialog_new_single 		(GtkWindow     *parent, 
									 XedDocument *doc,
 									 gboolean       logout_mode);

const GList	*xed_close_confirmation_dialog_get_unsaved_documents  (XedCloseConfirmationDialog *dlg);

GList		*xed_close_confirmation_dialog_get_selected_documents	(XedCloseConfirmationDialog *dlg);

#endif /* __XED_CLOSE_CONFIRMATION_DIALOG_H__ */

