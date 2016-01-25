/*
 * xedit-smart-charset-converter.h
 * This file is part of xedit
 *
 * Copyright (C) 2009 - Ignacio Casal Quinteiro
 *
 * xedit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xedit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xedit; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#ifndef __XEDIT_SMART_CHARSET_CONVERTER_H__
#define __XEDIT_SMART_CHARSET_CONVERTER_H__

#include <glib-object.h>

#include "xedit-encodings.h"

G_BEGIN_DECLS

#define XEDIT_TYPE_SMART_CHARSET_CONVERTER		(xedit_smart_charset_converter_get_type ())
#define XEDIT_SMART_CHARSET_CONVERTER(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_SMART_CHARSET_CONVERTER, XeditSmartCharsetConverter))
#define XEDIT_SMART_CHARSET_CONVERTER_CONST(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_SMART_CHARSET_CONVERTER, XeditSmartCharsetConverter const))
#define XEDIT_SMART_CHARSET_CONVERTER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_SMART_CHARSET_CONVERTER, XeditSmartCharsetConverterClass))
#define XEDIT_IS_SMART_CHARSET_CONVERTER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XEDIT_TYPE_SMART_CHARSET_CONVERTER))
#define XEDIT_IS_SMART_CHARSET_CONVERTER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_SMART_CHARSET_CONVERTER))
#define XEDIT_SMART_CHARSET_CONVERTER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), XEDIT_TYPE_SMART_CHARSET_CONVERTER, XeditSmartCharsetConverterClass))

typedef struct _XeditSmartCharsetConverter		XeditSmartCharsetConverter;
typedef struct _XeditSmartCharsetConverterClass		XeditSmartCharsetConverterClass;
typedef struct _XeditSmartCharsetConverterPrivate	XeditSmartCharsetConverterPrivate;

struct _XeditSmartCharsetConverter
{
	GObject parent;
	
	XeditSmartCharsetConverterPrivate *priv;
};

struct _XeditSmartCharsetConverterClass
{
	GObjectClass parent_class;
};

GType xedit_smart_charset_converter_get_type (void) G_GNUC_CONST;

XeditSmartCharsetConverter	*xedit_smart_charset_converter_new		(GSList *candidate_encodings);

const XeditEncoding		*xedit_smart_charset_converter_get_guessed	(XeditSmartCharsetConverter *smart);

guint				 xedit_smart_charset_converter_get_num_fallbacks(XeditSmartCharsetConverter *smart);

G_END_DECLS

#endif /* __XEDIT_SMART_CHARSET_CONVERTER_H__ */
