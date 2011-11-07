/*
 * gedit-document.h
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */
 
/*
 * Modified by the gedit Team, 1998-2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */
 
#ifndef __GEDIT_DOCUMENT_H__
#define __GEDIT_DOCUMENT_H__

#include <gio/gio.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourcebuffer.h>

#include <gedit/gedit-encodings.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GEDIT_TYPE_DOCUMENT              (gedit_document_get_type())
#define GEDIT_DOCUMENT(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GEDIT_TYPE_DOCUMENT, GeditDocument))
#define GEDIT_DOCUMENT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GEDIT_TYPE_DOCUMENT, GeditDocumentClass))
#define GEDIT_IS_DOCUMENT(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GEDIT_TYPE_DOCUMENT))
#define GEDIT_IS_DOCUMENT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_DOCUMENT))
#define GEDIT_DOCUMENT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GEDIT_TYPE_DOCUMENT, GeditDocumentClass))

#ifdef G_OS_WIN32
#define GEDIT_METADATA_ATTRIBUTE_POSITION "position"
#define GEDIT_METADATA_ATTRIBUTE_ENCODING "encoding"
#define GEDIT_METADATA_ATTRIBUTE_LANGUAGE "language"
#else
#define GEDIT_METADATA_ATTRIBUTE_POSITION "metadata::gedit-position"
#define GEDIT_METADATA_ATTRIBUTE_ENCODING "metadata::gedit-encoding"
#define GEDIT_METADATA_ATTRIBUTE_LANGUAGE "metadata::gedit-language"
#endif

typedef enum
{
	GEDIT_DOCUMENT_NEWLINE_TYPE_LF,
	GEDIT_DOCUMENT_NEWLINE_TYPE_CR,
	GEDIT_DOCUMENT_NEWLINE_TYPE_CR_LF
} GeditDocumentNewlineType;

#ifdef G_OS_WIN32
#define GEDIT_DOCUMENT_NEWLINE_TYPE_DEFAULT GEDIT_DOCUMENT_NEWLINE_TYPE_CR_LF
#else
#define GEDIT_DOCUMENT_NEWLINE_TYPE_DEFAULT GEDIT_DOCUMENT_NEWLINE_TYPE_LF
#endif

typedef enum
{
	GEDIT_SEARCH_DONT_SET_FLAGS	= 1 << 0, 
	GEDIT_SEARCH_ENTIRE_WORD	= 1 << 1,
	GEDIT_SEARCH_CASE_SENSITIVE	= 1 << 2

} GeditSearchFlags;

/**
 * GeditDocumentSaveFlags:
 * @GEDIT_DOCUMENT_SAVE_IGNORE_MTIME: save file despite external modifications.
 * @GEDIT_DOCUMENT_SAVE_IGNORE_BACKUP: write the file directly without attempting to backup.
 * @GEDIT_DOCUMENT_SAVE_PRESERVE_BACKUP: preserve previous backup file, needed to support autosaving.
 */
typedef enum
{
	GEDIT_DOCUMENT_SAVE_IGNORE_MTIME 	= 1 << 0,
	GEDIT_DOCUMENT_SAVE_IGNORE_BACKUP	= 1 << 1,
	GEDIT_DOCUMENT_SAVE_PRESERVE_BACKUP	= 1 << 2
} GeditDocumentSaveFlags;

/* Private structure type */
typedef struct _GeditDocumentPrivate    GeditDocumentPrivate;

/*
 * Main object structure
 */
typedef struct _GeditDocument           GeditDocument;
 
struct _GeditDocument
{
	GtkSourceBuffer buffer;
	
	/*< private > */
	GeditDocumentPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GeditDocumentClass 	GeditDocumentClass;

struct _GeditDocumentClass
{
	GtkSourceBufferClass parent_class;

	/* Signals */ // CHECK: ancora da rivedere

	void (* cursor_moved)		(GeditDocument    *document);

	/* Document load */
	void (* load)			(GeditDocument       *document,
					 const gchar         *uri,
					 const GeditEncoding *encoding,
					 gint                 line_pos,
					 gboolean             create);

