/*
 * xed-smart-charset-converter.h
 * This file is part of xed
 *
 * Copyright (C) 2009 - Ignacio Casal Quinteiro
 *
 * xed is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xed is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xed; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#ifndef __XED_SMART_CHARSET_CONVERTER_H__
#define __XED_SMART_CHARSET_CONVERTER_H__

#include <glib-object.h>

#include "xed-encodings.h"

G_BEGIN_DECLS

#define XED_TYPE_SMART_CHARSET_CONVERTER		(xed_smart_charset_converter_get_type ())
#define XED_SMART_CHARSET_CONVERTER(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_SMART_CHARSET_CONVERTER, XedSmartCharsetConverter))
#define XED_SMART_CHARSET_CONVERTER_CONST(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_SMART_CHARSET_CONVERTER, XedSmartCharsetConverter const))
#define XED_SMART_CHARSET_CONVERTER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_SMART_CHARSET_CONVERTER, XedSmartCharsetConverterClass))
#define XED_IS_SMART_CHARSET_CONVERTER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_SMART_CHARSET_CONVERTER))
#define XED_IS_SMART_CHARSET_CONVERTER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_SMART_CHARSET_CONVERTER))
#define XED_SMART_CHARSET_CONVERTER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), XED_TYPE_SMART_CHARSET_CONVERTER, XedSmartCharsetConverterClass))

typedef struct _XedSmartCharsetConverter		XedSmartCharsetConverter;
typedef struct _XedSmartCharsetConverterClass		XedSmartCharsetConverterClass;
typedef struct _XedSmartCharsetConverterPrivate	XedSmartCharsetConverterPrivate;

struct _XedSmartCharsetConverter
{
	GObject parent;
	
	XedSmartCharsetConverterPrivate *priv;
};

struct _XedSmartCharsetConverterClass
{
	GObjectClass parent_class;
};

GType xed_smart_charset_converter_get_type (void) G_GNUC_CONST;

XedSmartCharsetConverter	*xed_smart_charset_converter_new		(GSList *candidate_encodings);

const XedEncoding		*xed_smart_charset_converter_get_guessed	(XedSmartCharsetConverter *smart);

guint				 xed_smart_charset_converter_get_num_fallbacks(XedSmartCharsetConverter *smart);

G_END_DECLS

#endif /* __XED_SMART_CHARSET_CONVERTER_H__ */
