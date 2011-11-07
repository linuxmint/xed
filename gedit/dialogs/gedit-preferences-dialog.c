/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gedit-preferences-dialog.c
 * This file is part of gedit
 *
 * Copyright (C) 2001-2005 Paolo Maggi 
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
 * Modified by the gedit Team, 2001-2003. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include <glib/gi18n.h>
#include <mateconf/mateconf-client.h>

#include <gedit/gedit-prefs-manager.h>

#include "gedit-preferences-dialog.h"
#include "gedit-utils.h"
#include "gedit-debug.h"
#include "gedit-document.h"
#include "gedit-style-scheme-manager.h"
#include "gedit-plugin-manager.h"
#include "gedit-help.h"
#include "gedit-dirs.h"

/*
 * gedit-preferences dialog is a singleton since we don't
 * want two dialogs showing an inconsistent state of the
 * preferences.
 * When gedit_show_preferences_dialog is called and there
 * is already a prefs dialog dialog open, it is reparented
 * and shown.
 */

static GtkWidget *preferences_dialog = NULL;


enum
{
	ID_COLUMN = 0,
	NAME_COLUMN,
	DESC_COLUMN,
	NUM_COLUMNS
};


#define GEDIT_PREFERENCES_DIALOG_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
						     GEDIT_TYPE_PREFERENCES_DIALOG, \
						     GeditPreferencesDialogPrivate))

struct _GeditPreferencesDialogPrivate
{
	GtkWidget	*notebook;

	/* Font */
	GtkWidget	*default_font_checkbutton;
	GtkWidget	*font_button;
	GtkWidget	*font_hbox;

	/* Style Scheme */
	GtkListStore	*schemes_treeview_model;
	GtkWidget	*schemes_treeview;
	GtkWidget	*install_scheme_button;
	GtkWidget	*uninstall_scheme_button;
	
	GtkWidget	*install_scheme_file_schooser;

	/* Tabs */
	GtkWidget	*tabs_width_spinbutton;
	GtkWidget	*insert_spaces_checkbutton;
	GtkWidget	*tabs_width_hbox;

	/* Auto indentation */
	GtkWidget	*auto_indent_checkbutton;

	/* Text Wrapping */
	GtkWidget	*wrap_text_checkbutton;
	GtkWidget	*split_checkbutton;

	/* File Saving */
	GtkWidget	*backup_copy_checkbutton;
	GtkWidget	*auto_save_checkbutton;
	GtkWidget	*auto_save_spinbutton;
	GtkWidget	*autosave_hbox;
	
	/* Line numbers */
	GtkWidget	*display_line_numbers_checkbutton;

	/* Highlight current line */
	GtkWidget	*highlight_current_line_checkbutton;
	
	/* Highlight matching bracket */
	GtkWidget	*bracket_matching_checkbutton;
	
	/* Right margin */
	GtkWidget	*right_margin_checkbutton;
	GtkWidget	*right_margin_position_spinbutton;
	GtkWidget	*right_margin_position_hbox;

	/* Plugins manager */
	GtkWidget	*plugin_manager_place_holder;

	/* Style Scheme editor dialog */
	GtkWidget	*style_scheme_dialog;
};


G_DEFINE_TYPE(GeditPreferencesDialog, gedit_preferences_dialog, GTK_TYPE_DIALOG)


static void 
gedit_preferences_dialog_class_init (GeditPreferencesDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (GeditPreferencesDialogPrivate));
}

static void
dialog_response_handler (GtkDialog *dlg, 
			 gint       res_id)
{
	gedit_debug (DEBUG_PREFS);

	switch (res_id)
	{
		case GTK_RESPONSE_HELP:
			gedit_help_display (GTK_WINDOW (dlg),
					    NULL,
					    "gedit-prefs");

			g_signal_stop_emission_by_name (dlg, "response");

			break;

		default:
			gtk_widget_destroy (GTK_WIDGET(dlg));
	}
}

static void
tabs_width_spinbutton_value_changed (GtkSpinButton          *spin_button,
				     GeditPreferencesDialog *dlg)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (spin_button == GTK_SPIN_BUTTON (dlg->priv->tabs_width_spinbutton));

	gedit_prefs_manager_set_tabs_size (gtk_spin_button_get_value_as_int (spin_button));
}
	
static void
insert_spaces_checkbutton_toggled (GtkToggleButton        *button,
				   GeditPreferencesDialog *dlg)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (button == GTK_TOGGLE_BUTTON (dlg->priv->insert_spaces_checkbutton));

	gedit_prefs_manager_set_insert_spaces (gtk_toggle_button_get_active (button));
}

static void
auto_indent_checkbutton_toggled (GtkToggleButton        *button,
				 GeditPreferencesDialog *dlg)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (button == GTK_TOGGLE_BUTTON (dlg->priv->auto_indent_checkbutton));

	gedit_prefs_manager_set_auto_indent (gtk_toggle_button_get_active (button));
}