	void (* loading)		(GeditDocument    *document,
					 goffset	   size,
					 goffset	   total_size);

	void (* loaded)			(GeditDocument    *document,
					 const GError     *error);

	/* Document save */
	void (* save)			(GeditDocument          *document,
					 const gchar            *uri,
					 const GeditEncoding    *encoding,
					 GeditDocumentSaveFlags  flags);

	void (* saving)			(GeditDocument    *document,
					 goffset	   size,
					 goffset	   total_size);

	void (* saved)  		(GeditDocument    *document,
					 const GError     *error);

	void (* search_highlight_updated)
					(GeditDocument    *document,
					 GtkTextIter      *start,
					 GtkTextIter      *end);
};


#define GEDIT_DOCUMENT_ERROR gedit_document_error_quark ()

enum
{
	GEDIT_DOCUMENT_ERROR_EXTERNALLY_MODIFIED,
	GEDIT_DOCUMENT_ERROR_CANT_CREATE_BACKUP,
	GEDIT_DOCUMENT_ERROR_TOO_BIG,
	GEDIT_DOCUMENT_ERROR_ENCODING_AUTO_DETECTION_FAILED,
	GEDIT_DOCUMENT_ERROR_CONVERSION_FALLBACK,
	GEDIT_DOCUMENT_NUM_ERRORS
};

GQuark		 gedit_document_error_quark	(void);

GType		 gedit_document_get_type      	(void) G_GNUC_CONST;

GeditDocument   *gedit_document_new 		(void);

GFile		*gedit_document_get_location	(GeditDocument       *doc);

gchar		*gedit_document_get_uri 	(GeditDocument       *doc);
void		 gedit_document_set_uri		(GeditDocument       *doc,
						 const gchar 	     *uri);

gchar		*gedit_document_get_uri_for_display
						(GeditDocument       *doc);
gchar		*gedit_document_get_short_name_for_display
					 	(GeditDocument       *doc);

void		 gedit_document_set_short_name_for_display
						(GeditDocument       *doc,
						 const gchar         *name);

gchar		*gedit_document_get_content_type
					 	(GeditDocument       *doc);

void		 gedit_document_set_content_type
					 	(GeditDocument       *doc,
					 	 const gchar         *content_type);

gchar		*gedit_document_get_mime_type 	(GeditDocument       *doc);

gboolean	 gedit_document_get_readonly 	(GeditDocument       *doc);

void		 gedit_document_load 		(GeditDocument       *doc,
						 const gchar         *uri,
						 const GeditEncoding *encoding,
						 gint                 line_pos,
						 gboolean             create); 

gboolean	 gedit_document_insert_file	(GeditDocument       *doc,
						 GtkTextIter         *iter, 
						 const gchar         *uri, 
						 const GeditEncoding *encoding);

gboolean	 gedit_document_load_cancel	(GeditDocument       *doc);

void		 gedit_document_save 		(GeditDocument       *doc,
						 GeditDocumentSaveFlags flags);

void		 gedit_document_save_as 	(GeditDocument       *doc,	
						 const gchar         *uri, 
						 const GeditEncoding *encoding,
						 GeditDocumentSaveFlags flags);

gboolean	 gedit_document_is_untouched 	(GeditDocument       *doc);
gboolean	 gedit_document_is_untitled 	(GeditDocument       *doc);

gboolean	 gedit_document_is_local	(GeditDocument       *doc);

gboolean	 gedit_document_get_deleted	(GeditDocument       *doc);

gboolean	 gedit_document_goto_line 	(GeditDocument       *doc, 
						 gint                 line);

gboolean	 gedit_document_goto_line_offset(GeditDocument *doc,
						 gint           line,
						 gint           line_offset);

void		 gedit_document_set_search_text	(GeditDocument       *doc,
						 const gchar         *text,
						 guint                flags);
						 
gchar		*gedit_document_get_search_text	(GeditDocument       *doc,
						 guint               *flags);

gboolean	 gedit_document_get_can_search_again
						(GeditDocument       *doc);

gboolean	 gedit_document_search_forward	(GeditDocument       *doc,
						 const GtkTextIter   *start,
						 const GtkTextIter   *end,
						 GtkTextIter         *match_start,
						 GtkTextIter         *match_end);
						 
gboolean	 gedit_document_search_backward	(GeditDocument       *doc,
						 const GtkTextIter   *start,
						 const GtkTextIter   *end,
						 GtkTextIter         *match_start,
						 GtkTextIter         *match_end);

gint		 gedit_document_replace_all 	(GeditDocument       *doc,
				            	 const gchar         *find, 
						 const gchar         *replace, 
					    	 guint                flags);

void 		 gedit_document_set_language 	(GeditDocument       *doc,
						 GtkSourceLanguage   *lang);
GtkSourceLanguage 
		*gedit_document_get_language 	(GeditDocument       *doc);

const GeditEncoding 
		*gedit_document_get_encoding	(GeditDocument       *doc);

void		 gedit_document_set_enable_search_highlighting 
						(GeditDocument       *doc,
						 gboolean             enable);

gboolean	 gedit_document_get_enable_search_highlighting
						(GeditDocument       *doc);

void		 gedit_document_set_newline_type (GeditDocument           *doc,
						  GeditDocumentNewlineType newline_type);

GeditDocumentNewlineType
		 gedit_document_get_newline_type (GeditDocument *doc);

gchar		*gedit_document_get_metadata	(GeditDocument *doc,
						 const gchar   *key);

void		 gedit_document_set_metadata	(GeditDocument *doc,
						 const gchar   *first_key,
						 ...);

/* 
 * Non exported functions
 */
void		 _gedit_document_set_readonly 	(GeditDocument       *doc,
						 gboolean             readonly);

glong		 _gedit_document_get_seconds_since_last_save_or_load 
						(GeditDocument       *doc);

/* Note: this is a sync stat: use only on local files */
gboolean	_gedit_document_check_externally_modified
						(GeditDocument       *doc);

void		_gedit_document_search_region   (GeditDocument       *doc,
						 const GtkTextIter   *start,
						 const GtkTextIter   *end);
						  
/* Search macros */
#define GEDIT_SEARCH_IS_DONT_SET_FLAGS(sflags) ((sflags & GEDIT_SEARCH_DONT_SET_FLAGS) != 0)
#define GEDIT_SEARCH_SET_DONT_SET_FLAGS(sflags,state) ((state == TRUE) ? \
(sflags |= GEDIT_SEARCH_DONT_SET_FLAGS) : (sflags &= ~GEDIT_SEARCH_DONT_SET_FLAGS))

