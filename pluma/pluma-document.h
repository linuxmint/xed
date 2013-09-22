/*
 * pluma-document.h
 * This file is part of pluma
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
 * Modified by the pluma Team, 1998-2005. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */
 
#ifndef __PLUMA_DOCUMENT_H__
#define __PLUMA_DOCUMENT_H__

#include <gio/gio.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourcebuffer.h>

#include <pluma/pluma-encodings.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_DOCUMENT              (pluma_document_get_type())
#define PLUMA_DOCUMENT(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_DOCUMENT, PlumaDocument))
#define PLUMA_DOCUMENT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), PLUMA_TYPE_DOCUMENT, PlumaDocumentClass))
#define PLUMA_IS_DOCUMENT(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), PLUMA_TYPE_DOCUMENT))
#define PLUMA_IS_DOCUMENT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_DOCUMENT))
#define PLUMA_DOCUMENT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), PLUMA_TYPE_DOCUMENT, PlumaDocumentClass))

#ifdef G_OS_WIN32
#define PLUMA_METADATA_ATTRIBUTE_POSITION "position"
#define PLUMA_METADATA_ATTRIBUTE_ENCODING "encoding"
#define PLUMA_METADATA_ATTRIBUTE_LANGUAGE "language"
#else
#define PLUMA_METADATA_ATTRIBUTE_POSITION "metadata::pluma-position"
#define PLUMA_METADATA_ATTRIBUTE_ENCODING "metadata::pluma-encoding"
#define PLUMA_METADATA_ATTRIBUTE_LANGUAGE "metadata::pluma-language"
#endif

typedef enum
{
	PLUMA_DOCUMENT_NEWLINE_TYPE_LF,
	PLUMA_DOCUMENT_NEWLINE_TYPE_CR,
	PLUMA_DOCUMENT_NEWLINE_TYPE_CR_LF
} PlumaDocumentNewlineType;

#ifdef G_OS_WIN32
#define PLUMA_DOCUMENT_NEWLINE_TYPE_DEFAULT PLUMA_DOCUMENT_NEWLINE_TYPE_CR_LF
#else
#define PLUMA_DOCUMENT_NEWLINE_TYPE_DEFAULT PLUMA_DOCUMENT_NEWLINE_TYPE_LF
#endif

typedef enum
{
	PLUMA_SEARCH_DONT_SET_FLAGS	= 1 << 0, 
	PLUMA_SEARCH_ENTIRE_WORD	= 1 << 1,
	PLUMA_SEARCH_CASE_SENSITIVE	= 1 << 2,
	PLUMA_SEARCH_PARSE_ESCAPES	= 1 << 3

} PlumaSearchFlags;

/**
 * PlumaDocumentSaveFlags:
 * @PLUMA_DOCUMENT_SAVE_IGNORE_MTIME: save file despite external modifications.
 * @PLUMA_DOCUMENT_SAVE_IGNORE_BACKUP: write the file directly without attempting to backup.
 * @PLUMA_DOCUMENT_SAVE_PRESERVE_BACKUP: preserve previous backup file, needed to support autosaving.
 */
typedef enum
{
	PLUMA_DOCUMENT_SAVE_IGNORE_MTIME 	= 1 << 0,
	PLUMA_DOCUMENT_SAVE_IGNORE_BACKUP	= 1 << 1,
	PLUMA_DOCUMENT_SAVE_PRESERVE_BACKUP	= 1 << 2
} PlumaDocumentSaveFlags;

/* Private structure type */
typedef struct _PlumaDocumentPrivate    PlumaDocumentPrivate;

/*
 * Main object structure
 */
typedef struct _PlumaDocument           PlumaDocument;
 
struct _PlumaDocument
{
	GtkSourceBuffer buffer;
	
	/*< private > */
	PlumaDocumentPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _PlumaDocumentClass 	PlumaDocumentClass;

struct _PlumaDocumentClass
{
	GtkSourceBufferClass parent_class;

	/* Signals */ // CHECK: ancora da rivedere

	void (* cursor_moved)		(PlumaDocument    *document);

	/* Document load */
	void (* load)			(PlumaDocument       *document,
					 const gchar         *uri,
					 const PlumaEncoding *encoding,
					 gint                 line_pos,
					 gboolean             create);

