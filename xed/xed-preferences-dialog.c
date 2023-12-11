/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xed-preferences-dialog.c
 * This file is part of xed
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/*
 * Modified by the xed Team, 2001-2003. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <libpeas-gtk/peas-gtk-plugin-manager.h>

#include "xed-preferences-dialog.h"
#include "xed-utils.h"
#include "xed-debug.h"
#include "xed-document.h"
#include "xed-dirs.h"
#include "xed-settings.h"
#include "xed-utils.h"

/*
 * xed-preferences dialog is a singleton since we don't
 * want two dialogs showing an inconsistent state of the
 * preferences.
 * When xed_show_preferences_dialog is called and there
 * is already a prefs dialog dialog open, it is reparented
 * and shown.
 */

static GtkWidget *preferences_dialog = NULL;

#define XED_TYPE_PREFERENCES_DIALOG (xed_preferences_dialog_get_type ())

G_DECLARE_FINAL_TYPE (XedPreferencesDialog, xed_preferences_dialog, XED, PREFERENCES_DIALOG, XAppPreferencesWindow)

struct _XedPreferencesDialog
{
    XAppPreferencesWindow parent_instance;

    GSettings *editor_settings;
    GSettings *ui_settings;

    /* Main pages */
    GtkWidget *editor_page;
    GtkWidget *save_page;
    GtkWidget *theme_page;
    GtkWidget *plugins_page;

    /* File Saving */
    GtkWidget *backup_copy_switch;
    GtkWidget *auto_save_switch;
    GtkWidget *auto_save_spin;
    GtkWidget *auto_save_revealer;
    GtkWidget *ensure_newline_switch;

    /* Font */
    GtkWidget *fixed_width_font_label;
    GtkWidget *fixed_width_font_switch;
    GtkWidget *font_button_revealer;
    GtkWidget *font_button;

    /* Display */
    GtkWidget *display_line_numbers_switch;

    /* Minimap */
    GtkWidget *minimap_switch;

    /* Right margin */
    GtkWidget *display_right_margin_switch;
    GtkWidget *right_margin_spin;
    GtkWidget *right_margin_revealer;

    /* Draw whitespace */
    GtkWidget *draw_whitespace_switch;
    GtkWidget *draw_whitespace_revealer;
    GtkWidget *draw_whitespace_leading_switch;
    GtkWidget *draw_whitespace_trailing_switch;
    GtkWidget *draw_whitespace_inside_switch;
    GtkWidget *draw_whitespace_newline_switch;

    /* Highlight current line */
    GtkWidget *highlight_current_line_switch;

    /* Highlight matching bracket */
    GtkWidget *highlight_matching_bracket_switch;

    /* Tabs */
    GtkWidget *tab_width_spin;
    GtkWidget *use_spaces_switch;
    GtkWidget *automatic_indentation_switch;

    /* Word wrap */
    GtkWidget *word_wrap_switch;
    GtkWidget *split_words_revealer;
    GtkWidget *split_words_switch;

    /* Tab scrolling */
    GtkWidget *tab_scrolling_switch;

    /* Auto close*/
    GtkWidget *auto_close_switch;

    /* Style scheme */
    GtkWidget *prefer_dark_theme_switch;
    GtkWidget *schemes_list;
    GtkWidget *install_scheme_button;
    GtkWidget *uninstall_scheme_button;
    GtkWidget *install_scheme_file_schooser;

    /* Plugins manager */
     GtkWidget *plugin_manager_place_holder;
};


G_DEFINE_TYPE(XedPreferencesDialog, xed_preferences_dialog, XAPP_TYPE_PREFERENCES_WINDOW)

static void
xed_preferences_dialog_dispose (GObject *object)
{
    XedPreferencesDialog *dlg = XED_PREFERENCES_DIALOG (object);

    g_clear_object (&dlg->editor_settings);
    g_clear_object (&dlg->ui_settings);

    G_OBJECT_CLASS (xed_preferences_dialog_parent_class)->dispose (object);
}

