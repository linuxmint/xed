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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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


enum
{
    ID_COLUMN = 0,
    NAME_COLUMN,
    DESC_COLUMN,
    NUM_COLUMNS
};


#define XED_PREFERENCES_DIALOG_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
                                                   XED_TYPE_PREFERENCES_DIALOG, \
                                                   XedPreferencesDialogPrivate))

struct _XedPreferencesDialogPrivate
{
    GSettings *editor;
    GSettings *ui;

    GtkWidget   *notebook;

    /* Font */
    GtkWidget *default_font_checkbutton;
    GtkWidget *font_button;
    GtkWidget *font_hbox;

    /* Style Scheme */
    GtkWidget *prefer_dark_theme_checkbutton;
    GtkListStore *schemes_treeview_model;
    GtkWidget *schemes_treeview;
    GtkWidget *install_scheme_button;
    GtkWidget *uninstall_scheme_button;

    GtkWidget *install_scheme_file_schooser;

    /* Tabs */
    GtkWidget *tabs_width_spinbutton;
    GtkWidget *insert_spaces_checkbutton;
    GtkWidget *tabs_width_hbox;

    /* Auto indentation */
    GtkWidget *auto_indent_checkbutton;

    /* Text Wrapping */
    GtkWidget *wrap_text_checkbutton;
    GtkWidget *split_checkbutton;

    /* File Saving */
    GtkWidget *backup_copy_checkbutton;
    GtkWidget *auto_save_checkbutton;
    GtkWidget *auto_save_spinbutton;
    GtkWidget *autosave_hbox;

    /* Line numbers */
    GtkWidget *display_line_numbers_checkbutton;

    /* Highlight current line */
    GtkWidget *highlight_current_line_checkbutton;

    /* Highlight matching bracket */
    GtkWidget *bracket_matching_checkbutton;

    /* Right margin */
    GtkWidget *right_margin_checkbutton;
    GtkWidget *right_margin_position_spinbutton;
    GtkWidget *right_margin_position_hbox;

    /* Tab scrolling */
    GtkWidget *tab_scrolling_checkbutton;

    /* Plugins manager */
    GtkWidget *plugin_manager_place_holder;

    /* Style Scheme editor dialog */
    GtkWidget *style_scheme_dialog;
};


G_DEFINE_TYPE(XedPreferencesDialog, xed_preferences_dialog, GTK_TYPE_DIALOG)

static void
xed_preferences_dialog_dispose (GObject *object)
{
    XedPreferencesDialog *dlg = XED_PREFERENCES_DIALOG (object);

    g_clear_object (&dlg->priv->editor);
    g_clear_object (&dlg->priv->ui);

    G_OBJECT_CLASS (xed_preferences_dialog_parent_class)->dispose (object);
}

static void
xed_preferences_dialog_class_init (XedPreferencesDialogClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = xed_preferences_dialog_dispose;

    g_type_class_add_private (object_class, sizeof (XedPreferencesDialogPrivate));
}

static void
dialog_response_handler (GtkDialog *dlg,
                         gint       res_id)
{
    xed_debug (DEBUG_PREFS);

    switch (res_id)
    {
        case GTK_RESPONSE_HELP:
            xed_app_show_help (XED_APP (g_application_get_default ()), GTK_WINDOW (dlg), NULL, "xed-prefs");
            g_signal_stop_emission_by_name (dlg, "response");
            break;
        default:
            gtk_widget_destroy (GTK_WIDGET (dlg));
    }
}

static void
on_auto_save_changed (GSettings            *settings,
                      const gchar          *key,
                      XedPreferencesDialog *dlg)
{
    gboolean value;

    xed_debug (DEBUG_PREFS);

    value = g_settings_get_boolean (settings, key);

    gtk_widget_set_sensitive (dlg->priv->auto_save_spinbutton, value);
}

