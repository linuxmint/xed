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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>

#include "gedittextregion.h"


#undef ENABLE_DEBUG
/*
#define ENABLE_DEBUG
*/

#ifdef ENABLE_DEBUG
#define DEBUG(x) (x)
#else
#define DEBUG(x)
#endif

typedef struct _Subregion {
	GtkTextMark *start;
	GtkTextMark *end;
} Subregion;

struct _GeditTextRegion {
	GtkTextBuffer *buffer;
	GList         *subregions;
	guint32        time_stamp;
};

typedef struct _GeditTextRegionIteratorReal GeditTextRegionIteratorReal;

struct _GeditTextRegionIteratorReal {
	GeditTextRegion *region;
	guint32        region_time_stamp;
	
	GList         *subregions;
};


/* ----------------------------------------------------------------------
   Private interface
   ---------------------------------------------------------------------- */

/* Find and return a subregion node which contains the given text
   iter.  If left_side is TRUE, return the subregion which contains
   the text iter or which is the leftmost; else return the rightmost
   subregion */
static GList * 
find_nearest_subregion (GeditTextRegion     *region,
			const GtkTextIter *iter,
			GList             *begin,
			gboolean           leftmost,
			gboolean           include_edges)
{
	GList *l, *retval;
	
	g_return_val_if_fail (region != NULL && iter != NULL, NULL);

	if (!begin)
		begin = region->subregions;
	
	if (begin)
		retval = begin->prev;
	else
		retval = NULL;
	
	for (l = begin; l; l = l->next) {
		GtkTextIter sr_iter;
		Subregion *sr = l->data;
		gint cmp;

		if (!leftmost) {
			gtk_text_buffer_get_iter_at_mark (region->buffer, &sr_iter, sr->end);
			cmp = gtk_text_iter_compare (iter, &sr_iter);
			if (cmp < 0 || (cmp == 0 && include_edges)) {
				retval = l;
				break;
			}

		} else {
			gtk_text_buffer_get_iter_at_mark (region->buffer, &sr_iter, sr->start);
			cmp = gtk_text_iter_compare (iter, &sr_iter);
			if (cmp > 0 || (cmp == 0 && include_edges))
				retval = l;
			else
				break;
		}
	}
	return retval;
}

/* ----------------------------------------------------------------------
   Public interface
   ---------------------------------------------------------------------- */

GeditTextRegion *
gedit_text_region_new (GtkTextBuffer *buffer)
{
	GeditTextRegion *region;

	g_return_val_if_fail (buffer != NULL, NULL);
	
	region = g_new (GeditTextRegion, 1);
	region->buffer = buffer;
	region->subregions = NULL;
	region->time_stamp = 0;
	
	return region;
}

void 
gedit_text_region_destroy (GeditTextRegion *region, gboolean delete_marks)
{
	g_return_if_fail (region != NULL);

	while (region->subregions) {
		Subregion *sr = region->subregions->data;
		if (delete_marks) {
			gtk_text_buffer_delete_mark (region->buffer, sr->start);
			gtk_text_buffer_delete_mark (region->buffer, sr->end);
		}
		g_free (sr);
		region->subregions = g_list_delete_link (region->subregions,
							 region->subregions);
	}
	region->buffer = NULL;
	region->time_stamp = 0;
	
	g_free (region);
}

GtkTextBuffer *
gedit_text_region_get_buffer (GeditTextRegion *region)
{
	g_return_val_if_fail (region != NULL, NULL);
	
	return region->buffer;
}

static void
gedit_text_region_clear_zero_length_subregions (GeditTextRegion *region)
{
	GtkTextIter start, end;
	GList *node;
	
	g_return_if_fail (region != NULL);

	for (node = region->subregions; node; ) {
		Subregion *sr = node->data;
		gtk_text_buffer_get_iter_at_mark (region->buffer, &start, sr->start);
		gtk_text_buffer_get_iter_at_mark (region->buffer, &end, sr->end);
		if (gtk_text_iter_equal (&start, &end)) {
			gtk_text_buffer_delete_mark (region->buffer, sr->start);
			gtk_text_buffer_delete_mark (region->buffer, sr->end);
			g_free (sr);
			if (node == region->subregions)
				region->subregions = node = g_list_delete_link (node, node);
			else
				node = g_list_delete_link (node, node);

			++region->time_stamp;
			
		} else {
			node = node->next;
		}
	}
}

