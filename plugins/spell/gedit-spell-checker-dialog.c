/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gedit-spell-checker-dialog.c
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gedit/gedit-utils.h>
#include "gedit-spell-checker-dialog.h"
#include "gedit-spell-marshal.h"

struct _GeditSpellCheckerDialog 
{
	GtkWindow parent_instance;

	GeditSpellChecker 	*spell_checker;

	gchar			*misspelled_word;

	GtkWidget 		*misspelled_word_label;
	GtkWidget		*word_entry;
	GtkWidget		*check_word_button;
	GtkWidget		*ignore_button;
	GtkWidget		*ignore_all_button;
	GtkWidget		*change_button;
	GtkWidget		*change_all_button;
	GtkWidget		*add_word_button;
	GtkWidget		*close_button;
	GtkWidget		*suggestions_list;
	GtkWidget		*language_label;

	GtkTreeModel		*suggestions_list_model;
};

enum
{
	IGNORE,
	IGNORE_ALL,
	CHANGE,
	CHANGE_ALL,
	ADD_WORD_TO_PERSONAL,
	LAST_SIGNAL
};

enum
{
	COLUMN_SUGGESTIONS,
	NUM_COLUMNS
};

static void	update_suggestions_list_model 			(GeditSpellCheckerDialog *dlg, 
								 GSList *suggestions);

static void	word_entry_changed_handler			(GtkEditable *editable, 
								 GeditSpellCheckerDialog *dlg);
static void	close_button_clicked_handler 			(GtkButton *button, 
								 GeditSpellCheckerDialog *dlg);
static void	suggestions_list_selection_changed_handler 	(GtkTreeSelection *selection,
								 GeditSpellCheckerDialog *dlg);
static void	check_word_button_clicked_handler 		(GtkButton *button, 
								 GeditSpellCheckerDialog *dlg);
static void	add_word_button_clicked_handler 		(GtkButton *button, 
								 GeditSpellCheckerDialog *dlg);
static void	ignore_button_clicked_handler 			(GtkButton *button, 
								 GeditSpellCheckerDialog *dlg);
static void	ignore_all_button_clicked_handler 		(GtkButton *button, 
								 GeditSpellCheckerDialog *dlg);
static void	change_button_clicked_handler 			(GtkButton *button, 
								 GeditSpellCheckerDialog *dlg);
static void	change_all_button_clicked_handler 		(GtkButton *button, 
								 GeditSpellCheckerDialog *dlg);
static void	suggestions_list_row_activated_handler		(GtkTreeView *view,
								 GtkTreePath *path,
								 GtkTreeViewColumn *column,
								 GeditSpellCheckerDialog *dlg);


static guint signals [LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(GeditSpellCheckerDialog, gedit_spell_checker_dialog, GTK_TYPE_WINDOW)

static void
gedit_spell_checker_dialog_destroy (GtkObject *object)
{
	GeditSpellCheckerDialog *dlg = GEDIT_SPELL_CHECKER_DIALOG (object);

	if (dlg->spell_checker != NULL)
	{
		g_object_unref (dlg->spell_checker);
		dlg->spell_checker = NULL;
	}

	if (dlg->misspelled_word != NULL)
	{
		g_free (dlg->misspelled_word);
		dlg->misspelled_word = NULL;
	}

	GTK_OBJECT_CLASS (gedit_spell_checker_dialog_parent_class)->destroy (object);
}

static void
gedit_spell_checker_dialog_class_init (GeditSpellCheckerDialogClass * klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);

	GTK_OBJECT_CLASS (object_class)->destroy = gedit_spell_checker_dialog_destroy;

	signals[IGNORE] = 
		g_signal_new ("ignore",
 			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GeditSpellCheckerDialogClass, ignore),
			      NULL, NULL,
			      gedit_marshal_VOID__STRING,
			      G_TYPE_NONE, 
			      1, 
			      G_TYPE_STRING);

	signals[IGNORE_ALL] = 
		g_signal_new ("ignore_all",
 			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GeditSpellCheckerDialogClass, ignore_all),
			      NULL, NULL,
			      gedit_marshal_VOID__STRING,
			      G_TYPE_NONE, 
			      1, 
			      G_TYPE_STRING);

	signals[CHANGE] = 
		g_signal_new ("change",
 			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GeditSpellCheckerDialogClass, change),
			      NULL, NULL,
			      gedit_marshal_VOID__STRING_STRING,
			      G_TYPE_NONE, 
			      2, 
			      G_TYPE_STRING,
			      G_TYPE_STRING);
	
	signals[CHANGE_ALL] = 
		g_signal_new ("change_all",
 			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GeditSpellCheckerDialogClass, change_all),
			      NULL, NULL,
			      gedit_marshal_VOID__STRING_STRING,
			      G_TYPE_NONE, 
			      2, 
			      G_TYPE_STRING,
			      G_TYPE_STRING);

	signals[ADD_WORD_TO_PERSONAL] = 
		g_signal_new ("add_word_to_personal",
 			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GeditSpellCheckerDialogClass, add_word_to_personal),
			      NULL, NULL,
			      gedit_marshal_VOID__STRING,
			      G_TYPE_NONE, 
			      1, 
			      G_TYPE_STRING);
}

