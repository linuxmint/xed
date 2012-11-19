/*
 * pluma-document-saver.h
 * This file is part of pluma
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
 * Modified by the pluma Team, 2005. See the AUTHORS file for a
 * list of people on the pluma Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __PLUMA_DOCUMENT_SAVER_H__
#define __PLUMA_DOCUMENT_SAVER_H__

#include <pluma/pluma-document.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_DOCUMENT_SAVER              (pluma_document_saver_get_type())
#define PLUMA_DOCUMENT_SAVER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_DOCUMENT_SAVER, PlumaDocumentSaver))
#define PLUMA_DOCUMENT_SAVER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), PLUMA_TYPE_DOCUMENT_SAVER, PlumaDocumentSaverClass))
#define PLUMA_IS_DOCUMENT_SAVER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), PLUMA_TYPE_DOCUMENT_SAVER))
#define PLUMA_IS_DOCUMENT_SAVER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_DOCUMENT_SAVER))
#define PLUMA_DOCUMENT_SAVER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), PLUMA_TYPE_DOCUMENT_SAVER, PlumaDocumentSaverClass))

/*
 * Main object structure
 */
typedef struct _PlumaDocumentSaver PlumaDocumentSaver;

struct _PlumaDocumentSaver 
{
	GObject object;

	/*< private >*/
	GFileInfo		 *info;
	PlumaDocument		 *document;
	gboolean		  used;

	gchar			 *uri;
	const PlumaEncoding      *encoding;
	PlumaDocumentNewlineType  newline_type;

	PlumaDocumentSaveFlags    flags;

	gboolean		  keep_backup;
};

/*
 * Class definition
 */
typedef struct _PlumaDocumentSaverClass PlumaDocumentSaverClass;

struct _PlumaDocumentSaverClass 
{
	GObjectClass parent_class;

	/* Signals */
	void (* saving) (PlumaDocumentSaver *saver,
			 gboolean             completed,
			 const GError        *error);

	/* VTable */
	void			(* save)		(PlumaDocumentSaver *saver,
							 GTimeVal           *old_mtime);
	goffset			(* get_file_size)	(PlumaDocumentSaver *saver);
	goffset			(* get_bytes_written)	(PlumaDocumentSaver *saver);
};

/*
 * Public methods
 */
GType 		 	 pluma_document_saver_get_type		(void) G_GNUC_CONST;

/* If enconding == NULL, the encoding will be autodetected */
PlumaDocumentSaver 	*pluma_document_saver_new 		(PlumaDocument           *doc,
								 const gchar             *uri,
								 const PlumaEncoding     *encoding,
								 PlumaDocumentNewlineType newline_type,
								 PlumaDocumentSaveFlags   flags);

void			 pluma_document_saver_saving		(PlumaDocumentSaver *saver,
								 gboolean            completed,
								 GError             *error);
void			 pluma_document_saver_save		(PlumaDocumentSaver  *saver,
								 GTimeVal            *old_mtime);

#if 0
void			 pluma_document_saver_cancel		(PlumaDocumentSaver  *saver);
#endif

PlumaDocument		*pluma_document_saver_get_document	(PlumaDocumentSaver  *saver);

const gchar		*pluma_document_saver_get_uri		(PlumaDocumentSaver  *saver);

/* If backup_uri is NULL no backup will be made */
const gchar		*pluma_document_saver_get_backup_uri	(PlumaDocumentSaver  *saver);
void			*pluma_document_saver_set_backup_uri	(PlumaDocumentSaver  *saver,
							 	 const gchar         *backup_uri);

/* Returns 0 if file size is unknown */
goffset			 pluma_document_saver_get_file_size	(PlumaDocumentSaver  *saver);

goffset			 pluma_document_saver_get_bytes_written	(PlumaDocumentSaver  *saver);

GFileInfo		*pluma_document_saver_get_info		(PlumaDocumentSaver  *saver);

G_END_DECLS

#endif  /* __PLUMA_DOCUMENT_SAVER_H__  */
