/*
 * gedit-smart-charset-converter.h
 * This file is part of gedit
 *
 * Copyright (C) 2009 - Ignacio Casal Quinteiro
 *
 * gedit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * gedit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gedit; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#ifndef __GEDIT_SMART_CHARSET_CONVERTER_H__
#define __GEDIT_SMART_CHARSET_CONVERTER_H__

#include <glib-object.h>

#include "gedit-encodings.h"

G_BEGIN_DECLS

#define GEDIT_TYPE_SMART_CHARSET_CONVERTER		(gedit_smart_charset_converter_get_type ())
#define GEDIT_SMART_CHARSET_CONVERTER(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_SMART_CHARSET_CONVERTER, GeditSmartCharsetConverter))
#define GEDIT_SMART_CHARSET_CONVERTER_CONST(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_SMART_CHARSET_CONVERTER, GeditSmartCharsetConverter const))
#define GEDIT_SMART_CHARSET_CONVERTER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_SMART_CHARSET_CONVERTER, GeditSmartCharsetConverterClass))
#define GEDIT_IS_SMART_CHARSET_CONVERTER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_SMART_CHARSET_CONVERTER))
#define GEDIT_IS_SMART_CHARSET_CONVERTER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_SMART_CHARSET_CONVERTER))
#define GEDIT_SMART_CHARSET_CONVERTER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_TYPE_SMART_CHARSET_CONVERTER, GeditSmartCharsetConverterClass))

typedef struct _GeditSmartCharsetConverter		GeditSmartCharsetConverter;
typedef struct _GeditSmartCharsetConverterClass		GeditSmartCharsetConverterClass;
typedef struct _GeditSmartCharsetConverterPrivate	GeditSmartCharsetConverterPrivate;

struct _GeditSmartCharsetConverter
{
	GObject parent;
	
	GeditSmartCharsetConverterPrivate *priv;
};

struct _GeditSmartCharsetConverterClass
{
	GObjectClass parent_class;
};

GType gedit_smart_charset_converter_get_type (void) G_GNUC_CONST;

GeditSmartCharsetConverter	*gedit_smart_charset_converter_new		(GSList *candidate_encodings);

const GeditEncoding		*gedit_smart_charset_converter_get_guessed	(GeditSmartCharsetConverter *smart);

guint				 gedit_smart_charset_converter_get_num_fallbacks(GeditSmartCharsetConverter *smart);

G_END_DECLS

#endif /* __GEDIT_SMART_CHARSET_CONVERTER_H__ */