static void
auto_save_checkbutton_toggled (GtkToggleButton        *button,
			       GeditPreferencesDialog *dlg)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (button == GTK_TOGGLE_BUTTON (dlg->priv->auto_save_checkbutton));

	if (gtk_toggle_button_get_active (button))
	{
		gtk_widget_set_sensitive (dlg->priv->auto_save_spinbutton, 
					  gedit_prefs_manager_auto_save_interval_can_set());

		gedit_prefs_manager_set_auto_save (TRUE);
	}
	else	
	{
		gtk_widget_set_sensitive (dlg->priv->auto_save_spinbutton, FALSE);
		gedit_prefs_manager_set_auto_save (FALSE);
	}
}

static void
backup_copy_checkbutton_toggled (GtkToggleButton        *button,
				 GeditPreferencesDialog *dlg)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (button == GTK_TOGGLE_BUTTON (dlg->priv->backup_copy_checkbutton));
	
	gedit_prefs_manager_set_create_backup_copy (gtk_toggle_button_get_active (button));
}

static void
auto_save_spinbutton_value_changed (GtkSpinButton          *spin_button,
				    GeditPreferencesDialog *dlg)
{
	g_return_if_fail (spin_button == GTK_SPIN_BUTTON (dlg->priv->auto_save_spinbutton));

	gedit_prefs_manager_set_auto_save_interval (
			MAX (1, gtk_spin_button_get_value_as_int (spin_button)));
}

static void
setup_editor_page (GeditPreferencesDialog *dlg)
{
	gboolean auto_save;
	gint auto_save_interval;

	gedit_debug (DEBUG_PREFS);

	/* Set initial state */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (dlg->priv->tabs_width_spinbutton),
				   (guint) gedit_prefs_manager_get_tabs_size ());
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->insert_spaces_checkbutton), 
				      gedit_prefs_manager_get_insert_spaces ());
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->auto_indent_checkbutton), 
				      gedit_prefs_manager_get_auto_indent ());
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->backup_copy_checkbutton),
				      gedit_prefs_manager_get_create_backup_copy ());

	auto_save = gedit_prefs_manager_get_auto_save ();
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->auto_save_checkbutton),
				      auto_save);

	auto_save_interval = gedit_prefs_manager_get_auto_save_interval ();
	if (auto_save_interval <= 0)
		auto_save_interval = GPM_DEFAULT_AUTO_SAVE_INTERVAL;

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (dlg->priv->auto_save_spinbutton),
				   auto_save_interval);

	/* Set widget sensitivity */
	gtk_widget_set_sensitive (dlg->priv->tabs_width_hbox, 
				  gedit_prefs_manager_tabs_size_can_set ());
	gtk_widget_set_sensitive (dlg->priv->insert_spaces_checkbutton,
				  gedit_prefs_manager_insert_spaces_can_set ());
	gtk_widget_set_sensitive (dlg->priv->auto_indent_checkbutton,
				  gedit_prefs_manager_auto_indent_can_set ());
	gtk_widget_set_sensitive (dlg->priv->backup_copy_checkbutton,
				  gedit_prefs_manager_create_backup_copy_can_set ());
	gtk_widget_set_sensitive (dlg->priv->autosave_hbox, 
				  gedit_prefs_manager_auto_save_can_set ()); 
	gtk_widget_set_sensitive (dlg->priv->auto_save_spinbutton, 
			          auto_save &&
				  gedit_prefs_manager_auto_save_interval_can_set ());

	/* Connect signal */
	g_signal_connect (dlg->priv->tabs_width_spinbutton,
			  "value_changed",
			  G_CALLBACK (tabs_width_spinbutton_value_changed),
			  dlg);
	g_signal_connect (dlg->priv->insert_spaces_checkbutton,
			 "toggled",
			  G_CALLBACK (insert_spaces_checkbutton_toggled),
			  dlg);
	g_signal_connect (dlg->priv->auto_indent_checkbutton,
			  "toggled",
			  G_CALLBACK (auto_indent_checkbutton_toggled),
			  dlg);
	g_signal_connect (dlg->priv->auto_save_checkbutton,
			  "toggled",
			  G_CALLBACK (auto_save_checkbutton_toggled),
			  dlg);
	g_signal_connect (dlg->priv->backup_copy_checkbutton,
			  "toggled",
			  G_CALLBACK (backup_copy_checkbutton_toggled),
			  dlg);
	g_signal_connect (dlg->priv->auto_save_spinbutton,
			  "value_changed",
			  G_CALLBACK (auto_save_spinbutton_value_changed),
			  dlg);
}

