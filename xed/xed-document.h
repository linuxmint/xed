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

#include <gio/gio.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourcebuffer.h>

#include <xed/xed-encodings.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XED_TYPE_DOCUMENT              (xed_document_get_type())
#define XED_DOCUMENT(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_DOCUMENT, XedDocument))
#define XED_DOCUMENT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_DOCUMENT, XedDocumentClass))
#define XED_IS_DOCUMENT(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_DOCUMENT))
#define XED_IS_DOCUMENT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_DOCUMENT))
#define XED_DOCUMENT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_DOCUMENT, XedDocumentClass))

#define XED_METADATA_ATTRIBUTE_POSITION "metadata::xed-position"
#define XED_METADATA_ATTRIBUTE_ENCODING "metadata::xed-encoding"
#define XED_METADATA_ATTRIBUTE_LANGUAGE "metadata::xed-language"

typedef enum
{
	XED_DOCUMENT_NEWLINE_TYPE_LF,
	XED_DOCUMENT_NEWLINE_TYPE_CR,
	XED_DOCUMENT_NEWLINE_TYPE_CR_LF
} XedDocumentNewlineType;

#define XED_DOCUMENT_NEWLINE_TYPE_DEFAULT XED_DOCUMENT_NEWLINE_TYPE_LF

/**
 * XedDocumentSaveFlags:
 * @XED_DOCUMENT_SAVE_IGNORE_MTIME: save file despite external modifications.
 * @XED_DOCUMENT_SAVE_IGNORE_BACKUP: write the file directly without attempting to backup.
 * @XED_DOCUMENT_SAVE_PRESERVE_BACKUP: preserve previous backup file, needed to support autosaving.
 */
typedef enum
{
	XED_DOCUMENT_SAVE_IGNORE_MTIME 	       = 1 << 0,
	XED_DOCUMENT_SAVE_IGNORE_BACKUP	       = 1 << 1,
	XED_DOCUMENT_SAVE_PRESERVE_BACKUP	   = 1 << 2,
    XED_DOCUMENT_SAVE_IGNORE_INVALID_CHARS = 1 << 3
} XedDocumentSaveFlags;

/* Private structure type */
typedef struct _XedDocumentPrivate    XedDocumentPrivate;

/*
 * Main object structure
 */
typedef struct _XedDocument           XedDocument;

struct _XedDocument
{
	GtkSourceBuffer buffer;

	/*< private > */
	XedDocumentPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XedDocumentClass 	XedDocumentClass;

struct _XedDocumentClass
{
	GtkSourceBufferClass parent_class;

	/* Signals */ // CHECK: ancora da rivedere

	void (* cursor_moved)		(XedDocument    *document);

	/* Document load */
	void (* load)			(XedDocument       *document,
					 GFile          *location,
					 const XedEncoding *encoding,
					 gint                 line_pos,
					 gboolean             create);

	void (* loading)		(XedDocument    *document,
					 goffset	   size,
					 goffset	   total_size);

	void (* loaded)			(XedDocument    *document,
					 const GError     *error);

	/* Document save */
	void (* save)			(XedDocument          *document,
					 GFile *location,
					 const XedEncoding    *encoding,
					 XedDocumentSaveFlags  flags);

	void (* saving)			(XedDocument    *document,
					 goffset	   size,
					 goffset	   total_size);

