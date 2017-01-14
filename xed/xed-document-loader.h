/*
 * xed-document-loader.h
 * This file is part of xed
 *
 * Copyright (C) 2005 - Paolo Maggi
 * Copyright (C) 2007 - Paolo Maggi, Steve Frécinaux
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
 * Modified by the xed Team, 2005-2007. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __XED_DOCUMENT_LOADER_H__
#define __XED_DOCUMENT_LOADER_H__

#include <xed/xed-document.h>

G_BEGIN_DECLS

#define XED_TYPE_DOCUMENT_LOADER              (xed_document_loader_get_type())
#define XED_DOCUMENT_LOADER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_DOCUMENT_LOADER, XedDocumentLoader))
#define XED_DOCUMENT_LOADER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_DOCUMENT_LOADER, XedDocumentLoaderClass))
#define XED_IS_DOCUMENT_LOADER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_DOCUMENT_LOADER))
#define XED_IS_DOCUMENT_LOADER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_DOCUMENT_LOADER))
#define XED_DOCUMENT_LOADER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_DOCUMENT_LOADER, XedDocumentLoaderClass))

typedef struct _XedDocumentLoader           XedDocumentLoader;
typedef struct _XedDocumentLoaderPrivate    XedDocumentLoaderPrivate;
typedef struct _XedDocumentLoaderClass      XedDocumentLoaderClass;

struct _XedDocumentLoader
{
    GObject object;

    XedDocumentLoaderPrivate *priv;
};

struct _XedDocumentLoaderClass
{
    GObjectClass parent_class;

    /* Signals */
    void (* loading) (XedDocumentLoader *loader,
                      gboolean           completed,
                      const GError      *error);
};

GType xed_document_loader_get_type (void) G_GNUC_CONST;

/* If enconding == NULL, the encoding will be autodetected */
XedDocumentLoader *xed_document_loader_new (XedDocument       *doc,
                                            GFile             *location,
                                            const XedEncoding *encoding);

void xed_document_loader_loading (XedDocumentLoader *loader,
                                  gboolean           completed,
                                  GError            *error);

void xed_document_loader_load (XedDocumentLoader *loader);
#if 0
gboolean         xed_document_loader_load_from_stdin    (XedDocumentLoader *loader);
#endif
gboolean xed_document_loader_cancel (XedDocumentLoader *loader);

XedDocument *xed_document_loader_get_document (XedDocumentLoader *loader);

/* Returns STDIN_URI if loading from stdin */
#define STDIN_URI "stdin:"
GFile *xed_document_loader_get_location (XedDocumentLoader *loader);

const XedEncoding *xed_document_loader_get_encoding (XedDocumentLoader *loader);

XedDocumentNewlineType xed_document_loader_get_newline_type (XedDocumentLoader *loader);

goffset xed_document_loader_get_bytes_read (XedDocumentLoader *loader);

/* You can get from the info: content_type, time_modified, standard_size, access_can_write
   and also the metadata*/
GFileInfo *xed_document_loader_get_info (XedDocumentLoader *loader);

G_END_DECLS

#endif  /* __XED_DOCUMENT_LOADER_H__  */
