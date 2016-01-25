/*
 * xedit-spell-plugin.c
 * 
 * Copyright (C) 2002-2005 Paolo Maggi 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xedit-spell-plugin.h"
#include "xedit-spell-utils.h"

#include <string.h> /* For strlen */

#include <glib/gi18n.h>
#include <gmodule.h>

#include <xedit/xedit-debug.h>
#include <xedit/xedit-help.h>
#include <xedit/xedit-prefs-manager.h>
#include <xedit/xedit-statusbar.h>
#include <xedit/xedit-utils.h>

#include "xedit-spell-checker.h"
#include "xedit-spell-checker-dialog.h"
#include "xedit-spell-language-dialog.h"
#include "xedit-automatic-spell-checker.h"

#define XEDIT_METADATA_ATTRIBUTE_SPELL_LANGUAGE "metadata::xedit-spell-language"
#define XEDIT_METADATA_ATTRIBUTE_SPELL_ENABLED  "metadata::xedit-spell-enabled"

#define WINDOW_DATA_KEY "XeditSpellPluginWindowData"
#define MENU_PATH "/MenuBar/ToolsMenu/ToolsOps_1"

#define XEDIT_SPELL_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
					       XEDIT_TYPE_SPELL_PLUGIN, \
					       XeditSpellPluginPrivate))

/* GSettings keys */
#define SPELL_SCHEMA		"org.x.editor.plugins.spell"
#define AUTOCHECK_TYPE_KEY	"autocheck-type"

XEDIT_PLUGIN_REGISTER_TYPE(XeditSpellPlugin, xedit_spell_plugin)

typedef struct
{
	GtkActionGroup *action_group;
	guint           ui_id;
	guint           message_cid;
	gulong          tab_added_id;
	gulong          tab_removed_id;
	XeditSpellPlugin *plugin;
} WindowData;

typedef struct
{
	XeditPlugin *plugin;
	XeditWindow *window;
} ActionData;

struct _XeditSpellPluginPrivate
{
	GSettings *settings;
};

static void	spell_cb	(GtkAction *action, ActionData *action_data);
static void	set_language_cb	(GtkAction *action, ActionData *action_data);
static void	auto_spell_cb	(GtkAction *action, XeditWindow *window);

/* UI actions. */
static const GtkActionEntry action_entries[] =
{
	{ "CheckSpell",
	  GTK_STOCK_SPELL_CHECK,
	  N_("_Check Spelling..."),
	  "<shift>F7",
	  N_("Check the current document for incorrect spelling"),
	  G_CALLBACK (spell_cb)
	},

	{ "ConfigSpell",
	  NULL,
	  N_("Set _Language..."),
	  NULL,
	  N_("Set the language of the current document"),
	  G_CALLBACK (set_language_cb)
	}
};

static const GtkToggleActionEntry toggle_action_entries[] =
{
	{ "AutoSpell",
	  NULL,
	  N_("_Autocheck Spelling"),
	  NULL,
	  N_("Automatically spell-check the current document"),
	  G_CALLBACK (auto_spell_cb),
	  FALSE
	}
};

typedef struct _SpellConfigureDialog SpellConfigureDialog;

struct _SpellConfigureDialog
{
	GtkWidget *dialog;

	GtkWidget *never;
	GtkWidget *always;
	GtkWidget *document;

	XeditSpellPlugin *plugin;
};

typedef enum
{
	AUTOCHECK_NEVER = 0,
	AUTOCHECK_DOCUMENT,
	AUTOCHECK_ALWAYS
} XeditSpellPluginAutocheckType;

typedef struct _CheckRange CheckRange;

struct _CheckRange
{
	GtkTextMark *start_mark;
	GtkTextMark *end_mark;

	gint mw_start; /* misspelled word start */
	gint mw_end;   /* end */

	GtkTextMark *current_mark;
};

static GQuark spell_checker_id = 0;
static GQuark check_range_id = 0;

static void
xedit_spell_plugin_init (XeditSpellPlugin *plugin)
{
	xedit_debug_message (DEBUG_PLUGINS, "XeditSpellPlugin initializing");

	plugin->priv = XEDIT_SPELL_PLUGIN_GET_PRIVATE (plugin);

	plugin->priv->settings = g_settings_new (SPELL_SCHEMA);
}

static void
xedit_spell_plugin_finalize (GObject *object)
{
	XeditSpellPlugin *plugin = XEDIT_SPELL_PLUGIN (object);

	xedit_debug_message (DEBUG_PLUGINS, "XeditSpellPlugin finalizing");

	g_object_unref (G_OBJECT (plugin->priv->settings));

	G_OBJECT_CLASS (xedit_spell_plugin_parent_class)->finalize (object);
}

static void 
set_spell_language_cb (XeditSpellChecker   *spell,
		       const XeditSpellCheckerLanguage *lang,
		       XeditDocument 	   *doc)
{
	const gchar *key;

	g_return_if_fail (XEDIT_IS_DOCUMENT (doc));
	g_return_if_fail (lang != NULL);

	key = xedit_spell_checker_language_to_key (lang);
	g_return_if_fail (key != NULL);

	xedit_document_set_metadata (doc, XEDIT_METADATA_ATTRIBUTE_SPELL_LANGUAGE,
				     key, NULL);
}