static void
create_dialog (GeditSpellCheckerDialog *dlg,
	       const gchar *data_dir)
{
	GtkWidget *error_widget;
	GtkWidget *content;
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;
	GtkTreeSelection *selection;
	gchar *root_objects[] = {
		"content",
		"check_word_image",
		"add_word_image",
                "ignore_image",
                "change_image",
                "ignore_all_image",
                "change_all_image",
		NULL
	};
	gboolean ret;
	gchar *ui_file;
	
	g_return_if_fail (dlg != NULL);

	dlg->spell_checker = NULL;
	dlg->misspelled_word = NULL;

	ui_file = g_build_filename (data_dir, "spell-checker.ui", NULL);
	ret = gedit_utils_get_ui_objects (ui_file,
		root_objects,
		&error_widget,

		"content", &content,
		"misspelled_word_label", &dlg->misspelled_word_label,
		"word_entry", &dlg->word_entry,
		"check_word_button", &dlg->check_word_button,
		"ignore_button", &dlg->ignore_button,
		"ignore_all_button", &dlg->ignore_all_button,
		"change_button", &dlg->change_button,
		"change_all_button", &dlg->change_all_button,
		"add_word_button", &dlg->add_word_button,
		"close_button", &dlg->close_button,
		"suggestions_list", &dlg->suggestions_list,
		"language_label", &dlg->language_label,
		NULL);
	g_free (ui_file);

	if (!ret)
	{
		gtk_widget_show (error_widget);

		gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dlg))),
				    error_widget, TRUE, TRUE, 0);

		return;
	}

	gtk_label_set_label (GTK_LABEL (dlg->misspelled_word_label), "");
	gtk_widget_set_sensitive (dlg->word_entry, FALSE);
	gtk_widget_set_sensitive (dlg->check_word_button, FALSE);
	gtk_widget_set_sensitive (dlg->ignore_button, FALSE);
	gtk_widget_set_sensitive (dlg->ignore_all_button, FALSE);
	gtk_widget_set_sensitive (dlg->change_button, FALSE);
	gtk_widget_set_sensitive (dlg->change_all_button, FALSE);
	gtk_widget_set_sensitive (dlg->add_word_button, FALSE);
	
	gtk_label_set_label (GTK_LABEL (dlg->language_label), "");
			
	gtk_container_add (GTK_CONTAINER (dlg), content);
	g_object_unref (content);

	gtk_window_set_resizable (GTK_WINDOW (dlg), FALSE);
	gtk_window_set_title (GTK_WINDOW (dlg), _("Check Spelling"));

	/* Suggestion list */
	dlg->suggestions_list_model = GTK_TREE_MODEL (
			gtk_list_store_new (NUM_COLUMNS, G_TYPE_STRING));

	gtk_tree_view_set_model (GTK_TREE_VIEW (dlg->suggestions_list), 
			dlg->suggestions_list_model);

	/* Add the suggestions column */
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Suggestions"), cell, 
			"text", COLUMN_SUGGESTIONS, NULL);
	
	gtk_tree_view_append_column (GTK_TREE_VIEW (dlg->suggestions_list), column);

	gtk_tree_view_set_search_column (GTK_TREE_VIEW (dlg->suggestions_list),
					 COLUMN_SUGGESTIONS);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dlg->suggestions_list));

	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

	/* Set default button */
	GTK_WIDGET_SET_FLAGS (dlg->change_button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default (dlg->change_button);

	gtk_entry_set_activates_default (GTK_ENTRY (dlg->word_entry), TRUE);

	/* Connect signals */
	g_signal_connect (dlg->word_entry, "changed",
			  G_CALLBACK (word_entry_changed_handler), dlg);
	g_signal_connect (dlg->close_button, "clicked",
			  G_CALLBACK (close_button_clicked_handler), dlg);
	g_signal_connect (selection, "changed", 
			  G_CALLBACK (suggestions_list_selection_changed_handler), 
			  dlg);
	g_signal_connect (dlg->check_word_button, "clicked",
			  G_CALLBACK (check_word_button_clicked_handler), dlg);
	g_signal_connect (dlg->add_word_button, "clicked",
			  G_CALLBACK (add_word_button_clicked_handler), dlg);
	g_signal_connect (dlg->ignore_button, "clicked",
			  G_CALLBACK (ignore_button_clicked_handler), dlg);
	g_signal_connect (dlg->ignore_all_button, "clicked",
			  G_CALLBACK (ignore_all_button_clicked_handler), dlg);
	g_signal_connect (dlg->change_button, "clicked",
			  G_CALLBACK (change_button_clicked_handler), dlg);
	g_signal_connect (dlg->change_all_button, "clicked",
			  G_CALLBACK (change_all_button_clicked_handler), dlg);
	g_signal_connect (dlg->suggestions_list, "row-activated",
			  G_CALLBACK (suggestions_list_row_activated_handler), dlg);
}

