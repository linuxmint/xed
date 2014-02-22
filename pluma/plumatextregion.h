/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- 
 *
 * plumatextregion.h - GtkTextMark based region utility functions
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

#ifndef __PLUMA_TEXT_REGION_H__
#define __PLUMA_TEXT_REGION_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _PlumaTextRegion		PlumaTextRegion;
typedef struct _PlumaTextRegionIterator	PlumaTextRegionIterator;

struct _PlumaTextRegionIterator {
	/* PlumaTextRegionIterator is an opaque datatype; ignore all these fields.
	 * Initialize the iter with pluma_text_region_get_iterator
	 * function
	 */
	/*< private >*/
	gpointer dummy1;
	guint32  dummy2;
	gpointer dummy3;	
};

PlumaTextRegion *pluma_text_region_new                          (GtkTextBuffer *buffer);
void           pluma_text_region_destroy                      (PlumaTextRegion *region,
							     gboolean       delete_marks);

GtkTextBuffer *pluma_text_region_get_buffer                   (PlumaTextRegion *region);

void           pluma_text_region_add                          (PlumaTextRegion     *region,
							     const GtkTextIter *_start,
							     const GtkTextIter *_end);

void           pluma_text_region_subtract                     (PlumaTextRegion     *region,
							     const GtkTextIter *_start,
							     const GtkTextIter *_end);

gint           pluma_text_region_subregions                   (PlumaTextRegion *region);

gboolean       pluma_text_region_nth_subregion                (PlumaTextRegion *region,
							     guint          subregion,
							     GtkTextIter   *start,
							     GtkTextIter   *end);

PlumaTextRegion *pluma_text_region_intersect                    (PlumaTextRegion     *region,
							     const GtkTextIter *_start,
							     const GtkTextIter *_end);

void           pluma_text_region_get_iterator                 (PlumaTextRegion         *region,
                                                             PlumaTextRegionIterator *iter,
                                                             guint                  start);

gboolean       pluma_text_region_iterator_is_end              (PlumaTextRegionIterator *iter);

/* Returns FALSE if iterator is the end iterator */
gboolean       pluma_text_region_iterator_next	            (PlumaTextRegionIterator *iter);

void           pluma_text_region_iterator_get_subregion       (PlumaTextRegionIterator *iter,
							     GtkTextIter           *start,
							     GtkTextIter           *end);

void           pluma_text_region_debug_print                  (PlumaTextRegion *region);

G_END_DECLS

#endif /* __PLUMA_TEXT_REGION_H__ */
