/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- 
 *
 * gedittextregion.h - GtkTextMark based region utility functions
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
 * Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 */

#ifndef __GEDIT_TEXT_REGION_H__
#define __GEDIT_TEXT_REGION_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _GeditTextRegion		GeditTextRegion;
typedef struct _GeditTextRegionIterator	GeditTextRegionIterator;

struct _GeditTextRegionIterator {
	/* GeditTextRegionIterator is an opaque datatype; ignore all these fields.
	 * Initialize the iter with gedit_text_region_get_iterator
	 * function
	 */
	/*< private >*/
	gpointer dummy1;
	guint32  dummy2;
	gpointer dummy3;	
};

GeditTextRegion *gedit_text_region_new                          (GtkTextBuffer *buffer);
void           gedit_text_region_destroy                      (GeditTextRegion *region,
							     gboolean       delete_marks);

GtkTextBuffer *gedit_text_region_get_buffer                   (GeditTextRegion *region);

void           gedit_text_region_add                          (GeditTextRegion     *region,
							     const GtkTextIter *_start,
							     const GtkTextIter *_end);

void           gedit_text_region_subtract                     (GeditTextRegion     *region,
							     const GtkTextIter *_start,
							     const GtkTextIter *_end);

gint           gedit_text_region_subregions                   (GeditTextRegion *region);

gboolean       gedit_text_region_nth_subregion                (GeditTextRegion *region,
							     guint          subregion,
							     GtkTextIter   *start,
							     GtkTextIter   *end);

GeditTextRegion *gedit_text_region_intersect                    (GeditTextRegion     *region,
							     const GtkTextIter *_start,
							     const GtkTextIter *_end);

void           gedit_text_region_get_iterator                 (GeditTextRegion         *region,
                                                             GeditTextRegionIterator *iter,
                                                             guint                  start);

gboolean       gedit_text_region_iterator_is_end              (GeditTextRegionIterator *iter);

/* Returns FALSE if iterator is the end iterator */
gboolean       gedit_text_region_iterator_next	            (GeditTextRegionIterator *iter);

void           gedit_text_region_iterator_get_subregion       (GeditTextRegionIterator *iter,
							     GtkTextIter           *start,
							     GtkTextIter           *end);

void           gedit_text_region_debug_print                  (GeditTextRegion *region);

G_END_DECLS

#endif /* __GEDIT_TEXT_REGION_H__ */