	void (* loading)		(PlumaDocument    *document,
					 goffset	   size,
					 goffset	   total_size);

	void (* loaded)			(PlumaDocument    *document,
					 const GError     *error);

	/* Document save */
	void (* save)			(PlumaDocument          *document,
					 const gchar            *uri,
					 const PlumaEncoding    *encoding,
					 PlumaDocumentSaveFlags  flags);

	void (* saving)			(PlumaDocument    *document,
					 goffset	   size,
					 goffset	   total_size);

	void (* saved)  		(PlumaDocument    *document,
					 const GError     *error);

	void (* search_highlight_updated)
					(PlumaDocument    *document,
					 GtkTextIter      *start,
					 GtkTextIter      *end);
};


#define PLUMA_DOCUMENT_ERROR pluma_document_error_quark ()

enum
{
	PLUMA_DOCUMENT_ERROR_EXTERNALLY_MODIFIED,
	PLUMA_DOCUMENT_ERROR_CANT_CREATE_BACKUP,
	PLUMA_DOCUMENT_ERROR_TOO_BIG,
	PLUMA_DOCUMENT_ERROR_ENCODING_AUTO_DETECTION_FAILED,
	PLUMA_DOCUMENT_ERROR_CONVERSION_FALLBACK,
	PLUMA_DOCUMENT_NUM_ERRORS
};

GQuark		 pluma_document_error_quark	(void);

GType		 pluma_document_get_type      	(void) G_GNUC_CONST;

PlumaDocument   *pluma_document_new 		(void);

GFile		*pluma_document_get_location	(PlumaDocument       *doc);

gchar		*pluma_document_get_uri 	(PlumaDocument       *doc);
void		 pluma_document_set_uri		(PlumaDocument       *doc,
						 const gchar 	     *uri);

gchar		*pluma_document_get_uri_for_display
						(PlumaDocument       *doc);
gchar		*pluma_document_get_short_name_for_display
					 	(PlumaDocument       *doc);

void		 pluma_document_set_short_name_for_display
						(PlumaDocument       *doc,
						 const gchar         *name);

gchar		*pluma_document_get_content_type
					 	(PlumaDocument       *doc);

void		 pluma_document_set_content_type
					 	(PlumaDocument       *doc,
					 	 const gchar         *content_type);

gchar		*pluma_document_get_mime_type 	(PlumaDocument       *doc);

gboolean	 pluma_document_get_readonly 	(PlumaDocument       *doc);

void		 pluma_document_load 		(PlumaDocument       *doc,
						 const gchar         *uri,
						 const PlumaEncoding *encoding,
						 gint                 line_pos,
						 gboolean             create); 

gboolean	 pluma_document_insert_file	(PlumaDocument       *doc,
						 GtkTextIter         *iter, 
						 const gchar         *uri, 
						 const PlumaEncoding *encoding);

gboolean	 pluma_document_load_cancel	(PlumaDocument       *doc);

void		 pluma_document_save 		(PlumaDocument       *doc,
						 PlumaDocumentSaveFlags flags);

void		 pluma_document_save_as 	(PlumaDocument       *doc,	
						 const gchar         *uri, 
						 const PlumaEncoding *encoding,
						 PlumaDocumentSaveFlags flags);

gboolean	 pluma_document_is_untouched 	(PlumaDocument       *doc);
gboolean	 pluma_document_is_untitled 	(PlumaDocument       *doc);

gboolean	 pluma_document_is_local	(PlumaDocument       *doc);

gboolean	 pluma_document_get_deleted	(PlumaDocument       *doc);

gboolean	 pluma_document_goto_line 	(PlumaDocument       *doc, 
						 gint                 line);

gboolean	 pluma_document_goto_line_offset(PlumaDocument *doc,
						 gint           line,
						 gint           line_offset);

void		 pluma_document_set_search_text	(PlumaDocument       *doc,
						 const gchar         *text,
						 guint                flags);
						 
gchar		*pluma_document_get_search_text	(PlumaDocument       *doc,
						 guint               *flags);

gboolean	 pluma_document_get_can_search_again
						(PlumaDocument       *doc);

gboolean	 pluma_document_search_forward	(PlumaDocument       *doc,
						 const GtkTextIter   *start,
						 const GtkTextIter   *end,
						 GtkTextIter         *match_start,
						 GtkTextIter         *match_end);
						 
gboolean	 pluma_document_search_backward	(PlumaDocument       *doc,
						 const GtkTextIter   *start,
						 const GtkTextIter   *end,
						 GtkTextIter         *match_start,
						 GtkTextIter         *match_end);

gint		 pluma_document_replace_all 	(PlumaDocument       *doc,
				            	 const gchar         *find, 
						 const gchar         *replace, 
					    	 guint                flags);

void 		 pluma_document_set_language 	(PlumaDocument       *doc,
						 GtkSourceLanguage   *lang);
GtkSourceLanguage 
		*pluma_document_get_language 	(PlumaDocument       *doc);

const PlumaEncoding 
		*pluma_document_get_encoding	(PlumaDocument       *doc);

void		 pluma_document_set_enable_search_highlighting 
						(PlumaDocument       *doc,
						 gboolean             enable);

gboolean	 pluma_document_get_enable_search_highlighting
						(PlumaDocument       *doc);

void		 pluma_document_set_newline_type (PlumaDocument           *doc,
						  PlumaDocumentNewlineType newline_type);

PlumaDocumentNewlineType
		 pluma_document_get_newline_type (PlumaDocument *doc);

gchar		*pluma_document_get_metadata	(PlumaDocument *doc,
						 const gchar   *key);

void		 pluma_document_set_metadata	(PlumaDocument *doc,
						 const gchar   *first_key,
						 ...);

/* 
 * Non exported functions
 */
void		 _pluma_document_set_readonly 	(PlumaDocument       *doc,
						 gboolean             readonly);

glong		 _pluma_document_get_seconds_since_last_save_or_load 
						(PlumaDocument       *doc);

/* Note: this is a sync stat: use only on local files */
gboolean	_pluma_document_check_externally_modified
						(PlumaDocument       *doc);

void		_pluma_document_search_region   (PlumaDocument       *doc,
						 const GtkTextIter   *start,
						 const GtkTextIter   *end);
						  
/* Search macros */
#define PLUMA_SEARCH_IS_DONT_SET_FLAGS(sflags) ((sflags & PLUMA_SEARCH_DONT_SET_FLAGS) != 0)
#define PLUMA_SEARCH_SET_DONT_SET_FLAGS(sflags,state) ((state == TRUE) ? \
(sflags |= PLUMA_SEARCH_DONT_SET_FLAGS) : (sflags &= ~PLUMA_SEARCH_DONT_SET_FLAGS))

