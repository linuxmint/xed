/*
 * xedit-document.h
 * This file is part of xedit
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
 * Modified by the xedit Team, 1998-2005. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */
 
#ifndef __XEDIT_DOCUMENT_H__
#define __XEDIT_DOCUMENT_H__

#include <gio/gio.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourcebuffer.h>

#include <xedit/xedit-encodings.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XEDIT_TYPE_DOCUMENT              (xedit_document_get_type())
#define XEDIT_DOCUMENT(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XEDIT_TYPE_DOCUMENT, XeditDocument))
#define XEDIT_DOCUMENT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XEDIT_TYPE_DOCUMENT, XeditDocumentClass))
#define XEDIT_IS_DOCUMENT(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XEDIT_TYPE_DOCUMENT))
#define XEDIT_IS_DOCUMENT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_DOCUMENT))
#define XEDIT_DOCUMENT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XEDIT_TYPE_DOCUMENT, XeditDocumentClass))

#define XEDIT_METADATA_ATTRIBUTE_POSITION "metadata::xedit-position"
#define XEDIT_METADATA_ATTRIBUTE_ENCODING "metadata::xedit-encoding"
#define XEDIT_METADATA_ATTRIBUTE_LANGUAGE "metadata::xedit-language"

typedef enum
{
	XEDIT_DOCUMENT_NEWLINE_TYPE_LF,
	XEDIT_DOCUMENT_NEWLINE_TYPE_CR,
	XEDIT_DOCUMENT_NEWLINE_TYPE_CR_LF
} XeditDocumentNewlineType;

#define XEDIT_DOCUMENT_NEWLINE_TYPE_DEFAULT XEDIT_DOCUMENT_NEWLINE_TYPE_LF

typedef enum
{
	XEDIT_SEARCH_DONT_SET_FLAGS	= 1 << 0, 
	XEDIT_SEARCH_ENTIRE_WORD	= 1 << 1,
	XEDIT_SEARCH_CASE_SENSITIVE	= 1 << 2,
	XEDIT_SEARCH_PARSE_ESCAPES	= 1 << 3

} XeditSearchFlags;

/**
 * XeditDocumentSaveFlags:
 * @XEDIT_DOCUMENT_SAVE_IGNORE_MTIME: save file despite external modifications.
 * @XEDIT_DOCUMENT_SAVE_IGNORE_BACKUP: write the file directly without attempting to backup.
 * @XEDIT_DOCUMENT_SAVE_PRESERVE_BACKUP: preserve previous backup file, needed to support autosaving.
 */
typedef enum
{
	XEDIT_DOCUMENT_SAVE_IGNORE_MTIME 	= 1 << 0,
	XEDIT_DOCUMENT_SAVE_IGNORE_BACKUP	= 1 << 1,
	XEDIT_DOCUMENT_SAVE_PRESERVE_BACKUP	= 1 << 2
} XeditDocumentSaveFlags;

/* Private structure type */
typedef struct _XeditDocumentPrivate    XeditDocumentPrivate;

/*
 * Main object structure
 */
typedef struct _XeditDocument           XeditDocument;
 
struct _XeditDocument
{
	GtkSourceBuffer buffer;
	
	/*< private > */
	XeditDocumentPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XeditDocumentClass 	XeditDocumentClass;

struct _XeditDocumentClass
{
	GtkSourceBufferClass parent_class;

	/* Signals */ // CHECK: ancora da rivedere

	void (* cursor_moved)		(XeditDocument    *document);

	/* Document load */
	void (* load)			(XeditDocument       *document,
					 const gchar         *uri,
					 const XeditEncoding *encoding,
					 gint                 line_pos,
					 gboolean             create);

	void (* loading)		(XeditDocument    *document,
					 goffset	   size,
					 goffset	   total_size);

	void (* loaded)			(XeditDocument    *document,
					 const GError     *error);

	/* Document save */
	void (* save)			(XeditDocument          *document,
					 const gchar            *uri,
					 const XeditEncoding    *encoding,
					 XeditDocumentSaveFlags  flags);