static void
set_language_from_metadata (XeditSpellChecker *spell,
			    XeditDocument     *doc)
{
	const XeditSpellCheckerLanguage *lang = NULL;
	gchar *value = NULL;

	value = xedit_document_get_metadata (doc, XEDIT_METADATA_ATTRIBUTE_SPELL_LANGUAGE);

	if (value != NULL)
	{
		lang = xedit_spell_checker_language_from_key (value);
		g_free (value);
	}

	if (lang != NULL)
	{
		g_signal_handlers_block_by_func (spell, set_spell_language_cb, doc);
		xedit_spell_checker_set_language (spell, lang);
		g_signal_handlers_unblock_by_func (spell, set_spell_language_cb, doc);
	}
}

static XeditSpellPluginAutocheckType
get_autocheck_type (XeditSpellPlugin *plugin)
{
	XeditSpellPluginAutocheckType autocheck_type;

	autocheck_type = g_settings_get_enum (plugin->priv->settings,
					      AUTOCHECK_TYPE_KEY);

	return autocheck_type;
}

static void
set_autocheck_type (XeditSpellPlugin *plugin,
		    XeditSpellPluginAutocheckType autocheck_type)
{
	if (!g_settings_is_writable (plugin->priv->settings,
				     AUTOCHECK_TYPE_KEY))
	{
		return;
	}

	g_settings_set_enum (plugin->priv->settings,
			     AUTOCHECK_TYPE_KEY,
			     autocheck_type);
}

static XeditSpellChecker *
get_spell_checker_from_document (XeditDocument *doc)
{
	XeditSpellChecker *spell;
	gpointer data;

	xedit_debug (DEBUG_PLUGINS);

	g_return_val_if_fail (doc != NULL, NULL);

	data = g_object_get_qdata (G_OBJECT (doc), spell_checker_id);

	if (data == NULL)
	{
		spell = xedit_spell_checker_new ();

		set_language_from_metadata (spell, doc);

		g_object_set_qdata_full (G_OBJECT (doc), 
					 spell_checker_id, 
					 spell, 
					 (GDestroyNotify) g_object_unref);

		g_signal_connect (spell,
				  "set_language",
				  G_CALLBACK (set_spell_language_cb),
				  doc);
	}
	else
	{
		g_return_val_if_fail (XEDIT_IS_SPELL_CHECKER (data), NULL);
		spell = XEDIT_SPELL_CHECKER (data);
	}

	return spell;
}

static CheckRange *
get_check_range (XeditDocument *doc)
{
	CheckRange *range;

	xedit_debug (DEBUG_PLUGINS);

	g_return_val_if_fail (doc != NULL, NULL);

	range = (CheckRange *) g_object_get_qdata (G_OBJECT (doc), check_range_id);

	return range;
}

static void
update_current (XeditDocument *doc,
		gint           current)
{
	CheckRange *range;
	GtkTextIter iter;
	GtkTextIter end_iter;

	xedit_debug (DEBUG_PLUGINS);

	g_return_if_fail (doc != NULL);
	g_return_if_fail (current >= 0);

	range = get_check_range (doc);
	g_return_if_fail (range != NULL);

	gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), 
					    &iter, current);

	if (!gtk_text_iter_inside_word (&iter))
	{	
		/* if we're not inside a word,
		 * we must be in some spaces.
		 * skip forward to the beginning of the next word. */
		if (!gtk_text_iter_is_end (&iter))
		{
			gtk_text_iter_forward_word_end (&iter);
			gtk_text_iter_backward_word_start (&iter);	
		}
	}
	else
	{
		if (!gtk_text_iter_starts_word (&iter))
			gtk_text_iter_backward_word_start (&iter);	
	}

	gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (doc),
					  &end_iter,
					  range->end_mark);

	if (gtk_text_iter_compare (&end_iter, &iter) < 0)
	{	
		gtk_text_buffer_move_mark (GTK_TEXT_BUFFER (doc),
					   range->current_mark,
					   &end_iter);
	}
	else
	{
		gtk_text_buffer_move_mark (GTK_TEXT_BUFFER (doc),
					   range->current_mark,
					   &iter);
	}
}

