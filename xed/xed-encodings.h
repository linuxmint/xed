/*
 * xed-encodings.h
 * This file is part of xed
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/*
 * Modified by the xed Team, 2002-2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __XED_ENCODINGS_H__
#define __XED_ENCODINGS_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _XedEncoding XedEncoding;

#define XED_TYPE_ENCODING     (xed_encoding_get_type ())

GType              	 xed_encoding_get_type (void) G_GNUC_CONST;

const XedEncoding	*xed_encoding_get_from_charset (const gchar         *charset);
const XedEncoding	*xed_encoding_get_from_index	 (gint                 index);

gchar 			*xed_encoding_to_string	 (const XedEncoding *enc);

const gchar		*xed_encoding_get_name	 (const XedEncoding *enc);
const gchar		*xed_encoding_get_charset	 (const XedEncoding *enc);

const XedEncoding 	*xed_encoding_get_utf8	 (void);
const XedEncoding 	*xed_encoding_get_current	 (void);

/* These should not be used, they are just to make python bindings happy */
XedEncoding		*xed_encoding_copy		 (const XedEncoding *enc);
void               	 xed_encoding_free		 (XedEncoding       *enc);

GSList *_xed_encoding_strv_to_list (const gchar * const *enc_str);
gchar **_xed_encoding_list_to_strv (const GSList *enc);

G_END_DECLS

#endif  /* __XED_ENCODINGS_H__ */