static void
setup_editor_page (XedPreferencesDialog *dlg)
{
    gboolean auto_save;

    xed_debug (DEBUG_PREFS);

    /* Get values */
    auto_save = g_settings_get_boolean (dlg->priv->editor, XED_SETTINGS_AUTO_SAVE);

    /* Set widget sensitivity */
    gtk_widget_set_sensitive (dlg->priv->auto_save_spinbutton, auto_save);

    /* Connect signal */
    g_settings_bind (dlg->priv->editor,
                     XED_SETTINGS_TABS_SIZE,
                     dlg->priv->tabs_width_spinbutton,
                     "value",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);
    g_settings_bind (dlg->priv->editor,
                     XED_SETTINGS_INSERT_SPACES,
                     dlg->priv->insert_spaces_checkbutton,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);
    g_settings_bind (dlg->priv->editor,
                     XED_SETTINGS_AUTO_INDENT,
                     dlg->priv->auto_indent_checkbutton,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);
    g_settings_bind (dlg->priv->editor,
                     XED_SETTINGS_CREATE_BACKUP_COPY,
                     dlg->priv->backup_copy_checkbutton,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);
    g_settings_bind (dlg->priv->editor,
                     XED_SETTINGS_BRACKET_MATCHING,
                     dlg->priv->bracket_matching_checkbutton,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);
    g_settings_bind (dlg->priv->editor,
                     XED_SETTINGS_AUTO_SAVE_INTERVAL,
                     dlg->priv->auto_save_spinbutton,
                     "value",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);
    g_signal_connect (dlg->priv->editor, "changed::auto_save",
                      G_CALLBACK (on_auto_save_changed), dlg);
    g_settings_bind (dlg->priv->editor,
                     XED_SETTINGS_AUTO_SAVE,
                     dlg->priv->auto_save_checkbutton,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);
    g_settings_bind (dlg->priv->ui,
                     XED_SETTINGS_ENABLE_TAB_SCROLLING,
                     dlg->priv->tab_scrolling_checkbutton,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);
}

static gboolean split_button_state = TRUE;

static void
wrap_mode_checkbutton_toggled (GtkToggleButton      *button,
                               XedPreferencesDialog *dlg)
{
    GtkWrapMode mode;

    if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dlg->priv->wrap_text_checkbutton)))
    {
        mode = GTK_WRAP_NONE;

        gtk_widget_set_sensitive (dlg->priv->split_checkbutton, FALSE);
        gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON (dlg->priv->split_checkbutton), TRUE);
    }
    else
    {
        gtk_widget_set_sensitive (dlg->priv->split_checkbutton, TRUE);
        gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON (dlg->priv->split_checkbutton), FALSE);


        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dlg->priv->split_checkbutton)))
        {
            split_button_state = TRUE;
            mode = GTK_WRAP_WORD;
        }
        else
        {
            split_button_state = FALSE;
            mode = GTK_WRAP_CHAR;
        }
    }

    g_settings_set_enum (dlg->priv->editor, XED_SETTINGS_WRAP_MODE, mode);
}

static void
right_margin_checkbutton_toggled (GtkToggleButton      *button,
                                  XedPreferencesDialog *dlg)
{
    gboolean active;

    g_return_if_fail (button == GTK_TOGGLE_BUTTON (dlg->priv->right_margin_checkbutton));

    active = gtk_toggle_button_get_active (button);

    g_settings_set_boolean (dlg->priv->editor, XED_SETTINGS_DISPLAY_RIGHT_MARGIN, active);

    gtk_widget_set_sensitive (dlg->priv->right_margin_position_hbox, active);
}

