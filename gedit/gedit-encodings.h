/*
 * gedit-encodings.h
 * This file is part of gedit
 *
 * Copyright (C) 2002-2005 Paolo Maggi 
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */
 
/*
 * Modified by the gedit Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __GEDIT_ENCODINGS_H__
#define __GEDIT_ENCODINGS_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _GeditEncoding GeditEncoding;

#define GEDIT_TYPE_ENCODING     (gedit_encoding_get_type ())

GType              	 gedit_encoding_get_type (void) G_GNUC_CONST;

const GeditEncoding	*gedit_encoding_get_from_charset (const gchar         *charset);
const GeditEncoding	*gedit_encoding_get_from_index	 (gint                 index);

gchar 			*gedit_encoding_to_string	 (const GeditEncoding *enc);

const gchar		*gedit_encoding_get_name	 (const GeditEncoding *enc);
const gchar		*gedit_encoding_get_charset	 (const GeditEncoding *enc);

const GeditEncoding 	*gedit_encoding_get_utf8	 (void);
const GeditEncoding 	*gedit_encoding_get_current	 (void);

/* These should not be used, they are just to make python bindings happy */
GeditEncoding		*gedit_encoding_copy		 (const GeditEncoding *enc);
void               	 gedit_encoding_free		 (GeditEncoding       *enc);

G_END_DECLS

#endif  /* __GEDIT_ENCODINGS_H__ */