static void
gedit_spell_checker_dialog_init (GeditSpellCheckerDialog *dlg)
{
}

GtkWidget *
gedit_spell_checker_dialog_new (const gchar *data_dir)
{
	GeditSpellCheckerDialog *dlg;

	dlg = GEDIT_SPELL_CHECKER_DIALOG (
			g_object_new (GEDIT_TYPE_SPELL_CHECKER_DIALOG, NULL));

	g_return_val_if_fail (dlg != NULL, NULL);
	
	create_dialog (dlg, data_dir);

	return GTK_WIDGET (dlg);
}

GtkWidget *
gedit_spell_checker_dialog_new_from_spell_checker (GeditSpellChecker *spell,
						   const gchar *data_dir)
{
	GeditSpellCheckerDialog *dlg;

	g_return_val_if_fail (spell != NULL, NULL);

	dlg = GEDIT_SPELL_CHECKER_DIALOG (
			g_object_new (GEDIT_TYPE_SPELL_CHECKER_DIALOG, NULL));

	g_return_val_if_fail (dlg != NULL, NULL);
	
	create_dialog (dlg, data_dir);

	gedit_spell_checker_dialog_set_spell_checker (dlg, spell);

	return GTK_WIDGET (dlg);
}

void
gedit_spell_checker_dialog_set_spell_checker (GeditSpellCheckerDialog *dlg, GeditSpellChecker *spell)
{
	const GeditSpellCheckerLanguage* language;
	const gchar *lang;
	gchar *tmp;
	
	g_return_if_fail (GEDIT_IS_SPELL_CHECKER_DIALOG (dlg));
	g_return_if_fail (spell != NULL);
	
	if (dlg->spell_checker != NULL)
		g_object_unref (dlg->spell_checker);

	dlg->spell_checker = spell;
	g_object_ref (dlg->spell_checker);
	
	language = gedit_spell_checker_get_language (dlg->spell_checker);

	lang = gedit_spell_checker_language_to_string (language);
	tmp = g_strdup_printf("<b>%s</b>", lang);

	gtk_label_set_label (GTK_LABEL (dlg->language_label), tmp);
	g_free (tmp);

	if (dlg->misspelled_word != NULL)
		gedit_spell_checker_dialog_set_misspelled_word (dlg, dlg->misspelled_word, -1);
	else
		gtk_list_store_clear (GTK_LIST_STORE (dlg->suggestions_list_model));

	/* TODO: reset all widgets */
}

