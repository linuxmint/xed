/*
 * xed-highlight-mode-selector.c
 * This file is part of xed
 *
 * Copyright (C) 2013 - Ignacio Casal Quinteiro
 *
 * xed is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xed is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xed. If not, see <http://www.gnu.org/licenses/>.
 */

#include "xed-highlight-mode-selector.h"

#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include <string.h>

enum
{
	COLUMN_NAME,
	COLUMN_LANG,
	N_COLUMNS
};

struct _XedHighlightModeSelector
{
	GtkGrid parent_instance;

	GtkWidget *treeview;
	GtkWidget *entry;
	GtkListStore *liststore;
	GtkTreeModelFilter *treemodelfilter;
	GtkTreeSelection *treeview_selection;
};

/* Signals */
enum
{
	LANGUAGE_SELECTED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (XedHighlightModeSelector, xed_highlight_mode_selector, GTK_TYPE_GRID)

static void
xed_highlight_mode_selector_language_selected (XedHighlightModeSelector *widget,
                                                 GtkSourceLanguage          *language)
{
}

static void
xed_highlight_mode_selector_class_init (XedHighlightModeSelectorClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	signals[LANGUAGE_SELECTED] =
		g_signal_new_class_handler ("language-selected",
		                            G_TYPE_FROM_CLASS (klass),
		                            G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		                            G_CALLBACK (xed_highlight_mode_selector_language_selected),
		                            NULL, NULL, NULL,
		                            G_TYPE_NONE,
		                            1,
		                            GTK_SOURCE_TYPE_LANGUAGE);

	/* Bind class to template */
	gtk_widget_class_set_template_from_resource (widget_class,
	                                             "/org/x/editor/ui/xed-highlight-mode-selector.ui");
	gtk_widget_class_bind_template_child (widget_class, XedHighlightModeSelector, treeview);
	gtk_widget_class_bind_template_child (widget_class, XedHighlightModeSelector, entry);
	gtk_widget_class_bind_template_child (widget_class, XedHighlightModeSelector, liststore);
	gtk_widget_class_bind_template_child (widget_class, XedHighlightModeSelector, treemodelfilter);
	gtk_widget_class_bind_template_child (widget_class, XedHighlightModeSelector, treeview_selection);
}

static gboolean
visible_func (GtkTreeModel               *model,
              GtkTreeIter                *iter,
              XedHighlightModeSelector *selector)
{
	const gchar *entry_text;
	gchar *name;
	gchar *name_normalized;
	gchar *name_casefolded;
	gchar *text_normalized;
	gchar *text_casefolded;
	gboolean visible = FALSE;

	entry_text = gtk_entry_get_text (GTK_ENTRY (selector->entry));

	if (*entry_text == '\0')
	{
		return TRUE;
	}

	gtk_tree_model_get (model, iter, COLUMN_NAME, &name, -1);

	name_normalized = g_utf8_normalize (name, -1, G_NORMALIZE_ALL);
	g_free (name);

	name_casefolded = g_utf8_casefold (name_normalized, -1);
	g_free (name_normalized);

	text_normalized = g_utf8_normalize (entry_text, -1, G_NORMALIZE_ALL);
	text_casefolded = g_utf8_casefold (text_normalized, -1);
	g_free (text_normalized);

	if (strstr (name_casefolded, text_casefolded) != NULL)
	{
		visible = TRUE;
	}

	g_free (name_casefolded);
	g_free (text_casefolded);

	return visible;
}

static void
on_entry_activate (GtkEntry                   *entry,
                   XedHighlightModeSelector *selector)
{
	xed_highlight_mode_selector_activate_selected_language (selector);
}

static void
on_entry_changed (GtkEntry                   *entry,
                  XedHighlightModeSelector *selector)
{
	GtkTreeIter iter;

	gtk_tree_model_filter_refilter (selector->treemodelfilter);

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (selector->treemodelfilter), &iter))
	{
		gtk_tree_selection_select_iter (selector->treeview_selection, &iter);
	}
}

static gboolean
move_selection (XedHighlightModeSelector *selector,
                gint                      howmany)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	gint *indices;
	gint ret = FALSE;

	if (!gtk_tree_selection_get_selected (selector->treeview_selection, NULL, &iter) &&
	    !gtk_tree_model_get_iter_first (GTK_TREE_MODEL (selector->treemodelfilter), &iter))
	{
		return FALSE;
	}

	path = gtk_tree_model_get_path (GTK_TREE_MODEL (selector->treemodelfilter), &iter);
	indices = gtk_tree_path_get_indices (path);

	if (indices)
	{
		gint num;
		gint idx;
		GtkTreePath *new_path;

		idx = indices[0];
		num = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (selector->treemodelfilter), NULL);

		if ((idx + howmany) < 0)
		{
			idx = 0;
		}
		else if ((idx + howmany) >= num)
		{
			idx = num - 1;
		}
		else
		{
			idx = idx + howmany;
		}

		new_path = gtk_tree_path_new_from_indices (idx, -1);
		gtk_tree_selection_select_path (selector->treeview_selection, new_path);
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (selector->treeview),
		                              new_path, NULL, TRUE, 0.5, 0);
		gtk_tree_path_free (new_path);

		ret = TRUE;
	}

	gtk_tree_path_free (path);

	return ret;
}

