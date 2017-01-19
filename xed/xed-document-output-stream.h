/*
 * xed-document-output-stream.h
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


#ifndef __XED_DOCUMENT_OUTPUT_STREAM_H__
#define __XED_DOCUMENT_OUTPUT_STREAM_H__

#include <gio/gio.h>
#include "xed-document.h"
#include "xed-encodings.h"

G_BEGIN_DECLS

#define XED_TYPE_DOCUMENT_OUTPUT_STREAM             (xed_document_output_stream_get_type ())
#define XED_DOCUMENT_OUTPUT_STREAM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_DOCUMENT_OUTPUT_STREAM, XedDocumentOutputStream))
#define XED_DOCUMENT_OUTPUT_STREAM_CONST(obj)       (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_DOCUMENT_OUTPUT_STREAM, XedDocumentOutputStream const))
#define XED_DOCUMENT_OUTPUT_STREAM_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_DOCUMENT_OUTPUT_STREAM, XedDocumentOutputStreamClass))
#define XED_IS_DOCUMENT_OUTPUT_STREAM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_DOCUMENT_OUTPUT_STREAM))
#define XED_IS_DOCUMENT_OUTPUT_STREAM_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_DOCUMENT_OUTPUT_STREAM))
#define XED_DOCUMENT_OUTPUT_STREAM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), XED_TYPE_DOCUMENT_OUTPUT_STREAM, XedDocumentOutputStreamClass))

typedef struct _XedDocumentOutputStream         XedDocumentOutputStream;
typedef struct _XedDocumentOutputStreamPrivate  XedDocumentOutputStreamPrivate;
typedef struct _XedDocumentOutputStreamClass    XedDocumentOutputStreamClass;

struct _XedDocumentOutputStream
{
    GOutputStream parent;

    XedDocumentOutputStreamPrivate *priv;
};

struct _XedDocumentOutputStreamClass
{
    GOutputStreamClass parent_class;
};

GType xed_document_output_stream_get_type (void) G_GNUC_CONST;

GOutputStream *xed_document_output_stream_new (XedDocument *doc,
                                               GSList      *candidate_encodings);

XedDocumentNewlineType xed_document_output_stream_detect_newline_type (XedDocumentOutputStream *stream);

const XedEncoding *xed_document_output_stream_get_guessed (XedDocumentOutputStream *stream);

guint xed_document_output_stream_get_num_fallbacks (XedDocumentOutputStream *stream);

G_END_DECLS

#endif /* __XED_DOCUMENT_OUTPUT_STREAM_H__ */
