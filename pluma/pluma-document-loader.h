/*
 * pluma-document-loader.h
 * This file is part of pluma
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
 * Modified by the pluma Team, 2005-2007. See the AUTHORS file for a
 * list of people on the pluma Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __PLUMA_DOCUMENT_LOADER_H__
#define __PLUMA_DOCUMENT_LOADER_H__

#include <pluma/pluma-document.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_DOCUMENT_LOADER              (pluma_document_loader_get_type())
#define PLUMA_DOCUMENT_LOADER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_DOCUMENT_LOADER, PlumaDocumentLoader))
#define PLUMA_DOCUMENT_LOADER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), PLUMA_TYPE_DOCUMENT_LOADER, PlumaDocumentLoaderClass))
#define PLUMA_IS_DOCUMENT_LOADER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), PLUMA_TYPE_DOCUMENT_LOADER))
#define PLUMA_IS_DOCUMENT_LOADER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_DOCUMENT_LOADER))
#define PLUMA_DOCUMENT_LOADER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), PLUMA_TYPE_DOCUMENT_LOADER, PlumaDocumentLoaderClass))

/* Private structure type */
typedef struct _PlumaDocumentLoaderPrivate PlumaDocumentLoaderPrivate;

/*
 * Main object structure
 */
typedef struct _PlumaDocumentLoader PlumaDocumentLoader;

struct _PlumaDocumentLoader 
{
	GObject object;

	PlumaDocument		 *document;
	gboolean		  used;

	/* Info on the current file */
	GFileInfo		 *info;
	gchar			 *uri;
	const PlumaEncoding	 *encoding;
	const PlumaEncoding	 *auto_detected_encoding;
	PlumaDocumentNewlineType  auto_detected_newline_type;
};

/*
 * Class definition
 */
typedef struct _PlumaDocumentLoaderClass PlumaDocumentLoaderClass;

struct _PlumaDocumentLoaderClass 
{
	GObjectClass parent_class;

	/* Signals */
	void (* loading) (PlumaDocumentLoader *loader,
			  gboolean             completed,
			  const GError        *error);

	/* VTable */
	void			(* load)		(PlumaDocumentLoader *loader);
	gboolean		(* cancel)		(PlumaDocumentLoader *loader);
	goffset			(* get_bytes_read)	(PlumaDocumentLoader *loader);
};

/*
 * Public methods
 */
GType 		 	 pluma_document_loader_get_type		(void) G_GNUC_CONST;

/* If enconding == NULL, the encoding will be autodetected */
PlumaDocumentLoader 	*pluma_document_loader_new 		(PlumaDocument       *doc,
								 const gchar         *uri,
								 const PlumaEncoding *encoding);

void			 pluma_document_loader_loading		(PlumaDocumentLoader *loader,
								 gboolean             completed,
								 GError              *error);

void			 pluma_document_loader_load		(PlumaDocumentLoader *loader);
#if 0
gboolean		 pluma_document_loader_load_from_stdin	(PlumaDocumentLoader *loader);
#endif		 
gboolean		 pluma_document_loader_cancel		(PlumaDocumentLoader *loader);

PlumaDocument		*pluma_document_loader_get_document	(PlumaDocumentLoader *loader);

/* Returns STDIN_URI if loading from stdin */
#define STDIN_URI "stdin:" 
const gchar		*pluma_document_loader_get_uri		(PlumaDocumentLoader *loader);

const PlumaEncoding	*pluma_document_loader_get_encoding	(PlumaDocumentLoader *loader);

PlumaDocumentNewlineType pluma_document_loader_get_newline_type (PlumaDocumentLoader *loader);

goffset			 pluma_document_loader_get_bytes_read	(PlumaDocumentLoader *loader);

/* You can get from the info: content_type, time_modified, standard_size, access_can_write 
   and also the metadata*/
GFileInfo		*pluma_document_loader_get_info		(PlumaDocumentLoader *loader);

G_END_DECLS

#endif  /* __PLUMA_DOCUMENT_LOADER_H__  */
