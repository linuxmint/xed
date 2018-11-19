/*
 * xed-document.h
 * This file is part of xed
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi
 * Copyright (C) 2002-2005 Paolo Maggi
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
 * Modified by the xed Team, 1998-2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __XED_DOCUMENT_H__
#define __XED_DOCUMENT_H__

#include <gtksourceview/gtksource.h>

G_BEGIN_DECLS

#define XED_TYPE_DOCUMENT (xed_document_get_type())

G_DECLARE_DERIVABLE_TYPE (XedDocument, xed_document, XED, DOCUMENT, GtkSourceBuffer)

#define XED_METADATA_ATTRIBUTE_POSITION "metadata::xed-position"
#define XED_METADATA_ATTRIBUTE_ENCODING "metadata::xed-encoding"
#define XED_METADATA_ATTRIBUTE_LANGUAGE "metadata::xed-language"

struct _XedDocumentClass
{
    GtkSourceBufferClass parent_class;

    /* Signals */

    void (* cursor_moved)   (XedDocument *document);

    void (* load)           (XedDocument *document);

    void (* loaded)         (XedDocument *document);

    void (* save)           (XedDocument *document);

    void (* saved)          (XedDocument *document);
};

XedDocument *xed_document_new (void);

GtkSourceFile *xed_document_get_file (XedDocument *doc);

GFile *xed_document_get_location (XedDocument *doc);

void xed_document_set_location (XedDocument *doc,
                                GFile       *location);

gchar *xed_document_get_uri_for_display (XedDocument *doc);

gchar *xed_document_get_short_name_for_display (XedDocument *doc);

void xed_document_set_short_name_for_display (XedDocument *doc,
                                              const gchar *short_name);

gchar *xed_document_get_content_type (XedDocument *doc);

void xed_document_set_content_type (XedDocument *doc,
                                    const gchar *content_type);

gchar *xed_document_get_mime_type (XedDocument *doc);

gboolean xed_document_get_readonly (XedDocument *doc);

gboolean xed_document_is_untouched (XedDocument *doc);
gboolean xed_document_is_untitled (XedDocument *doc);

gboolean xed_document_is_local (XedDocument *doc);

gboolean xed_document_get_deleted (XedDocument *doc);

gboolean xed_document_goto_line (XedDocument *doc,
                                 gint         line);

gboolean xed_document_goto_line_offset (XedDocument *doc,
                                        gint         line,
                                        gint         line_offset);

void  xed_document_set_language (XedDocument       *doc,
                                 GtkSourceLanguage *lang);
GtkSourceLanguage *xed_document_get_language (XedDocument *doc);

const GtkSourceEncoding *xed_document_get_encoding (XedDocument *doc);

GtkSourceNewlineType xed_document_get_newline_type (XedDocument *doc);

gchar *xed_document_get_metadata (XedDocument *doc,
                                  const gchar *key);

void xed_document_set_metadata (XedDocument *doc,
                                const gchar *first_key,
                                ...);

void xed_document_set_search_context (XedDocument            *doc,
                                      GtkSourceSearchContext *search_context);

GtkSourceSearchContext *xed_document_get_search_context (XedDocument *doc);

G_END_DECLS

#endif /* __XED_DOCUMENT_H__ */
