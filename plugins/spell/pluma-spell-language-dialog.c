/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pluma-spell-language-dialog.c
 * This file is part of pluma
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA 02110-1301, USA. 
 */
 
/*
 * Modified by the pluma Team, 2002. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <pluma/pluma-utils.h>
#include <pluma/pluma-help.h>
#include "pluma-spell-language-dialog.h"
#include "pluma-spell-checker-language.h"


enum
{
	COLUMN_LANGUAGE_NAME = 0,
	COLUMN_LANGUAGE_POINTER,
	ENCODING_NUM_COLS
};


struct _PlumaSpellLanguageDialog 
{
	GtkDialog dialog;

	GtkWidget *languages_treeview;
	GtkTreeModel *model;
};

G_DEFINE_TYPE(PlumaSpellLanguageDialog, pluma_spell_language_dialog, GTK_TYPE_DIALOG)


static void 
pluma_spell_language_dialog_class_init (PlumaSpellLanguageDialogClass *klass)
{
	/* GObjectClass *object_class = G_OBJECT_CLASS (klass); */
}

static void
dialog_response_handler (GtkDialog *dlg,
			 gint       res_id)
{
	if (res_id == GTK_RESPONSE_HELP)
	{
		pluma_help_display (GTK_WINDOW (dlg),
				    NULL,
				    "pluma-spell-checker-plugin");

		g_signal_stop_emission_by_name (dlg, "response");
	}
}

static void 
scroll_to_selected (GtkTreeView *tree_view)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model (tree_view);
	g_return_if_fail (model != NULL);

	/* Scroll to selected */
	selection = gtk_tree_view_get_selection (tree_view);
	g_return_if_fail (selection != NULL);

	if (gtk_tree_selection_get_selected (selection, NULL, &iter))
	{
		GtkTreePath* path;

		path = gtk_tree_model_get_path (model, &iter);
		g_return_if_fail (path != NULL);

		gtk_tree_view_scroll_to_cell (tree_view,
					      path, NULL, TRUE, 1.0, 0.0);
		gtk_tree_path_free (path);
	}
}

static void
language_row_activated (GtkTreeView *tree_view,
			GtkTreePath *path,
			GtkTreeViewColumn *column,
			PlumaSpellLanguageDialog *dialog)
{
	gtk_dialog_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
}

static void
create_dialog (PlumaSpellLanguageDialog *dlg,
	       const gchar *data_dir)
{
	GtkWidget *error_widget;
	GtkWidget *content;
	gboolean ret;
	GtkCellRenderer *cell;
	GtkTreeViewColumn *column;
	gchar *ui_file;
	gchar *root_objects[] = {
		"content",
		NULL
	};
	
	gtk_dialog_add_buttons (GTK_DIALOG (dlg),
				GTK_STOCK_CANCEL,
				GTK_RESPONSE_CANCEL,
				GTK_STOCK_OK,
				GTK_RESPONSE_OK,
				GTK_STOCK_HELP,
				GTK_RESPONSE_HELP,
				NULL);

	gtk_window_set_title (GTK_WINDOW (dlg), _("Set language"));
#if !GTK_CHECK_VERSION (3, 0, 0)
	gtk_dialog_set_has_separator (GTK_DIALOG (dlg), FALSE);
#endif
	gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (dlg), TRUE);

	/* HIG defaults */
	gtk_container_set_border_width (GTK_CONTAINER (dlg), 5);
	gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dlg))),
			     2); /* 2 * 5 + 2 = 12 */
	gtk_container_set_border_width (GTK_CONTAINER (gtk_dialog_get_action_area (GTK_DIALOG (dlg))),
					5);
	gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_action_area (GTK_DIALOG (dlg))),
			     6);

	g_signal_connect (dlg,
			  "response",
			  G_CALLBACK (dialog_response_handler),
			  NULL);

	ui_file = g_build_filename (data_dir, "languages-dialog.ui", NULL);
	ret = pluma_utils_get_ui_objects (ui_file,
					  root_objects,	
					  &error_widget,
					  "content", &content,
					  "languages_treeview", &dlg->languages_treeview,
					  NULL);
	g_free (ui_file);
	
	if (!ret)
	{
		gtk_widget_show (error_widget);

		gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dlg))),
		                    error_widget,
		                    TRUE, TRUE, 0);

		return;
	}

	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dlg))),
			    content, TRUE, TRUE, 0);
	g_object_unref (content);
	gtk_container_set_border_width (GTK_CONTAINER (content), 5);

	dlg->model = GTK_TREE_MODEL (gtk_list_store_new (ENCODING_NUM_COLS,
							 G_TYPE_STRING,
							 G_TYPE_POINTER));

	gtk_tree_view_set_model (GTK_TREE_VIEW (dlg->languages_treeview),
				 dlg->model);

	g_object_unref (dlg->model);

	/* Add the encoding column */
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Languages"),
							   cell, 
							   "text",
							   COLUMN_LANGUAGE_NAME,
							   NULL);
	
	gtk_tree_view_append_column (GTK_TREE_VIEW (dlg->languages_treeview),
				     column);

	gtk_tree_view_set_search_column (GTK_TREE_VIEW (dlg->languages_treeview),
					 COLUMN_LANGUAGE_NAME);

	g_signal_connect (dlg->languages_treeview,
			  "realize",
			  G_CALLBACK (scroll_to_selected),
			  dlg);
	g_signal_connect (dlg->languages_treeview,
			  "row-activated", 
			  G_CALLBACK (language_row_activated),
			  dlg);
}

