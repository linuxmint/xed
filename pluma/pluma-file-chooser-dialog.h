/*
 * pluma-file-chooser-dialog.h
 * This file is part of pluma
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
 * Modified by the pluma Team, 2005. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __PLUMA_FILE_CHOOSER_DIALOG_H__
#define __PLUMA_FILE_CHOOSER_DIALOG_H__

#include <gtk/gtk.h>

#include <pluma/pluma-encodings.h>
#include <pluma/pluma-enum-types.h>
#include <pluma/pluma-document.h>

G_BEGIN_DECLS

#define PLUMA_TYPE_FILE_CHOOSER_DIALOG             (pluma_file_chooser_dialog_get_type ())
#define PLUMA_FILE_CHOOSER_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_FILE_CHOOSER_DIALOG, PlumaFileChooserDialog))
#define PLUMA_FILE_CHOOSER_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), PLUMA_TYPE_FILE_CHOOSER_DIALOG, PlumaFileChooserDialogClass))
#define PLUMA_IS_FILE_CHOOSER_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUMA_TYPE_FILE_CHOOSER_DIALOG))
#define PLUMA_IS_FILE_CHOOSER_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_FILE_CHOOSER_DIALOG))
#define PLUMA_FILE_CHOOSER_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), PLUMA_TYPE_FILE_CHOOSER_DIALOG, PlumaFileChooserDialogClass))

typedef struct _PlumaFileChooserDialog      PlumaFileChooserDialog;
typedef struct _PlumaFileChooserDialogClass PlumaFileChooserDialogClass;

typedef struct _PlumaFileChooserDialogPrivate PlumaFileChooserDialogPrivate;

struct _PlumaFileChooserDialogClass
{
	GtkFileChooserDialogClass parent_class;
};

struct _PlumaFileChooserDialog
{
	GtkFileChooserDialog parent_instance;

	PlumaFileChooserDialogPrivate *priv;
};

GType		 pluma_file_chooser_dialog_get_type	(void) G_GNUC_CONST;

GtkWidget	*pluma_file_chooser_dialog_new		(const gchar            *title,
							 GtkWindow              *parent,
							 GtkFileChooserAction    action,
							 const PlumaEncoding    *encoding,
							 const gchar            *first_button_text,
							 ...);

void		 pluma_file_chooser_dialog_set_encoding (PlumaFileChooserDialog *dialog,
							 const PlumaEncoding    *encoding);

const PlumaEncoding
		*pluma_file_chooser_dialog_get_encoding (PlumaFileChooserDialog *dialog);

void		 pluma_file_chooser_dialog_set_newline_type (PlumaFileChooserDialog  *dialog,
							     PlumaDocumentNewlineType newline_type);

PlumaDocumentNewlineType
		 pluma_file_chooser_dialog_get_newline_type (PlumaFileChooserDialog *dialog);

G_END_DECLS

#endif /* __PLUMA_FILE_CHOOSER_DIALOG_H__ */