	void (* saving)			(XeditDocument    *document,
					 goffset	   size,
					 goffset	   total_size);

	void (* saved)  		(XeditDocument    *document,
					 const GError     *error);

	void (* search_highlight_updated)
					(XeditDocument    *document,
					 GtkTextIter      *start,
					 GtkTextIter      *end);
};


#define XEDIT_DOCUMENT_ERROR xedit_document_error_quark ()

enum
{
	XEDIT_DOCUMENT_ERROR_EXTERNALLY_MODIFIED,
	XEDIT_DOCUMENT_ERROR_CANT_CREATE_BACKUP,
	XEDIT_DOCUMENT_ERROR_TOO_BIG,
	XEDIT_DOCUMENT_ERROR_ENCODING_AUTO_DETECTION_FAILED,
	XEDIT_DOCUMENT_ERROR_CONVERSION_FALLBACK,
	XEDIT_DOCUMENT_NUM_ERRORS
};

GQuark		 xedit_document_error_quark	(void);

GType		 xedit_document_get_type      	(void) G_GNUC_CONST;

XeditDocument   *xedit_document_new 		(void);

GFile		*xedit_document_get_location	(XeditDocument       *doc);

gchar		*xedit_document_get_uri 	(XeditDocument       *doc);
void		 xedit_document_set_uri		(XeditDocument       *doc,
						 const gchar 	     *uri);

gchar		*xedit_document_get_uri_for_display
						(XeditDocument       *doc);
gchar		*xedit_document_get_short_name_for_display
					 	(XeditDocument       *doc);

void		 xedit_document_set_short_name_for_display
						(XeditDocument       *doc,
						 const gchar         *name);

gchar		*xedit_document_get_content_type
					 	(XeditDocument       *doc);

void		 xedit_document_set_content_type
					 	(XeditDocument       *doc,
					 	 const gchar         *content_type);

gchar		*xedit_document_get_mime_type 	(XeditDocument       *doc);

gboolean	 xedit_document_get_readonly 	(XeditDocument       *doc);

void		 xedit_document_load 		(XeditDocument       *doc,
						 const gchar         *uri,
						 const XeditEncoding *encoding,
						 gint                 line_pos,
						 gboolean             create); 

gboolean	 xedit_document_insert_file	(XeditDocument       *doc,
						 GtkTextIter         *iter, 
						 const gchar         *uri, 
						 const XeditEncoding *encoding);

gboolean	 xedit_document_load_cancel	(XeditDocument       *doc);

void		 xedit_document_save 		(XeditDocument       *doc,
						 XeditDocumentSaveFlags flags);

void		 xedit_document_save_as 	(XeditDocument       *doc,	
						 const gchar         *uri, 
						 const XeditEncoding *encoding,
						 XeditDocumentSaveFlags flags);

gboolean	 xedit_document_is_untouched 	(XeditDocument       *doc);
gboolean	 xedit_document_is_untitled 	(XeditDocument       *doc);

gboolean	 xedit_document_is_local	(XeditDocument       *doc);

gboolean	 xedit_document_get_deleted	(XeditDocument       *doc);

gboolean	 xedit_document_goto_line 	(XeditDocument       *doc, 
						 gint                 line);

gboolean	 xedit_document_goto_line_offset(XeditDocument *doc,
						 gint           line,
						 gint           line_offset);

void		 xedit_document_set_search_text	(XeditDocument       *doc,
						 const gchar         *text,
						 guint                flags);
						 
gchar		*xedit_document_get_search_text	(XeditDocument       *doc,
						 guint               *flags);

gboolean	 xedit_document_get_can_search_again
						(XeditDocument       *doc);

gboolean	 xedit_document_search_forward	(XeditDocument       *doc,
						 const GtkTextIter   *start,
						 const GtkTextIter   *end,
						 GtkTextIter         *match_start,
						 GtkTextIter         *match_end);
						 
gboolean	 xedit_document_search_backward	(XeditDocument       *doc,
						 const GtkTextIter   *start,
						 const GtkTextIter   *end,
						 GtkTextIter         *match_start,
						 GtkTextIter         *match_end);

gint		 xedit_document_replace_all 	(XeditDocument       *doc,
				            	 const gchar         *find, 
						 const gchar         *replace, 
					    	 guint                flags);

void 		 xedit_document_set_language 	(XeditDocument       *doc,
						 GtkSourceLanguage   *lang);
GtkSourceLanguage 
		*xedit_document_get_language 	(XeditDocument       *doc);

const XeditEncoding 
		*xedit_document_get_encoding	(XeditDocument       *doc);

void		 xedit_document_set_enable_search_highlighting 
						(XeditDocument       *doc,
						 gboolean             enable);

gboolean	 xedit_document_get_enable_search_highlighting
						(XeditDocument       *doc);

void		 xedit_document_set_newline_type (XeditDocument           *doc,
						  XeditDocumentNewlineType newline_type);

XeditDocumentNewlineType
		 xedit_document_get_newline_type (XeditDocument *doc);

gchar		*xedit_document_get_metadata	(XeditDocument *doc,
						 const gchar   *key);

void		 xedit_document_set_metadata	(XeditDocument *doc,
						 const gchar   *first_key,
						 ...);

/* 
 * Non exported functions
 */
void		 _xedit_document_set_readonly 	(XeditDocument       *doc,
						 gboolean             readonly);

glong		 _xedit_document_get_seconds_since_last_save_or_load 
						(XeditDocument       *doc);

/* Note: this is a sync stat: use only on local files */
gboolean	_xedit_document_check_externally_modified
						(XeditDocument       *doc);

void		_xedit_document_search_region   (XeditDocument       *doc,
						 const GtkTextIter   *start,
						 const GtkTextIter   *end);
						  
/* Search macros */
#define XEDIT_SEARCH_IS_DONT_SET_FLAGS(sflags) ((sflags & XEDIT_SEARCH_DONT_SET_FLAGS) != 0)
#define XEDIT_SEARCH_SET_DONT_SET_FLAGS(sflags,state) ((state == TRUE) ? \
(sflags |= XEDIT_SEARCH_DONT_SET_FLAGS) : (sflags &= ~XEDIT_SEARCH_DONT_SET_FLAGS))