	void (* saved)  		(XedDocument    *document,
					 const GError     *error);
};


#define XED_DOCUMENT_ERROR xed_document_error_quark ()

enum
{
	XED_DOCUMENT_ERROR_EXTERNALLY_MODIFIED,
	XED_DOCUMENT_ERROR_CANT_CREATE_BACKUP,
	XED_DOCUMENT_ERROR_TOO_BIG,
	XED_DOCUMENT_ERROR_ENCODING_AUTO_DETECTION_FAILED,
	XED_DOCUMENT_ERROR_CONVERSION_FALLBACK,
	XED_DOCUMENT_NUM_ERRORS
};

GQuark		 xed_document_error_quark	(void);

GType		 xed_document_get_type      	(void) G_GNUC_CONST;

XedDocument   *xed_document_new 		(void);

GFile		*xed_document_get_location	(XedDocument       *doc);

void xed_document_set_location (XedDocument *doc,
								GFile *location);

gchar		*xed_document_get_uri_for_display
						(XedDocument       *doc);
gchar		*xed_document_get_short_name_for_display
					 	(XedDocument       *doc);

void		 xed_document_set_short_name_for_display
						(XedDocument       *doc,
						 const gchar         *name);

gchar		*xed_document_get_content_type
					 	(XedDocument       *doc);

void		 xed_document_set_content_type
					 	(XedDocument       *doc,
					 	 const gchar         *content_type);

gchar		*xed_document_get_mime_type 	(XedDocument       *doc);

gboolean	 xed_document_get_readonly 	(XedDocument       *doc);

void		 xed_document_load 		(XedDocument       *doc,
						 GFile     *location,
						 const XedEncoding *encoding,
						 gint                 line_pos,
						 gboolean             create);

gboolean	 xed_document_load_cancel	(XedDocument       *doc);

void		 xed_document_save 		(XedDocument       *doc,
						 XedDocumentSaveFlags flags);

void		 xed_document_save_as 	(XedDocument       *doc,
						 GFile     *location,
						 const XedEncoding *encoding,
						 XedDocumentSaveFlags flags);

gboolean	 xed_document_is_untouched 	(XedDocument       *doc);
gboolean	 xed_document_is_untitled 	(XedDocument       *doc);

gboolean	 xed_document_is_local	(XedDocument       *doc);

gboolean	 xed_document_get_deleted	(XedDocument       *doc);

gboolean	 xed_document_goto_line 	(XedDocument       *doc,
						 gint                 line);

gboolean	 xed_document_goto_line_offset(XedDocument *doc,
						 gint           line,
						 gint           line_offset);

void 		 xed_document_set_language 	(XedDocument       *doc,
						 GtkSourceLanguage   *lang);
GtkSourceLanguage
		*xed_document_get_language 	(XedDocument       *doc);

const XedEncoding
		*xed_document_get_encoding	(XedDocument       *doc);

void		 xed_document_set_newline_type (XedDocument           *doc,
						  XedDocumentNewlineType newline_type);

XedDocumentNewlineType
		 xed_document_get_newline_type (XedDocument *doc);

gchar		*xed_document_get_metadata	(XedDocument *doc,
						 const gchar   *key);

void		 xed_document_set_metadata	(XedDocument *doc,
						 const gchar   *first_key,
						 ...);

/*
 * Non exported functions
 */
void		 _xed_document_set_readonly 	(XedDocument       *doc,
						 gboolean             readonly);

glong		 _xed_document_get_seconds_since_last_save_or_load
						(XedDocument       *doc);

void _xed_document_apply_error_style (XedDocument *doc,
                                      GtkTextIter *start,
                                      GtkTextIter *end);

void _xed_document_apply_error_style (XedDocument *doc,
                                      GtkTextIter *start,
                                      GtkTextIter *end);

/* Note: this is a sync stat: use only on local files */
gboolean	_xed_document_check_externally_modified
						(XedDocument       *doc);

typedef GMountOperation *(*XedMountOperationFactory)(XedDocument *doc,
						       gpointer       userdata);

void		 _xed_document_set_mount_operation_factory
						(XedDocument	            *doc,
						 XedMountOperationFactory  callback,
						 gpointer	             userdata);
GMountOperation
		*_xed_document_create_mount_operation
						(XedDocument	     *doc);

void _xed_document_set_search_context (XedDocument            *doc,
                                       GtkSourceSearchContext *search_context);

GtkSourceSearchContext *_xed_document_get_search_context (XedDocument *doc);

G_END_DECLS

#endif /* __XED_DOCUMENT_H__ */
