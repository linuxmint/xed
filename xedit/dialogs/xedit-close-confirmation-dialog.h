/*
 * xedit-close-confirmation-dialog.h
 * This file is part of xedit
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
 * Modified by the xedit Team, 2004-2005. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifndef __XEDIT_CLOSE_CONFIRMATION_DIALOG_H__
#define __XEDIT_CLOSE_CONFIRMATION_DIALOG_H__

#include <glib.h>
#include <gtk/gtk.h>

#include <xedit/xedit-document.h>

#define XEDIT_TYPE_CLOSE_CONFIRMATION_DIALOG		(xedit_close_confirmation_dialog_get_type ())
#define XEDIT_CLOSE_CONFIRMATION_DIALOG(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_CLOSE_CONFIRMATION_DIALOG, XeditCloseConfirmationDialog))
#define XEDIT_CLOSE_CONFIRMATION_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_CLOSE_CONFIRMATION_DIALOG, XeditCloseConfirmationDialogClass))
#define XEDIT_IS_CLOSE_CONFIRMATION_DIALOG(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XEDIT_TYPE_CLOSE_CONFIRMATION_DIALOG))
#define XEDIT_IS_CLOSE_CONFIRMATION_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_CLOSE_CONFIRMATION_DIALOG))
#define XEDIT_CLOSE_CONFIRMATION_DIALOG_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj),XEDIT_TYPE_CLOSE_CONFIRMATION_DIALOG, XeditCloseConfirmationDialogClass))

typedef struct _XeditCloseConfirmationDialog 		XeditCloseConfirmationDialog;
typedef struct _XeditCloseConfirmationDialogClass 	XeditCloseConfirmationDialogClass;
typedef struct _XeditCloseConfirmationDialogPrivate 	XeditCloseConfirmationDialogPrivate;

struct _XeditCloseConfirmationDialog 
{
	GtkDialog parent;

	/*< private > */
	XeditCloseConfirmationDialogPrivate *priv;
};

struct _XeditCloseConfirmationDialogClass 
{
	GtkDialogClass parent_class;
};

GType 		 xedit_close_confirmation_dialog_get_type		(void) G_GNUC_CONST;

GtkWidget	*xedit_close_confirmation_dialog_new			(GtkWindow     *parent,
									 GList         *unsaved_documents,
									 gboolean       logout_mode);
GtkWidget 	*xedit_close_confirmation_dialog_new_single 		(GtkWindow     *parent, 
									 XeditDocument *doc,
 									 gboolean       logout_mode);

const GList	*xedit_close_confirmation_dialog_get_unsaved_documents  (XeditCloseConfirmationDialog *dlg);

GList		*xedit_close_confirmation_dialog_get_selected_documents	(XeditCloseConfirmationDialog *dlg);

#endif /* __XEDIT_CLOSE_CONFIRMATION_DIALOG_H__ */