#define PLUMA_SEARCH_IS_ENTIRE_WORD(sflags) ((sflags & PLUMA_SEARCH_ENTIRE_WORD) != 0)
#define PLUMA_SEARCH_SET_ENTIRE_WORD(sflags,state) ((state == TRUE) ? \
(sflags |= PLUMA_SEARCH_ENTIRE_WORD) : (sflags &= ~PLUMA_SEARCH_ENTIRE_WORD))

#define PLUMA_SEARCH_IS_CASE_SENSITIVE(sflags) ((sflags &  PLUMA_SEARCH_CASE_SENSITIVE) != 0)
#define PLUMA_SEARCH_SET_CASE_SENSITIVE(sflags,state) ((state == TRUE) ? \
(sflags |= PLUMA_SEARCH_CASE_SENSITIVE) : (sflags &= ~PLUMA_SEARCH_CASE_SENSITIVE))

#define PLUMA_SEARCH_IS_PARSE_ESCAPES(sflags) ((sflags &  PLUMA_SEARCH_PARSE_ESCAPES) != 0)
#define PLUMA_SEARCH_SET_PARSE_ESCAPES(sflags,state) ((state == TRUE) ? \
(sflags |= PLUMA_SEARCH_PARSE_ESCAPES) : (sflags &= ~PLUMA_SEARCH_PARSE_ESCAPES))

typedef GMountOperation *(*PlumaMountOperationFactory)(PlumaDocument *doc, 
						       gpointer       userdata);

void		 _pluma_document_set_mount_operation_factory
						(PlumaDocument	            *doc,
						 PlumaMountOperationFactory  callback,
						 gpointer	             userdata);
GMountOperation
		*_pluma_document_create_mount_operation
						(PlumaDocument	     *doc);

G_END_DECLS

#endif /* __PLUMA_DOCUMENT_H__ */