static void
display_line_numbers_checkbutton_toggled (GtkToggleButton        *button,
					  GeditPreferencesDialog *dlg)
{
	g_return_if_fail (button == 
			GTK_TOGGLE_BUTTON (dlg->priv->display_line_numbers_checkbutton));

	gedit_prefs_manager_set_display_line_numbers (gtk_toggle_button_get_active (button));
}

static void
highlight_current_line_checkbutton_toggled (GtkToggleButton        *button,
					    GeditPreferencesDialog *dlg)
{
	g_return_if_fail (button == 
			GTK_TOGGLE_BUTTON (dlg->priv->highlight_current_line_checkbutton));

	gedit_prefs_manager_set_highlight_current_line (gtk_toggle_button_get_active (button));
}

static void
bracket_matching_checkbutton_toggled (GtkToggleButton        *button,
				      GeditPreferencesDialog *dlg)
{
	g_return_if_fail (button == 
			GTK_TOGGLE_BUTTON (dlg->priv->bracket_matching_checkbutton));

	gedit_prefs_manager_set_bracket_matching (
				gtk_toggle_button_get_active (button));
}

static gboolean split_button_state = TRUE;

static void
wrap_mode_checkbutton_toggled (GtkToggleButton        *button, 
			       GeditPreferencesDialog *dlg)
{
	if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dlg->priv->wrap_text_checkbutton)))
	{
		gedit_prefs_manager_set_wrap_mode (GTK_WRAP_NONE);
		
		gtk_widget_set_sensitive (dlg->priv->split_checkbutton, 
					  FALSE);
		gtk_toggle_button_set_inconsistent (
			GTK_TOGGLE_BUTTON (dlg->priv->split_checkbutton), TRUE);
	}
	else
	{
		gtk_widget_set_sensitive (dlg->priv->split_checkbutton, 
					  TRUE);

		gtk_toggle_button_set_inconsistent (
			GTK_TOGGLE_BUTTON (dlg->priv->split_checkbutton), FALSE);


		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dlg->priv->split_checkbutton)))
		{
			split_button_state = TRUE;
			
			gedit_prefs_manager_set_wrap_mode (GTK_WRAP_WORD);
		}
		else
		{
			split_button_state = FALSE;
			
			gedit_prefs_manager_set_wrap_mode (GTK_WRAP_CHAR);
		}
	}
}

static void
right_margin_checkbutton_toggled (GtkToggleButton        *button,
				  GeditPreferencesDialog *dlg)
{
	gboolean active;
	
	g_return_if_fail (button == GTK_TOGGLE_BUTTON (dlg->priv->right_margin_checkbutton));

	active = gtk_toggle_button_get_active (button);
	
	gedit_prefs_manager_set_display_right_margin (active);

	gtk_widget_set_sensitive (dlg->priv->right_margin_position_hbox,
				  active && 
				  gedit_prefs_manager_right_margin_position_can_set ());
}

static void
right_margin_position_spinbutton_value_changed (GtkSpinButton          *spin_button, 
						GeditPreferencesDialog *dlg)
{
	gint value;
	
	g_return_if_fail (spin_button == GTK_SPIN_BUTTON (dlg->priv->right_margin_position_spinbutton));

	value = CLAMP (gtk_spin_button_get_value_as_int (spin_button), 1, 160);

	gedit_prefs_manager_set_right_margin_position (value);
}