static void
set_check_range (XeditDocument *doc,
		 GtkTextIter   *start,
		 GtkTextIter   *end)
{
	CheckRange *range;
	GtkTextIter iter;

	xedit_debug (DEBUG_PLUGINS);

	range = get_check_range (doc);

	if (range == NULL)
	{
		xedit_debug_message (DEBUG_PLUGINS, "There was not a previous check range");

		gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (doc), &iter);

		range = g_new0 (CheckRange, 1);

		range->start_mark = gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (doc),
				"check_range_start_mark", &iter, TRUE);

		range->end_mark = gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (doc),
				"check_range_end_mark", &iter, FALSE);

		range->current_mark = gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (doc),
				"check_range_current_mark", &iter, TRUE);

		g_object_set_qdata_full (G_OBJECT (doc), 
				 check_range_id, 
				 range, 
				 (GDestroyNotify)g_free);
	}

	if (xedit_spell_utils_skip_no_spell_check (start, end))
	 {
		if (!gtk_text_iter_inside_word (end))
		{
			/* if we're neither inside a word,
			 * we must be in some spaces.
			 * skip backward to the end of the previous word. */
			if (!gtk_text_iter_is_end (end))
			{
				gtk_text_iter_backward_word_start (end);
				gtk_text_iter_forward_word_end (end);
			}
		}
		else
		{
			if (!gtk_text_iter_ends_word (end))
				gtk_text_iter_forward_word_end (end);
		}
	}
	else
	{
		/* no spell checking in the specified range */
		start = end;
	}

	gtk_text_buffer_move_mark (GTK_TEXT_BUFFER (doc),
				   range->start_mark,
				   start);
	gtk_text_buffer_move_mark (GTK_TEXT_BUFFER (doc),
				   range->end_mark,
				   end);

	range->mw_start = -1;
	range->mw_end = -1;

	update_current (doc, gtk_text_iter_get_offset (start));
}

static gchar *
get_current_word (XeditDocument *doc, gint *start, gint *end)
{
	const CheckRange *range;
	GtkTextIter end_iter;
	GtkTextIter current_iter;
	gint range_end;

	xedit_debug (DEBUG_PLUGINS);

	g_return_val_if_fail (doc != NULL, NULL);
	g_return_val_if_fail (start != NULL, NULL);
	g_return_val_if_fail (end != NULL, NULL);

	range = get_check_range (doc);
	g_return_val_if_fail (range != NULL, NULL);

	gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (doc), 
			&end_iter, range->end_mark);

	range_end = gtk_text_iter_get_offset (&end_iter);

	gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (doc), 
			&current_iter, range->current_mark);

	end_iter = current_iter;

	if (!gtk_text_iter_is_end (&end_iter))
	{
		xedit_debug_message (DEBUG_PLUGINS, "Current is not end");

		gtk_text_iter_forward_word_end (&end_iter);
	}

	*start = gtk_text_iter_get_offset (&current_iter);
	*end = MIN (gtk_text_iter_get_offset (&end_iter), range_end);

	xedit_debug_message (DEBUG_PLUGINS, "Current word extends [%d, %d]", *start, *end);

	if (!(*start < *end))
		return NULL;

	return gtk_text_buffer_get_slice (GTK_TEXT_BUFFER (doc),
					  &current_iter,
					  &end_iter,
					  TRUE);
}

static gboolean
goto_next_word (XeditDocument *doc)
{
	CheckRange *range;
	GtkTextIter current_iter;
	GtkTextIter old_current_iter;
	GtkTextIter end_iter;

	xedit_debug (DEBUG_PLUGINS);

	g_return_val_if_fail (doc != NULL, FALSE);

	range = get_check_range (doc);
	g_return_val_if_fail (range != NULL, FALSE);

	gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (doc), 
					  &current_iter,
					  range->current_mark);
	gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (doc), &end_iter);

	old_current_iter = current_iter;

	gtk_text_iter_forward_word_ends (&current_iter, 2);
	gtk_text_iter_backward_word_start (&current_iter);

	if (xedit_spell_utils_skip_no_spell_check (&current_iter, &end_iter) &&
	    (gtk_text_iter_compare (&old_current_iter, &current_iter) < 0) &&
	    (gtk_text_iter_compare (&current_iter, &end_iter) < 0))
	{
		update_current (doc, gtk_text_iter_get_offset (&current_iter));
		return TRUE;
	}

	return FALSE;
}

static gchar *
get_next_misspelled_word (XeditView *view)
{
	XeditDocument *doc;
	CheckRange *range;
	gint start, end;
	gchar *word;
	XeditSpellChecker *spell;

	g_return_val_if_fail (view != NULL, NULL);

	doc = XEDIT_DOCUMENT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)));
	g_return_val_if_fail (doc != NULL, NULL);

	range = get_check_range (doc);
	g_return_val_if_fail (range != NULL, NULL);

	spell = get_spell_checker_from_document (doc);
	g_return_val_if_fail (spell != NULL, NULL);

	word = get_current_word (doc, &start, &end);
	if (word == NULL)
		return NULL;

	xedit_debug_message (DEBUG_PLUGINS, "Word to check: %s", word);

	while (xedit_spell_checker_check_word (spell, word, -1))
	{
		g_free (word);

		if (!goto_next_word (doc))
			return NULL;

		/* may return null if we reached the end of the selection */
		word = get_current_word (doc, &start, &end);
		if (word == NULL)
			return NULL;

		xedit_debug_message (DEBUG_PLUGINS, "Word to check: %s", word);
	}

	if (!goto_next_word (doc))
		update_current (doc, gtk_text_buffer_get_char_count (GTK_TEXT_BUFFER (doc)));

	if (word != NULL)
	{
		GtkTextIter s, e;

		range->mw_start = start;
		range->mw_end = end;

		xedit_debug_message (DEBUG_PLUGINS, "Select [%d, %d]", start, end);

		gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &s, start);
		gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &e, end);

		gtk_text_buffer_select_range (GTK_TEXT_BUFFER (doc), &s, &e);

		xedit_view_scroll_to_cursor (view);
	}
	else
	{
		range->mw_start = -1;
		range->mw_end = -1;
	}

	return word;
}