#define XEDIT_SEARCH_IS_ENTIRE_WORD(sflags) ((sflags & XEDIT_SEARCH_ENTIRE_WORD) != 0)
#define XEDIT_SEARCH_SET_ENTIRE_WORD(sflags,state) ((state == TRUE) ? \
(sflags |= XEDIT_SEARCH_ENTIRE_WORD) : (sflags &= ~XEDIT_SEARCH_ENTIRE_WORD))

#define XEDIT_SEARCH_IS_CASE_SENSITIVE(sflags) ((sflags &  XEDIT_SEARCH_CASE_SENSITIVE) != 0)
#define XEDIT_SEARCH_SET_CASE_SENSITIVE(sflags,state) ((state == TRUE) ? \
(sflags |= XEDIT_SEARCH_CASE_SENSITIVE) : (sflags &= ~XEDIT_SEARCH_CASE_SENSITIVE))

#define XEDIT_SEARCH_IS_PARSE_ESCAPES(sflags) ((sflags &  XEDIT_SEARCH_PARSE_ESCAPES) != 0)
#define XEDIT_SEARCH_SET_PARSE_ESCAPES(sflags,state) ((state == TRUE) ? \
(sflags |= XEDIT_SEARCH_PARSE_ESCAPES) : (sflags &= ~XEDIT_SEARCH_PARSE_ESCAPES))

typedef GMountOperation *(*XeditMountOperationFactory)(XeditDocument *doc, 
						       gpointer       userdata);

void		 _xedit_document_set_mount_operation_factory
						(XeditDocument	            *doc,
						 XeditMountOperationFactory  callback,
						 gpointer	             userdata);
GMountOperation
		*_xedit_document_create_mount_operation
						(XeditDocument	     *doc);

G_END_DECLS

#endif /* __XEDIT_DOCUMENT_H__ */
