/*
 * pluma-smart-charset-converter.h
 * This file is part of pluma
 *
 * Copyright (C) 2009 - Ignacio Casal Quinteiro
 *
 * pluma is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * pluma is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with pluma; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#ifndef __PLUMA_SMART_CHARSET_CONVERTER_H__
#define __PLUMA_SMART_CHARSET_CONVERTER_H__

#include <glib-object.h>

#include "pluma-encodings.h"

G_BEGIN_DECLS

#define PLUMA_TYPE_SMART_CHARSET_CONVERTER		(pluma_smart_charset_converter_get_type ())
#define PLUMA_SMART_CHARSET_CONVERTER(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_SMART_CHARSET_CONVERTER, PlumaSmartCharsetConverter))
#define PLUMA_SMART_CHARSET_CONVERTER_CONST(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_SMART_CHARSET_CONVERTER, PlumaSmartCharsetConverter const))
#define PLUMA_SMART_CHARSET_CONVERTER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PLUMA_TYPE_SMART_CHARSET_CONVERTER, PlumaSmartCharsetConverterClass))
#define PLUMA_IS_SMART_CHARSET_CONVERTER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUMA_TYPE_SMART_CHARSET_CONVERTER))
#define PLUMA_IS_SMART_CHARSET_CONVERTER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_SMART_CHARSET_CONVERTER))
#define PLUMA_SMART_CHARSET_CONVERTER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), PLUMA_TYPE_SMART_CHARSET_CONVERTER, PlumaSmartCharsetConverterClass))

typedef struct _PlumaSmartCharsetConverter		PlumaSmartCharsetConverter;
typedef struct _PlumaSmartCharsetConverterClass		PlumaSmartCharsetConverterClass;
typedef struct _PlumaSmartCharsetConverterPrivate	PlumaSmartCharsetConverterPrivate;

struct _PlumaSmartCharsetConverter
{
	GObject parent;
	
	PlumaSmartCharsetConverterPrivate *priv;
};

struct _PlumaSmartCharsetConverterClass
{
	GObjectClass parent_class;
};

GType pluma_smart_charset_converter_get_type (void) G_GNUC_CONST;

PlumaSmartCharsetConverter	*pluma_smart_charset_converter_new		(GSList *candidate_encodings);

const PlumaEncoding		*pluma_smart_charset_converter_get_guessed	(PlumaSmartCharsetConverter *smart);

guint				 pluma_smart_charset_converter_get_num_fallbacks(PlumaSmartCharsetConverter *smart);

G_END_DECLS

#endif /* __PLUMA_SMART_CHARSET_CONVERTER_H__ */