static void
ignore_cb (XeditSpellCheckerDialog *dlg,
	   const gchar             *w,
	   XeditView               *view)
{
	gchar *word = NULL;

	xedit_debug (DEBUG_PLUGINS);

	g_return_if_fail (w != NULL);
	g_return_if_fail (view != NULL);

	word = get_next_misspelled_word (view);
	if (word == NULL)
	{
		xedit_spell_checker_dialog_set_completed (dlg);
		
		return;
	}

	xedit_spell_checker_dialog_set_misspelled_word (XEDIT_SPELL_CHECKER_DIALOG (dlg),
							word,
							-1);

	g_free (word);
}

static void
change_cb (XeditSpellCheckerDialog *dlg,
	   const gchar             *word,
	   const gchar             *change,
	   XeditView               *view)
{
	XeditDocument *doc;
	CheckRange *range;
	gchar *w = NULL;
	GtkTextIter start, end;

	xedit_debug (DEBUG_PLUGINS);

	g_return_if_fail (view != NULL);
	g_return_if_fail (word != NULL);
	g_return_if_fail (change != NULL);

	doc = XEDIT_DOCUMENT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)));
	g_return_if_fail (doc != NULL);

	range = get_check_range (doc);
	g_return_if_fail (range != NULL);

	gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &start, range->mw_start);
	if (range->mw_end < 0)
		gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (doc), &end);
	else
		gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &end, range->mw_end);

	w = gtk_text_buffer_get_slice (GTK_TEXT_BUFFER (doc), &start, &end, TRUE);
	g_return_if_fail (w != NULL);

	if (strcmp (w, word) != 0)
	{
		g_free (w);
		return;
	}

	g_free (w);

	gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER(doc));

	gtk_text_buffer_delete (GTK_TEXT_BUFFER (doc), &start, &end);
	gtk_text_buffer_insert (GTK_TEXT_BUFFER (doc), &start, change, -1);

	gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER(doc));

	update_current (doc, range->mw_start + g_utf8_strlen (change, -1));

	/* go to next misspelled word */
	ignore_cb (dlg, word, view);
}

static void
change_all_cb (XeditSpellCheckerDialog *dlg,
	       const gchar             *word,
	       const gchar             *change,
	       XeditView               *view)
{
	XeditDocument *doc;
	CheckRange *range;
	gchar *w = NULL;
	GtkTextIter start, end;
	gint flags = 0;

	xedit_debug (DEBUG_PLUGINS);

	g_return_if_fail (view != NULL);
	g_return_if_fail (word != NULL);
	g_return_if_fail (change != NULL);

	doc = XEDIT_DOCUMENT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)));
	g_return_if_fail (doc != NULL);

	range = get_check_range (doc);
	g_return_if_fail (range != NULL);

	gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &start, range->mw_start);
	if (range->mw_end < 0)
		gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (doc), &end);
	else
		gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &end, range->mw_end);

	w = gtk_text_buffer_get_slice (GTK_TEXT_BUFFER (doc), &start, &end, TRUE);
	g_return_if_fail (w != NULL);

	if (strcmp (w, word) != 0)
	{
		g_free (w);
		return;
	}

	g_free (w);

	XEDIT_SEARCH_SET_CASE_SENSITIVE (flags, TRUE);
	XEDIT_SEARCH_SET_ENTIRE_WORD (flags, TRUE);

	/* CHECK: currently this function does escaping etc */
	xedit_document_replace_all (doc, word, change, flags);

	update_current (doc, range->mw_start + g_utf8_strlen (change, -1));

	/* go to next misspelled word */
	ignore_cb (dlg, word, view);
}

static void
add_word_cb (XeditSpellCheckerDialog *dlg,
	     const gchar             *word,
	     XeditView               *view)
{
	g_return_if_fail (view != NULL);
	g_return_if_fail (word != NULL);

	/* go to next misspelled word */
	ignore_cb (dlg, word, view);
}

static void
language_dialog_response (GtkDialog         *dlg,
			  gint               res_id,
			  XeditSpellChecker *spell)
{
	if (res_id == GTK_RESPONSE_OK)
	{
		const XeditSpellCheckerLanguage *lang;

		lang = xedit_spell_language_get_selected_language (XEDIT_SPELL_LANGUAGE_DIALOG (dlg));
		if (lang != NULL)
			xedit_spell_checker_set_language (spell, lang);
	}

	gtk_widget_destroy (GTK_WIDGET (dlg));
}

