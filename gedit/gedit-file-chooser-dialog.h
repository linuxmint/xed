/*
 * gedit-file-chooser-dialog.h
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */
 
/*
 * Modified by the gedit Team, 2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __GEDIT_FILE_CHOOSER_DIALOG_H__
#define __GEDIT_FILE_CHOOSER_DIALOG_H__

#include <gtk/gtk.h>

#include <gedit/gedit-encodings.h>
#include <gedit/gedit-enum-types.h>
#include <gedit/gedit-document.h>

G_BEGIN_DECLS

#define GEDIT_TYPE_FILE_CHOOSER_DIALOG             (gedit_file_chooser_dialog_get_type ())
#define GEDIT_FILE_CHOOSER_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_FILE_CHOOSER_DIALOG, GeditFileChooserDialog))
#define GEDIT_FILE_CHOOSER_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_FILE_CHOOSER_DIALOG, GeditFileChooserDialogClass))
#define GEDIT_IS_FILE_CHOOSER_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_FILE_CHOOSER_DIALOG))
#define GEDIT_IS_FILE_CHOOSER_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_FILE_CHOOSER_DIALOG))
#define GEDIT_FILE_CHOOSER_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_TYPE_FILE_CHOOSER_DIALOG, GeditFileChooserDialogClass))

typedef struct _GeditFileChooserDialog      GeditFileChooserDialog;
typedef struct _GeditFileChooserDialogClass GeditFileChooserDialogClass;

typedef struct _GeditFileChooserDialogPrivate GeditFileChooserDialogPrivate;

struct _GeditFileChooserDialogClass
{
	GtkFileChooserDialogClass parent_class;
};

struct _GeditFileChooserDialog
{
	GtkFileChooserDialog parent_instance;

	GeditFileChooserDialogPrivate *priv;
};

GType		 gedit_file_chooser_dialog_get_type	(void) G_GNUC_CONST;

GtkWidget	*gedit_file_chooser_dialog_new		(const gchar            *title,
							 GtkWindow              *parent,
							 GtkFileChooserAction    action,
							 const GeditEncoding    *encoding,
							 const gchar            *first_button_text,
							 ...);

void		 gedit_file_chooser_dialog_set_encoding (GeditFileChooserDialog *dialog,
							 const GeditEncoding    *encoding);

const GeditEncoding
		*gedit_file_chooser_dialog_get_encoding (GeditFileChooserDialog *dialog);

void		 gedit_file_chooser_dialog_set_newline_type (GeditFileChooserDialog  *dialog,
							     GeditDocumentNewlineType newline_type);

GeditDocumentNewlineType
		 gedit_file_chooser_dialog_get_newline_type (GeditFileChooserDialog *dialog);

G_END_DECLS

#endif /* __GEDIT_FILE_CHOOSER_DIALOG_H__ */