void
gedit_spell_checker_dialog_set_misspelled_word (GeditSpellCheckerDialog *dlg, 
						const gchar             *word,
						gint                     len)
{
	gchar *tmp;
	GSList *sug;
	
	g_return_if_fail (GEDIT_IS_SPELL_CHECKER_DIALOG (dlg));
	g_return_if_fail (word != NULL);

	g_return_if_fail (dlg->spell_checker != NULL);
	g_return_if_fail (!gedit_spell_checker_check_word (dlg->spell_checker, word, -1));

	/* build_suggestions_list */
	if (dlg->misspelled_word != NULL)
		g_free (dlg->misspelled_word);

	dlg->misspelled_word = g_strdup (word);
	
	tmp = g_strdup_printf("<b>%s</b>", word);
	gtk_label_set_label (GTK_LABEL (dlg->misspelled_word_label), tmp);
	g_free (tmp);

	sug = gedit_spell_checker_get_suggestions (dlg->spell_checker,
		       				   dlg->misspelled_word, 
		       				   -1);
	
	update_suggestions_list_model (dlg, sug);

	/* free the suggestion list */
	g_slist_foreach (sug, (GFunc)g_free, NULL);
	g_slist_free (sug);

	gtk_widget_set_sensitive (dlg->ignore_button, TRUE);
	gtk_widget_set_sensitive (dlg->ignore_all_button, TRUE);
	gtk_widget_set_sensitive (dlg->add_word_button, TRUE);	
}
	
static void
update_suggestions_list_model (GeditSpellCheckerDialog *dlg, GSList *suggestions)
{
	GtkListStore *store;
	GtkTreeIter iter;
	GtkTreeSelection *sel;
	
	g_return_if_fail (GEDIT_IS_SPELL_CHECKER_DIALOG (dlg));
	g_return_if_fail (GTK_IS_LIST_STORE (dlg->suggestions_list_model));
			
	store = GTK_LIST_STORE (dlg->suggestions_list_model);
	gtk_list_store_clear (store);

	gtk_widget_set_sensitive (dlg->word_entry, TRUE);

	if (suggestions == NULL)
	{
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
		                    /* Translators: Displayed in the "Check Spelling" dialog if there are no suggestions
		                     * for the current misspelled word */
				    COLUMN_SUGGESTIONS, _("(no suggested words)"),
				    -1);

		gtk_entry_set_text (GTK_ENTRY (dlg->word_entry), "");

		gtk_widget_set_sensitive (dlg->suggestions_list, FALSE);
	
		return;
	}

	gtk_widget_set_sensitive (dlg->suggestions_list, TRUE);

	gtk_entry_set_text (GTK_ENTRY (dlg->word_entry), (gchar*)suggestions->data);

	while (suggestions != NULL)
	{
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
				    COLUMN_SUGGESTIONS, (gchar*)suggestions->data,
				    -1);

		suggestions = g_slist_next (suggestions);
	}

	sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (dlg->suggestions_list));
	gtk_tree_model_get_iter_first (dlg->suggestions_list_model, &iter);
	gtk_tree_selection_select_iter (sel, &iter);
}