static void
setup_view_page (GeditPreferencesDialog *dlg)
{
	GtkWrapMode wrap_mode;
	gboolean display_right_margin;
	gboolean wrap_mode_can_set;

	gedit_debug (DEBUG_PREFS);
	
	/* Set initial state */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->display_line_numbers_checkbutton),
				      gedit_prefs_manager_get_display_line_numbers ());
	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->highlight_current_line_checkbutton),
				      gedit_prefs_manager_get_highlight_current_line ());
	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->bracket_matching_checkbutton),
				      gedit_prefs_manager_get_bracket_matching ());

	wrap_mode = gedit_prefs_manager_get_wrap_mode ();
	switch (wrap_mode )
	{
		case GTK_WRAP_WORD:
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (dlg->priv->wrap_text_checkbutton), TRUE);
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (dlg->priv->split_checkbutton), TRUE);
			break;
		case GTK_WRAP_CHAR:
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (dlg->priv->wrap_text_checkbutton), TRUE);
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (dlg->priv->split_checkbutton), FALSE);
			break;
		default:
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (dlg->priv->wrap_text_checkbutton), FALSE);
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (dlg->priv->split_checkbutton), split_button_state);
			gtk_toggle_button_set_inconsistent (
				GTK_TOGGLE_BUTTON (dlg->priv->split_checkbutton), TRUE);

	}

	display_right_margin = gedit_prefs_manager_get_display_right_margin ();
	
	gtk_toggle_button_set_active (
		GTK_TOGGLE_BUTTON (dlg->priv->right_margin_checkbutton), 
		display_right_margin);
	
	gtk_spin_button_set_value (
		GTK_SPIN_BUTTON (dlg->priv->right_margin_position_spinbutton),
		(guint)CLAMP (gedit_prefs_manager_get_right_margin_position (), 1, 160));
		
	/* Set widgets sensitivity */
	gtk_widget_set_sensitive (dlg->priv->display_line_numbers_checkbutton,
				  gedit_prefs_manager_display_line_numbers_can_set ());
	gtk_widget_set_sensitive (dlg->priv->highlight_current_line_checkbutton,
				  gedit_prefs_manager_highlight_current_line_can_set ());
	gtk_widget_set_sensitive (dlg->priv->bracket_matching_checkbutton,
				  gedit_prefs_manager_bracket_matching_can_set ());
	wrap_mode_can_set = gedit_prefs_manager_wrap_mode_can_set ();
	gtk_widget_set_sensitive (dlg->priv->wrap_text_checkbutton, 
				  wrap_mode_can_set);
	gtk_widget_set_sensitive (dlg->priv->split_checkbutton, 
				  wrap_mode_can_set && 
				  (wrap_mode != GTK_WRAP_NONE));
	gtk_widget_set_sensitive (dlg->priv->right_margin_checkbutton,
				  gedit_prefs_manager_display_right_margin_can_set ());
	gtk_widget_set_sensitive (dlg->priv->right_margin_position_hbox,
				  display_right_margin && 
				  gedit_prefs_manager_right_margin_position_can_set ());
				  
	/* Connect signals */
	g_signal_connect (dlg->priv->display_line_numbers_checkbutton,
			  "toggled",
			  G_CALLBACK (display_line_numbers_checkbutton_toggled), 
			  dlg);
	g_signal_connect (dlg->priv->highlight_current_line_checkbutton,
			  "toggled", 
			  G_CALLBACK (highlight_current_line_checkbutton_toggled), 
			  dlg);
	g_signal_connect (dlg->priv->bracket_matching_checkbutton,
			  "toggled", 
			  G_CALLBACK (bracket_matching_checkbutton_toggled), 
			  dlg);
	g_signal_connect (dlg->priv->wrap_text_checkbutton,
			  "toggled", 
			  G_CALLBACK (wrap_mode_checkbutton_toggled), 
			  dlg);
	g_signal_connect (dlg->priv->split_checkbutton,
			  "toggled", 
			  G_CALLBACK (wrap_mode_checkbutton_toggled), 
			  dlg);
	g_signal_connect (dlg->priv->right_margin_checkbutton,
			  "toggled", 
			  G_CALLBACK (right_margin_checkbutton_toggled), 
			  dlg);
	g_signal_connect (dlg->priv->right_margin_position_spinbutton,
			  "value_changed",
			  G_CALLBACK (right_margin_position_spinbutton_value_changed), 
			  dlg);
}

static void
default_font_font_checkbutton_toggled (GtkToggleButton        *button,
				       GeditPreferencesDialog *dlg)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (button == GTK_TOGGLE_BUTTON (dlg->priv->default_font_checkbutton));

	if (gtk_toggle_button_get_active (button))
	{
		gtk_widget_set_sensitive (dlg->priv->font_hbox, FALSE);
		gedit_prefs_manager_set_use_default_font (TRUE);
	}
	else
	{
		gtk_widget_set_sensitive (dlg->priv->font_hbox, 
					  gedit_prefs_manager_editor_font_can_set ());
		gedit_prefs_manager_set_use_default_font (FALSE);
	}
}

static void
editor_font_button_font_set (GtkFontButton          *font_button,
			     GeditPreferencesDialog *dlg)
{
	const gchar *font_name;

	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (font_button == GTK_FONT_BUTTON (dlg->priv->font_button));

	/* FIXME: Can this fail? Gtk docs are a bit terse... 21-02-2004 pbor */
	font_name = gtk_font_button_get_font_name (font_button);
	if (!font_name)
	{
		g_warning ("Could not get font name");
		return;
	}

	gedit_prefs_manager_set_editor_font (font_name);
}

