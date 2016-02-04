/*
 * xed-document-saver.h
 * This file is part of xed
 *
 * Copyright (C) 2005 - Paolo Maggi
 * Copyrhing (C) 2007 - Paolo Maggi, Steve Frécinaux
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
 * Modified by the xed Team, 2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __XED_DOCUMENT_SAVER_H__
#define __XED_DOCUMENT_SAVER_H__

#include <xed/xed-document.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XED_TYPE_DOCUMENT_SAVER              (xed_document_saver_get_type())
#define XED_DOCUMENT_SAVER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_DOCUMENT_SAVER, XedDocumentSaver))
#define XED_DOCUMENT_SAVER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_DOCUMENT_SAVER, XedDocumentSaverClass))
#define XED_IS_DOCUMENT_SAVER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_DOCUMENT_SAVER))
#define XED_IS_DOCUMENT_SAVER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_DOCUMENT_SAVER))
#define XED_DOCUMENT_SAVER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_DOCUMENT_SAVER, XedDocumentSaverClass))

/*
 * Main object structure
 */
typedef struct _XedDocumentSaver XedDocumentSaver;

struct _XedDocumentSaver 
{
	GObject object;

	/*< private >*/
	GFileInfo		 *info;
	XedDocument		 *document;
	gboolean		  used;

	gchar			 *uri;
	const XedEncoding      *encoding;
	XedDocumentNewlineType  newline_type;

	XedDocumentSaveFlags    flags;

	gboolean		  keep_backup;
};

/*
 * Class definition
 */
typedef struct _XedDocumentSaverClass XedDocumentSaverClass;

struct _XedDocumentSaverClass 
{
	GObjectClass parent_class;

	/* Signals */
	void (* saving) (XedDocumentSaver *saver,
			 gboolean             completed,
			 const GError        *error);

	/* VTable */
	void			(* save)		(XedDocumentSaver *saver,
							 GTimeVal           *old_mtime);
	goffset			(* get_file_size)	(XedDocumentSaver *saver);
	goffset			(* get_bytes_written)	(XedDocumentSaver *saver);
};

/*
 * Public methods
 */
GType 		 	 xed_document_saver_get_type		(void) G_GNUC_CONST;

/* If enconding == NULL, the encoding will be autodetected */
XedDocumentSaver 	*xed_document_saver_new 		(XedDocument           *doc,
								 const gchar             *uri,
								 const XedEncoding     *encoding,
								 XedDocumentNewlineType newline_type,
								 XedDocumentSaveFlags   flags);

void			 xed_document_saver_saving		(XedDocumentSaver *saver,
								 gboolean            completed,
								 GError             *error);
void			 xed_document_saver_save		(XedDocumentSaver  *saver,
								 GTimeVal            *old_mtime);

#if 0
void			 xed_document_saver_cancel		(XedDocumentSaver  *saver);
#endif

XedDocument		*xed_document_saver_get_document	(XedDocumentSaver  *saver);

const gchar		*xed_document_saver_get_uri		(XedDocumentSaver  *saver);

/* If backup_uri is NULL no backup will be made */
const gchar		*xed_document_saver_get_backup_uri	(XedDocumentSaver  *saver);
void			*xed_document_saver_set_backup_uri	(XedDocumentSaver  *saver,
							 	 const gchar         *backup_uri);

/* Returns 0 if file size is unknown */
goffset			 xed_document_saver_get_file_size	(XedDocumentSaver  *saver);

goffset			 xed_document_saver_get_bytes_written	(XedDocumentSaver  *saver);

GFileInfo		*xed_document_saver_get_info		(XedDocumentSaver  *saver);

G_END_DECLS

#endif  /* __XED_DOCUMENT_SAVER_H__  */