static void
word_entry_changed_handler (GtkEditable *editable, GeditSpellCheckerDialog *dlg)
{
	const gchar *text;
	
	g_return_if_fail (GEDIT_IS_SPELL_CHECKER_DIALOG (dlg));

	text =  gtk_entry_get_text (GTK_ENTRY (dlg->word_entry));

	if (g_utf8_strlen (text, -1) > 0)
	{
		gtk_widget_set_sensitive (dlg->check_word_button, TRUE);
		gtk_widget_set_sensitive (dlg->change_button, TRUE);
		gtk_widget_set_sensitive (dlg->change_all_button, TRUE);
	}
	else
	{
		gtk_widget_set_sensitive (dlg->check_word_button, FALSE);
		gtk_widget_set_sensitive (dlg->change_button, FALSE);
		gtk_widget_set_sensitive (dlg->change_all_button, FALSE);
	}
}

static void
close_button_clicked_handler (GtkButton *button, GeditSpellCheckerDialog *dlg)
{
	g_return_if_fail (GEDIT_IS_SPELL_CHECKER_DIALOG (dlg));

	gtk_widget_destroy (GTK_WIDGET (dlg));	
}

static void
suggestions_list_selection_changed_handler (GtkTreeSelection *selection, 
		GeditSpellCheckerDialog *dlg)
{
 	GtkTreeIter iter;
	GValue value = {0, };
	const gchar *text;

	g_return_if_fail (GEDIT_IS_SPELL_CHECKER_DIALOG (dlg));

	if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
		return;

	gtk_tree_model_get_value (dlg->suggestions_list_model, &iter,
			    COLUMN_SUGGESTIONS,
			    &value);

	text = g_value_get_string (&value);

	gtk_entry_set_text (GTK_ENTRY (dlg->word_entry), text);
	
	g_value_unset (&value);
}

static void
check_word_button_clicked_handler (GtkButton *button, GeditSpellCheckerDialog *dlg)
{
	const gchar *word;
	gssize len;
	
	g_return_if_fail (GEDIT_IS_SPELL_CHECKER_DIALOG (dlg));

	word = gtk_entry_get_text (GTK_ENTRY (dlg->word_entry));
	len = strlen (word);
	g_return_if_fail (len > 0);
	
	if (gedit_spell_checker_check_word (dlg->spell_checker, word, len))
	{
		GtkListStore *store;
		GtkTreeIter iter;
		
		store = GTK_LIST_STORE (dlg->suggestions_list_model);
		gtk_list_store_clear (store);

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
		                    /* Translators: Displayed in the "Check Spelling" dialog if the current word isn't misspelled */
				    COLUMN_SUGGESTIONS, _("(correct spelling)"),
				    -1);

		gtk_widget_set_sensitive (dlg->suggestions_list, FALSE);
	}
	else
	{
		GSList *sug;

		sug = gedit_spell_checker_get_suggestions (dlg->spell_checker,
							   word,
							   len);
	
		update_suggestions_list_model (dlg, sug);

		/* free the suggestion list */
		g_slist_foreach (sug, (GFunc)g_free, NULL);
		g_slist_free (sug);
	}
}

static void
add_word_button_clicked_handler (GtkButton *button, GeditSpellCheckerDialog *dlg)
{
	gchar *word;	
	
	g_return_if_fail (GEDIT_IS_SPELL_CHECKER_DIALOG (dlg));
	g_return_if_fail (dlg->misspelled_word != NULL);

	gedit_spell_checker_add_word_to_personal (dlg->spell_checker, 
						  dlg->misspelled_word, 
						  -1);

	word = g_strdup (dlg->misspelled_word);

	g_signal_emit (G_OBJECT (dlg), signals [ADD_WORD_TO_PERSONAL], 0, word);

	g_free (word);
}

static void
ignore_button_clicked_handler (GtkButton *button, GeditSpellCheckerDialog *dlg)
{
	gchar *word;
	
	g_return_if_fail (GEDIT_IS_SPELL_CHECKER_DIALOG (dlg));
	g_return_if_fail (dlg->misspelled_word != NULL);

	word = g_strdup (dlg->misspelled_word);

	g_signal_emit (G_OBJECT (dlg), signals [IGNORE], 0, word);

	g_free (word);
}