void 
gedit_text_region_add (GeditTextRegion     *region,
		     const GtkTextIter *_start,
		     const GtkTextIter *_end)
{
	GList *start_node, *end_node;
	GtkTextIter start, end;
	
	g_return_if_fail (region != NULL && _start != NULL && _end != NULL);
	
	start = *_start;
	end = *_end;
	
	DEBUG (g_print ("---\n"));
	DEBUG (gedit_text_region_debug_print (region));
	DEBUG (g_message ("region_add (%d, %d)",
			  gtk_text_iter_get_offset (&start),
			  gtk_text_iter_get_offset (&end)));

	gtk_text_iter_order (&start, &end);
	
	/* don't add zero-length regions */
	if (gtk_text_iter_equal (&start, &end))
		return;

	/* find bounding subregions */
	start_node = find_nearest_subregion (region, &start, NULL, FALSE, TRUE);
	end_node = find_nearest_subregion (region, &end, start_node, TRUE, TRUE);

	if (start_node == NULL || end_node == NULL || end_node == start_node->prev) {
		/* create the new subregion */
		Subregion *sr = g_new0 (Subregion, 1);
		sr->start = gtk_text_buffer_create_mark (region->buffer, NULL, &start, TRUE);
		sr->end = gtk_text_buffer_create_mark (region->buffer, NULL, &end, FALSE);
		
		if (start_node == NULL) {
			/* append the new region */
			region->subregions = g_list_append (region->subregions, sr);
			
		} else if (end_node == NULL) {
			/* prepend the new region */
			region->subregions = g_list_prepend (region->subregions, sr);

		} else {
			/* we are in the middle of two subregions */
			region->subregions = g_list_insert_before (region->subregions,
								   start_node, sr);
		}
	}
	else {
		GtkTextIter iter;
		Subregion *sr = start_node->data;
		if (start_node != end_node) {
			/* we need to merge some subregions */
			GList *l = start_node->next;
			Subregion *q;
			
			gtk_text_buffer_delete_mark (region->buffer, sr->end);
			while (l != end_node) {
				q = l->data;
				gtk_text_buffer_delete_mark (region->buffer, q->start);
				gtk_text_buffer_delete_mark (region->buffer, q->end);
				g_free (q);
				l = g_list_delete_link (l, l);
			}
			q = l->data;
			gtk_text_buffer_delete_mark (region->buffer, q->start);
			sr->end = q->end;
			g_free (q);
			l = g_list_delete_link (l, l);
		}
		/* now move marks if that action expands the region */
		gtk_text_buffer_get_iter_at_mark (region->buffer, &iter, sr->start);
		if (gtk_text_iter_compare (&iter, &start) > 0)
			gtk_text_buffer_move_mark (region->buffer, sr->start, &start);
		gtk_text_buffer_get_iter_at_mark (region->buffer, &iter, sr->end);
		if (gtk_text_iter_compare (&iter, &end) < 0)
			gtk_text_buffer_move_mark (region->buffer, sr->end, &end);
	}

	++region->time_stamp;

	DEBUG (gedit_text_region_debug_print (region));
}

