/*
 * xedit-document-loader.h
 * This file is part of xedit
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
 * Modified by the xedit Team, 2005-2007. See the AUTHORS file for a
 * list of people on the xedit Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __XEDIT_DOCUMENT_LOADER_H__
#define __XEDIT_DOCUMENT_LOADER_H__

#include <xedit/xedit-document.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XEDIT_TYPE_DOCUMENT_LOADER              (xedit_document_loader_get_type())
#define XEDIT_DOCUMENT_LOADER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XEDIT_TYPE_DOCUMENT_LOADER, XeditDocumentLoader))
#define XEDIT_DOCUMENT_LOADER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XEDIT_TYPE_DOCUMENT_LOADER, XeditDocumentLoaderClass))
#define XEDIT_IS_DOCUMENT_LOADER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XEDIT_TYPE_DOCUMENT_LOADER))
#define XEDIT_IS_DOCUMENT_LOADER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_DOCUMENT_LOADER))
#define XEDIT_DOCUMENT_LOADER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XEDIT_TYPE_DOCUMENT_LOADER, XeditDocumentLoaderClass))

/* Private structure type */
typedef struct _XeditDocumentLoaderPrivate XeditDocumentLoaderPrivate;

/*
 * Main object structure
 */
typedef struct _XeditDocumentLoader XeditDocumentLoader;

struct _XeditDocumentLoader 
{
	GObject object;

	XeditDocument		 *document;
	gboolean		  used;

	/* Info on the current file */
	GFileInfo		 *info;
	gchar			 *uri;
	const XeditEncoding	 *encoding;
	const XeditEncoding	 *auto_detected_encoding;
	XeditDocumentNewlineType  auto_detected_newline_type;
};

/*
 * Class definition
 */
typedef struct _XeditDocumentLoaderClass XeditDocumentLoaderClass;

struct _XeditDocumentLoaderClass 
{
	GObjectClass parent_class;

	/* Signals */
	void (* loading) (XeditDocumentLoader *loader,
			  gboolean             completed,
			  const GError        *error);

	/* VTable */
	void			(* load)		(XeditDocumentLoader *loader);
	gboolean		(* cancel)		(XeditDocumentLoader *loader);
	goffset			(* get_bytes_read)	(XeditDocumentLoader *loader);
};

/*
 * Public methods
 */
GType 		 	 xedit_document_loader_get_type		(void) G_GNUC_CONST;

/* If enconding == NULL, the encoding will be autodetected */
XeditDocumentLoader 	*xedit_document_loader_new 		(XeditDocument       *doc,
								 const gchar         *uri,
								 const XeditEncoding *encoding);

void			 xedit_document_loader_loading		(XeditDocumentLoader *loader,
								 gboolean             completed,
								 GError              *error);

void			 xedit_document_loader_load		(XeditDocumentLoader *loader);
#if 0
gboolean		 xedit_document_loader_load_from_stdin	(XeditDocumentLoader *loader);
#endif		 
gboolean		 xedit_document_loader_cancel		(XeditDocumentLoader *loader);

XeditDocument		*xedit_document_loader_get_document	(XeditDocumentLoader *loader);

/* Returns STDIN_URI if loading from stdin */
#define STDIN_URI "stdin:" 
const gchar		*xedit_document_loader_get_uri		(XeditDocumentLoader *loader);

const XeditEncoding	*xedit_document_loader_get_encoding	(XeditDocumentLoader *loader);

XeditDocumentNewlineType xedit_document_loader_get_newline_type (XeditDocumentLoader *loader);

goffset			 xedit_document_loader_get_bytes_read	(XeditDocumentLoader *loader);

/* You can get from the info: content_type, time_modified, standard_size, access_can_write 
   and also the metadata*/
GFileInfo		*xedit_document_loader_get_info		(XeditDocumentLoader *loader);

G_END_DECLS

#endif  /* __XEDIT_DOCUMENT_LOADER_H__  */
