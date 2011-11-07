/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gedit-automatic-spell-checker.c
 * This file is part of gedit
 *
 * Copyright (C) 2002 Paolo Maggi 
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
 * Modified by the gedit Team, 2002. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 */

/* This is a modified version of gtkspell 2.0.5  (gtkspell.sf.net) */
/* gtkspell - a spell-checking addon for GTK's TextView widget
 * Copyright (c) 2002 Evan Martin.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <glib/gi18n.h>

#include "gedit-automatic-spell-checker.h"
#include "gedit-spell-utils.h"

struct _GeditAutomaticSpellChecker {
	GeditDocument		*doc;
	GSList 			*views;
	
	GtkTextMark 		*mark_insert_start;
	GtkTextMark		*mark_insert_end;
	gboolean 		 deferred_check;

	GtkTextTag 		*tag_highlight;
	GtkTextMark		*mark_click;

       	GeditSpellChecker	*spell_checker;
};

static GQuark automatic_spell_checker_id = 0;
static GQuark suggestion_id = 0;

static void gedit_automatic_spell_checker_free_internal (GeditAutomaticSpellChecker *spell);

static void
view_destroy (GeditView *view, GeditAutomaticSpellChecker *spell)
{
	gedit_automatic_spell_checker_detach_view (spell, view);
}

static void
check_word (GeditAutomaticSpellChecker *spell, GtkTextIter *start, GtkTextIter *end) 
{
	gchar *word;

	word = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (spell->doc), start, end, FALSE);

	/*
	g_print ("Check word: %s [%d - %d]\n", word, gtk_text_iter_get_offset (start),
						gtk_text_iter_get_offset (end));
	*/

	if (!gedit_spell_checker_check_word (spell->spell_checker, word, -1)) 
	{
		/*
		g_print ("Apply tag: [%d - %d]\n", gtk_text_iter_get_offset (start),
						gtk_text_iter_get_offset (end));
		*/
		gtk_text_buffer_apply_tag (GTK_TEXT_BUFFER (spell->doc), 
					   spell->tag_highlight, 
					   start, 
					   end);
	}
	
	g_free (word);
}

static void
check_range (GeditAutomaticSpellChecker *spell, 
	     GtkTextIter                 start, 
	     GtkTextIter                 end,
	     gboolean                    force_all) 
{
	/* we need to "split" on word boundaries.
	 * luckily, Pango knows what "words" are 
	 * so we don't have to figure it out. */

	GtkTextIter wstart;
	GtkTextIter wend;
	GtkTextIter cursor; 
	GtkTextIter precursor;
  	gboolean    highlight;

	/*
	g_print ("Check range: [%d - %d]\n", gtk_text_iter_get_offset (&start),
						gtk_text_iter_get_offset (&end));
	*/

	if (gtk_text_iter_inside_word (&end))
		gtk_text_iter_forward_word_end (&end);
	
	if (!gtk_text_iter_starts_word (&start)) 
	{
		if (gtk_text_iter_inside_word (&start) || 
		    gtk_text_iter_ends_word (&start)) 
		{
			gtk_text_iter_backward_word_start (&start);
		} 
		else 
		{
			/* if we're neither at the beginning nor inside a word,
			 * me must be in some spaces.
			 * skip forward to the beginning of the next word. */
			
			if (gtk_text_iter_forward_word_end (&start))
				gtk_text_iter_backward_word_start (&start);
		}
	}

	gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (spell->doc), 
					  &cursor,
					  gtk_text_buffer_get_insert (GTK_TEXT_BUFFER (spell->doc)));
  	 
	precursor = cursor;
	gtk_text_iter_backward_char (&precursor);
	
  	highlight = gtk_text_iter_has_tag (&cursor, spell->tag_highlight) ||
  	            gtk_text_iter_has_tag (&precursor, spell->tag_highlight);
	
	gtk_text_buffer_remove_tag (GTK_TEXT_BUFFER (spell->doc), 
				    spell->tag_highlight, 
				    &start, 
				    &end);

	/* Fix a corner case when replacement occurs at beginning of buffer:
	 * An iter at offset 0 seems to always be inside a word,
  	 * even if it's not.  Possibly a pango bug.
	 */
  	if (gtk_text_iter_get_offset (&start) == 0) 
	{
		gtk_text_iter_forward_word_end(&start);
		gtk_text_iter_backward_word_start(&start);
	}

	wstart = start;
	
	while (gedit_spell_utils_skip_no_spell_check (&wstart, &end) &&
	       gtk_text_iter_compare (&wstart, &end) < 0) 
	{
		gboolean inword; 

		/* move wend to the end of the current word. */
		wend = wstart;
		
		gtk_text_iter_forward_word_end (&wend);

		inword = (gtk_text_iter_compare (&wstart, &cursor) < 0) &&
			 (gtk_text_iter_compare (&cursor, &wend) <= 0);
  	 
		if (inword && !force_all)
		{
			/* this word is being actively edited,
			 * only check if it's already highligted,
			 * otherwise defer this check until later. */
			if (highlight)
				check_word (spell, &wstart, &wend);
			else
				spell->deferred_check = TRUE;
		} 
		else 
		{
			check_word (spell, &wstart, &wend);
			spell->deferred_check = FALSE;
		}

		/* now move wend to the beginning of the next word, */
		gtk_text_iter_forward_word_end (&wend);
		gtk_text_iter_backward_word_start (&wend);
		
		/* make sure we've actually advanced
		 * (we don't advance in some corner cases), */
		if (gtk_text_iter_equal (&wstart, &wend))
			break; /* we're done in these cases.. */

		/* and then pick this as the new next word beginning. */
		wstart = wend;
	}
}