static SpellConfigureDialog *
get_configure_dialog (XeditSpellPlugin *plugin)
{
	SpellConfigureDialog *dialog = NULL;
	gchar *data_dir;
	gchar *ui_file;
	GtkWidget *content;
	XeditSpellPluginAutocheckType autocheck_type;
	GtkWidget *error_widget;
	gboolean ret;
	gchar *root_objects[] = {
		"spell_dialog_content",
		NULL
	};

	xedit_debug (DEBUG_PLUGINS);

	GtkWidget *dlg = gtk_dialog_new_with_buttons (_("Configure Spell Checker plugin..."),
							NULL,
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_STOCK_CANCEL,
							GTK_RESPONSE_CANCEL,
							GTK_STOCK_OK,
							GTK_RESPONSE_OK,
							GTK_STOCK_HELP,
							GTK_RESPONSE_HELP,
							NULL);

	g_return_val_if_fail (dlg != NULL, NULL);

	dialog = g_new0 (SpellConfigureDialog, 1);
	dialog->dialog = dlg;


	/* HIG defaults */
	gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (dialog->dialog)), 5);
	gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog->dialog))),
					2); /* 2 * 5 + 2 = 12 */
	gtk_container_set_border_width (GTK_CONTAINER (gtk_dialog_get_action_area (GTK_DIALOG (dialog->dialog))),
					5);
	gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_action_area (GTK_DIALOG (dialog->dialog))), 6);

	data_dir = xedit_plugin_get_data_dir (XEDIT_PLUGIN (plugin));
	ui_file = g_build_filename (data_dir, "xedit-spell-setup-dialog.ui", NULL);
	ret = xedit_utils_get_ui_objects (ui_file,
					  root_objects,
					  &error_widget,
					  "spell_dialog_content", &content,
					  "autocheck_never", &dialog->never,
					  "autocheck_document", &dialog->document,
					  "autocheck_always", &dialog->always,
					  NULL);

	g_free (data_dir);
	g_free (ui_file);

	if (!ret)
	{
		gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog->dialog))),
					error_widget, TRUE, TRUE, 0);

		gtk_container_set_border_width (GTK_CONTAINER (error_widget), 5);

		gtk_widget_show (error_widget);

		return dialog;
	}

	gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), FALSE);

	autocheck_type = get_autocheck_type (plugin);

	if (autocheck_type == AUTOCHECK_ALWAYS)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->always), TRUE);
	}
	else if (autocheck_type == AUTOCHECK_DOCUMENT)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->document), TRUE);
	}
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->never), TRUE);
	}

	gtk_window_set_default_size (GTK_WIDGET (content), 15, 120);

	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog->dialog))),
				content, FALSE, FALSE, 0);
	g_object_unref (content);
	gtk_container_set_border_width (GTK_CONTAINER (content), 5);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog->dialog),
					 GTK_RESPONSE_OK);

	return dialog;
}

static void
ok_button_pressed (SpellConfigureDialog *dialog)
{
	xedit_debug (DEBUG_PLUGINS);

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->always)))
	{
		set_autocheck_type (dialog->plugin, AUTOCHECK_ALWAYS);
	}
	else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->document)))
	{
		set_autocheck_type (dialog->plugin, AUTOCHECK_DOCUMENT);
	}
	else
	{
		set_autocheck_type (dialog->plugin, AUTOCHECK_NEVER);
	}
}

static void
configure_dialog_response_cb (GtkWidget *widget,
			      gint response,
			      SpellConfigureDialog *dialog)
{
	switch (response)
	{
		case GTK_RESPONSE_HELP:
		{
			xedit_debug_message (DEBUG_PLUGINS, "GTK_RESPONSE_HELP");

			xedit_help_display (GTK_WINDOW (widget),
					    NULL,
					    "xedit-spell-checker-plugin");
			break;
                }
		case GTK_RESPONSE_OK:
		{
			xedit_debug_message (DEBUG_PLUGINS, "GTK_RESPONSE_OK");

			ok_button_pressed (dialog);

			gtk_widget_destroy (dialog->dialog);
			break;
		}
		case GTK_RESPONSE_CANCEL:
		{
			xedit_debug_message (DEBUG_PLUGINS, "GTK_RESPONSE_CANCEL");
			gtk_widget_destroy (dialog->dialog);
		}
	}
}

static void
set_language_cb (GtkAction   *action,
		 ActionData *action_data)
{
	XeditDocument *doc;
	XeditSpellChecker *spell;
	const XeditSpellCheckerLanguage *lang;
	GtkWidget *dlg;
	GtkWindowGroup *wg;
	gchar *data_dir;

	xedit_debug (DEBUG_PLUGINS);

	doc = xedit_window_get_active_document (action_data->window);
	g_return_if_fail (doc != NULL);

	spell = get_spell_checker_from_document (doc);
	g_return_if_fail (spell != NULL);

	lang = xedit_spell_checker_get_language (spell);

	data_dir = xedit_plugin_get_data_dir (action_data->plugin);
	dlg = xedit_spell_language_dialog_new (GTK_WINDOW (action_data->window),
					       lang,
					       data_dir);
	g_free (data_dir);

	wg = xedit_window_get_group (action_data->window);

	gtk_window_group_add_window (wg, GTK_WINDOW (dlg));

	gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);

	g_signal_connect (dlg,
			  "response",
			  G_CALLBACK (language_dialog_response),
			  spell);

	gtk_widget_show (dlg);
}

