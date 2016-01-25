/*
 * xedit-document-saver.h
 * This file is part of xedit
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
 * Modified by the xedit Team, 2005. See the AUTHORS file for a
 * list of people on the xedit Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __XEDIT_DOCUMENT_SAVER_H__
#define __XEDIT_DOCUMENT_SAVER_H__

#include <xedit/xedit-document.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XEDIT_TYPE_DOCUMENT_SAVER              (xedit_document_saver_get_type())
#define XEDIT_DOCUMENT_SAVER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XEDIT_TYPE_DOCUMENT_SAVER, XeditDocumentSaver))
#define XEDIT_DOCUMENT_SAVER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XEDIT_TYPE_DOCUMENT_SAVER, XeditDocumentSaverClass))
#define XEDIT_IS_DOCUMENT_SAVER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XEDIT_TYPE_DOCUMENT_SAVER))
#define XEDIT_IS_DOCUMENT_SAVER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_DOCUMENT_SAVER))
#define XEDIT_DOCUMENT_SAVER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XEDIT_TYPE_DOCUMENT_SAVER, XeditDocumentSaverClass))

/*
 * Main object structure
 */
typedef struct _XeditDocumentSaver XeditDocumentSaver;

struct _XeditDocumentSaver 
{
	GObject object;

	/*< private >*/
	GFileInfo		 *info;
	XeditDocument		 *document;
	gboolean		  used;

	gchar			 *uri;
	const XeditEncoding      *encoding;
	XeditDocumentNewlineType  newline_type;

	XeditDocumentSaveFlags    flags;

	gboolean		  keep_backup;
};

/*
 * Class definition
 */
typedef struct _XeditDocumentSaverClass XeditDocumentSaverClass;

struct _XeditDocumentSaverClass 
{
	GObjectClass parent_class;

	/* Signals */
	void (* saving) (XeditDocumentSaver *saver,
			 gboolean             completed,
			 const GError        *error);

	/* VTable */
	void			(* save)		(XeditDocumentSaver *saver,
							 GTimeVal           *old_mtime);
	goffset			(* get_file_size)	(XeditDocumentSaver *saver);
	goffset			(* get_bytes_written)	(XeditDocumentSaver *saver);
};

/*
 * Public methods
 */
GType 		 	 xedit_document_saver_get_type		(void) G_GNUC_CONST;

/* If enconding == NULL, the encoding will be autodetected */
XeditDocumentSaver 	*xedit_document_saver_new 		(XeditDocument           *doc,
								 const gchar             *uri,
								 const XeditEncoding     *encoding,
								 XeditDocumentNewlineType newline_type,
								 XeditDocumentSaveFlags   flags);

void			 xedit_document_saver_saving		(XeditDocumentSaver *saver,
								 gboolean            completed,
								 GError             *error);
void			 xedit_document_saver_save		(XeditDocumentSaver  *saver,
								 GTimeVal            *old_mtime);

#if 0
void			 xedit_document_saver_cancel		(XeditDocumentSaver  *saver);
#endif

XeditDocument		*xedit_document_saver_get_document	(XeditDocumentSaver  *saver);

const gchar		*xedit_document_saver_get_uri		(XeditDocumentSaver  *saver);

/* If backup_uri is NULL no backup will be made */
const gchar		*xedit_document_saver_get_backup_uri	(XeditDocumentSaver  *saver);
void			*xedit_document_saver_set_backup_uri	(XeditDocumentSaver  *saver,
							 	 const gchar         *backup_uri);

/* Returns 0 if file size is unknown */
goffset			 xedit_document_saver_get_file_size	(XeditDocumentSaver  *saver);

goffset			 xedit_document_saver_get_bytes_written	(XeditDocumentSaver  *saver);

GFileInfo		*xedit_document_saver_get_info		(XeditDocumentSaver  *saver);

G_END_DECLS

#endif  /* __XEDIT_DOCUMENT_SAVER_H__  */