static void
setup_font_colors_page_font_section (GeditPreferencesDialog *dlg)
{
	gboolean use_default_font;
	gchar *editor_font = NULL;
	gchar *label;

	gedit_debug (DEBUG_PREFS);

	gtk_widget_set_tooltip_text (dlg->priv->font_button,
			 _("Click on this button to select the font to be used by the editor"));

	gedit_utils_set_atk_relation (dlg->priv->font_button,
				      dlg->priv->default_font_checkbutton,
				      ATK_RELATION_CONTROLLED_BY);
	gedit_utils_set_atk_relation (dlg->priv->default_font_checkbutton,
				      dlg->priv->font_button, 
				      ATK_RELATION_CONTROLLER_FOR);	

	editor_font = gedit_prefs_manager_get_system_font ();
	label = g_strdup_printf(_("_Use the system fixed width font (%s)"),
				editor_font);
	gtk_button_set_label (GTK_BUTTON (dlg->priv->default_font_checkbutton),
			      label);
	g_free (editor_font);
	g_free (label);

	/* read current config and setup initial state */
	use_default_font = gedit_prefs_manager_get_use_default_font ();
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->default_font_checkbutton),
				      use_default_font);

	editor_font = gedit_prefs_manager_get_editor_font ();
	if (editor_font != NULL)
	{
		gtk_font_button_set_font_name (GTK_FONT_BUTTON (dlg->priv->font_button),
					       editor_font);
		g_free (editor_font);
	}

	/* Connect signals */
	g_signal_connect (dlg->priv->default_font_checkbutton,
			  "toggled",
			  G_CALLBACK (default_font_font_checkbutton_toggled),
			  dlg);
	g_signal_connect (dlg->priv->font_button,
			  "font_set",
			  G_CALLBACK (editor_font_button_font_set),
			  dlg);

	/* Set initial widget sensitivity */
	gtk_widget_set_sensitive (dlg->priv->default_font_checkbutton, 
				  gedit_prefs_manager_use_default_font_can_set ());

	if (use_default_font)
		gtk_widget_set_sensitive (dlg->priv->font_hbox, FALSE);
	else
		gtk_widget_set_sensitive (dlg->priv->font_hbox, 
					  gedit_prefs_manager_editor_font_can_set ());
}

static void
set_buttons_sensisitivity_according_to_scheme (GeditPreferencesDialog *dlg,
					       const gchar            *scheme_id)
{
	gboolean editable;
	
	editable = (scheme_id != NULL) && 
	           _gedit_style_scheme_manager_scheme_is_gedit_user_scheme (
						gedit_get_style_scheme_manager (),
						scheme_id);

	gtk_widget_set_sensitive (dlg->priv->uninstall_scheme_button,
				  editable);
}

static void
style_scheme_changed (GtkWidget              *treeview,
		      GeditPreferencesDialog *dlg)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	gchar *id;

	gtk_tree_view_get_cursor (GTK_TREE_VIEW (dlg->priv->schemes_treeview), &path, NULL);
	gtk_tree_model_get_iter (GTK_TREE_MODEL (dlg->priv->schemes_treeview_model),
				 &iter, path);
	gtk_tree_path_free (path);
	gtk_tree_model_get (GTK_TREE_MODEL (dlg->priv->schemes_treeview_model),
			    &iter, ID_COLUMN, &id, -1);

	gedit_prefs_manager_set_source_style_scheme (id);

	set_buttons_sensisitivity_according_to_scheme (dlg, id);

	g_free (id);
}

static const gchar *
ensure_color_scheme_id (const gchar *id)
{
	GtkSourceStyleScheme *scheme = NULL;
	GtkSourceStyleSchemeManager *manager = gedit_get_style_scheme_manager ();

	if (id == NULL)
	{
		gchar *pref_id;

		pref_id = gedit_prefs_manager_get_source_style_scheme ();
		scheme = gtk_source_style_scheme_manager_get_scheme (manager,
								     pref_id);
		g_free (pref_id);
	}
	else
	{
		scheme = gtk_source_style_scheme_manager_get_scheme (manager,
								     id);
	}

	if (scheme == NULL)
	{
		/* Fall-back to classic style scheme */
		scheme = gtk_source_style_scheme_manager_get_scheme (manager,
								     "classic");
	}

	if (scheme == NULL)
	{
		/* Cannot determine default style scheme -> broken GtkSourceView installation */
		return NULL;
	}

	return 	gtk_source_style_scheme_get_id (scheme);
}

/* If def_id is NULL, use the default scheme as returned by 
 * gedit_style_scheme_manager_get_default_scheme. If this one returns NULL
 * use the first available scheme as default */