void 
gedit_text_region_subtract (GeditTextRegion     *region,
			  const GtkTextIter *_start,
			  const GtkTextIter *_end)
{
	GList *start_node, *end_node, *node;
	GtkTextIter sr_start_iter, sr_end_iter;
	gboolean done;
	gboolean start_is_outside, end_is_outside;
	Subregion *sr;
	GtkTextIter start, end;

	g_return_if_fail (region != NULL && _start != NULL && _end != NULL);
	
	start = *_start;
	end = *_end;
	
	DEBUG (g_print ("---\n"));
	DEBUG (gedit_text_region_debug_print (region));
	DEBUG (g_message ("region_substract (%d, %d)",
			  gtk_text_iter_get_offset (&start),
			  gtk_text_iter_get_offset (&end)));
	
	gtk_text_iter_order (&start, &end);
	
	/* find bounding subregions */
	start_node = find_nearest_subregion (region, &start, NULL, FALSE, FALSE);
	end_node = find_nearest_subregion (region, &end, start_node, TRUE, FALSE);

	/* easy case first */
	if (start_node == NULL || end_node == NULL || end_node == start_node->prev)
		return;
	
	/* deal with the start point */
	start_is_outside = end_is_outside = FALSE;
	
	sr = start_node->data;
	gtk_text_buffer_get_iter_at_mark (region->buffer, &sr_start_iter, sr->start);
	gtk_text_buffer_get_iter_at_mark (region->buffer, &sr_end_iter, sr->end);

	if (gtk_text_iter_in_range (&start, &sr_start_iter, &sr_end_iter) &&
	    !gtk_text_iter_equal (&start, &sr_start_iter)) {
		/* the starting point is inside the first subregion */
		if (gtk_text_iter_in_range (&end, &sr_start_iter, &sr_end_iter) &&
		    !gtk_text_iter_equal (&end, &sr_end_iter)) {
			/* the ending point is also inside the first
                           subregion: we need to split */
			Subregion *new_sr = g_new0 (Subregion, 1);
			new_sr->end = sr->end;
			new_sr->start = gtk_text_buffer_create_mark (region->buffer,
								     NULL, &end, TRUE);
			start_node = g_list_insert_before (start_node, start_node->next, new_sr);

			sr->end = gtk_text_buffer_create_mark (region->buffer,
							       NULL, &start, FALSE);

			/* no further processing needed */
			DEBUG (g_message ("subregion splitted"));
			
			return;
		} else {
			/* the ending point is outside, so just move
                           the end of the subregion to the starting point */
			gtk_text_buffer_move_mark (region->buffer, sr->end, &start);
		}
	} else {
		/* the starting point is outside (and so to the left)
                   of the first subregion */
		DEBUG (g_message ("start is outside"));
			
		start_is_outside = TRUE;
	}
	
	/* deal with the end point */
	if (start_node != end_node) {
		sr = end_node->data;
		gtk_text_buffer_get_iter_at_mark (region->buffer, &sr_start_iter, sr->start);
		gtk_text_buffer_get_iter_at_mark (region->buffer, &sr_end_iter, sr->end);
	}
	
	if (gtk_text_iter_in_range (&end, &sr_start_iter, &sr_end_iter) &&
	    !gtk_text_iter_equal (&end, &sr_end_iter)) {
		/* ending point is inside, move the start mark */
		gtk_text_buffer_move_mark (region->buffer, sr->start, &end);
	} else {
		end_is_outside = TRUE;
		DEBUG (g_message ("end is outside"));
		
	}
	
	/* finally remove any intermediate subregions */
	done = FALSE;
	node = start_node;
	
	while (!done) {
		if (node == end_node)
			/* we are done, exit in the next iteration */
			done = TRUE;
		
		if ((node == start_node && !start_is_outside) ||
		    (node == end_node && !end_is_outside)) {
			/* skip starting or ending node */
			node = node->next;
		} else {
			GList *l = node->next;
			sr = node->data;
			gtk_text_buffer_delete_mark (region->buffer, sr->start);
			gtk_text_buffer_delete_mark (region->buffer, sr->end);
			g_free (sr);
			region->subregions = g_list_delete_link (region->subregions,
								 node);
			node = l;
		}
	}

	++region->time_stamp;

	DEBUG (gedit_text_region_debug_print (region));

	/* now get rid of empty subregions */
	gedit_text_region_clear_zero_length_subregions (region);

	DEBUG (gedit_text_region_debug_print (region));
}

gint 
gedit_text_region_subregions (GeditTextRegion *region)
{
	g_return_val_if_fail (region != NULL, 0);

	return g_list_length (region->subregions);
}