static void
setup_view_page (XedPreferencesDialog *dlg)
{
    GtkWrapMode wrap_mode;
    gboolean display_right_margin;

    xed_debug (DEBUG_PREFS);

    /* Get values */
    display_right_margin = g_settings_get_boolean (dlg->priv->editor, XED_SETTINGS_DISPLAY_RIGHT_MARGIN);

    /* Set initial state */
    wrap_mode = g_settings_get_enum (dlg->priv->editor, XED_SETTINGS_WRAP_MODE);

    switch (wrap_mode)
    {
        case GTK_WRAP_WORD:
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->wrap_text_checkbutton), TRUE);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->split_checkbutton), TRUE);
            break;
        case GTK_WRAP_CHAR:
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->wrap_text_checkbutton), TRUE);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->split_checkbutton), FALSE);
            break;
        default:
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->wrap_text_checkbutton), FALSE);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->split_checkbutton), split_button_state);
            gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON (dlg->priv->split_checkbutton), TRUE);

    }

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->right_margin_checkbutton), display_right_margin);

    /* Set widgets sensitivity */
    gtk_widget_set_sensitive (dlg->priv->split_checkbutton, (wrap_mode != GTK_WRAP_NONE));
    gtk_widget_set_sensitive (dlg->priv->right_margin_position_hbox, display_right_margin);

    /* Connect signals */
    g_settings_bind (dlg->priv->editor,
                     XED_SETTINGS_DISPLAY_LINE_NUMBERS,
                     dlg->priv->display_line_numbers_checkbutton,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);
    g_settings_bind (dlg->priv->editor,
                     XED_SETTINGS_HIGHLIGHT_CURRENT_LINE,
                     dlg->priv->highlight_current_line_checkbutton,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);
    g_settings_bind (dlg->priv->editor,
                     XED_SETTINGS_RIGHT_MARGIN_POSITION,
                     dlg->priv->right_margin_position_spinbutton,
                     "value",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);
    g_signal_connect (dlg->priv->wrap_text_checkbutton, "toggled",
                      G_CALLBACK (wrap_mode_checkbutton_toggled), dlg);
    g_signal_connect (dlg->priv->split_checkbutton, "toggled",
                      G_CALLBACK (wrap_mode_checkbutton_toggled), dlg);
    g_signal_connect (dlg->priv->right_margin_checkbutton, "toggled",
                      G_CALLBACK (right_margin_checkbutton_toggled), dlg);
}

static void
on_use_default_font_changed (GSettings            *settings,
                             const gchar          *key,
                             XedPreferencesDialog *dlg)
{
    gboolean value;

    xed_debug (DEBUG_PREFS);

    value = g_settings_get_boolean (settings, key);
    gtk_widget_set_sensitive (dlg->priv->font_hbox, !value);
}

static void
setup_font_colors_page_font_section (XedPreferencesDialog *dlg)
{
    GObject *settings;
    gboolean use_default_font;
    gchar *system_font = NULL;
    gchar *label;

    xed_debug (DEBUG_PREFS);

    gtk_widget_set_tooltip_text (dlg->priv->font_button,
                                 _("Click on this button to select the font to be used by the editor"));

    xed_utils_set_atk_relation (dlg->priv->font_button,
                                dlg->priv->default_font_checkbutton,
                                ATK_RELATION_CONTROLLED_BY);
    xed_utils_set_atk_relation (dlg->priv->default_font_checkbutton,
                                dlg->priv->font_button,
                                ATK_RELATION_CONTROLLER_FOR);

    /* Get values */
    settings = _xed_app_get_settings (XED_APP (g_application_get_default ()));
    system_font = xed_settings_get_system_font (XED_SETTINGS (settings));
    use_default_font = g_settings_get_boolean (dlg->priv->editor, XED_SETTINGS_USE_DEFAULT_FONT);

    label = g_strdup_printf(_("_Use the system fixed width font (%s)"), system_font);
    gtk_button_set_label (GTK_BUTTON (dlg->priv->default_font_checkbutton), label);
    g_free (system_font);
    g_free (label);

    /* read current config and setup initial state */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->default_font_checkbutton), use_default_font);

    /* Connect signals */
    g_signal_connect (dlg->priv->editor, "changed::use-default-font",
                      G_CALLBACK (on_use_default_font_changed), dlg);
    g_settings_bind (dlg->priv->editor,
                     XED_SETTINGS_USE_DEFAULT_FONT,
                     dlg->priv->default_font_checkbutton,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);
    g_settings_bind (dlg->priv->editor,
                     XED_SETTINGS_EDITOR_FONT,
                     dlg->priv->font_button,
                     "font-name",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    /* Set initial widget sensitivity */
    gtk_widget_set_sensitive (dlg->priv->font_hbox, !use_default_font);
}