static void
spell_cb (GtkAction   *action,
	  ActionData *action_data)
{
	XeditView *view;
	XeditDocument *doc;
	XeditSpellChecker *spell;
	GtkWidget *dlg;
	GtkTextIter start, end;
	gchar *word;
	gchar *data_dir;

	xedit_debug (DEBUG_PLUGINS);

	view = xedit_window_get_active_view (action_data->window);
	g_return_if_fail (view != NULL);

	doc = XEDIT_DOCUMENT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)));
	g_return_if_fail (doc != NULL);

	spell = get_spell_checker_from_document (doc);
	g_return_if_fail (spell != NULL);

	if (gtk_text_buffer_get_char_count (GTK_TEXT_BUFFER (doc)) <= 0)
	{
		WindowData *data;
		GtkWidget *statusbar;

		data = (WindowData *) g_object_get_data (G_OBJECT (action_data->window),
							 WINDOW_DATA_KEY);
		g_return_if_fail (data != NULL);

		statusbar = xedit_window_get_statusbar (action_data->window);
		xedit_statusbar_flash_message (XEDIT_STATUSBAR (statusbar),
					       data->message_cid,
					       _("The document is empty."));

		return;
	}

	if (!gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (doc),
						   &start,
						   &end))
	{
		/* no selection, get the whole doc */
		gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (doc),
					    &start,
					    &end);
	}

	set_check_range (doc, &start, &end);

	word = get_next_misspelled_word (view);
	if (word == NULL)
	{
		WindowData *data;
		GtkWidget *statusbar;

		data = (WindowData *) g_object_get_data (G_OBJECT (action_data->window),
							 WINDOW_DATA_KEY);
		g_return_if_fail (data != NULL);

		statusbar = xedit_window_get_statusbar (action_data->window);
		xedit_statusbar_flash_message (XEDIT_STATUSBAR (statusbar),
					       data->message_cid,
					       _("No misspelled words"));

		return;
	}

	data_dir = xedit_plugin_get_data_dir (action_data->plugin);
	dlg = xedit_spell_checker_dialog_new_from_spell_checker (spell, data_dir);
	g_free (data_dir);
	gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (dlg),
				      GTK_WINDOW (action_data->window));

	g_signal_connect (dlg, "ignore", G_CALLBACK (ignore_cb), view);
	g_signal_connect (dlg, "ignore_all", G_CALLBACK (ignore_cb), view);

	g_signal_connect (dlg, "change", G_CALLBACK (change_cb), view);
	g_signal_connect (dlg, "change_all", G_CALLBACK (change_all_cb), view);

	g_signal_connect (dlg, "add_word_to_personal", G_CALLBACK (add_word_cb), view);

	xedit_spell_checker_dialog_set_misspelled_word (XEDIT_SPELL_CHECKER_DIALOG (dlg),
							word,
							-1);

	g_free (word);

	gtk_widget_show (dlg);
}

static void
set_auto_spell (XeditWindow   *window,
		XeditDocument *doc,
		gboolean       active)
{
	XeditAutomaticSpellChecker *autospell;
	XeditSpellChecker *spell;

	spell = get_spell_checker_from_document (doc);
	g_return_if_fail (spell != NULL);

	autospell = xedit_automatic_spell_checker_get_from_document (doc);

	if (active)
	{
		if (autospell == NULL)
		{
			XeditView *active_view;

			active_view = xedit_window_get_active_view (window);
			g_return_if_fail (active_view != NULL);

			autospell = xedit_automatic_spell_checker_new (doc, spell);
			xedit_automatic_spell_checker_attach_view (autospell, active_view);
			xedit_automatic_spell_checker_recheck_all (autospell);
		}
	}
	else
	{
		if (autospell != NULL)
			xedit_automatic_spell_checker_free (autospell);
	}
}

static void
auto_spell_cb (GtkAction   *action,
	       XeditWindow *window)
{
	
	XeditDocument *doc;
	gboolean active;
	WindowData *data;

	xedit_debug (DEBUG_PLUGINS);

	active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

	xedit_debug_message (DEBUG_PLUGINS, active ? "Auto Spell activated" : "Auto Spell deactivated");

	doc = xedit_window_get_active_document (window);
	if (doc == NULL)
		return;

	data = g_object_get_data (G_OBJECT (window),
					  WINDOW_DATA_KEY);

	if (get_autocheck_type(data->plugin) == AUTOCHECK_DOCUMENT)
	{

		xedit_document_set_metadata (doc,
				     XEDIT_METADATA_ATTRIBUTE_SPELL_ENABLED,
				     active ? "1" : NULL, NULL);
	}

	set_auto_spell (window, doc, active);
}

static void
free_window_data (WindowData *data)
{
	g_return_if_fail (data != NULL);

	g_object_unref (data->action_group);
	g_slice_free (WindowData, data);
}

static void
free_action_data (gpointer data)
{
	g_return_if_fail (data != NULL);

	g_slice_free (ActionData, data);
}

