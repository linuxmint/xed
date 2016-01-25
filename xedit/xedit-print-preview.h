/*
 * xedit-print-preview.h
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
 * Modified by the xedit Team, 1998-2006. See the AUTHORS file for a
 * list of people on the xedit Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id: xedit-commands-search.c 5931 2007-09-25 20:05:40Z pborelli $
 */


#ifndef __XEDIT_PRINT_PREVIEW_H__
#define __XEDIT_PRINT_PREVIEW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XEDIT_TYPE_PRINT_PREVIEW            (xedit_print_preview_get_type ())
#define XEDIT_PRINT_PREVIEW(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), XEDIT_TYPE_PRINT_PREVIEW, XeditPrintPreview))
#define XEDIT_PRINT_PREVIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_PRINT_PREVIEW, XeditPrintPreviewClass))
#define XEDIT_IS_PRINT_PREVIEW(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), XEDIT_TYPE_PRINT_PREVIEW))
#define XEDIT_IS_PRINT_PREVIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_PRINT_PREVIEW))
#define XEDIT_PRINT_PREVIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XEDIT_TYPE_PRINT_PREVIEW, XeditPrintPreviewClass))

typedef struct _XeditPrintPreview        XeditPrintPreview;
typedef struct _XeditPrintPreviewPrivate XeditPrintPreviewPrivate;
typedef struct _XeditPrintPreviewClass   XeditPrintPreviewClass;

struct _XeditPrintPreview
{
#if GTK_CHECK_VERSION (3, 0, 0)
	GtkBox parent;
#else
	GtkVBox parent;
#endif
	XeditPrintPreviewPrivate *priv;
};

struct _XeditPrintPreviewClass
{
#if GTK_CHECK_VERSION (3, 0, 0)
	GtkBoxClass parent_class;
#else
	GtkVBoxClass parent_class;
#endif

	void (* close)		(XeditPrintPreview          *preview);
};


GType		 xedit_print_preview_get_type	(void) G_GNUC_CONST;

GtkWidget	*xedit_print_preview_new	(GtkPrintOperation		*op,
						 GtkPrintOperationPreview	*gtk_preview,
						 GtkPrintContext		*context);

G_END_DECLS

#endif /* __XEDIT_PRINT_PREVIEW_H__ */