static gboolean
is_xed_user_style_scheme (const gchar *scheme_id)
{
    GtkSourceStyleSchemeManager *manager;
    GtkSourceStyleScheme *scheme;
    gboolean res = FALSE;

    manager = gtk_source_style_scheme_manager_get_default ();
    scheme = gtk_source_style_scheme_manager_get_scheme (manager, scheme_id);
    if (scheme != NULL)
    {
        const gchar *filename;

        filename = gtk_source_style_scheme_get_filename (scheme);
        if (filename != NULL)
        {
           res = g_str_has_prefix (filename, xed_dirs_get_user_styles_dir ());
        }
    }

   return res;
}

static void
set_buttons_sensisitivity_according_to_scheme (XedPreferencesDialog *dlg,
                                               const gchar          *scheme_id)
{
    gboolean editable;

    editable = ((scheme_id != NULL) && is_xed_user_style_scheme (scheme_id));

    gtk_widget_set_sensitive (dlg->priv->uninstall_scheme_button, editable);
}

static void
style_scheme_changed (GtkWidget            *treeview,
                      XedPreferencesDialog *dlg)
{
    GtkTreePath *path;
    GtkTreeIter iter;
    gchar *id;

    gtk_tree_view_get_cursor (GTK_TREE_VIEW (dlg->priv->schemes_treeview), &path, NULL);
    gtk_tree_model_get_iter (GTK_TREE_MODEL (dlg->priv->schemes_treeview_model), &iter, path);
    gtk_tree_path_free (path);
    gtk_tree_model_get (GTK_TREE_MODEL (dlg->priv->schemes_treeview_model), &iter, ID_COLUMN, &id, -1);

    g_settings_set_string (dlg->priv->editor, XED_SETTINGS_SCHEME, id);

    set_buttons_sensisitivity_according_to_scheme (dlg, id);

    g_free (id);
}

static const gchar *
ensure_color_scheme_id (XedPreferencesDialog *dlg,
                        const gchar          *id)
{
    GtkSourceStyleSchemeManager *manager;
    GtkSourceStyleScheme *scheme = NULL;

    manager = gtk_source_style_scheme_manager_get_default ();
    if (id == NULL)
    {
        gchar *pref_id;

        pref_id = g_settings_get_string (dlg->priv->editor, XED_SETTINGS_SCHEME);
        scheme = gtk_source_style_scheme_manager_get_scheme (manager, pref_id);
        g_free (pref_id);
    }
    else
    {
        scheme = gtk_source_style_scheme_manager_get_scheme (manager, id);
    }

    if (scheme == NULL)
    {
        /* Fall-back to classic style scheme */
        scheme = gtk_source_style_scheme_manager_get_scheme (manager, "classic");
    }

    if (scheme == NULL)
    {
        /* Cannot determine default style scheme -> broken GtkSourceView installation */
        return NULL;
    }

    return  gtk_source_style_scheme_get_id (scheme);
}