gboolean 
gedit_text_region_nth_subregion (GeditTextRegion *region,
			       guint          subregion,
			       GtkTextIter   *start,
			       GtkTextIter   *end)
{
	Subregion *sr;
	
	g_return_val_if_fail (region != NULL, FALSE);

	sr = g_list_nth_data (region->subregions, subregion);
	if (sr == NULL)
		return FALSE;

	if (start)
		gtk_text_buffer_get_iter_at_mark (region->buffer, start, sr->start);
	if (end)
		gtk_text_buffer_get_iter_at_mark (region->buffer, end, sr->end);

	return TRUE;
}

GeditTextRegion * 
gedit_text_region_intersect (GeditTextRegion     *region,
			   const GtkTextIter *_start,
			   const GtkTextIter *_end)
{
	GList *start_node, *end_node, *node;
	GtkTextIter sr_start_iter, sr_end_iter;
	Subregion *sr, *new_sr;
	gboolean done;
	GeditTextRegion *new_region;
	GtkTextIter start, end;
	
	g_return_val_if_fail (region != NULL && _start != NULL && _end != NULL, NULL);
	
	start = *_start;
	end = *_end;
	
	gtk_text_iter_order (&start, &end);
	
	/* find bounding subregions */
	start_node = find_nearest_subregion (region, &start, NULL, FALSE, FALSE);
	end_node = find_nearest_subregion (region, &end, start_node, TRUE, FALSE);

	/* easy case first */
	if (start_node == NULL || end_node == NULL || end_node == start_node->prev)
		return NULL;
	
	new_region = gedit_text_region_new (region->buffer);
	done = FALSE;
	
	sr = start_node->data;
	gtk_text_buffer_get_iter_at_mark (region->buffer, &sr_start_iter, sr->start);
	gtk_text_buffer_get_iter_at_mark (region->buffer, &sr_end_iter, sr->end);

	/* starting node */
	if (gtk_text_iter_in_range (&start, &sr_start_iter, &sr_end_iter)) {
		new_sr = g_new0 (Subregion, 1);
		new_region->subregions = g_list_prepend (new_region->subregions, new_sr);

		new_sr->start = gtk_text_buffer_create_mark (new_region->buffer, NULL,
							     &start, TRUE);
		if (start_node == end_node) {
			/* things will finish shortly */
			done = TRUE;
			if (gtk_text_iter_in_range (&end, &sr_start_iter, &sr_end_iter))
				new_sr->end = gtk_text_buffer_create_mark (new_region->buffer,
									   NULL, &end, FALSE);
			else
				new_sr->end = gtk_text_buffer_create_mark (new_region->buffer,
									   NULL, &sr_end_iter,
									   FALSE);
		} else {
			new_sr->end = gtk_text_buffer_create_mark (new_region->buffer, NULL,
								   &sr_end_iter, FALSE);
		}
		node = start_node->next;
	} else {
		/* start should be the same as the subregion, so copy it in the loop */
		node = start_node;
	}

	if (!done) {
		while (node != end_node) {
			/* copy intermediate subregions verbatim */
			sr = node->data;
			gtk_text_buffer_get_iter_at_mark (region->buffer, &sr_start_iter,
							  sr->start);
			gtk_text_buffer_get_iter_at_mark (region->buffer, &sr_end_iter, sr->end);
			
			new_sr = g_new0 (Subregion, 1);
			new_region->subregions = g_list_prepend (new_region->subregions, new_sr);
			new_sr->start = gtk_text_buffer_create_mark (new_region->buffer, NULL,
								     &sr_start_iter, TRUE);
			new_sr->end = gtk_text_buffer_create_mark (new_region->buffer, NULL,
								   &sr_end_iter, FALSE);
			/* next node */
			node = node->next;
		}

		/* ending node */
		sr = node->data;
		gtk_text_buffer_get_iter_at_mark (region->buffer, &sr_start_iter, sr->start);
		gtk_text_buffer_get_iter_at_mark (region->buffer, &sr_end_iter, sr->end);
		
		new_sr = g_new0 (Subregion, 1);
		new_region->subregions = g_list_prepend (new_region->subregions, new_sr);
		
		new_sr->start = gtk_text_buffer_create_mark (new_region->buffer, NULL,
							     &sr_start_iter, TRUE);

		if (gtk_text_iter_in_range (&end, &sr_start_iter, &sr_end_iter))
			new_sr->end = gtk_text_buffer_create_mark (new_region->buffer, NULL,
								   &end, FALSE);
		else
			new_sr->end = gtk_text_buffer_create_mark (new_region->buffer, NULL,
								   &sr_end_iter, FALSE);
	}

	new_region->subregions = g_list_reverse (new_region->subregions);
	return new_region;
}

