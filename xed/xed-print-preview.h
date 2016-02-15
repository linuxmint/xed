/*
 * xed-print-preview.h
 *
 * Copyright (C) 2008 Paolo Borelli
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
 * Modified by the xed Team, 1998-2006. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id: xed-commands-search.c 5931 2007-09-25 20:05:40Z pborelli $
 */


#ifndef __XED_PRINT_PREVIEW_H__
#define __XED_PRINT_PREVIEW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XED_TYPE_PRINT_PREVIEW            (xed_print_preview_get_type ())
#define XED_PRINT_PREVIEW(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), XED_TYPE_PRINT_PREVIEW, XedPrintPreview))
#define XED_PRINT_PREVIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_PRINT_PREVIEW, XedPrintPreviewClass))
#define XED_IS_PRINT_PREVIEW(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), XED_TYPE_PRINT_PREVIEW))
#define XED_IS_PRINT_PREVIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_PRINT_PREVIEW))
#define XED_PRINT_PREVIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XED_TYPE_PRINT_PREVIEW, XedPrintPreviewClass))

typedef struct _XedPrintPreview        XedPrintPreview;
typedef struct _XedPrintPreviewPrivate XedPrintPreviewPrivate;
typedef struct _XedPrintPreviewClass   XedPrintPreviewClass;

struct _XedPrintPreview
{
	GtkBox parent;

	XedPrintPreviewPrivate *priv;
};

struct _XedPrintPreviewClass
{
	GtkBoxClass parent_class;

	void (* close)		(XedPrintPreview          *preview);
};


GType		 xed_print_preview_get_type	(void) G_GNUC_CONST;

GtkWidget	*xed_print_preview_new	(GtkPrintOperation		*op,
						 GtkPrintOperationPreview	*gtk_preview,
						 GtkPrintContext		*context);

G_END_DECLS

#endif /* __XED_PRINT_PREVIEW_H__ */
