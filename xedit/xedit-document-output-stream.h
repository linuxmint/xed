/*
 * xedit-document-output-stream.h
 * This file is part of xedit
 *
 * Copyright (C) 2010 - Ignacio Casal Quinteiro
 *
 * xedit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xedit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xedit; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */


#ifndef __XEDIT_DOCUMENT_OUTPUT_STREAM_H__
#define __XEDIT_DOCUMENT_OUTPUT_STREAM_H__

#include <gio/gio.h>
#include "xedit-document.h"

G_BEGIN_DECLS

#define XEDIT_TYPE_DOCUMENT_OUTPUT_STREAM		(xedit_document_output_stream_get_type ())
#define XEDIT_DOCUMENT_OUTPUT_STREAM(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_DOCUMENT_OUTPUT_STREAM, XeditDocumentOutputStream))
#define XEDIT_DOCUMENT_OUTPUT_STREAM_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_DOCUMENT_OUTPUT_STREAM, XeditDocumentOutputStream const))
#define XEDIT_DOCUMENT_OUTPUT_STREAM_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_DOCUMENT_OUTPUT_STREAM, XeditDocumentOutputStreamClass))
#define XEDIT_IS_DOCUMENT_OUTPUT_STREAM(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XEDIT_TYPE_DOCUMENT_OUTPUT_STREAM))
#define XEDIT_IS_DOCUMENT_OUTPUT_STREAM_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_DOCUMENT_OUTPUT_STREAM))
#define XEDIT_DOCUMENT_OUTPUT_STREAM_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), XEDIT_TYPE_DOCUMENT_OUTPUT_STREAM, XeditDocumentOutputStreamClass))

typedef struct _XeditDocumentOutputStream		XeditDocumentOutputStream;
typedef struct _XeditDocumentOutputStreamClass		XeditDocumentOutputStreamClass;
typedef struct _XeditDocumentOutputStreamPrivate	XeditDocumentOutputStreamPrivate;

struct _XeditDocumentOutputStream
{
	GOutputStream parent;

	XeditDocumentOutputStreamPrivate *priv;
};

struct _XeditDocumentOutputStreamClass
{
	GOutputStreamClass parent_class;
};

GType			 xedit_document_output_stream_get_type		(void) G_GNUC_CONST;

GOutputStream		*xedit_document_output_stream_new		(XeditDocument *doc);

XeditDocumentNewlineType xedit_document_output_stream_detect_newline_type (XeditDocumentOutputStream *stream);

G_END_DECLS

#endif /* __XEDIT_DOCUMENT_OUTPUT_STREAM_H__ */