static void
check_deferred_range (GeditAutomaticSpellChecker *spell, 
		      gboolean                    force_all) 
{
	GtkTextIter start, end;

	gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (spell->doc), 
					  &start, 
					  spell->mark_insert_start);
	gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (spell->doc),
					  &end, 
					  spell->mark_insert_end);

	check_range (spell, start, end, force_all);
}

/* insertion works like this:
 *  - before the text is inserted, we mark the position in the buffer.
 *  - after the text is inserted, we see where our mark is and use that and
 *    the current position to check the entire range of inserted text.
 *
 * this may be overkill for the common case (inserting one character). */

static void
insert_text_before (GtkTextBuffer *buffer, GtkTextIter *iter,
		gchar *text, gint len, GeditAutomaticSpellChecker *spell) 
{
	gtk_text_buffer_move_mark (buffer, spell->mark_insert_start, iter);
}

static void
insert_text_after (GtkTextBuffer *buffer, GtkTextIter *iter,
                  gchar *text, gint len, GeditAutomaticSpellChecker *spell) 
{
	GtkTextIter start;

	/* we need to check a range of text. */
	gtk_text_buffer_get_iter_at_mark (buffer, &start, spell->mark_insert_start);
	
	check_range (spell, start, *iter, FALSE);

	gtk_text_buffer_move_mark (buffer, spell->mark_insert_end, iter);
}

/* deleting is more simple:  we're given the range of deleted text.
 * after deletion, the start and end iters should be at the same position
 * (because all of the text between them was deleted!).
 * this means we only really check the words immediately bounding the
 * deletion.
 */

static void
delete_range_after (GtkTextBuffer *buffer, GtkTextIter *start, GtkTextIter *end, 
		GeditAutomaticSpellChecker *spell) 
{
	check_range (spell, *start, *end, FALSE);
}

static void
mark_set (GtkTextBuffer              *buffer, 
	  GtkTextIter                *iter,
	  GtkTextMark                *mark, 
	  GeditAutomaticSpellChecker *spell) 
{
	/* if the cursor has moved and there is a deferred check so handle it now */
	if ((mark == gtk_text_buffer_get_insert (buffer)) && spell->deferred_check)
		check_deferred_range (spell, FALSE);
}

static void
get_word_extents_from_mark (GtkTextBuffer *buffer, 
			    GtkTextIter   *start, 
			    GtkTextIter   *end,
			    GtkTextMark   *mark)
{
	gtk_text_buffer_get_iter_at_mark(buffer, start, mark);
	
	if (!gtk_text_iter_starts_word (start)) 
		gtk_text_iter_backward_word_start (start);
	
	*end = *start;
	
	if (gtk_text_iter_inside_word (end))
		gtk_text_iter_forward_word_end (end);
}