static void
xed_preferences_dialog_class_init (XedPreferencesDialogClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->dispose = xed_preferences_dialog_dispose;

    gtk_widget_class_set_template_from_resource (widget_class, "/org/x/editor/ui/xed-preferences-dialog.ui");

    /* Pages */
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, editor_page);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, save_page);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, theme_page);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, plugins_page);

    /* Editor Page widgets */
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, fixed_width_font_label);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, fixed_width_font_switch);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, font_button_revealer);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, font_button);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, display_line_numbers_switch);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, minimap_switch);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, display_right_margin_switch);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, right_margin_spin);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, right_margin_revealer);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, draw_whitespace_switch);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, draw_whitespace_revealer);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, draw_whitespace_leading_switch);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, draw_whitespace_trailing_switch);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, draw_whitespace_inside_switch);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, draw_whitespace_newline_switch);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, highlight_current_line_switch);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, highlight_matching_bracket_switch);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, tab_width_spin);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, use_spaces_switch);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, automatic_indentation_switch);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, word_wrap_switch);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, split_words_revealer);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, split_words_switch);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, tab_scrolling_switch);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, auto_close_switch);

    /* Save page widgets */
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, backup_copy_switch);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, auto_save_switch);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, auto_save_spin);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, auto_save_revealer);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, ensure_newline_switch);

    /* Theme page widgets */
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, prefer_dark_theme_switch);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, schemes_list);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, install_scheme_button);
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, uninstall_scheme_button);

    /* Plugin page widgets */
    gtk_widget_class_bind_template_child (widget_class, XedPreferencesDialog, plugin_manager_place_holder);
}

static void
close_button_clicked (GtkButton *button,
                      gpointer   data)
{
    XedPreferencesDialog *dlg = XED_PREFERENCES_DIALOG (data);

    xed_debug (DEBUG_PREFS);

    gtk_widget_destroy (GTK_WIDGET (dlg));
}

static void
help_button_clicked (GtkButton *button,
                     gpointer   data)
{
    XedPreferencesDialog *dlg = XED_PREFERENCES_DIALOG (data);

    xed_debug (DEBUG_PREFS);

    xed_app_show_help (XED_APP (g_application_get_default ()), GTK_WINDOW (dlg), NULL, "xed-prefs");
}

static void
word_wrap_switch_toggled (GObject    *toggle_switch,
                          GParamSpec *pspec,
                          gpointer    data)
{
    XedPreferencesDialog *dlg = XED_PREFERENCES_DIALOG (data);
    GtkWrapMode mode;

    if (!gtk_switch_get_active (GTK_SWITCH (dlg->word_wrap_switch)))
    {
        mode = GTK_WRAP_NONE;

        gtk_revealer_set_reveal_child (GTK_REVEALER (dlg->split_words_revealer), FALSE);
    }
    else
    {
        gtk_revealer_set_reveal_child (GTK_REVEALER (dlg->split_words_revealer), TRUE);

        if (gtk_switch_get_active (GTK_SWITCH (dlg->split_words_switch)))
        {
            mode = GTK_WRAP_CHAR;
        }
        else
        {
            mode = GTK_WRAP_WORD;
        }
    }

    g_settings_set_enum (dlg->editor_settings, XED_SETTINGS_WRAP_MODE, mode);
}