static void
update_ui_real (XeditWindow *window,
		WindowData *data)
{
	XeditDocument *doc;
	XeditView *view;
	gboolean autospell;
	GtkAction *action;

	xedit_debug (DEBUG_PLUGINS);

	doc = xedit_window_get_active_document (window);
	view = xedit_window_get_active_view (window);

	autospell = (doc != NULL &&
	             xedit_automatic_spell_checker_get_from_document (doc) != NULL);

	if (doc != NULL)
	{
		XeditTab *tab;
		XeditTabState state;

		tab = xedit_window_get_active_tab (window);
		state = xedit_tab_get_state (tab);

		/* If the document is loading we can't get the metadata so we
		   endup with an useless speller */
		if (state == XEDIT_TAB_STATE_NORMAL)
		{
			action = gtk_action_group_get_action (data->action_group,
							      "AutoSpell");
	
			g_signal_handlers_block_by_func (action, auto_spell_cb,
							 window);
			set_auto_spell (window, doc, autospell);
			gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
						      autospell);
			g_signal_handlers_unblock_by_func (action, auto_spell_cb,
							   window);
		}
	}

	gtk_action_group_set_sensitive (data->action_group,
					(view != NULL) &&
					gtk_text_view_get_editable (GTK_TEXT_VIEW (view)));
}

static void
set_auto_spell_from_metadata (XeditWindow    *window,
			      XeditDocument  *doc,
			      GtkActionGroup *action_group)
{
	gboolean active = FALSE;
	gchar *active_str = NULL;
	XeditDocument *active_doc;
	XeditSpellPluginAutocheckType autocheck_type;
	WindowData *data;

	data = g_object_get_data (G_OBJECT (window),
					  WINDOW_DATA_KEY);

	autocheck_type = get_autocheck_type(data->plugin);

	switch (autocheck_type)
	{
		case AUTOCHECK_ALWAYS:
		{
			active = TRUE;
			break;
		}
		case AUTOCHECK_DOCUMENT:
		{
			active_str = xedit_document_get_metadata (doc,
						  XEDIT_METADATA_ATTRIBUTE_SPELL_ENABLED);
			break;
		}
		case AUTOCHECK_NEVER:
		default:
			active = FALSE;
			break;
	}

	if (active_str)
	{
		active = *active_str == '1';
	
		g_free (active_str);
	}

	set_auto_spell (window, doc, active);

	/* In case that the doc is the active one we mark the spell action */
	active_doc = xedit_window_get_active_document (window);

	if (active_doc == doc && action_group != NULL)
	{
		GtkAction *action;
		
		action = gtk_action_group_get_action (action_group,
						      "AutoSpell");

		g_signal_handlers_block_by_func (action, auto_spell_cb,
						 window);
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
					      active);
		g_signal_handlers_unblock_by_func (action, auto_spell_cb,
						   window);
	}
}

static void
on_document_loaded (XeditDocument *doc,
		    const GError  *error,
		    XeditWindow   *window)
{
	if (error == NULL)
	{
		WindowData *data;
		XeditSpellChecker *spell;

		spell = XEDIT_SPELL_CHECKER (g_object_get_qdata (G_OBJECT (doc),
								 spell_checker_id));
		if (spell != NULL)
		{
			set_language_from_metadata (spell, doc);
		}

		data = g_object_get_data (G_OBJECT (window),
					  WINDOW_DATA_KEY);

		set_auto_spell_from_metadata (window, doc, data->action_group);
	}
}

static void
on_document_saved (XeditDocument *doc,
		   const GError  *error,
		   XeditWindow   *window)
{
	XeditAutomaticSpellChecker *autospell;
	XeditSpellChecker *spell;
	const gchar *key;
	WindowData *data;

	if (error != NULL)
	{
		return;
	}

	/* Make sure to save the metadata here too */
	autospell = xedit_automatic_spell_checker_get_from_document (doc);
	spell = XEDIT_SPELL_CHECKER (g_object_get_qdata (G_OBJECT (doc), spell_checker_id));

	if (spell != NULL)
	{
		key = xedit_spell_checker_language_to_key (xedit_spell_checker_get_language (spell));
	}
	else
	{
		key = NULL;
	}

	data = g_object_get_data (G_OBJECT (window),
					  WINDOW_DATA_KEY);

	if (get_autocheck_type(data->plugin) == AUTOCHECK_DOCUMENT)
	{

		xedit_document_set_metadata (doc,
				XEDIT_METADATA_ATTRIBUTE_SPELL_ENABLED,
				autospell != NULL ? "1" : NULL,
				XEDIT_METADATA_ATTRIBUTE_SPELL_LANGUAGE,
				key,
				NULL);
	}
	else
	{
		xedit_document_set_metadata (doc,
				XEDIT_METADATA_ATTRIBUTE_SPELL_LANGUAGE,
				key,
				NULL);
	}
}

static void
tab_added_cb (XeditWindow *window,
	      XeditTab    *tab,
	      gpointer     useless)
{
	XeditDocument *doc;

	doc = xedit_tab_get_document (tab);

	g_signal_connect (doc, "loaded",
			  G_CALLBACK (on_document_loaded),
			  window);

	g_signal_connect (doc, "saved",
			  G_CALLBACK (on_document_saved),
			  window);
}

static void
tab_removed_cb (XeditWindow *window,
		XeditTab    *tab,
		gpointer     useless)
{
	XeditDocument *doc;

	doc = xedit_tab_get_document (tab);
	
	g_signal_handlers_disconnect_by_func (doc, on_document_loaded, window);
	g_signal_handlers_disconnect_by_func (doc, on_document_saved, window);
}