static void
remove_tag_to_word (GeditAutomaticSpellChecker *spell, const gchar *word)
{
	GtkTextIter iter;
	GtkTextIter match_start, match_end;

	gboolean found;

	gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (spell->doc), &iter, 0);
	
	found = TRUE;

	while (found)
	{
		found = gtk_text_iter_forward_search (&iter, 
				word, 
				GTK_TEXT_SEARCH_VISIBLE_ONLY | GTK_TEXT_SEARCH_TEXT_ONLY,
				&match_start, 
				&match_end,
				NULL);	

		if (found)
		{
			if (gtk_text_iter_starts_word (&match_start) &&
			    gtk_text_iter_ends_word (&match_end))
			{
				gtk_text_buffer_remove_tag (GTK_TEXT_BUFFER (spell->doc),
						spell->tag_highlight,
						&match_start, 
						&match_end);
			}

			iter = match_end;
		}
	}
}

static void
add_to_dictionary (GtkWidget *menuitem, GeditAutomaticSpellChecker *spell) 
{
	gchar *word;
	
	GtkTextIter start, end;
	
	get_word_extents_from_mark (GTK_TEXT_BUFFER (spell->doc), &start, &end, spell->mark_click);	

	word = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (spell->doc), 
					 &start, 
					 &end, 
					 FALSE);
	
	gedit_spell_checker_add_word_to_personal (spell->spell_checker, word, -1);

	g_free (word);
}

static void
ignore_all (GtkWidget *menuitem, GeditAutomaticSpellChecker *spell) 
{
	gchar *word;
	
	GtkTextIter start, end;
	
	get_word_extents_from_mark (GTK_TEXT_BUFFER (spell->doc), &start, &end, spell->mark_click);	

	word = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (spell->doc), 
					 &start, 
					 &end, 
					 FALSE);
	
	gedit_spell_checker_add_word_to_session (spell->spell_checker, word, -1);

	g_free (word);
}

static void
replace_word (GtkWidget *menuitem, GeditAutomaticSpellChecker *spell) 
{
	gchar *oldword;
	const gchar *newword;
	
	GtkTextIter start, end;

	get_word_extents_from_mark (GTK_TEXT_BUFFER (spell->doc), &start, &end, spell->mark_click);	

	oldword = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (spell->doc), &start, &end, FALSE);
	
	newword =  g_object_get_qdata (G_OBJECT (menuitem), suggestion_id);
	g_return_if_fail (newword != NULL);

	gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER (spell->doc));

	gtk_text_buffer_delete (GTK_TEXT_BUFFER (spell->doc), &start, &end);
	gtk_text_buffer_insert (GTK_TEXT_BUFFER (spell->doc), &start, newword, -1);

	gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (spell->doc));

	gedit_spell_checker_set_correction (spell->spell_checker, 
				oldword, strlen (oldword),
				newword, strlen (newword));

	g_free (oldword);
}

