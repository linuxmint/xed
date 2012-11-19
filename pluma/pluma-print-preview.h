/*
 * pluma-print-preview.h
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
 * Modified by the pluma Team, 1998-2006. See the AUTHORS file for a
 * list of people on the pluma Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id: pluma-commands-search.c 5931 2007-09-25 20:05:40Z pborelli $
 */


#ifndef __PLUMA_PRINT_PREVIEW_H__
#define __PLUMA_PRINT_PREVIEW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PLUMA_TYPE_PRINT_PREVIEW            (pluma_print_preview_get_type ())
#define PLUMA_PRINT_PREVIEW(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), PLUMA_TYPE_PRINT_PREVIEW, PlumaPrintPreview))
#define PLUMA_PRINT_PREVIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PLUMA_TYPE_PRINT_PREVIEW, PlumaPrintPreviewClass))
#define PLUMA_IS_PRINT_PREVIEW(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), PLUMA_TYPE_PRINT_PREVIEW))
#define PLUMA_IS_PRINT_PREVIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_PRINT_PREVIEW))
#define PLUMA_PRINT_PREVIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PLUMA_TYPE_PRINT_PREVIEW, PlumaPrintPreviewClass))

typedef struct _PlumaPrintPreview        PlumaPrintPreview;
typedef struct _PlumaPrintPreviewPrivate PlumaPrintPreviewPrivate;
typedef struct _PlumaPrintPreviewClass   PlumaPrintPreviewClass;

struct _PlumaPrintPreview
{
	GtkVBox parent;
	PlumaPrintPreviewPrivate *priv;
};

struct _PlumaPrintPreviewClass
{
	GtkVBoxClass parent_class;

	void (* close)		(PlumaPrintPreview          *preview);
};


GType		 pluma_print_preview_get_type	(void) G_GNUC_CONST;

GtkWidget	*pluma_print_preview_new	(GtkPrintOperation		*op,
						 GtkPrintOperationPreview	*gtk_preview,
						 GtkPrintContext		*context);

G_END_DECLS

#endif /* __PLUMA_PRINT_PREVIEW_H__ */
