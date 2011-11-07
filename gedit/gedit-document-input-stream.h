/*
 * gedit-document-input-stream.h
 * This file is part of gedit
 *
 * Copyright (C) 2010 - Ignacio Casal Quinteiro
 *
 * gedit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * gedit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gedit; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#ifndef __GEDIT_DOCUMENT_INPUT_STREAM_H__
#define __GEDIT_DOCUMENT_INPUT_STREAM_H__

#include <gio/gio.h>
#include <gtk/gtk.h>

#include "gedit-document.h"

G_BEGIN_DECLS

#define GEDIT_TYPE_DOCUMENT_INPUT_STREAM		(gedit_document_input_stream_get_type ())
#define GEDIT_DOCUMENT_INPUT_STREAM(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_DOCUMENT_INPUT_STREAM, GeditDocumentInputStream))
#define GEDIT_DOCUMENT_INPUT_STREAM_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_DOCUMENT_INPUT_STREAM, GeditDocumentInputStream const))
#define GEDIT_DOCUMENT_INPUT_STREAM_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_DOCUMENT_INPUT_STREAM, GeditDocumentInputStreamClass))
#define GEDIT_IS_DOCUMENT_INPUT_STREAM(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_DOCUMENT_INPUT_STREAM))
#define GEDIT_IS_DOCUMENT_INPUT_STREAM_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_DOCUMENT_INPUT_STREAM))
#define GEDIT_DOCUMENT_INPUT_STREAM_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_TYPE_DOCUMENT_INPUT_STREAM, GeditDocumentInputStreamClass))

typedef struct _GeditDocumentInputStream	GeditDocumentInputStream;
typedef struct _GeditDocumentInputStreamClass	GeditDocumentInputStreamClass;
typedef struct _GeditDocumentInputStreamPrivate	GeditDocumentInputStreamPrivate;

struct _GeditDocumentInputStream
{
	GInputStream parent;
	
	GeditDocumentInputStreamPrivate *priv;
};

struct _GeditDocumentInputStreamClass
{
	GInputStreamClass parent_class;
};

GType				 gedit_document_input_stream_get_type		(void) G_GNUC_CONST;

GInputStream			*gedit_document_input_stream_new		(GtkTextBuffer           *buffer,
										 GeditDocumentNewlineType type);

gsize				 gedit_document_input_stream_get_total_size	(GeditDocumentInputStream *stream);

gsize				 gedit_document_input_stream_tell		(GeditDocumentInputStream *stream);

G_END_DECLS

#endif /* __GEDIT_DOCUMENT_INPUT_STREAM_H__ */