static GtkWidget *
build_suggestion_menu (GeditAutomaticSpellChecker *spell, const gchar *word) 
{
	GtkWidget *topmenu, *menu;
	GtkWidget *mi;
	GSList *suggestions;
	GSList *list;
	gchar *label_text;
	
	topmenu = menu = gtk_menu_new();

	suggestions = gedit_spell_checker_get_suggestions (spell->spell_checker, word, -1);

	list = suggestions;

	if (suggestions == NULL) 
	{		
		/* no suggestions.  put something in the menu anyway... */
		GtkWidget *label;
		/* Translators: Displayed in the "Check Spelling" dialog if there are no suggestions for the current misspelled word */
		label = gtk_label_new (_("(no suggested words)"));
		
		mi = gtk_menu_item_new ();
		gtk_widget_set_sensitive (mi, FALSE);
		gtk_container_add (GTK_CONTAINER(mi), label);
		gtk_widget_show_all (mi);
		gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), mi);
	} 
	else 
	{
		gint count = 0;
		
		/* build a set of menus with suggestions. */
		while (suggestions != NULL) 
		{
			GtkWidget *label;

			if (count == 10) 
			{
				/* Separator */
				mi = gtk_menu_item_new ();
				gtk_widget_show (mi);
				gtk_menu_shell_append (GTK_MENU_SHELL (menu), mi);
	
				mi = gtk_menu_item_new_with_mnemonic (_("_More..."));
				gtk_widget_show (mi);
				gtk_menu_shell_append (GTK_MENU_SHELL (menu), mi);

				menu = gtk_menu_new ();
				gtk_menu_item_set_submenu (GTK_MENU_ITEM (mi), menu);
				count = 0;
			}
			
			label_text = g_strdup_printf ("<b>%s</b>", (gchar*) suggestions->data);
			
			label = gtk_label_new (label_text);
			gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
			gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);

			mi = gtk_menu_item_new ();
			gtk_container_add (GTK_CONTAINER(mi), label);
			
			gtk_widget_show_all (mi);
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), mi);

			g_object_set_qdata_full (G_OBJECT (mi), 
				 suggestion_id, 
				 g_strdup (suggestions->data), 
				 (GDestroyNotify)g_free);

			g_free (label_text);
			g_signal_connect (mi,
					  "activate",
					  G_CALLBACK (replace_word),
					  spell);

			count++;

			suggestions = g_slist_next (suggestions);
		}
	}

	/* free the suggestion list */
	suggestions = list;

	while (list)
	{
		g_free (list->data);
		list = g_slist_next (list);
	}

	g_slist_free (suggestions);

	/* Separator */
	mi = gtk_menu_item_new ();
	gtk_widget_show (mi);
	gtk_menu_shell_append (GTK_MENU_SHELL (topmenu), mi);

	/* Ignore all */
	mi = gtk_image_menu_item_new_with_mnemonic (_("_Ignore All"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (mi),
				       gtk_image_new_from_stock (GTK_STOCK_GOTO_BOTTOM, 
					       			 GTK_ICON_SIZE_MENU));
	
	g_signal_connect (mi,
			  "activate",
			  G_CALLBACK(ignore_all),
			  spell);

	gtk_widget_show_all (mi);

	gtk_menu_shell_append (GTK_MENU_SHELL (topmenu), mi);

	/* + Add to Dictionary */
	mi = gtk_image_menu_item_new_with_mnemonic (_("_Add"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (mi),
				       gtk_image_new_from_stock (GTK_STOCK_ADD, 
					       			 GTK_ICON_SIZE_MENU));

	g_signal_connect (mi,
			  "activate",
			  G_CALLBACK (add_to_dictionary),
			  spell);

	gtk_widget_show_all (mi);
	
	gtk_menu_shell_append (GTK_MENU_SHELL (topmenu), mi);

	return topmenu;
}

static void
populate_popup (GtkTextView *textview, GtkMenu *menu, GeditAutomaticSpellChecker *spell) 
{
	GtkWidget *img, *mi;
	GtkTextIter start, end;
	char *word;

	/* we need to figure out if they picked a misspelled word. */
	get_word_extents_from_mark (GTK_TEXT_BUFFER (spell->doc), &start, &end, spell->mark_click);	

	/* if our highlight algorithm ever messes up, 
	 * this isn't correct, either. */
	if (!gtk_text_iter_has_tag (&start, spell->tag_highlight))
		return; /* word wasn't misspelled. */

	/* menu separator comes first. */
	mi = gtk_menu_item_new ();
	gtk_widget_show (mi);
	gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), mi);

	/* then, on top of it, the suggestions menu. */
	img = gtk_image_new_from_stock (GTK_STOCK_SPELL_CHECK, GTK_ICON_SIZE_MENU);
	mi = gtk_image_menu_item_new_with_mnemonic (_("_Spelling Suggestions..."));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (mi), img);

	word = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (spell->doc), &start, &end, FALSE);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (mi),
				   build_suggestion_menu (spell, word));
	g_free(word);

	gtk_widget_show_all (mi);
	gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), mi);
}

void
gedit_automatic_spell_checker_recheck_all (GeditAutomaticSpellChecker *spell)
{
	GtkTextIter start, end;

	g_return_if_fail (spell != NULL);

	gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (spell->doc), &start, &end);

	check_range (spell, start, end, TRUE);
}

static void 
add_word_signal_cb (GeditSpellChecker          *checker, 
		    const gchar                *word, 
		    gint                        len,
		    GeditAutomaticSpellChecker *spell)
{
	gchar *w;

	if (len < 0)
		w = g_strdup (word);
	else
		w = g_strndup (word, len);

	remove_tag_to_word (spell, w);

	g_free (w);
}