static const gchar *
populate_color_scheme_list (GeditPreferencesDialog *dlg, const gchar *def_id)
{
	GSList *schemes;
	GSList *l;
	
	gtk_list_store_clear (dlg->priv->schemes_treeview_model);
	
	def_id = ensure_color_scheme_id (def_id);
	if (def_id == NULL) 
	{
		g_warning ("Cannot build the list of available color schemes.\n"
		           "Please check your GtkSourceView installation.");
		return NULL;
	}
	
	schemes = gedit_style_scheme_manager_list_schemes_sorted (gedit_get_style_scheme_manager ());
	l = schemes;
	while (l != NULL)
	{
		GtkSourceStyleScheme *scheme;
		const gchar *id;
		const gchar *name;
		const gchar *description;
		GtkTreeIter iter;

		scheme = GTK_SOURCE_STYLE_SCHEME (l->data);
				
		id = gtk_source_style_scheme_get_id (scheme);
		name = gtk_source_style_scheme_get_name (scheme);
		description = gtk_source_style_scheme_get_description (scheme);

		gtk_list_store_append (dlg->priv->schemes_treeview_model, &iter);
		gtk_list_store_set (dlg->priv->schemes_treeview_model,
				    &iter,
				    ID_COLUMN, id,
				    NAME_COLUMN, name,
				    DESC_COLUMN, description,
				    -1);

		g_return_val_if_fail (def_id != NULL, NULL);
		if (strcmp (id, def_id) == 0)
		{
			GtkTreeSelection *selection;
			
			selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dlg->priv->schemes_treeview));
			gtk_tree_selection_select_iter (selection, &iter);
		}
		
		l = g_slist_next (l);
	}
	
	g_slist_free (schemes);
	
	return def_id;
}

static void
add_scheme_chooser_response_cb (GtkDialog              *chooser, 
				gint                    res_id,
				GeditPreferencesDialog *dlg)
{
	gchar* filename; 
	const gchar *scheme_id;
	
	if (res_id != GTK_RESPONSE_ACCEPT)
	{
		gtk_widget_hide (GTK_WIDGET (chooser));
		return;
	}
	
	filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
	if (filename == NULL)
		return;
	
	gtk_widget_hide (GTK_WIDGET (chooser));
	
	scheme_id = _gedit_style_scheme_manager_install_scheme (
					gedit_get_style_scheme_manager (),
					filename);
	g_free (filename);

	if (scheme_id == NULL)
	{
		gedit_warning (GTK_WINDOW (dlg),
			       _("The selected color scheme cannot be installed."));

		return;
	}

	gedit_prefs_manager_set_source_style_scheme (scheme_id);

	scheme_id = populate_color_scheme_list (dlg, scheme_id);

	set_buttons_sensisitivity_according_to_scheme (dlg, scheme_id);
}
			 
static void
install_scheme_clicked (GtkButton              *button,
			GeditPreferencesDialog *dlg)
{
	GtkWidget      *chooser;
	GtkFileFilter  *filter;
	
	if (dlg->priv->install_scheme_file_schooser != NULL) {
		gtk_window_present (GTK_WINDOW (dlg->priv->install_scheme_file_schooser));
		gtk_widget_grab_focus (dlg->priv->install_scheme_file_schooser);
		return;
	}

	chooser = gtk_file_chooser_dialog_new (_("Add Scheme"),
					       GTK_WINDOW (dlg),
					       GTK_FILE_CHOOSER_ACTION_OPEN,
					       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					       NULL);

	gedit_dialog_add_button (GTK_DIALOG (chooser), 
				 _("A_dd Scheme"),
				 GTK_STOCK_ADD,
				 GTK_RESPONSE_ACCEPT);

	gtk_window_set_destroy_with_parent (GTK_WINDOW (chooser), TRUE);
		
	/* Filters */
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("Color Scheme Files"));
	gtk_file_filter_add_pattern (filter, "*.xml");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (chooser), filter);
	
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("All Files"));
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

	gtk_dialog_set_default_response (GTK_DIALOG (chooser), GTK_RESPONSE_ACCEPT);
			
	g_signal_connect (chooser,
			  "response",
			  G_CALLBACK (add_scheme_chooser_response_cb),
			  dlg);

	dlg->priv->install_scheme_file_schooser = chooser;
	
	g_object_add_weak_pointer (G_OBJECT (chooser),
				   (gpointer) &dlg->priv->install_scheme_file_schooser);
				   
	gtk_widget_show (chooser);	
}

