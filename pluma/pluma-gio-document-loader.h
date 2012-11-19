/*
 * pluma-gio-document-loader.h
 * This file is part of pluma
 *
 * Copyright (C) 2005 - Paolo Maggi
 * Copyright (C) 2007 - Paolo Maggi, Steve Frécinaux
 * Copyright (C) 2008 - Jesse van den Kieboom
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
 * Modified by the pluma Team, 2005-2008. See the AUTHORS file for a
 * list of people on the pluma Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __PLUMA_GIO_DOCUMENT_LOADER_H__
#define __PLUMA_GIO_DOCUMENT_LOADER_H__

#include <pluma/pluma-document.h>
#include "pluma-document-loader.h"

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_GIO_DOCUMENT_LOADER              (pluma_gio_document_loader_get_type())
#define PLUMA_GIO_DOCUMENT_LOADER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_GIO_DOCUMENT_LOADER, PlumaGioDocumentLoader))
#define PLUMA_GIO_DOCUMENT_LOADER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), PLUMA_TYPE_GIO_DOCUMENT_LOADER, PlumaGioDocumentLoaderClass))
#define PLUMA_IS_GIO_DOCUMENT_LOADER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), PLUMA_TYPE_GIO_DOCUMENT_LOADER))
#define PLUMA_IS_GIO_DOCUMENT_LOADER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_GIO_DOCUMENT_LOADER))
#define PLUMA_GIO_DOCUMENT_LOADER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), PLUMA_TYPE_GIO_DOCUMENT_LOADER, PlumaGioDocumentLoaderClass))

/* Private structure type */
typedef struct _PlumaGioDocumentLoaderPrivate PlumaGioDocumentLoaderPrivate;

/*
 * Main object structure
 */
typedef struct _PlumaGioDocumentLoader PlumaGioDocumentLoader;

struct _PlumaGioDocumentLoader
{
	PlumaDocumentLoader loader;

	/*< private > */
	PlumaGioDocumentLoaderPrivate *priv;
};

/*
 * Class definition
 */
typedef PlumaDocumentLoaderClass PlumaGioDocumentLoaderClass;

/*
 * Public methods
 */
GType 		 	 pluma_gio_document_loader_get_type	(void) G_GNUC_CONST;

G_END_DECLS

#endif  /* __PLUMA_GIO_DOCUMENT_LOADER_H__  */
