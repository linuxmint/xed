/*
 * pluma-document-input-stream.h
 * This file is part of pluma
 *
 * Copyright (C) 2010 - Ignacio Casal Quinteiro
 *
 * pluma is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * pluma is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with pluma; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#ifndef __PLUMA_DOCUMENT_INPUT_STREAM_H__
#define __PLUMA_DOCUMENT_INPUT_STREAM_H__

#include <gio/gio.h>
#include <gtk/gtk.h>

#include "pluma-document.h"

G_BEGIN_DECLS

#define PLUMA_TYPE_DOCUMENT_INPUT_STREAM		(pluma_document_input_stream_get_type ())
#define PLUMA_DOCUMENT_INPUT_STREAM(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_DOCUMENT_INPUT_STREAM, PlumaDocumentInputStream))
#define PLUMA_DOCUMENT_INPUT_STREAM_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_DOCUMENT_INPUT_STREAM, PlumaDocumentInputStream const))
#define PLUMA_DOCUMENT_INPUT_STREAM_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PLUMA_TYPE_DOCUMENT_INPUT_STREAM, PlumaDocumentInputStreamClass))
#define PLUMA_IS_DOCUMENT_INPUT_STREAM(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUMA_TYPE_DOCUMENT_INPUT_STREAM))
#define PLUMA_IS_DOCUMENT_INPUT_STREAM_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_DOCUMENT_INPUT_STREAM))
#define PLUMA_DOCUMENT_INPUT_STREAM_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), PLUMA_TYPE_DOCUMENT_INPUT_STREAM, PlumaDocumentInputStreamClass))

typedef struct _PlumaDocumentInputStream	PlumaDocumentInputStream;
typedef struct _PlumaDocumentInputStreamClass	PlumaDocumentInputStreamClass;
typedef struct _PlumaDocumentInputStreamPrivate	PlumaDocumentInputStreamPrivate;

struct _PlumaDocumentInputStream
{
	GInputStream parent;
	
	PlumaDocumentInputStreamPrivate *priv;
};

struct _PlumaDocumentInputStreamClass
{
	GInputStreamClass parent_class;
};

GType				 pluma_document_input_stream_get_type		(void) G_GNUC_CONST;

GInputStream			*pluma_document_input_stream_new		(GtkTextBuffer           *buffer,
										 PlumaDocumentNewlineType type);

gsize				 pluma_document_input_stream_get_total_size	(PlumaDocumentInputStream *stream);

gsize				 pluma_document_input_stream_tell		(PlumaDocumentInputStream *stream);

G_END_DECLS

#endif /* __PLUMA_DOCUMENT_INPUT_STREAM_H__ */