static void
uninstall_scheme_clicked (GtkButton              *button,
			  GeditPreferencesDialog *dlg)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dlg->priv->schemes_treeview));
	model = GTK_TREE_MODEL (dlg->priv->schemes_treeview_model);

	if (gtk_tree_selection_get_selected (selection,
					     &model,
					     &iter))
	{
		gchar *id;
		gchar *name;

		gtk_tree_model_get (model, &iter,
				    ID_COLUMN, &id,
				    NAME_COLUMN, &name,
				    -1);
	
		if (!_gedit_style_scheme_manager_uninstall_scheme (gedit_get_style_scheme_manager (), id))
		{
			gedit_warning (GTK_WINDOW (dlg),
				       _("Could not remove color scheme \"%s\"."),
				       name);
		}
		else
		{	
			const gchar *real_new_id;
			gchar *new_id = NULL;
			GtkTreePath *path;
			GtkTreeIter new_iter;
			gboolean new_iter_set = FALSE;

			/* If the removed style scheme is the last of the list,
			 * set as new default style scheme the previous one,
			 * otherwise set the next one.
			 * To make this possible, we need to get the id of the
			 * new default style scheme before re-populating the list.
			 * Fall back to "classic" if it is not possible to get
			 * the id
			 */
			path = gtk_tree_model_get_path (model, &iter);

			/* Try to move to the next path */
			gtk_tree_path_next (path);
			if (!gtk_tree_model_get_iter (model, &new_iter, path))
			{
				/* It seems the removed style scheme was the 
				 * last of the list. Try to move to the 
				 * previous one */
				gtk_tree_path_free (path);

				path = gtk_tree_model_get_path (model, &iter);
				
				gtk_tree_path_prev (path);
				if (gtk_tree_model_get_iter (model, &new_iter, path))
					new_iter_set = TRUE;
			}
			else
				new_iter_set = TRUE;

			gtk_tree_path_free (path);
						
			if (new_iter_set)
				gtk_tree_model_get (model, &new_iter,
						    ID_COLUMN, &new_id,
						    -1);
						    			
			real_new_id = populate_color_scheme_list (dlg, new_id);
			g_free (new_id);
	
			set_buttons_sensisitivity_according_to_scheme (dlg, real_new_id);
			
			if (real_new_id != NULL)
				gedit_prefs_manager_set_source_style_scheme (real_new_id);
		}

		g_free (id);
		g_free (name);
	}
}

static void
scheme_description_cell_data_func (GtkTreeViewColumn *column,
				   GtkCellRenderer   *renderer,
				   GtkTreeModel      *model,
				   GtkTreeIter       *iter,
				   gpointer           data)
{
	gchar *name;
	gchar *desc;
	gchar *text;

	gtk_tree_model_get (model, iter,
			    NAME_COLUMN, &name,
			    DESC_COLUMN, &desc,
			    -1);

	if (desc != NULL)
	{
		text = g_markup_printf_escaped ("<b>%s</b> - %s",
						name,
						desc);
	}
	else
	{
		text = g_markup_printf_escaped ("<b>%s</b>",
						name);
	}

	g_free (name);
	g_free (desc);

	g_object_set (G_OBJECT (renderer),
		      "markup",
		      text,
		      NULL);

	g_free (text);
}

static void
setup_font_colors_page_style_scheme_section (GeditPreferencesDialog *dlg)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;
	const gchar *def_id;
	
	gedit_debug (DEBUG_PREFS);
			  
	/* Create GtkListStore for styles & setup treeview. */
	dlg->priv->schemes_treeview_model = gtk_list_store_new (NUM_COLUMNS,
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_STRING);

	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (dlg->priv->schemes_treeview_model),
					      0,
					      GTK_SORT_ASCENDING);
	gtk_tree_view_set_model (GTK_TREE_VIEW (dlg->priv->schemes_treeview),
				 GTK_TREE_MODEL (dlg->priv->schemes_treeview_model));

	column = gtk_tree_view_column_new ();

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
						 renderer,
						 scheme_description_cell_data_func,
						 dlg,
						 NULL);

	gtk_tree_view_append_column (GTK_TREE_VIEW (dlg->priv->schemes_treeview),
				     column);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dlg->priv->schemes_treeview));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);

	def_id = populate_color_scheme_list (dlg, NULL);
	
	/* Connect signals */
	g_signal_connect (dlg->priv->schemes_treeview,
			  "cursor-changed",
			  G_CALLBACK (style_scheme_changed),
			  dlg);
	g_signal_connect (dlg->priv->install_scheme_button,
			  "clicked",
			  G_CALLBACK (install_scheme_clicked),
			  dlg);
	g_signal_connect (dlg->priv->uninstall_scheme_button,
			  "clicked",
			  G_CALLBACK (uninstall_scheme_clicked),
			  dlg);
		
	/* Set initial widget sensitivity */
	set_buttons_sensisitivity_according_to_scheme (dlg, def_id);
}

static void
setup_font_colors_page (GeditPreferencesDialog *dlg)
{
	setup_font_colors_page_font_section (dlg);
	setup_font_colors_page_style_scheme_section (dlg);
}

static void
setup_plugins_page (GeditPreferencesDialog *dlg)
{
	GtkWidget *page_content;

	gedit_debug (DEBUG_PREFS);

	page_content = gedit_plugin_manager_new ();
	g_return_if_fail (page_content != NULL);

	gtk_box_pack_start (GTK_BOX (dlg->priv->plugin_manager_place_holder),
			    page_content,
			    TRUE,
			    TRUE,
			    0);

	gtk_widget_show_all (page_content);
}

