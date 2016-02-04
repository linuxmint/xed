/*
 * xed-document-input-stream.h
 * This file is part of xed
 *
 * Copyright (C) 2010 - Ignacio Casal Quinteiro
 *
 * xed is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xed is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xed; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#ifndef __XED_DOCUMENT_INPUT_STREAM_H__
#define __XED_DOCUMENT_INPUT_STREAM_H__

#include <gio/gio.h>
#include <gtk/gtk.h>

#include "xed-document.h"

G_BEGIN_DECLS

#define XED_TYPE_DOCUMENT_INPUT_STREAM		(xed_document_input_stream_get_type ())
#define XED_DOCUMENT_INPUT_STREAM(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_DOCUMENT_INPUT_STREAM, XedDocumentInputStream))
#define XED_DOCUMENT_INPUT_STREAM_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_DOCUMENT_INPUT_STREAM, XedDocumentInputStream const))
#define XED_DOCUMENT_INPUT_STREAM_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_DOCUMENT_INPUT_STREAM, XedDocumentInputStreamClass))
#define XED_IS_DOCUMENT_INPUT_STREAM(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_DOCUMENT_INPUT_STREAM))
#define XED_IS_DOCUMENT_INPUT_STREAM_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_DOCUMENT_INPUT_STREAM))
#define XED_DOCUMENT_INPUT_STREAM_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), XED_TYPE_DOCUMENT_INPUT_STREAM, XedDocumentInputStreamClass))

typedef struct _XedDocumentInputStream	XedDocumentInputStream;
typedef struct _XedDocumentInputStreamClass	XedDocumentInputStreamClass;
typedef struct _XedDocumentInputStreamPrivate	XedDocumentInputStreamPrivate;

struct _XedDocumentInputStream
{
	GInputStream parent;
	
	XedDocumentInputStreamPrivate *priv;
};

struct _XedDocumentInputStreamClass
{
	GInputStreamClass parent_class;
};

GType				 xed_document_input_stream_get_type		(void) G_GNUC_CONST;

GInputStream			*xed_document_input_stream_new		(GtkTextBuffer           *buffer,
										 XedDocumentNewlineType type);

gsize				 xed_document_input_stream_get_total_size	(XedDocumentInputStream *stream);

gsize				 xed_document_input_stream_tell		(XedDocumentInputStream *stream);

G_END_DECLS

#endif /* __XED_DOCUMENT_INPUT_STREAM_H__ */