static void 
set_language_cb (GeditSpellChecker               *checker,
		 const GeditSpellCheckerLanguage *lang,
		 GeditAutomaticSpellChecker      *spell)
{
	gedit_automatic_spell_checker_recheck_all (spell);
}

static void 
clear_session_cb (GeditSpellChecker          *checker,
		  GeditAutomaticSpellChecker *spell)
{
	gedit_automatic_spell_checker_recheck_all (spell);
}

/* When the user right-clicks on a word, they want to check that word.
 * Here, we do NOT  move the cursor to the location of the clicked-upon word
 * since that prevents the use of edit functions on the context menu. 
 */
static gboolean
button_press_event (GtkTextView *view,
		    GdkEventButton *event,
		    GeditAutomaticSpellChecker *spell) 
{
	if (event->button == 3) 
	{
		gint x, y;
		GtkTextIter iter;

		GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);

		/* handle deferred check if it exists */
  	        if (spell->deferred_check)
			check_deferred_range (spell, TRUE);

		gtk_text_view_window_to_buffer_coords (view, 
				GTK_TEXT_WINDOW_TEXT, 
				event->x, event->y,
				&x, &y);
		
		gtk_text_view_get_iter_at_location (view, &iter, x, y);

		gtk_text_buffer_move_mark (buffer, spell->mark_click, &iter);
	}

	return FALSE; /* false: let gtk process this event, too.
			 we don't want to eat any events. */
}

/* Move the insert mark before popping up the menu, otherwise it
 * will contain the wrong set of suggestions.
 */
static gboolean
popup_menu_event (GtkTextView *view, GeditAutomaticSpellChecker *spell) 
{
	GtkTextIter iter;
	GtkTextBuffer *buffer;

	buffer = gtk_text_view_get_buffer (view);

	/* handle deferred check if it exists */
	if (spell->deferred_check)
		check_deferred_range (spell, TRUE);

	gtk_text_buffer_get_iter_at_mark (buffer, &iter,
					  gtk_text_buffer_get_insert (buffer));
	gtk_text_buffer_move_mark (buffer, spell->mark_click, &iter);

	return FALSE;
}

static void
tag_table_changed (GtkTextTagTable            *table,
		   GeditAutomaticSpellChecker *spell)
{
	g_return_if_fail (spell->tag_highlight !=  NULL);

	gtk_text_tag_set_priority (spell->tag_highlight,
				   gtk_text_tag_table_get_size (table) - 1);
}

static void 
tag_added_or_removed (GtkTextTagTable            *table,
		      GtkTextTag                 *tag,
		      GeditAutomaticSpellChecker *spell)
{
	tag_table_changed (table, spell);
}

static void 
tag_changed (GtkTextTagTable            *table,
	     GtkTextTag                 *tag,
	     gboolean                    size_changed,
	     GeditAutomaticSpellChecker *spell)
{
	tag_table_changed (table, spell);
}

static void
highlight_updated (GtkSourceBuffer            *buffer,
                   GtkTextIter                *start,
                   GtkTextIter                *end,
                   GeditAutomaticSpellChecker *spell)
{
	check_range (spell, *start, *end, FALSE);
}

static void
spell_tag_destroyed (GeditAutomaticSpellChecker *spell,
                     GObject                    *where_the_object_was)
{
	spell->tag_highlight = NULL;
}