static void
pluma_spell_language_dialog_init (PlumaSpellLanguageDialog *dlg)
{
	
}

static void
populate_language_list (PlumaSpellLanguageDialog        *dlg,
			const PlumaSpellCheckerLanguage *cur_lang)
{
	GtkListStore *store;
	GtkTreeIter iter;

	const GSList* langs;

	/* create list store */
	store = GTK_LIST_STORE (dlg->model);

	langs = pluma_spell_checker_get_available_languages ();

	while (langs)
	{
		const gchar *name;

		name = pluma_spell_checker_language_to_string ((const PlumaSpellCheckerLanguage*)langs->data);

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
				    COLUMN_LANGUAGE_NAME, name,
				    COLUMN_LANGUAGE_POINTER, langs->data,
				    -1);

		if (langs->data == cur_lang)
		{
			GtkTreeSelection *selection;

			selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dlg->languages_treeview));
			g_return_if_fail (selection != NULL);

			gtk_tree_selection_select_iter (selection, &iter);
		}

		langs = g_slist_next (langs);
	}
}

GtkWidget *
pluma_spell_language_dialog_new (GtkWindow                       *parent,
				 const PlumaSpellCheckerLanguage *cur_lang,
				 const gchar *data_dir)
{
	PlumaSpellLanguageDialog *dlg;

	g_return_val_if_fail (GTK_IS_WINDOW (parent), NULL);

	dlg = g_object_new (PLUMA_TYPE_SPELL_LANGUAGE_DIALOG, NULL);

	create_dialog (dlg, data_dir);

	populate_language_list (dlg, cur_lang);

	gtk_window_set_transient_for (GTK_WINDOW (dlg), parent);
	gtk_widget_grab_focus (dlg->languages_treeview);					     

	return GTK_WIDGET (dlg);
}

const PlumaSpellCheckerLanguage *
pluma_spell_language_get_selected_language (PlumaSpellLanguageDialog *dlg)
{
	GValue value = {0, };
	const PlumaSpellCheckerLanguage* lang;

	GtkTreeIter iter;
	GtkTreeSelection *selection;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dlg->languages_treeview));
	g_return_val_if_fail (selection != NULL, NULL);

	if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
		return NULL;

	gtk_tree_model_get_value (dlg->model,
				  &iter,
				  COLUMN_LANGUAGE_POINTER,
				  &value);

	lang = (const PlumaSpellCheckerLanguage* ) g_value_get_pointer (&value);

	return lang;
}

