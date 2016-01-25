/*
 * xedit-file-chooser-dialog.h
 * This file is part of xedit
 *
 * Copyright (C) 2005 - Paolo Maggi 
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
 * Modified by the xedit Team, 2005. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __XEDIT_FILE_CHOOSER_DIALOG_H__
#define __XEDIT_FILE_CHOOSER_DIALOG_H__

#include <gtk/gtk.h>

#include <xedit/xedit-encodings.h>
#include <xedit/xedit-enum-types.h>
#include <xedit/xedit-document.h>

G_BEGIN_DECLS

#define XEDIT_TYPE_FILE_CHOOSER_DIALOG             (xedit_file_chooser_dialog_get_type ())
#define XEDIT_FILE_CHOOSER_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_FILE_CHOOSER_DIALOG, XeditFileChooserDialog))
#define XEDIT_FILE_CHOOSER_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_FILE_CHOOSER_DIALOG, XeditFileChooserDialogClass))
#define XEDIT_IS_FILE_CHOOSER_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XEDIT_TYPE_FILE_CHOOSER_DIALOG))
#define XEDIT_IS_FILE_CHOOSER_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_FILE_CHOOSER_DIALOG))
#define XEDIT_FILE_CHOOSER_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), XEDIT_TYPE_FILE_CHOOSER_DIALOG, XeditFileChooserDialogClass))

typedef struct _XeditFileChooserDialog      XeditFileChooserDialog;
typedef struct _XeditFileChooserDialogClass XeditFileChooserDialogClass;

typedef struct _XeditFileChooserDialogPrivate XeditFileChooserDialogPrivate;

struct _XeditFileChooserDialogClass
{
	GtkFileChooserDialogClass parent_class;
};

struct _XeditFileChooserDialog
{
	GtkFileChooserDialog parent_instance;

	XeditFileChooserDialogPrivate *priv;
};

GType		 xedit_file_chooser_dialog_get_type	(void) G_GNUC_CONST;

GtkWidget	*xedit_file_chooser_dialog_new		(const gchar            *title,
							 GtkWindow              *parent,
							 GtkFileChooserAction    action,
							 const XeditEncoding    *encoding,
							 const gchar            *first_button_text,
							 ...);

void		 xedit_file_chooser_dialog_set_encoding (XeditFileChooserDialog *dialog,
							 const XeditEncoding    *encoding);

const XeditEncoding
		*xedit_file_chooser_dialog_get_encoding (XeditFileChooserDialog *dialog);

void		 xedit_file_chooser_dialog_set_newline_type (XeditFileChooserDialog  *dialog,
							     XeditDocumentNewlineType newline_type);

XeditDocumentNewlineType
		 xedit_file_chooser_dialog_get_newline_type (XeditFileChooserDialog *dialog);

G_END_DECLS

#endif /* __XEDIT_FILE_CHOOSER_DIALOG_H__ */
