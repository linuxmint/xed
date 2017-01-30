/*
 * xed-file-chooser-dialog.h
 * This file is part of xed
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
 * Modified by the xed Team, 2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __XED_FILE_CHOOSER_DIALOG_H__
#define __XED_FILE_CHOOSER_DIALOG_H__

#include <gtksourceview/gtksource.h>

#include <xed/xed-enum-types.h>
#include <xed/xed-document.h>

G_BEGIN_DECLS

#define XED_TYPE_FILE_CHOOSER_DIALOG             (xed_file_chooser_dialog_get_type ())
#define XED_FILE_CHOOSER_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_FILE_CHOOSER_DIALOG, XedFileChooserDialog))
#define XED_FILE_CHOOSER_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_FILE_CHOOSER_DIALOG, XedFileChooserDialogClass))
#define XED_IS_FILE_CHOOSER_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_FILE_CHOOSER_DIALOG))
#define XED_IS_FILE_CHOOSER_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_FILE_CHOOSER_DIALOG))
#define XED_FILE_CHOOSER_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), XED_TYPE_FILE_CHOOSER_DIALOG, XedFileChooserDialogClass))

typedef struct _XedFileChooserDialog        XedFileChooserDialog;
typedef struct _XedFileChooserDialogPrivate XedFileChooserDialogPrivate;
typedef struct _XedFileChooserDialogClass   XedFileChooserDialogClass;

struct _XedFileChooserDialog
{
    GtkFileChooserDialog parent_instance;

    XedFileChooserDialogPrivate *priv;
};

struct _XedFileChooserDialogClass
{
    GtkFileChooserDialogClass parent_class;
};

GType xed_file_chooser_dialog_get_type (void) G_GNUC_CONST;

GtkWidget *xed_file_chooser_dialog_new (const gchar             *title,
                                        GtkWindow               *parent,
                                        GtkFileChooserAction     action,
                                        const GtkSourceEncoding *encoding,
                                        const gchar             *first_button_text,
                                        ...);

void xed_file_chooser_dialog_set_encoding (XedFileChooserDialog    *dialog,
                                           const GtkSourceEncoding *encoding);

const GtkSourceEncoding *xed_file_chooser_dialog_get_encoding (XedFileChooserDialog *dialog);

void xed_file_chooser_dialog_set_newline_type (XedFileChooserDialog *dialog,
                                               GtkSourceNewlineType  newline_type);

GtkSourceNewlineType xed_file_chooser_dialog_get_newline_type (XedFileChooserDialog *dialog);

G_END_DECLS

#endif /* __XED_FILE_CHOOSER_DIALOG_H__ */