#define GEDIT_SEARCH_IS_ENTIRE_WORD(sflags) ((sflags & GEDIT_SEARCH_ENTIRE_WORD) != 0)
#define GEDIT_SEARCH_SET_ENTIRE_WORD(sflags,state) ((state == TRUE) ? \
(sflags |= GEDIT_SEARCH_ENTIRE_WORD) : (sflags &= ~GEDIT_SEARCH_ENTIRE_WORD))

#define GEDIT_SEARCH_IS_CASE_SENSITIVE(sflags) ((sflags &  GEDIT_SEARCH_CASE_SENSITIVE) != 0)
#define GEDIT_SEARCH_SET_CASE_SENSITIVE(sflags,state) ((state == TRUE) ? \
(sflags |= GEDIT_SEARCH_CASE_SENSITIVE) : (sflags &= ~GEDIT_SEARCH_CASE_SENSITIVE))

typedef GMountOperation *(*GeditMountOperationFactory)(GeditDocument *doc, 
						       gpointer       userdata);

void		 _gedit_document_set_mount_operation_factory
						(GeditDocument	            *doc,
						 GeditMountOperationFactory  callback,
						 gpointer	             userdata);
GMountOperation
		*_gedit_document_create_mount_operation
						(GeditDocument	     *doc);

G_END_DECLS

#endif /* __GEDIT_DOCUMENT_H__ */