static gboolean
on_entry_key_press_event (GtkWidget                  *entry,
                          GdkEventKey                *event,
                          XedHighlightModeSelector *selector)
{
	if (event->keyval == GDK_KEY_Down)
	{
		return move_selection (selector, 1);
	}
	else if (event->keyval == GDK_KEY_Up)
	{
		return move_selection (selector, -1);
	}
	else if (event->keyval == GDK_KEY_Page_Down)
	{
		return move_selection (selector, 5);
	}
	else if (event->keyval == GDK_KEY_Page_Up)
	{
		return move_selection (selector, -5);
	}

	return FALSE;
}

static void
on_row_activated (GtkTreeView                *tree_view,
                  GtkTreePath                *path,
                  GtkTreeViewColumn          *column,
                  XedHighlightModeSelector *selector)
{
	xed_highlight_mode_selector_activate_selected_language (selector);
}

static void
xed_highlight_mode_selector_init (XedHighlightModeSelector *selector)
{
	GtkSourceLanguageManager *lm;
	const gchar * const *ids;
	gint i;
	GtkTreeIter iter;

	selector = xed_highlight_mode_selector_get_instance_private (selector);

	gtk_widget_init_template (GTK_WIDGET (selector));

	gtk_tree_model_filter_set_visible_func (selector->treemodelfilter,
	                                        (GtkTreeModelFilterVisibleFunc)visible_func,
	                                        selector,
	                                        NULL);

	g_signal_connect (selector->entry, "activate",
	                  G_CALLBACK (on_entry_activate), selector);
	g_signal_connect (selector->entry, "changed",
	                  G_CALLBACK (on_entry_changed), selector);
	g_signal_connect (selector->entry, "key-press-event",
	                  G_CALLBACK (on_entry_key_press_event), selector);

	g_signal_connect (selector->treeview, "row-activated",
	                  G_CALLBACK (on_row_activated), selector);

	/* Populate tree model */
	gtk_list_store_append (selector->liststore, &iter);
	gtk_list_store_set (selector->liststore, &iter,
	                    COLUMN_NAME, _("Plain Text"),
	                    COLUMN_LANG, NULL,
	                    -1);

	lm = gtk_source_language_manager_get_default ();
	ids = gtk_source_language_manager_get_language_ids (lm);

	for (i = 0; ids[i] != NULL; i++)
	{
		GtkSourceLanguage *lang;

		lang = gtk_source_language_manager_get_language (lm, ids[i]);

		if (!gtk_source_language_get_hidden (lang))
		{
			gtk_list_store_append (selector->liststore, &iter);
			gtk_list_store_set (selector->liststore, &iter,
			                    COLUMN_NAME, gtk_source_language_get_name (lang),
			                    COLUMN_LANG, lang,
			                    -1);
		}
	}

	/* select first item */
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (selector->treemodelfilter), &iter))
	{
		gtk_tree_selection_select_iter (selector->treeview_selection, &iter);
	}
}

XedHighlightModeSelector *
xed_highlight_mode_selector_new ()
{
	return g_object_new (XED_TYPE_HIGHLIGHT_MODE_SELECTOR, NULL);
}

void
xed_highlight_mode_selector_select_language (XedHighlightModeSelector *selector,
                                             GtkSourceLanguage        *language)
{
	GtkTreeIter iter;

	g_return_if_fail (XED_IS_HIGHLIGHT_MODE_SELECTOR (selector));

	if (language == NULL)
	{
		return;
	}

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (selector->treemodelfilter), &iter))
	{
		do
		{
			GtkSourceLanguage *lang;

			gtk_tree_model_get (GTK_TREE_MODEL (selector->treemodelfilter),
			                    &iter,
			                    COLUMN_LANG, &lang,
			                    -1);

			if (lang != NULL)
			{
				gboolean equal = (lang == language);

				g_object_unref (lang);

				if (equal)
				{
					GtkTreePath *path;

					path = gtk_tree_model_get_path (GTK_TREE_MODEL (selector->treemodelfilter), &iter);

					gtk_tree_selection_select_iter (selector->treeview_selection, &iter);
					gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (selector->treeview),
					                              path, NULL, TRUE, 0.5, 0);
					gtk_tree_path_free (path);
					break;
				}
			}
		}
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (selector->treemodelfilter), &iter));
	}
}

void
xed_highlight_mode_selector_activate_selected_language (XedHighlightModeSelector *selector)
{
	GtkSourceLanguage *lang;
	GtkTreeIter iter;

	g_return_if_fail (XED_IS_HIGHLIGHT_MODE_SELECTOR (selector));

	if (!gtk_tree_selection_get_selected (selector->treeview_selection, NULL, &iter))
	{
		return;
	}

	gtk_tree_model_get (GTK_TREE_MODEL (selector->treemodelfilter), &iter,
	                    COLUMN_LANG, &lang,
	                    -1);

	g_signal_emit (G_OBJECT (selector), signals[LANGUAGE_SELECTED], 0, lang);

	if (lang != NULL)
	{
		g_object_unref (lang);
	}
}

/* ex:set ts=8 noet: */
