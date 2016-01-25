/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- 
 *
 * xedittextregion.h - GtkTextMark based region utility functions
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

#ifndef __XEDIT_TEXT_REGION_H__
#define __XEDIT_TEXT_REGION_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _XeditTextRegion		XeditTextRegion;
typedef struct _XeditTextRegionIterator	XeditTextRegionIterator;

struct _XeditTextRegionIterator {
	/* XeditTextRegionIterator is an opaque datatype; ignore all these fields.
	 * Initialize the iter with xedit_text_region_get_iterator
	 * function
	 */
	/*< private >*/
	gpointer dummy1;
	guint32  dummy2;
	gpointer dummy3;	
};

XeditTextRegion *xedit_text_region_new                          (GtkTextBuffer *buffer);
void           xedit_text_region_destroy                      (XeditTextRegion *region,
							     gboolean       delete_marks);

GtkTextBuffer *xedit_text_region_get_buffer                   (XeditTextRegion *region);

void           xedit_text_region_add                          (XeditTextRegion     *region,
							     const GtkTextIter *_start,
							     const GtkTextIter *_end);

void           xedit_text_region_subtract                     (XeditTextRegion     *region,
							     const GtkTextIter *_start,
							     const GtkTextIter *_end);

gint           xedit_text_region_subregions                   (XeditTextRegion *region);

gboolean       xedit_text_region_nth_subregion                (XeditTextRegion *region,
							     guint          subregion,
							     GtkTextIter   *start,
							     GtkTextIter   *end);

XeditTextRegion *xedit_text_region_intersect                    (XeditTextRegion     *region,
							     const GtkTextIter *_start,
							     const GtkTextIter *_end);

void           xedit_text_region_get_iterator                 (XeditTextRegion         *region,
                                                             XeditTextRegionIterator *iter,
                                                             guint                  start);

gboolean       xedit_text_region_iterator_is_end              (XeditTextRegionIterator *iter);

/* Returns FALSE if iterator is the end iterator */
gboolean       xedit_text_region_iterator_next	            (XeditTextRegionIterator *iter);

void           xedit_text_region_iterator_get_subregion       (XeditTextRegionIterator *iter,
							     GtkTextIter           *start,
							     GtkTextIter           *end);

void           xedit_text_region_debug_print                  (XeditTextRegion *region);

G_END_DECLS

#endif /* __XEDIT_TEXT_REGION_H__ */