static void
setup_editor_page (XedPreferencesDialog *dlg)
{
    GObject *settings;
    gchar *system_font = NULL;
    gchar *label_text;
    GtkWrapMode wrap_mode;

    xed_debug (DEBUG_PREFS);

    /* Fonts */
    settings = _xed_app_get_settings (XED_APP (g_application_get_default ()));
    system_font = xed_settings_get_system_font (XED_SETTINGS (settings));

    label_text = g_strdup_printf(_("Use the system fixed width font (%s)"), system_font);
    gtk_label_set_text (GTK_LABEL (dlg->fixed_width_font_label), label_text);
    g_free (system_font);
    g_free (label_text);

    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_USE_DEFAULT_FONT,
                     dlg->fixed_width_font_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    g_object_bind_property (dlg->fixed_width_font_switch,
                            "active",
                            dlg->font_button_revealer,
                            "reveal-child",
                            G_BINDING_SYNC_CREATE | G_BINDING_DEFAULT | G_BINDING_INVERT_BOOLEAN);

    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_EDITOR_FONT,
                     dlg->font_button,
                     "font-name",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    /* Display */
    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_DISPLAY_RIGHT_MARGIN,
                     dlg->display_right_margin_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    g_object_bind_property (dlg->display_right_margin_switch,
                            "active",
                            dlg->right_margin_revealer,
                            "reveal-child",
                            G_BINDING_SYNC_CREATE | G_BINDING_DEFAULT);

    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_DISPLAY_LINE_NUMBERS,
                     dlg->display_line_numbers_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    g_settings_bind (dlg->ui_settings,
                     XED_SETTINGS_MINIMAP_VISIBLE,
                     dlg->minimap_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_RIGHT_MARGIN_POSITION,
                     dlg->right_margin_spin,
                     "value",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    /* whitespace */

    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_DRAW_WHITESPACE,
                     dlg->draw_whitespace_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    g_object_bind_property (dlg->draw_whitespace_switch,
                            "active",
                            dlg->draw_whitespace_revealer,
                            "reveal-child",
                            G_BINDING_SYNC_CREATE | G_BINDING_DEFAULT);

    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_DRAW_WHITESPACE_LEADING,
                     dlg->draw_whitespace_leading_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_DRAW_WHITESPACE_TRAILING,
                     dlg->draw_whitespace_trailing_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_DRAW_WHITESPACE_INSIDE,
                     dlg->draw_whitespace_inside_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_DRAW_WHITESPACE_NEWLINE,
                     dlg->draw_whitespace_newline_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    /* Highlighting */
    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_HIGHLIGHT_CURRENT_LINE,
                     dlg->highlight_current_line_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_BRACKET_MATCHING,
                     dlg->highlight_matching_bracket_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    /* Indentation */
    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_TABS_SIZE,
                     dlg->tab_width_spin,
                     "value",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_INSERT_SPACES,
                     dlg->use_spaces_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_AUTO_INDENT,
                     dlg->automatic_indentation_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    /* Word wrap */
    wrap_mode = g_settings_get_enum (dlg->editor_settings, XED_SETTINGS_WRAP_MODE);

    switch (wrap_mode)
    {
        case GTK_WRAP_WORD:
            gtk_switch_set_active (GTK_SWITCH (dlg->word_wrap_switch), TRUE);
            gtk_revealer_set_reveal_child (GTK_REVEALER (dlg->split_words_revealer), TRUE);
            gtk_switch_set_active (GTK_SWITCH (dlg->split_words_switch), FALSE);
            break;
        case GTK_WRAP_CHAR:
            gtk_switch_set_active (GTK_SWITCH (dlg->word_wrap_switch), TRUE);
            gtk_revealer_set_reveal_child (GTK_REVEALER (dlg->split_words_revealer), TRUE);
            gtk_switch_set_active (GTK_SWITCH (dlg->split_words_switch), TRUE);
            break;
        default:
            gtk_switch_set_active (GTK_SWITCH (dlg->word_wrap_switch), FALSE);
            gtk_revealer_set_reveal_child (GTK_REVEALER (dlg->split_words_revealer), FALSE);
    }

    g_signal_connect (dlg->word_wrap_switch, "notify::active",
                      G_CALLBACK (word_wrap_switch_toggled), dlg);
    g_signal_connect (dlg->split_words_switch, "notify::active",
                      G_CALLBACK (word_wrap_switch_toggled), dlg);

    /* Tab scrolling */
    g_settings_bind (dlg->ui_settings,
                     XED_SETTINGS_ENABLE_TAB_SCROLLING,
                     dlg->tab_scrolling_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    /* Auto close */
    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_AUTO_CLOSE,
                     dlg->auto_close_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    xapp_preferences_window_add_page (XAPP_PREFERENCES_WINDOW (dlg), dlg->editor_page, "editor", _("Editor"));
}

static void
setup_save_page (XedPreferencesDialog *dlg)
{
    xed_debug (DEBUG_PREFS);

    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_CREATE_BACKUP_COPY,
                     dlg->backup_copy_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_AUTO_SAVE,
                     dlg->auto_save_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    g_object_bind_property (dlg->auto_save_switch,
                            "active",
                            dlg->auto_save_revealer,
                            "reveal-child",
                            G_BINDING_SYNC_CREATE | G_BINDING_DEFAULT);

    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_AUTO_SAVE_INTERVAL,
                     dlg->auto_save_spin,
                     "value",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_ENSURE_TRAILING_NEWLINE,
                     dlg->ensure_newline_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    xapp_preferences_window_add_page (XAPP_PREFERENCES_WINDOW (dlg), dlg->save_page, "save", _("Save"));
}

static GtkSourceStyleScheme *
get_default_color_scheme (XedPreferencesDialog *dlg)
{
    GtkSourceStyleSchemeManager *manager;
    GtkSourceStyleScheme *scheme = NULL;
    gchar *pref_id;

    manager = gtk_source_style_scheme_manager_get_default ();
    pref_id = g_settings_get_string (dlg->editor_settings, XED_SETTINGS_SCHEME);
    scheme = gtk_source_style_scheme_manager_get_scheme (manager, pref_id);

    g_free (pref_id);

    if (scheme == NULL)
    {
        /* Fallback to classic style scheme */
        scheme = gtk_source_style_scheme_manager_get_scheme (manager, "classic");
    }

    return scheme;
}

static void
set_buttons_sensisitivity_according_to_scheme (XedPreferencesDialog *dlg,
                                               GtkSourceStyleScheme *scheme)
{
    gboolean editable = FALSE;

    if (scheme != NULL)
    {
        const gchar *filename;

        filename = gtk_source_style_scheme_get_filename (scheme);
        if (filename != NULL)
        {
            editable = g_str_has_prefix (filename, xed_dirs_get_user_styles_dir ());
        }
    }

    gtk_widget_set_sensitive (dlg->uninstall_scheme_button, editable);
}

static void
style_scheme_changed (GtkSourceStyleSchemeChooser *chooser,
                      GParamSpec                  *pspec,
                      XedPreferencesDialog        *dlg)
{
    GtkSourceStyleScheme *scheme;
    const gchar *id;

    scheme = gtk_source_style_scheme_chooser_get_style_scheme (chooser);
    id = gtk_source_style_scheme_get_id (scheme);

    g_settings_set_string (dlg->editor_settings, XED_SETTINGS_SCHEME, id);
    set_buttons_sensisitivity_according_to_scheme (dlg, scheme);
}

/*
 * file_copy:
 * @name: a pointer to a %NULL-terminated string, that names
 * the file to be copied, in the GLib file name encoding
 * @dest_name: a pointer to a %NULL-terminated string, that is the
 * name for the destination file, in the GLib file name encoding
 * @error: return location for a #GError, or %NULL
 *
 * Copies file @name to @dest_name.
 *
 * If the call was successful, it returns %TRUE. If the call was not
 * successful, it returns %FALSE and sets @error. The error domain
 * is #G_FILE_ERROR. Possible error
 * codes are those in the #GFileError enumeration.
 *
 * Return value: %TRUE on success, %FALSE otherwise.
 */
static gboolean
file_copy (const gchar  *name,
           const gchar  *dest_name,
           GError      **error)
{
    gchar *contents;
    gsize length;
    gchar *dest_dir;

    /* FIXME - Paolo (Aug. 13, 2007):
    * Since the style scheme files are relatively small, we can implement
    * file copy getting all the content of the source file in a buffer and
    * then write the content to the destination file. In this way we
    * can use the g_file_get_contents and g_file_set_contents and avoid to
    * write custom code to copy the file (with sane error management).
    * If needed we can improve this code later. */

    g_return_val_if_fail (name != NULL, FALSE);
    g_return_val_if_fail (dest_name != NULL, FALSE);
    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    /* Note: we allow to copy a file to itself since this is not a problem
    * in our use case */

    /* Ensure the destination directory exists */
    dest_dir = g_path_get_dirname (dest_name);

    errno = 0;
    if (g_mkdir_with_parents (dest_dir, 0755) != 0)
    {
        gint save_errno = errno;
        gchar *display_filename = g_filename_display_name (dest_dir);

        g_set_error (error,
                G_FILE_ERROR,
                g_file_error_from_errno (save_errno),
                _("Directory '%s' could not be created: g_mkdir_with_parents() failed: %s"),
                display_filename,
                g_strerror (save_errno));

        g_free (dest_dir);
        g_free (display_filename);

        return FALSE;
    }

    g_free (dest_dir);

    if (!g_file_get_contents (name, &contents, &length, error))
    {
        return FALSE;
    }

    if (!g_file_set_contents (dest_name, contents, length, error))
    {
        g_free (contents);
        return FALSE;
    }

    g_free (contents);

    return TRUE;
}

/*
 * install_style_scheme:
 * @manager: a #GtkSourceStyleSchemeManager
 * @fname: the file name of the style scheme to be installed
 *
 * Install a new user scheme.
 * This function copies @fname in #XED_STYLES_DIR and ask the style manager to
 * recompute the list of available style schemes. It then checks if a style
 * scheme with the right file name exists.
 *
 * If the call was succesful, it returns the id of the installed scheme
 * otherwise %NULL.
 *
 * Return value: the id of the installed scheme, %NULL otherwise.
 */
static GtkSourceStyleScheme *
install_style_scheme (const gchar *fname)
{
    GtkSourceStyleSchemeManager *manager;
    gchar *new_file_name = NULL;
    gchar *dirname;
    const gchar *styles_dir;
    GError *error = NULL;
    gboolean copied = FALSE;
    const gchar * const *ids;

    g_return_val_if_fail (fname != NULL, NULL);

    manager = gtk_source_style_scheme_manager_get_default ();

    dirname = g_path_get_dirname (fname);
    styles_dir = xed_dirs_get_user_styles_dir ();

    if (strcmp (dirname, styles_dir) != 0)
    {
        gchar *basename;

        basename = g_path_get_basename (fname);
        new_file_name = g_build_filename (styles_dir, basename, NULL);
        g_free (basename);

        /* Copy the style scheme file into XED_STYLES_DIR */
        if (!file_copy (fname, new_file_name, &error))
        {
            g_free (new_file_name);
            g_free (dirname);

            g_message ("Cannot install style scheme:\n%s", error->message);

            g_error_free (error);

           return NULL;
        }

        copied = TRUE;
    }
    else
    {
        new_file_name = g_strdup (fname);
    }

    g_free (dirname);

    /* Reload the available style schemes */
    gtk_source_style_scheme_manager_force_rescan (manager);

    /* Check the new style scheme has been actually installed */
    ids = gtk_source_style_scheme_manager_get_scheme_ids (manager);

    while (*ids != NULL)
    {
        GtkSourceStyleScheme *scheme;
        const gchar *filename;

        scheme = gtk_source_style_scheme_manager_get_scheme (manager, *ids);

        filename = gtk_source_style_scheme_get_filename (scheme);

        if (filename && (strcmp (filename, new_file_name) == 0))
        {
            /* The style scheme has been correctly installed */
            g_free (new_file_name);

            return scheme;
        }
        ++ids;
    }

    /* The style scheme has not been correctly installed */
    if (copied)
    {
        g_unlink (new_file_name);
    }

    g_free (new_file_name);

    return NULL;
}

static void
add_scheme_chooser_response_cb (GtkDialog            *chooser,
                                gint                  res_id,
                                XedPreferencesDialog *dlg)
{
    gchar* filename;
    GtkSourceStyleScheme *scheme;

    if (res_id != GTK_RESPONSE_ACCEPT)
    {
        gtk_widget_hide (GTK_WIDGET (chooser));
        return;
    }

    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
    if (filename == NULL)
    {
        return;
    }

    gtk_widget_hide (GTK_WIDGET (chooser));

    scheme = install_style_scheme (filename);
    g_free (filename);

    if (scheme == NULL)
    {
        xed_warning (GTK_WINDOW (dlg), _("The selected color scheme cannot be installed."));
        return;
    }

    g_settings_set_string (dlg->editor_settings, XED_SETTINGS_SCHEME, gtk_source_style_scheme_get_id (scheme));

    set_buttons_sensisitivity_according_to_scheme (dlg, scheme);
}

static void
install_scheme_clicked (GtkButton            *button,
                        XedPreferencesDialog *dlg)
{
    GtkWidget      *chooser;
    GtkFileFilter  *filter;

    if (dlg->install_scheme_file_schooser != NULL)
    {
        gtk_window_present (GTK_WINDOW (dlg->install_scheme_file_schooser));
        gtk_widget_grab_focus (dlg->install_scheme_file_schooser);
        return;
    }

    chooser = gtk_file_chooser_dialog_new (_("Add Scheme"),
                                           GTK_WINDOW (dlg),
                                           GTK_FILE_CHOOSER_ACTION_OPEN,
                                           _("Cancel"), GTK_RESPONSE_CANCEL,
                                           NULL);

    gtk_dialog_add_button (GTK_DIALOG (chooser), _("Add Scheme"), GTK_RESPONSE_ACCEPT);

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

    g_signal_connect (chooser, "response",
                      G_CALLBACK (add_scheme_chooser_response_cb), dlg);

    dlg->install_scheme_file_schooser = chooser;

    g_object_add_weak_pointer (G_OBJECT (chooser), (gpointer) &dlg->install_scheme_file_schooser);

    gtk_widget_show (chooser);
}

/**
 * uninstall_style_scheme:
 * @manager: a #GtkSourceStyleSchemeManager
 * @scheme: a #GtkSourceStyleScheme
 *
 * Uninstall a user scheme.
 *
 * If the call was succesful, it returns %TRUE
 * otherwise %FALSE.
 *
 * Return value: %TRUE on success, %FALSE otherwise.
 */
static gboolean
uninstall_style_scheme (GtkSourceStyleScheme *scheme)
{
    GtkSourceStyleSchemeManager *manager;
    const gchar *filename;

    g_return_val_if_fail (GTK_SOURCE_IS_STYLE_SCHEME (scheme), FALSE);

    manager = gtk_source_style_scheme_manager_get_default ();

    filename = gtk_source_style_scheme_get_filename (scheme);
    if (filename == NULL)
    {
        return FALSE;
    }

    if (g_unlink (filename) == -1)
    {
        return FALSE;
    }

    /* Reload the available style schemes */
    gtk_source_style_scheme_manager_force_rescan (manager);

    return TRUE;
}

static void
uninstall_scheme_clicked (GtkButton            *button,
                          XedPreferencesDialog *dlg)
{
    GtkSourceStyleScheme *scheme;

    scheme = gtk_source_style_scheme_chooser_get_style_scheme (GTK_SOURCE_STYLE_SCHEME_CHOOSER (dlg->schemes_list));

    if (!uninstall_style_scheme (scheme))
    {
        xed_warning (GTK_WINDOW (dlg), _("Could not remove color scheme \"%s\"."),
                     gtk_source_style_scheme_get_name (scheme));
    }
}

static void
setup_theme_page (XedPreferencesDialog *dlg)
{
    GtkSourceStyleScheme *scheme;

    xed_debug (DEBUG_PREFS);

    /* Prefer dark theme */
    g_settings_bind (dlg->editor_settings,
                     XED_SETTINGS_PREFER_DARK_THEME,
                     dlg->prefer_dark_theme_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    /* Style scheme */
    scheme = get_default_color_scheme (dlg);

    g_signal_connect (dlg->schemes_list, "notify::style-scheme",
                      G_CALLBACK (style_scheme_changed), dlg);
    g_signal_connect (dlg->install_scheme_button, "clicked",
                      G_CALLBACK (install_scheme_clicked), dlg);
    g_signal_connect (dlg->uninstall_scheme_button, "clicked",
                      G_CALLBACK (uninstall_scheme_clicked), dlg);

    gtk_source_style_scheme_chooser_set_style_scheme (GTK_SOURCE_STYLE_SCHEME_CHOOSER (dlg->schemes_list), scheme);
    set_buttons_sensisitivity_according_to_scheme (dlg, scheme);

    xapp_preferences_window_add_page (XAPP_PREFERENCES_WINDOW (dlg), dlg->theme_page, "theme", _("Theme"));
}

static void
setup_plugins_page (XedPreferencesDialog *dlg)
{
    GtkWidget *page_content;

    xed_debug (DEBUG_PREFS);

    page_content = peas_gtk_plugin_manager_new (NULL);
    g_return_if_fail (page_content != NULL);

    gtk_box_pack_start (GTK_BOX (dlg->plugin_manager_place_holder), page_content, TRUE, TRUE, 0);

    gtk_widget_show_all (page_content);

    xapp_preferences_window_add_page (XAPP_PREFERENCES_WINDOW (dlg), dlg->plugins_page, "plugins", _("Plugins"));
}

static void
setup_buttons (XedPreferencesDialog *dlg)
{
    GtkWidget *button;

    button = gtk_button_new_with_label (_("Help"));
    xapp_preferences_window_add_button (XAPP_PREFERENCES_WINDOW (dlg), button, GTK_PACK_START);
    g_signal_connect (button, "clicked",
                      G_CALLBACK (help_button_clicked), dlg);

    button = gtk_button_new_with_label (_("Close"));
    xapp_preferences_window_add_button (XAPP_PREFERENCES_WINDOW (dlg), button, GTK_PACK_END);
    g_signal_connect (button, "clicked",
                      G_CALLBACK (close_button_clicked), dlg);
}

static void
xed_preferences_dialog_init (XedPreferencesDialog *dlg)
{
    xed_debug (DEBUG_PREFS);

    dlg->editor_settings = g_settings_new ("org.x.editor.preferences.editor");
    dlg->ui_settings = g_settings_new ("org.x.editor.preferences.ui");

    gtk_window_set_title (GTK_WINDOW (dlg), _("Xed Preferences"));

    gtk_widget_init_template (GTK_WIDGET (dlg));

    setup_buttons (dlg);

    setup_editor_page (dlg);
    setup_save_page (dlg);
    setup_theme_page (dlg);
    setup_plugins_page (dlg);
    gtk_widget_show_all (GTK_WIDGET (dlg));
}

void
xed_show_preferences_dialog (XedWindow *parent)
{
    xed_debug (DEBUG_PREFS);

    g_return_if_fail (XED_IS_WINDOW (parent));

    if (preferences_dialog == NULL)
    {
        preferences_dialog = GTK_WIDGET (g_object_new (XED_TYPE_PREFERENCES_DIALOG, NULL));
        g_signal_connect (preferences_dialog, "destroy",
                          G_CALLBACK (gtk_widget_destroyed), &preferences_dialog);
    }

    if (GTK_WINDOW (parent) != gtk_window_get_transient_for (GTK_WINDOW (preferences_dialog)))
    {
        gtk_window_set_transient_for (GTK_WINDOW (preferences_dialog), GTK_WINDOW (parent));
    }

    gtk_window_present (GTK_WINDOW (preferences_dialog));
}