GeditAutomaticSpellChecker *
gedit_automatic_spell_checker_new (GeditDocument     *doc,
				   GeditSpellChecker *checker)
{
	GeditAutomaticSpellChecker *spell;
	GtkTextTagTable *tag_table;
	GtkTextIter start, end;

	g_return_val_if_fail (GEDIT_IS_DOCUMENT (doc), NULL);
	g_return_val_if_fail (GEDIT_IS_SPELL_CHECKER (checker), NULL);
	g_return_val_if_fail ((spell = gedit_automatic_spell_checker_get_from_document (doc)) == NULL,
			      spell);
	
	/* attach to the widget */
	spell = g_new0 (GeditAutomaticSpellChecker, 1);

	spell->doc = doc;
	spell->spell_checker = g_object_ref (checker);

	if (automatic_spell_checker_id == 0)
	{
		automatic_spell_checker_id = 
			g_quark_from_string ("GeditAutomaticSpellCheckerID");
	}
	if (suggestion_id == 0)
	{
		suggestion_id = g_quark_from_string ("GeditAutoSuggestionID");
	}

	g_object_set_qdata_full (G_OBJECT (doc), 
				 automatic_spell_checker_id, 
				 spell, 
				 (GDestroyNotify)gedit_automatic_spell_checker_free_internal);

	g_signal_connect (doc,
			  "insert-text",
			  G_CALLBACK (insert_text_before), 
			  spell);
	g_signal_connect_after (doc,
			  "insert-text",
			  G_CALLBACK (insert_text_after), 
			  spell);
	g_signal_connect_after (doc,
			  "delete-range",
			  G_CALLBACK (delete_range_after), 
			  spell);
	g_signal_connect (doc,
			  "mark-set",
			  G_CALLBACK (mark_set), 
			  spell);

	g_signal_connect (doc,
	                  "highlight-updated",
	                  G_CALLBACK (highlight_updated),
	                  spell);

	g_signal_connect (spell->spell_checker,
			  "add_word_to_session",
			  G_CALLBACK (add_word_signal_cb),
			  spell);
	g_signal_connect (spell->spell_checker,
			  "add_word_to_personal",
			  G_CALLBACK (add_word_signal_cb),
			  spell);
	g_signal_connect (spell->spell_checker,
			  "clear_session",
			  G_CALLBACK (clear_session_cb),
			  spell);
	g_signal_connect (spell->spell_checker,
			  "set_language",
			  G_CALLBACK (set_language_cb),
			  spell);

	spell->tag_highlight = gtk_text_buffer_create_tag (
				GTK_TEXT_BUFFER (doc),
				"gtkspell-misspelled",
				"underline", PANGO_UNDERLINE_ERROR,
				NULL);

	g_object_weak_ref (G_OBJECT (spell->tag_highlight),
	                   (GWeakNotify)spell_tag_destroyed,
	                   spell);

	tag_table = gtk_text_buffer_get_tag_table (GTK_TEXT_BUFFER (doc));

	gtk_text_tag_set_priority (spell->tag_highlight, 
				   gtk_text_tag_table_get_size (tag_table) - 1);

	g_signal_connect (tag_table,
			  "tag-added",
			  G_CALLBACK (tag_added_or_removed),
			  spell);
	g_signal_connect (tag_table,
			  "tag-removed",
			  G_CALLBACK (tag_added_or_removed),
			  spell);
	g_signal_connect (tag_table,
			  "tag-changed",
			  G_CALLBACK (tag_changed),
			  spell);

	/* we create the mark here, but we don't use it until text is
	 * inserted, so we don't really care where iter points.  */
	gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (doc), &start, &end);
	
	spell->mark_insert_start = gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (doc),
					"gedit-automatic-spell-checker-insert-start");

	if (spell->mark_insert_start == NULL)
	{
		spell->mark_insert_start = 
			gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (doc),
						     "gedit-automatic-spell-checker-insert-start",
						     &start, 
						     TRUE);
	}
	else
	{
		gtk_text_buffer_move_mark (GTK_TEXT_BUFFER (doc),
					   spell->mark_insert_start,
					   &start);
	}

	spell->mark_insert_end = gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (doc),
					"gedit-automatic-spell-checker-insert-end");

	if (spell->mark_insert_end == NULL)
	{
		spell->mark_insert_end = 
			gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (doc),
						     "gedit-automatic-spell-checker-insert-end",
						     &start, 
						     TRUE);
	}
	else
	{
		gtk_text_buffer_move_mark (GTK_TEXT_BUFFER (doc),
					   spell->mark_insert_end,
					   &start);
	}

	spell->mark_click = gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (doc),
					"gedit-automatic-spell-checker-click");

	if (spell->mark_click == NULL)
	{
		spell->mark_click = 
			gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (doc),
						     "gedit-automatic-spell-checker-click",
						     &start, 
						     TRUE);
	}
	else
	{
		gtk_text_buffer_move_mark (GTK_TEXT_BUFFER (doc),
					   spell->mark_click,
					   &start);
	}

	spell->deferred_check = FALSE;

	return spell;
}