static const gchar *
populate_color_scheme_list (XedPreferencesDialog *dlg,
                            const gchar          *def_id)
{
    GtkSourceStyleSchemeManager *manager;
    const gchar * const *ids;
    gint i;

    gtk_list_store_clear (dlg->priv->schemes_treeview_model);

    def_id = ensure_color_scheme_id (dlg, def_id);
    if (def_id == NULL)
    {
        g_warning ("Cannot build the list of available color schemes.\n"
                   "Please check your GtkSourceView installation.");
        return NULL;
    }

    manager = gtk_source_style_scheme_manager_get_default ();
    ids = gtk_source_style_scheme_manager_get_scheme_ids (manager);
    for (i = 0; ids[i] != NULL; i++)
    {
        GtkSourceStyleScheme *scheme;
        const gchar *name;
        const gchar *description;
        GtkTreeIter iter;

        scheme = gtk_source_style_scheme_manager_get_scheme (manager, ids[i]);
        name = gtk_source_style_scheme_get_name (scheme);
        description = gtk_source_style_scheme_get_description (scheme);

        gtk_list_store_append (dlg->priv->schemes_treeview_model, &iter);
        gtk_list_store_set (dlg->priv->schemes_treeview_model,
                            &iter,
                            ID_COLUMN, ids[i],
                            NAME_COLUMN, name,
                            DESC_COLUMN, description,
                            -1);

        g_return_val_if_fail (def_id != NULL, NULL);
        if (strcmp (ids[i], def_id) == 0)
        {
            GtkTreeSelection *selection;

            selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dlg->priv->schemes_treeview));
            gtk_tree_selection_select_iter (selection, &iter);
        }
    }

    return def_id;
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
static const gchar *
install_style_scheme (const gchar *fname)
{
    GtkSourceStyleSchemeManager *manager;
    gchar *new_file_name = NULL;
    gchar *dirname;
    const gchar *styles_dir;
    GError *error = NULL;
    gboolean copied = FALSE;
    const gchar* const *ids;

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

        /* Copy the style scheme file into GEDIT_STYLES_DIR */
        if (!file_copy (fname, new_file_name, &error))
        {
            g_free (new_file_name);

            g_message ("Cannot install style scheme:\n%s", error->message);

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

            return gtk_source_style_scheme_get_id (scheme);
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

/**
 * uninstall_style_scheme:
 * @manager: a #GtkSourceStyleSchemeManager
 * @id: the id of the style scheme to be uninstalled
 *
 * Uninstall a user scheme.
 *
 * If the call was succesful, it returns %TRUE
 * otherwise %FALSE.
 *
 * Return value: %TRUE on success, %FALSE otherwise.
 */
static gboolean
uninstall_style_scheme (const gchar *id)
{
    GtkSourceStyleSchemeManager *manager;
    GtkSourceStyleScheme *scheme;
    const gchar *filename;

    g_return_val_if_fail (id != NULL, FALSE);

    manager = gtk_source_style_scheme_manager_get_default ();

    scheme = gtk_source_style_scheme_manager_get_scheme (manager, id);
    if (scheme == NULL)
    {
       return FALSE;
    }

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
add_scheme_chooser_response_cb (GtkDialog            *chooser,
                                gint                  res_id,
                                XedPreferencesDialog *dlg)
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
    {
        return;
    }

    gtk_widget_hide (GTK_WIDGET (chooser));

    scheme_id = install_style_scheme (filename);
    g_free (filename);

    if (scheme_id == NULL)
    {
        xed_warning (GTK_WINDOW (dlg), _("The selected color scheme cannot be installed."));
        return;
    }

    g_settings_set_string (dlg->priv->editor, XED_SETTINGS_SCHEME, scheme_id);

    scheme_id = populate_color_scheme_list (dlg, scheme_id);

    set_buttons_sensisitivity_according_to_scheme (dlg, scheme_id);
}

static void
install_scheme_clicked (GtkButton            *button,
                        XedPreferencesDialog *dlg)
{
    GtkWidget      *chooser;
    GtkFileFilter  *filter;

    if (dlg->priv->install_scheme_file_schooser != NULL)
    {
        gtk_window_present (GTK_WINDOW (dlg->priv->install_scheme_file_schooser));
        gtk_widget_grab_focus (dlg->priv->install_scheme_file_schooser);
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

    dlg->priv->install_scheme_file_schooser = chooser;

    g_object_add_weak_pointer (G_OBJECT (chooser), (gpointer) &dlg->priv->install_scheme_file_schooser);

    gtk_widget_show (chooser);
}

static void
uninstall_scheme_clicked (GtkButton            *button,
                          XedPreferencesDialog *dlg)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dlg->priv->schemes_treeview));
    model = GTK_TREE_MODEL (dlg->priv->schemes_treeview_model);

    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        gchar *id;
        gchar *name;

        gtk_tree_model_get (model, &iter, ID_COLUMN, &id, NAME_COLUMN, &name, -1);

        if (!uninstall_style_scheme (id))
        {
            xed_warning (GTK_WINDOW (dlg), _("Could not remove color scheme \"%s\"."), name);
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
                {
                    new_iter_set = TRUE;
                }
            }
            else
            {
                new_iter_set = TRUE;
            }

            gtk_tree_path_free (path);

            if (new_iter_set)
            {
                gtk_tree_model_get (model, &new_iter, ID_COLUMN, &new_id, -1);
            }

            real_new_id = populate_color_scheme_list (dlg, new_id);
            g_free (new_id);

            set_buttons_sensisitivity_according_to_scheme (dlg, real_new_id);

            if (real_new_id != NULL)
            {
                g_settings_set_string (dlg->priv->editor, XED_SETTINGS_SCHEME, real_new_id);
            }
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

    gtk_tree_model_get (model, iter, NAME_COLUMN, &name, DESC_COLUMN, &desc, -1);

    if (desc != NULL)
    {
        text = g_markup_printf_escaped ("<b>%s</b> - %s", name, desc);
    }
    else
    {
        text = g_markup_printf_escaped ("<b>%s</b>", name);
    }

    g_free (name);
    g_free (desc);

    g_object_set (G_OBJECT (renderer), "markup", text, NULL);

    g_free (text);
}