static gboolean 
check_iterator (GeditTextRegionIteratorReal *real)
{
	if ((real->region == NULL) ||
	    (real->region_time_stamp != real->region->time_stamp))
	{
		g_warning("Invalid iterator: either the iterator "
                	  "is uninitialized, or the region "
                 	  "has been modified since the iterator "
                 	  "was created.");
                 	  
                return FALSE;
	}

	return TRUE;
}

void       
gedit_text_region_get_iterator (GeditTextRegion         *region,
                              GeditTextRegionIterator *iter,
                              guint                  start)
{
	GeditTextRegionIteratorReal *real;

	g_return_if_fail (region != NULL);
	g_return_if_fail (iter != NULL);	

	real = (GeditTextRegionIteratorReal *)iter;

	/* region->subregions may be NULL, -> end iter */

	real->region = region;
	real->subregions = g_list_nth (region->subregions, start);
	real->region_time_stamp = region->time_stamp;
}

gboolean
gedit_text_region_iterator_is_end (GeditTextRegionIterator *iter)
{
	GeditTextRegionIteratorReal *real;

	g_return_val_if_fail (iter != NULL, FALSE);	

	real = (GeditTextRegionIteratorReal *)iter;
	g_return_val_if_fail (check_iterator (real), FALSE);

	return (real->subregions == NULL);
}

gboolean
gedit_text_region_iterator_next (GeditTextRegionIterator *iter)
{
	GeditTextRegionIteratorReal *real;

	g_return_val_if_fail (iter != NULL, FALSE);	

	real = (GeditTextRegionIteratorReal *)iter;
	g_return_val_if_fail (check_iterator (real), FALSE);

	if (real->subregions != NULL) {
		real->subregions = g_list_next (real->subregions);
		return TRUE;
	}
	else
		return FALSE;
}

void       
gedit_text_region_iterator_get_subregion (GeditTextRegionIterator *iter,
					GtkTextIter           *start,
					GtkTextIter           *end)
{
	GeditTextRegionIteratorReal *real;
	Subregion *sr;

	g_return_if_fail (iter != NULL);

	real = (GeditTextRegionIteratorReal *)iter;
	g_return_if_fail (check_iterator (real));
	g_return_if_fail (real->subregions != NULL);

	sr = (Subregion*)real->subregions->data;
	g_return_if_fail (sr != NULL);

	if (start)
		gtk_text_buffer_get_iter_at_mark (real->region->buffer, start, sr->start);
	if (end)
		gtk_text_buffer_get_iter_at_mark (real->region->buffer, end, sr->end);
}

void 
gedit_text_region_debug_print (GeditTextRegion *region)
{
	GList *l;
	
	g_return_if_fail (region != NULL);

	g_print ("Subregions: ");
	l = region->subregions;
	while (l) {
		Subregion *sr = l->data;
		GtkTextIter iter1, iter2;
		gtk_text_buffer_get_iter_at_mark (region->buffer, &iter1, sr->start);
		gtk_text_buffer_get_iter_at_mark (region->buffer, &iter2, sr->end);
		g_print ("%d-%d ", gtk_text_iter_get_offset (&iter1),
			 gtk_text_iter_get_offset (&iter2));
		l = l->next;
	}
	g_print ("\n");
}