GeditAutomaticSpellChecker *
gedit_automatic_spell_checker_get_from_document (const GeditDocument *doc)
{
	g_return_val_if_fail (GEDIT_IS_DOCUMENT (doc), NULL);

	if (automatic_spell_checker_id == 0)
		return NULL;

	return g_object_get_qdata (G_OBJECT (doc), automatic_spell_checker_id);
}	

void
gedit_automatic_spell_checker_free (GeditAutomaticSpellChecker *spell)
{
	g_return_if_fail (spell != NULL);
	g_return_if_fail (gedit_automatic_spell_checker_get_from_document (spell->doc) == spell);
	
	if (automatic_spell_checker_id == 0)
		return;

	g_object_set_qdata (G_OBJECT (spell->doc), automatic_spell_checker_id, NULL);	
}

static void
gedit_automatic_spell_checker_free_internal (GeditAutomaticSpellChecker *spell)
{
	GtkTextTagTable *table;
	GtkTextIter start, end;
	GSList *list;
	
	g_return_if_fail (spell != NULL);

	table = gtk_text_buffer_get_tag_table (GTK_TEXT_BUFFER (spell->doc));

	if (table != NULL && spell->tag_highlight != NULL)
	{
		gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (spell->doc), 
					    &start, 
					    &end);
		gtk_text_buffer_remove_tag (GTK_TEXT_BUFFER (spell->doc), 
					    spell->tag_highlight, 
					    &start, 
					    &end);

		g_signal_handlers_disconnect_matched (G_OBJECT (table),
					G_SIGNAL_MATCH_DATA,
					0, 0, NULL, NULL,
					spell);

		gtk_text_tag_table_remove (table, spell->tag_highlight);
	}
		
	g_signal_handlers_disconnect_matched (G_OBJECT (spell->doc),
			G_SIGNAL_MATCH_DATA,
			0, 0, NULL, NULL,
			spell);

	g_signal_handlers_disconnect_matched (G_OBJECT (spell->spell_checker),
			G_SIGNAL_MATCH_DATA,
			0, 0, NULL, NULL,
			spell);

	g_object_unref (spell->spell_checker);

	list = spell->views;
	while (list != NULL)
	{
		GeditView *view = GEDIT_VIEW (list->data);
		
		g_signal_handlers_disconnect_matched (G_OBJECT (view),
				G_SIGNAL_MATCH_DATA,
				0, 0, NULL, NULL,
				spell);

		g_signal_handlers_disconnect_matched (G_OBJECT (view),
			G_SIGNAL_MATCH_DATA,
			0, 0, NULL, NULL,
			spell);

		list = g_slist_next (list);
	}

	g_slist_free (spell->views);
	
	g_free (spell);
}

void
gedit_automatic_spell_checker_attach_view (
		GeditAutomaticSpellChecker *spell, 
		GeditView *view)
{
	g_return_if_fail (spell != NULL);
	g_return_if_fail (GEDIT_IS_VIEW (view));

	g_return_if_fail (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)) ==
			  GTK_TEXT_BUFFER (spell->doc));

	g_signal_connect (view,
			  "button-press-event",
			  G_CALLBACK (button_press_event), 
			  spell);
	g_signal_connect (view,
			  "popup-menu",
			  G_CALLBACK (popup_menu_event), 
			  spell);
	g_signal_connect (view,
			  "populate-popup",
			  G_CALLBACK (populate_popup), 
			  spell);
	g_signal_connect (view,
			  "destroy",
			  G_CALLBACK (view_destroy), 
			  spell);

	spell->views = g_slist_prepend (spell->views, view);
}

void 
gedit_automatic_spell_checker_detach_view (
		GeditAutomaticSpellChecker *spell,
		GeditView *view)
{
	g_return_if_fail (spell != NULL);
	g_return_if_fail (GEDIT_IS_VIEW (view));

	g_return_if_fail (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)) ==
			  GTK_TEXT_BUFFER (spell->doc));
	g_return_if_fail (spell->views != NULL);

	g_signal_handlers_disconnect_matched (G_OBJECT (view),
			G_SIGNAL_MATCH_DATA,
			0, 0, NULL, NULL,
			spell);

	g_signal_handlers_disconnect_matched (G_OBJECT (view),
			G_SIGNAL_MATCH_DATA,
			0, 0, NULL, NULL,
			spell);

	spell->views = g_slist_remove (spell->views, view);
}