static void
setup_font_colors_page_style_scheme_section (XedPreferencesDialog *dlg)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreeSelection *selection;
    const gchar *def_id;

    xed_debug (DEBUG_PREFS);

    /* Create GtkListStore for styles & setup treeview. */
    dlg->priv->schemes_treeview_model = gtk_list_store_new (NUM_COLUMNS,
                                                            G_TYPE_STRING,
                                                            G_TYPE_STRING,
                                                            G_TYPE_STRING,
                                                            G_TYPE_STRING);

    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (dlg->priv->schemes_treeview_model),
                                          0, GTK_SORT_ASCENDING);
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

    gtk_tree_view_append_column (GTK_TREE_VIEW (dlg->priv->schemes_treeview), column);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dlg->priv->schemes_treeview));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);

    def_id = populate_color_scheme_list (dlg, NULL);

    /* Connect signals */
    g_signal_connect (dlg->priv->schemes_treeview, "cursor-changed",
                      G_CALLBACK (style_scheme_changed), dlg);
    g_signal_connect (dlg->priv->install_scheme_button, "clicked",
                      G_CALLBACK (install_scheme_clicked), dlg);
    g_signal_connect (dlg->priv->uninstall_scheme_button, "clicked",
                      G_CALLBACK (uninstall_scheme_clicked), dlg);

    /* Set initial widget sensitivity */
    set_buttons_sensisitivity_according_to_scheme (dlg, def_id);
}

static void
setup_font_colors_page (XedPreferencesDialog *dlg)
{
    setup_font_colors_page_font_section (dlg);
    setup_font_colors_page_style_scheme_section (dlg);

    g_settings_bind (dlg->priv->editor,
                     XED_SETTINGS_PREFER_DARK_THEME,
                     dlg->priv->prefer_dark_theme_checkbutton,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);
}

static void
setup_plugins_page (XedPreferencesDialog *dlg)
{
    GtkWidget *page_content;

    xed_debug (DEBUG_PREFS);

    page_content = peas_gtk_plugin_manager_new (NULL);
    g_return_if_fail (page_content != NULL);

    gtk_box_pack_start (GTK_BOX (dlg->priv->plugin_manager_place_holder), page_content, TRUE, TRUE, 0);

    gtk_widget_show_all (page_content);
}