static void
ignore_all_button_clicked_handler (GtkButton *button, GeditSpellCheckerDialog *dlg)
{
	gchar *word;
	
	g_return_if_fail (GEDIT_IS_SPELL_CHECKER_DIALOG (dlg));
	g_return_if_fail (dlg->misspelled_word != NULL);

	gedit_spell_checker_add_word_to_session (dlg->spell_checker,
						 dlg->misspelled_word, 
						 -1);

	word = g_strdup (dlg->misspelled_word);

	g_signal_emit (G_OBJECT (dlg), signals [IGNORE_ALL], 0, word);

	g_free (word);
}

static void
change_button_clicked_handler (GtkButton *button, GeditSpellCheckerDialog *dlg)
{
	gchar *word;
	gchar *change;

	g_return_if_fail (GEDIT_IS_SPELL_CHECKER_DIALOG (dlg));
	g_return_if_fail (dlg->misspelled_word != NULL);

	change = g_strdup (gtk_entry_get_text (GTK_ENTRY (dlg->word_entry)));
	g_return_if_fail (change != NULL);
	g_return_if_fail (*change != '\0');

	gedit_spell_checker_set_correction (dlg->spell_checker, 
					    dlg->misspelled_word, -1, 
					    change, -1);
	
	word = g_strdup (dlg->misspelled_word);

	g_signal_emit (G_OBJECT (dlg), signals [CHANGE], 0, word, change);

	g_free (word);
	g_free (change);	
}

/* double click on one of the suggestions is like clicking on "change" */
static void
suggestions_list_row_activated_handler (GtkTreeView *view,
		GtkTreePath *path,
		GtkTreeViewColumn *column,
		GeditSpellCheckerDialog *dlg)
{
	g_return_if_fail (GEDIT_IS_SPELL_CHECKER_DIALOG (dlg));

	change_button_clicked_handler (GTK_BUTTON (dlg->change_button), dlg);
}

static void
change_all_button_clicked_handler (GtkButton *button, GeditSpellCheckerDialog *dlg)
{
	gchar *word;
	gchar *change;
		
	g_return_if_fail (GEDIT_IS_SPELL_CHECKER_DIALOG (dlg));
	g_return_if_fail (dlg->misspelled_word != NULL);

	change = g_strdup (gtk_entry_get_text (GTK_ENTRY (dlg->word_entry)));
	g_return_if_fail (change != NULL);
	g_return_if_fail (*change != '\0');

	gedit_spell_checker_set_correction (dlg->spell_checker, 
					    dlg->misspelled_word, -1,
					    change, -1);
	
	word = g_strdup (dlg->misspelled_word);

	g_signal_emit (G_OBJECT (dlg), signals [CHANGE_ALL], 0, word, change);

	g_free (word);
	g_free (change);	
}

void 
gedit_spell_checker_dialog_set_completed (GeditSpellCheckerDialog *dlg)
{
	gchar *tmp;
	
	g_return_if_fail (GEDIT_IS_SPELL_CHECKER_DIALOG (dlg));

	tmp = g_strdup_printf("<b>%s</b>", _("Completed spell checking"));
	gtk_label_set_label (GTK_LABEL (dlg->misspelled_word_label), 
			     tmp);
	g_free (tmp);

	gtk_list_store_clear (GTK_LIST_STORE (dlg->suggestions_list_model));
	gtk_entry_set_text (GTK_ENTRY (dlg->word_entry), "");
	
	gtk_widget_set_sensitive (dlg->word_entry, FALSE);
	gtk_widget_set_sensitive (dlg->check_word_button, FALSE);
	gtk_widget_set_sensitive (dlg->ignore_button, FALSE);
	gtk_widget_set_sensitive (dlg->ignore_all_button, FALSE);
	gtk_widget_set_sensitive (dlg->change_button, FALSE);
	gtk_widget_set_sensitive (dlg->change_all_button, FALSE);
	gtk_widget_set_sensitive (dlg->add_word_button, FALSE);
	gtk_widget_set_sensitive (dlg->suggestions_list, FALSE);
}

