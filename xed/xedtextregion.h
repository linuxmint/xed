/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- 
 *
 * xedtextregion.h - GtkTextMark based region utility functions
 *
 * This file is part of the GtkSourceView widget
 *
 * Copyright (C) 2002 Gustavo Giráldez <gustavo.giraldez@gmx.net>
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

#ifndef __XED_TEXT_REGION_H__
#define __XED_TEXT_REGION_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _XedTextRegion		XedTextRegion;
typedef struct _XedTextRegionIterator	XedTextRegionIterator;

struct _XedTextRegionIterator {
	/* XedTextRegionIterator is an opaque datatype; ignore all these fields.
	 * Initialize the iter with xed_text_region_get_iterator
	 * function
	 */
	/*< private >*/
	gpointer dummy1;
	guint32  dummy2;
	gpointer dummy3;	
};

XedTextRegion *xed_text_region_new                          (GtkTextBuffer *buffer);
void           xed_text_region_destroy                      (XedTextRegion *region,
							     gboolean       delete_marks);

GtkTextBuffer *xed_text_region_get_buffer                   (XedTextRegion *region);

void           xed_text_region_add                          (XedTextRegion     *region,
							     const GtkTextIter *_start,
							     const GtkTextIter *_end);

void           xed_text_region_subtract                     (XedTextRegion     *region,
							     const GtkTextIter *_start,
							     const GtkTextIter *_end);

gint           xed_text_region_subregions                   (XedTextRegion *region);

gboolean       xed_text_region_nth_subregion                (XedTextRegion *region,
							     guint          subregion,
							     GtkTextIter   *start,
							     GtkTextIter   *end);

XedTextRegion *xed_text_region_intersect                    (XedTextRegion     *region,
							     const GtkTextIter *_start,
							     const GtkTextIter *_end);

void           xed_text_region_get_iterator                 (XedTextRegion         *region,
                                                             XedTextRegionIterator *iter,
                                                             guint                  start);

gboolean       xed_text_region_iterator_is_end              (XedTextRegionIterator *iter);

/* Returns FALSE if iterator is the end iterator */
gboolean       xed_text_region_iterator_next	            (XedTextRegionIterator *iter);

void           xed_text_region_iterator_get_subregion       (XedTextRegionIterator *iter,
							     GtkTextIter           *start,
							     GtkTextIter           *end);

void           xed_text_region_debug_print                  (XedTextRegion *region);

G_END_DECLS

#endif /* __XED_TEXT_REGION_H__ */