static void
gedit_preferences_dialog_init (GeditPreferencesDialog *dlg)
{
	GtkWidget *error_widget;
	gboolean ret;
	gchar *file;
	gchar *root_objects[] = {
		"notebook",
		"adjustment1",
		"adjustment2",
		"adjustment3",
		"install_scheme_image",
		NULL
	};

	gedit_debug (DEBUG_PREFS);

	dlg->priv = GEDIT_PREFERENCES_DIALOG_GET_PRIVATE (dlg);

	gtk_dialog_add_buttons (GTK_DIALOG (dlg),
				GTK_STOCK_CLOSE,
				GTK_RESPONSE_CLOSE,
				GTK_STOCK_HELP,
				GTK_RESPONSE_HELP,
				NULL);

	gtk_window_set_title (GTK_WINDOW (dlg), _("gedit Preferences"));
	gtk_window_set_resizable (GTK_WINDOW (dlg), FALSE);
	gtk_dialog_set_has_separator (GTK_DIALOG (dlg), FALSE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (dlg), TRUE);

	/* HIG defaults */
	gtk_container_set_border_width (GTK_CONTAINER (dlg), 5);
	gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dlg))), 2); /* 2 * 5 + 2 = 12 */
	gtk_container_set_border_width (GTK_CONTAINER (gtk_dialog_get_action_area (GTK_DIALOG (dlg))), 5);
	gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_action_area (GTK_DIALOG (dlg))), 6);
	
	g_signal_connect (dlg,
			  "response",
			  G_CALLBACK (dialog_response_handler),
			  NULL);
	
	file = gedit_dirs_get_ui_file ("gedit-preferences-dialog.ui");
	ret = gedit_utils_get_ui_objects (file,
		root_objects,
		&error_widget,

		"notebook", &dlg->priv->notebook,

		"display_line_numbers_checkbutton", &dlg->priv->display_line_numbers_checkbutton,
		"highlight_current_line_checkbutton", &dlg->priv->highlight_current_line_checkbutton,
		"bracket_matching_checkbutton", &dlg->priv->bracket_matching_checkbutton,
		"wrap_text_checkbutton", &dlg->priv->wrap_text_checkbutton,
		"split_checkbutton", &dlg->priv->split_checkbutton,

		"right_margin_checkbutton", &dlg->priv->right_margin_checkbutton,
		"right_margin_position_spinbutton", &dlg->priv->right_margin_position_spinbutton,
		"right_margin_position_hbox", &dlg->priv->right_margin_position_hbox,

		"tabs_width_spinbutton", &dlg->priv->tabs_width_spinbutton,
		"tabs_width_hbox", &dlg->priv->tabs_width_hbox,
		"insert_spaces_checkbutton", &dlg->priv->insert_spaces_checkbutton,

		"auto_indent_checkbutton", &dlg->priv->auto_indent_checkbutton,

		"autosave_hbox", &dlg->priv->autosave_hbox,
		"backup_copy_checkbutton", &dlg->priv->backup_copy_checkbutton,
		"auto_save_checkbutton", &dlg->priv->auto_save_checkbutton,
		"auto_save_spinbutton", &dlg->priv->auto_save_spinbutton,

		"default_font_checkbutton", &dlg->priv->default_font_checkbutton,
		"font_button", &dlg->priv->font_button,
		"font_hbox", &dlg->priv->font_hbox,

		"schemes_treeview", &dlg->priv->schemes_treeview,
		"install_scheme_button", &dlg->priv->install_scheme_button,
		"uninstall_scheme_button", &dlg->priv->uninstall_scheme_button,

		"plugin_manager_place_holder", &dlg->priv->plugin_manager_place_holder,

		NULL);
	g_free (file);

	if (!ret)
	{
		gtk_widget_show (error_widget);
			
		gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dlg))),
		                    error_widget,
		                    TRUE, TRUE, 0);
		
		return;
	}

	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dlg))),
			    dlg->priv->notebook, FALSE, FALSE, 0);
	g_object_unref (dlg->priv->notebook);
	gtk_container_set_border_width (GTK_CONTAINER (dlg->priv->notebook), 5);

	setup_editor_page (dlg);
	setup_view_page (dlg);
	setup_font_colors_page (dlg);
	setup_plugins_page (dlg);
}

void
gedit_show_preferences_dialog (GeditWindow *parent)
{
	gedit_debug (DEBUG_PREFS);

	g_return_if_fail (GEDIT_IS_WINDOW (parent));

	if (preferences_dialog == NULL)
	{
		preferences_dialog = GTK_WIDGET (g_object_new (GEDIT_TYPE_PREFERENCES_DIALOG, NULL));
		g_signal_connect (preferences_dialog,
				  "destroy",
				  G_CALLBACK (gtk_widget_destroyed),
				  &preferences_dialog);
	}

	if (GTK_WINDOW (parent) != gtk_window_get_transient_for (GTK_WINDOW (preferences_dialog)))
	{
		gtk_window_set_transient_for (GTK_WINDOW (preferences_dialog),
					      GTK_WINDOW (parent));
	}

	gtk_window_present (GTK_WINDOW (preferences_dialog));
}