static void
xed_preferences_dialog_init (XedPreferencesDialog *dlg)
{
    GtkBuilder *builder;
    gchar *root_objects[] = {
        "notebook",
        "adjustment1",
        "adjustment2",
        "adjustment3",
        NULL
    };

    xed_debug (DEBUG_PREFS);

    dlg->priv = XED_PREFERENCES_DIALOG_GET_PRIVATE (dlg);
    dlg->priv->editor = g_settings_new ("org.x.editor.preferences.editor");
    dlg->priv->ui = g_settings_new ("org.x.editor.preferences.ui");

    gtk_dialog_add_buttons (GTK_DIALOG (dlg),
                            _("Close"), GTK_RESPONSE_CLOSE,
                            _("Help"), GTK_RESPONSE_HELP,
                            NULL);

    gtk_window_set_title (GTK_WINDOW (dlg), _("Xed Preferences"));
    gtk_window_set_resizable (GTK_WINDOW (dlg), FALSE);
    gtk_window_set_destroy_with_parent (GTK_WINDOW (dlg), TRUE);

    /* HIG defaults */
    gtk_container_set_border_width (GTK_CONTAINER (dlg), 5);
    gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dlg))), 2); /* 2 * 5 + 2 = 12 */
    gtk_container_set_border_width (GTK_CONTAINER (gtk_dialog_get_action_area (GTK_DIALOG (dlg))), 5);
    gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_action_area (GTK_DIALOG (dlg))), 6);

    g_signal_connect (dlg, "response",
                      G_CALLBACK (dialog_response_handler), NULL);

    builder = gtk_builder_new ();
    gtk_builder_add_objects_from_resource (builder, "/org/x/editor/ui/xed-preferences-dialog.ui", root_objects, NULL);
    dlg->priv->notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    g_object_ref (dlg->priv->notebook);
    dlg->priv->display_line_numbers_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "display_line_numbers_checkbutton"));
    dlg->priv->highlight_current_line_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "highlight_current_line_checkbutton"));
    dlg->priv->bracket_matching_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "bracket_matching_checkbutton"));
    dlg->priv->wrap_text_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "wrap_text_checkbutton"));
    dlg->priv->split_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "split_checkbutton"));
    dlg->priv->right_margin_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "right_margin_checkbutton"));
    dlg->priv->right_margin_position_spinbutton = GTK_WIDGET (gtk_builder_get_object (builder, "right_margin_position_spinbutton"));
    dlg->priv->right_margin_position_hbox = GTK_WIDGET (gtk_builder_get_object (builder, "right_margin_position_hbox"));
    dlg->priv->tabs_width_spinbutton = GTK_WIDGET (gtk_builder_get_object (builder, "tabs_width_spinbutton"));
    dlg->priv->tabs_width_hbox = GTK_WIDGET (gtk_builder_get_object (builder, "tabs_width_hbox"));
    dlg->priv->insert_spaces_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "insert_spaces_checkbutton"));
    dlg->priv->auto_indent_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "auto_indent_checkbutton"));
    dlg->priv->autosave_hbox = GTK_WIDGET (gtk_builder_get_object (builder, "autosave_hbox"));
    dlg->priv->backup_copy_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "backup_copy_checkbutton"));
    dlg->priv->auto_save_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "auto_save_checkbutton"));
    dlg->priv->auto_save_spinbutton = GTK_WIDGET (gtk_builder_get_object (builder, "auto_save_spinbutton"));
    dlg->priv->tab_scrolling_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "tab_scrolling_checkbutton"));
    dlg->priv->default_font_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "default_font_checkbutton"));
    dlg->priv->font_button = GTK_WIDGET (gtk_builder_get_object (builder, "font_button"));
    dlg->priv->font_hbox = GTK_WIDGET (gtk_builder_get_object (builder, "font_hbox"));
    dlg->priv->prefer_dark_theme_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "prefer_dark_theme_checkbutton"));
    dlg->priv->schemes_treeview = GTK_WIDGET (gtk_builder_get_object (builder, "schemes_treeview"));
    dlg->priv->install_scheme_button = GTK_WIDGET (gtk_builder_get_object (builder, "install_scheme_button"));
    dlg->priv->uninstall_scheme_button = GTK_WIDGET (gtk_builder_get_object (builder, "uninstall_scheme_button"));
    dlg->priv->plugin_manager_place_holder = GTK_WIDGET (gtk_builder_get_object (builder, "plugin_manager_place_holder"));
    g_object_unref (builder);

    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dlg))), dlg->priv->notebook, FALSE, FALSE, 0);
    g_object_unref (dlg->priv->notebook);
    gtk_container_set_border_width (GTK_CONTAINER (dlg->priv->notebook), 5);

    setup_editor_page (dlg);
    setup_view_page (dlg);
    setup_font_colors_page (dlg);
    setup_plugins_page (dlg);
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