static void
impl_activate (XeditPlugin *plugin,
	       XeditWindow *window)
{
	GtkUIManager *manager;
	WindowData *data;
	ActionData *action_data;
	GList *docs, *l;

	xedit_debug (DEBUG_PLUGINS);

	data = g_slice_new (WindowData);
	data->plugin = XEDIT_SPELL_PLUGIN (plugin);
	action_data = g_slice_new (ActionData);
	action_data->plugin = plugin;
	action_data->window = window;

	manager = xedit_window_get_ui_manager (window);

	data->action_group = gtk_action_group_new ("XeditSpellPluginActions");
	gtk_action_group_set_translation_domain (data->action_group, 
						 GETTEXT_PACKAGE);
	gtk_action_group_add_actions_full (data->action_group,
					   action_entries,
					   G_N_ELEMENTS (action_entries),
					   action_data,
					   (GDestroyNotify) free_action_data);
	gtk_action_group_add_toggle_actions (data->action_group, 
					     toggle_action_entries,
					     G_N_ELEMENTS (toggle_action_entries),
					     window);

	gtk_ui_manager_insert_action_group (manager, data->action_group, -1);

	data->ui_id = gtk_ui_manager_new_merge_id (manager);

	data->message_cid = gtk_statusbar_get_context_id
			(GTK_STATUSBAR (xedit_window_get_statusbar (window)), 
			 "spell_plugin_message");

	g_object_set_data_full (G_OBJECT (window),
				WINDOW_DATA_KEY, 
				data,
				(GDestroyNotify) free_window_data);

	gtk_ui_manager_add_ui (manager,
			       data->ui_id,
			       MENU_PATH,
			       "CheckSpell",
			       "CheckSpell",
			       GTK_UI_MANAGER_MENUITEM, 
			       FALSE);

	gtk_ui_manager_add_ui (manager, 
			       data->ui_id, 
			       MENU_PATH,
			       "AutoSpell", 
			       "AutoSpell",
			       GTK_UI_MANAGER_MENUITEM, 
			       FALSE);

	gtk_ui_manager_add_ui (manager, 
			       data->ui_id, 
			       MENU_PATH,
			       "ConfigSpell", 
			       "ConfigSpell",
			       GTK_UI_MANAGER_MENUITEM, 
			       FALSE);

	update_ui_real (window, data);

	docs = xedit_window_get_documents (window);
	for (l = docs; l != NULL; l = g_list_next (l))
	{
		XeditDocument *doc = XEDIT_DOCUMENT (l->data);

		set_auto_spell_from_metadata (window, doc,
					      data->action_group);

		g_signal_handlers_disconnect_by_func (doc,
		                                      on_document_loaded,
		                                      window);

		g_signal_handlers_disconnect_by_func (doc,
		                                      on_document_saved,
		                                      window);
	}

	data->tab_added_id =
		g_signal_connect (window, "tab-added",
				  G_CALLBACK (tab_added_cb), NULL);
	data->tab_removed_id =
		g_signal_connect (window, "tab-removed",
				  G_CALLBACK (tab_removed_cb), NULL);
}

static void
impl_deactivate	(XeditPlugin *plugin,
		 XeditWindow *window)
{
	GtkUIManager *manager;
	WindowData *data;

	xedit_debug (DEBUG_PLUGINS);

	manager = xedit_window_get_ui_manager (window);

	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	gtk_ui_manager_remove_ui (manager, data->ui_id);
	gtk_ui_manager_remove_action_group (manager, data->action_group);

	g_signal_handler_disconnect (window, data->tab_added_id);
	g_signal_handler_disconnect (window, data->tab_removed_id);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static void
impl_update_ui (XeditPlugin *plugin,
		XeditWindow *window)
{
	WindowData *data;

	xedit_debug (DEBUG_PLUGINS);

	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	update_ui_real (window, data);
}

static GtkWidget *
impl_create_configure_dialog (XeditPlugin *plugin)
{
	SpellConfigureDialog *dialog;

	dialog = get_configure_dialog(XEDIT_SPELL_PLUGIN (plugin));

	dialog->plugin = XEDIT_SPELL_PLUGIN (plugin);

	g_signal_connect (dialog->dialog,
			"response",
			G_CALLBACK (configure_dialog_response_cb),
			dialog);

	return GTK_WIDGET (dialog->dialog);
}

static void
xedit_spell_plugin_class_init (XeditSpellPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	XeditPluginClass *plugin_class = XEDIT_PLUGIN_CLASS (klass);

	object_class->finalize = xedit_spell_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
	plugin_class->update_ui = impl_update_ui;

	plugin_class->create_configure_dialog = impl_create_configure_dialog;

	if (spell_checker_id == 0)
		spell_checker_id = g_quark_from_string ("XeditSpellCheckerID");

	if (check_range_id == 0)
		check_range_id = g_quark_from_string ("CheckRangeID");

	g_type_class_add_private (object_class, sizeof (XeditSpellPluginPrivate));
}
