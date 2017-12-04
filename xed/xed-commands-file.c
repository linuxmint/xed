/*
 * xed-commands-file.c
 * This file is part of xed
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi
 * Copyright (C) 2002-2005 Paolo Maggi
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
 * Modified by the xed Team, 1998-2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "xed-debug.h"
#include "xed-document.h"
#include "xed-document-private.h"
#include "xed-commands.h"
#include "xed-window.h"
#include "xed-window-private.h"
#include "xed-statusbar.h"
#include "xed-utils.h"
#include "xed-file-chooser-dialog.h"
#include "xed-close-confirmation-dialog.h"

/* Defined constants */
#define XED_OPEN_DIALOG_KEY         "xed-open-dialog-key"
#define XED_IS_CLOSING_ALL            "xed-is-closing-all"
#define XED_IS_QUITTING             "xed-is-quitting"
#define XED_IS_QUITTING_ALL     "xed-is-quitting-all"

static void tab_state_changed_while_saving (XedTab    *tab,
                                            GParamSpec  *pspec,
                                            XedWindow *window);

void
_xed_cmd_file_new (GtkAction   *action,
                   XedWindow *window)
{
    xed_debug (DEBUG_COMMANDS);

    xed_window_create_tab (window, TRUE);
}

static XedTab *
get_tab_from_file (GList *docs,
                   GFile *file)
{
    XedTab *tab = NULL;

    while (docs != NULL)
    {
        XedDocument *d;
        GtkSourceFile *source_file;
        GFile *l;

        d = XED_DOCUMENT (docs->data);
        source_file = xed_document_get_file (d);

        l = gtk_source_file_get_location (source_file);
        if (l != NULL && g_file_equal (l, file))
        {
            tab = xed_tab_get_from_document (d);
            break;
        }

        docs = g_list_next (docs);
    }

    return tab;
}

static gboolean
is_duplicated_file (GSList *files,
                    GFile *file)
{
    while (files != NULL)
    {
        if (g_file_equal (files->data, file))
        {
            return TRUE;
        }

        files = g_slist_next (files);
    }

    return FALSE;
}

/* File loading */
static GSList *
load_file_list (XedWindow               *window,
                const GSList            *files,
                const GtkSourceEncoding *encoding,
                gint                     line_pos,
                gboolean                 create)
{
    XedTab *tab;
    GSList *loaded_files = NULL; /* Number of files to load */
    gboolean jump_to = TRUE; /* Whether to jump to the new tab */
    GList *win_docs;
    GSList *files_to_load = NULL;
    const GSList *l;
    gint num_loaded_files = 0;

    xed_debug (DEBUG_COMMANDS);

    win_docs = xed_window_get_documents (window);

    /* Remove the uris corresponding to documents already open
     * in "window" and remove duplicates from "uris" list */
    for (l = files; l != NULL; l = l->next)
    {
        if (!is_duplicated_file (files_to_load, l->data))
        {
            tab = get_tab_from_file (win_docs, l->data);
            if (tab != NULL)
            {
                if (l == files)
                {
                    XedDocument *doc;

                    xed_window_set_active_tab (window, tab);
                    jump_to = FALSE;
                    doc = xed_tab_get_document (tab);

                    if (line_pos > 0)
                    {
                        xed_document_goto_line (doc, line_pos - 1);
                        xed_view_scroll_to_cursor (xed_tab_get_view (tab));
                    }
                }

                ++num_loaded_files;
                loaded_files = g_slist_prepend (loaded_files, xed_tab_get_document (tab));
            }
            else
            {
                files_to_load = g_slist_prepend (files_to_load, l->data);
            }
        }
    }

    g_list_free (win_docs);

    if (files_to_load == NULL)
    {
        return g_slist_reverse (loaded_files);
    }

    files_to_load = g_slist_reverse (files_to_load);
    l = files_to_load;

    tab = xed_window_get_active_tab (window);
    if (tab != NULL)
    {
        XedDocument *doc;

        doc = xed_tab_get_document (tab);

        if (xed_document_is_untouched (doc) && (xed_tab_get_state (tab) == XED_TAB_STATE_NORMAL))
        {
            _xed_tab_load (tab, l->data, encoding, line_pos, create);

            /* make sure the view has focus */
            gtk_widget_grab_focus (GTK_WIDGET (xed_tab_get_view (tab)));

            l = g_slist_next (l);
            jump_to = FALSE;

            ++num_loaded_files;
            loaded_files = g_slist_prepend (loaded_files, xed_tab_get_document (tab));
        }
    }

    while (l != NULL)
    {
        g_return_val_if_fail (l->data != NULL, 0);

        tab = xed_window_create_tab_from_location (window, l->data, encoding, line_pos, create, jump_to);

        if (tab != NULL)
        {
            jump_to = FALSE;
            ++num_loaded_files;
            loaded_files = g_slist_prepend (loaded_files, xed_tab_get_document (tab));
        }

        l = g_slist_next (l);
    }

    loaded_files = g_slist_reverse (loaded_files);

    if (num_loaded_files == 1)
    {
        XedDocument *doc;
        gchar *uri_for_display;

        g_return_val_if_fail (tab != NULL, loaded_files);

        doc = xed_tab_get_document (tab);
        uri_for_display = xed_document_get_uri_for_display (doc);

        xed_statusbar_flash_message (XED_STATUSBAR (window->priv->statusbar),
                                     window->priv->generic_message_cid,
                                     _("Loading file '%s'\342\200\246"),
                                     uri_for_display);

        g_free (uri_for_display);
    }
    else
    {
        xed_statusbar_flash_message (XED_STATUSBAR (window->priv->statusbar),
                                     window->priv->generic_message_cid,
                                     ngettext("Loading %d file\342\200\246",
                                     "Loading %d files\342\200\246",
                                     num_loaded_files),
                                     num_loaded_files);
    }

    /* Free uris_to_load. Note that l points to the first element of uris_to_load */
    g_slist_free (files_to_load);

    return loaded_files;
}

/**
 * xed_commands_load_location:
 * @window: a #XedWindow
 * @location: a #GFile to be loaded
 * @encoding: (allow-none): the #GtkSourceEncoding of @location
 * @line_pos: the line column to place the cursor when @location is loaded
 *
 * Loads @location. Ignores non-existing locations
 */
void
xed_commands_load_location (XedWindow               *window,
                            GFile                   *location,
                            const GtkSourceEncoding *encoding,
                            gint                     line_pos)
{
    GSList *locations = NULL;
    gchar *uri;
    GSList *ret;

    g_return_if_fail (XED_IS_WINDOW (window));
    g_return_if_fail (G_IS_FILE (location));
    g_return_if_fail (xed_utils_is_valid_location (location));

    uri = g_file_get_uri (location);
    xed_debug_message (DEBUG_COMMANDS, "Loading URI '%s'", uri);
    g_free (uri);

    locations = g_slist_prepend (locations, location);

    ret = load_file_list (window, locations, encoding, line_pos, FALSE);
    g_slist_free (ret);

    g_slist_free (locations);
}

/**
 * xed_commands_load_locations:
 * @window: a #XedWindow
 * @locations: (element-type Gio.File): the locations to load
 * @encoding: (allow-none): the #GtkSourceEncoding
 * @line_pos: the line position to place the cursor
 *
 * Loads @locataions. Ignore non-existing locations
 *
 * Returns: (element-type Xed.Document) (transfer container): the locations
 * that were loaded.
 */
GSList *
xed_commands_load_locations (XedWindow               *window,
                             const GSList            *locations,
                             const GtkSourceEncoding *encoding,
                             gint                     line_pos)
{
    g_return_val_if_fail (XED_IS_WINDOW (window), 0);
    g_return_val_if_fail ((locations != NULL) && (locations->data != NULL), 0);

    xed_debug (DEBUG_COMMANDS);

    return load_file_list (window, locations, encoding, line_pos, FALSE);
}

/*
 * From the command line we can specify a line position for the
 * first doc. Beside specifying a not existing uri creates a
 * titled document.
 */
GSList *
_xed_cmd_load_files_from_prompt (XedWindow               *window,
                                 GSList                  *files,
                                 const GtkSourceEncoding *encoding,
                                 gint                     line_pos)
{
    xed_debug (DEBUG_COMMANDS);

    return load_file_list (window, files, encoding, line_pos, TRUE);
}

static void
open_dialog_destroyed (XedWindow            *window,
                       XedFileChooserDialog *dialog)
{
    xed_debug (DEBUG_COMMANDS);

    g_object_set_data (G_OBJECT (window), XED_OPEN_DIALOG_KEY, NULL);
}

static void
open_dialog_response_cb (XedFileChooserDialog *dialog,
                         gint                  response_id,
                         XedWindow            *window)
{
    GSList *files;
    const GtkSourceEncoding *encoding;
    GSList *loaded;

    xed_debug (DEBUG_COMMANDS);

    if (response_id != GTK_RESPONSE_OK)
    {
        gtk_widget_destroy (GTK_WIDGET (dialog));

        return;
    }

    files = gtk_file_chooser_get_files (GTK_FILE_CHOOSER (dialog));
    g_return_if_fail (files != NULL);

    encoding = xed_file_chooser_dialog_get_encoding (dialog);

    gtk_widget_destroy (GTK_WIDGET (dialog));

    /* Remember the folder we navigated to */
     _xed_window_set_default_location (window, files->data);

    loaded = xed_commands_load_locations (window, files, encoding, 0);

    g_slist_free (loaded);

    g_slist_foreach (files, (GFunc) g_object_unref, NULL);
    g_slist_free (files);
}

void
_xed_cmd_file_open (GtkAction *action,
                    XedWindow *window)
{
    GtkWidget *open_dialog;
    gpointer data;
    XedDocument *doc;
    GFile *default_path = NULL;

    xed_debug (DEBUG_COMMANDS);

    data = g_object_get_data (G_OBJECT (window), XED_OPEN_DIALOG_KEY);

    if (data != NULL)
    {
        g_return_if_fail (XED_IS_FILE_CHOOSER_DIALOG (data));

        gtk_window_present (GTK_WINDOW (data));

        return;
    }

    /* Translators: "Open Files" is the title of the file chooser window */
    open_dialog = xed_file_chooser_dialog_new (_("Open Files"),
                                               GTK_WINDOW (window),
                                               GTK_FILE_CHOOSER_ACTION_OPEN,
                                               NULL,
                                               _("_Cancel"), GTK_RESPONSE_CANCEL,
                                               _("_Open"), GTK_RESPONSE_OK,
                                               NULL);

    g_object_set_data (G_OBJECT (window), XED_OPEN_DIALOG_KEY, open_dialog);

    g_object_weak_ref (G_OBJECT (open_dialog), (GWeakNotify) open_dialog_destroyed, window);

    /* Set the curret folder uri */
    doc = xed_window_get_active_document (window);
    if (doc != NULL)
    {
        GtkSourceFile *file = xed_document_get_file (doc);
        GFile *location = gtk_source_file_get_location (file);

        if (location != NULL)
        {
            default_path = g_file_get_parent (location);
        }
    }

    if (default_path == NULL)
    {
        default_path = _xed_window_get_default_location (window);
    }

    if (default_path != NULL)
    {
        gchar *uri;

        uri = g_file_get_uri (default_path);
        gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (open_dialog), uri);

        g_free (uri);
        g_object_unref (default_path);
    }

    g_signal_connect (open_dialog, "response", G_CALLBACK (open_dialog_response_cb), window);

    gtk_widget_show (open_dialog);
}

static gboolean
is_read_only (GFile *location)
{
    gboolean ret = TRUE; /* default to read only */
    GFileInfo *info;

    xed_debug (DEBUG_COMMANDS);

    info = g_file_query_info (location,
                              G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE,
                              G_FILE_QUERY_INFO_NONE,
                              NULL,
                              NULL);

    if (info != NULL)
    {
        if (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE))
        {
            ret = !g_file_info_get_attribute_boolean (info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
        }

        g_object_unref (info);
    }

    return ret;
}

/* FIXME: modify this dialog to be similar to the one provided by gtk+ for
 * already existing files - Paolo (Oct. 11, 2005) */
static gboolean
replace_read_only_file (GtkWindow *parent,
                        GFile     *file)
{
    GtkWidget *dialog;
    gint ret;
    gchar *parse_name;
    gchar *name_for_display;

    xed_debug (DEBUG_COMMANDS);

    parse_name = g_file_get_parse_name (file);

    /* Truncate the name so it doesn't get insanely wide. Note that even
     * though the dialog uses wrapped text, if the name doesn't contain
     * white space then the text-wrapping code is too stupid to wrap it.
     */
    name_for_display = xed_utils_str_middle_truncate (parse_name, 50);
    g_free (parse_name);

    dialog = gtk_message_dialog_new (parent,
                                     GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_MESSAGE_QUESTION,
                                     GTK_BUTTONS_NONE,
                                     _("The file \"%s\" is read-only."),
                                     name_for_display);
    g_free (name_for_display);

    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                              _("Do you want to try to replace it "
                                              "with the one you are saving?"));

    gtk_dialog_add_button (GTK_DIALOG (dialog), _("_Cancel"), GTK_RESPONSE_CANCEL);
    gtk_dialog_add_button (GTK_DIALOG (dialog), _("_Replace"), GTK_RESPONSE_YES);

    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);

    gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

    ret = gtk_dialog_run (GTK_DIALOG (dialog));

    gtk_widget_destroy (dialog);

    return (ret == GTK_RESPONSE_YES);
}

static void
tab_save_as_ready_cb (XedTab     *tab,
                      GAsyncResult *result,
                      GTask        *task)
{
    gboolean success = _xed_tab_save_finish (tab, result);
    g_task_return_boolean (task, success);
    g_object_unref (task);
}

static void
save_dialog_response_cb (XedFileChooserDialog *dialog,
                         gint                  response_id,
                         GTask                *task)
{
    XedTab *tab;
    XedWindow *window;
    XedDocument *doc;
    GtkSourceFile *file;
    GFile *location;
    gchar *parse_name;
    GtkSourceNewlineType newline_type;
    const GtkSourceEncoding *encoding;

    xed_debug (DEBUG_COMMANDS);

    tab = g_task_get_source_object (task);
    window = g_task_get_task_data (task);

    if (response_id != GTK_RESPONSE_OK)
    {
        gtk_widget_destroy (GTK_WIDGET (dialog));

        g_task_return_boolean (task, FALSE);
        g_object_unref (task);
        return;
    }

    doc = xed_tab_get_document (tab);
    location = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
    g_return_if_fail (location != NULL);

    encoding = xed_file_chooser_dialog_get_encoding (dialog);
    newline_type = xed_file_chooser_dialog_get_newline_type (dialog);

    gtk_widget_destroy (GTK_WIDGET (dialog));

    parse_name = g_file_get_parse_name (location);

    xed_statusbar_flash_message (XED_STATUSBAR (window->priv->statusbar),
                                 window->priv->generic_message_cid,
                                 _("Saving file '%s'\342\200\246"),
                                 parse_name);

    g_free (parse_name);

        /* let's remember the dir we navigated too, even if the saving fails... */
    _xed_window_set_default_location (window, location);

    _xed_tab_save_as_async (tab,
                            location,
                            encoding,
                            newline_type,
                            g_task_get_cancellable (task),
                            (GAsyncReadyCallback) tab_save_as_ready_cb,
                            task);

    g_object_unref (location);
}

static GtkFileChooserConfirmation
confirm_overwrite_callback (GtkFileChooser *dialog,
                            gpointer        data)
{
    gchar *uri;
    GFile *file;
    GtkFileChooserConfirmation res;

    xed_debug (DEBUG_COMMANDS);

    uri = gtk_file_chooser_get_uri (dialog);
    file = g_file_new_for_uri (uri);
    g_free (uri);

    if (is_read_only (file))
    {
        if (replace_read_only_file (GTK_WINDOW (dialog), file))
        {
            res = GTK_FILE_CHOOSER_CONFIRMATION_ACCEPT_FILENAME;
        }
        else
        {
            res = GTK_FILE_CHOOSER_CONFIRMATION_SELECT_AGAIN;
        }
    }
    else
    {
        /* fall back to the default confirmation dialog */
        res = GTK_FILE_CHOOSER_CONFIRMATION_CONFIRM;
    }

    g_object_unref (file);

    return res;
}

/* Call save_as_tab_finish() in @callback. */
static void
save_as_tab_async (XedTab              *tab,
                   XedWindow           *window,
                   GCancellable        *cancellable,
                   GAsyncReadyCallback  callback,
                   gpointer             user_data)
{
    GTask *task;
    GtkWidget *save_dialog;
    GtkWindowGroup *wg;
    XedDocument *doc;
    GtkSourceFile *file;
    GFile *location;
    const GtkSourceEncoding *encoding;
    GtkSourceNewlineType newline_type;

    g_return_if_fail (XED_IS_TAB (tab));
    g_return_if_fail (XED_IS_WINDOW (window));

    xed_debug (DEBUG_COMMANDS);

    task = g_task_new (tab, cancellable, callback, user_data);
    g_task_set_task_data (task, g_object_ref (window), g_object_unref);

    save_dialog = xed_file_chooser_dialog_new (_("Save As\342\200\246"),
                                               GTK_WINDOW (window),
                                               GTK_FILE_CHOOSER_ACTION_SAVE,
                                               NULL,
                                               _("_Cancel"), GTK_RESPONSE_CANCEL,
                                               _("_Save"), GTK_RESPONSE_OK,
                                               NULL);

    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (save_dialog), TRUE);
    g_signal_connect (save_dialog, "confirm-overwrite", G_CALLBACK (confirm_overwrite_callback), NULL);

    wg = xed_window_get_group (window);

    gtk_window_group_add_window (wg, GTK_WINDOW (save_dialog));

    /* Save As dialog is modal to its main window */
    gtk_window_set_modal (GTK_WINDOW (save_dialog), TRUE);

    /* Set the suggested file name */
    doc = xed_tab_get_document (tab);
    file = xed_document_get_file (doc);
    location = gtk_source_file_get_location (file);

    if (location != NULL)
    {
        gtk_file_chooser_set_file (GTK_FILE_CHOOSER (save_dialog), location, NULL);
    }


    else
    {
        GFile *default_path;
        gchar *docname;

        default_path = _xed_window_get_default_location (window);
        docname = xed_document_get_short_name_for_display (doc);

        if (default_path != NULL)
        {
            gchar *uri;

            uri = g_file_get_uri (default_path);
            gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (save_dialog), uri);

            g_free (uri);
            g_object_unref (default_path);
        }

        gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (save_dialog), docname);

        g_free (docname);
    }

    /* Set suggested encoding and newline type */
    encoding = gtk_source_file_get_encoding (file);

    if (encoding == NULL)
    {
        encoding = gtk_source_encoding_get_utf8 ();
    }

    newline_type = gtk_source_file_get_newline_type (file);

    xed_file_chooser_dialog_set_encoding (XED_FILE_CHOOSER_DIALOG (save_dialog), encoding);

    xed_file_chooser_dialog_set_newline_type (XED_FILE_CHOOSER_DIALOG (save_dialog), newline_type);

    g_signal_connect (save_dialog, "response", G_CALLBACK (save_dialog_response_cb), task);

    gtk_widget_show (save_dialog);
}

static gboolean
save_as_tab_finish (XedTab       *tab,
                    GAsyncResult *result)
{
   g_return_val_if_fail (g_task_is_valid (result, tab), FALSE);

   return g_task_propagate_boolean (G_TASK (result), NULL);
}

static void
save_as_tab_ready_cb (XedTab       *tab,
                      GAsyncResult *result,
                      GTask        *task)
{
    gboolean success = save_as_tab_finish (tab, result);

    g_task_return_boolean (task, success);
    g_object_unref (task);
}

static void
tab_save_ready_cb (XedTab       *tab,
                   GAsyncResult *result,
                   GTask        *task)
{
    gboolean success = _xed_tab_save_finish (tab, result);

    g_task_return_boolean (task, success);
    g_object_unref (task);
}

/**
 * xed_commands_save_document_async:
 * @document: the #XedDocument to save.
 * @window: a #XedWindow.
 * @cancellable: (nullable): optional #GCancellable object, %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the operation
 *   is finished.
 * @user_data: (closure): the data to pass to the @callback function.
 *
 * Asynchronously save the @document. @document must belong to @window. The
 * source object of the async task is @document (which will be the first
 * parameter of the #GAsyncReadyCallback).
 *
 * When the operation is finished, @callback will be called. You can then call
 * xed_commands_save_document_finish() to get the result of the operation.
 */
void
xed_commands_save_document_async (XedDocument         *document,
                                  XedWindow           *window,
                                  GCancellable        *cancellable,
                                  GAsyncReadyCallback  callback,
                                  gpointer             user_data)
{
    GTask *task;
    XedTab *tab;
    gchar *uri_for_display;

    xed_debug (DEBUG_COMMANDS);

    g_return_if_fail (XED_IS_DOCUMENT (document));
    g_return_if_fail (XED_IS_WINDOW (window));
    g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

    task = g_task_new (document, cancellable, callback, user_data);

    tab = xed_tab_get_from_document (document);

    if (xed_document_is_untitled (document) ||
        xed_document_get_readonly (document))
    {
        xed_debug_message (DEBUG_COMMANDS, "Untitled or Readonly");

        save_as_tab_async (tab,
                           window,
                           cancellable,
                           (GAsyncReadyCallback) save_as_tab_ready_cb,
                           task);

        return;
    }

    uri_for_display = xed_document_get_uri_for_display (document);
    xed_statusbar_flash_message (XED_STATUSBAR (window->priv->statusbar),
                                 window->priv->generic_message_cid,
                                 _("Saving file '%s'\342\200\246"),
                                 uri_for_display);

    g_free (uri_for_display);

    _xed_tab_save_async (tab,
                         cancellable,
                         (GAsyncReadyCallback) tab_save_ready_cb,
                         task);
}

/**
 * xed_commands_save_document_finish:
 * @document: a #XedDocument.
 * @result: a #GAsyncResult.
 *
 * Finishes an asynchronous document saving operation started with
 * xed_commands_save_document_async().
 *
 * Note that there is no error parameter because the errors are already handled
 * by xed.
 *
 * Returns: %TRUE if the document has been correctly saved, %FALSE otherwise.
 */
gboolean
xed_commands_save_document_finish (XedDocument  *document,
                                   GAsyncResult *result)
{
    g_return_val_if_fail (g_task_is_valid (result, document), FALSE);

    return g_task_propagate_boolean (G_TASK (result), NULL);
}

static void
save_tab_ready_cb (XedDocument  *doc,
                   GAsyncResult *result,
                   gpointer      user_data)
{
    xed_commands_save_document_finish (doc, result);
}

/* Save tab asynchronously, but without results. */
static void
save_tab (XedTab    *tab,
          XedWindow *window)
{
    XedDocument *doc = xed_tab_get_document (tab);

    xed_commands_save_document_async (doc,
                                      window,
                                      NULL,
                                      (GAsyncReadyCallback) save_tab_ready_cb,
                                      NULL);
}

void
_xed_cmd_file_save (GtkAction *action,
                    XedWindow *window)
{
    XedTab *tab;

    xed_debug (DEBUG_COMMANDS);

    tab = xed_window_get_active_tab (window);
    if (tab != NULL)
    {
        save_tab (tab, window);
    }
}

static void
_xed_cmd_file_save_as_cb (XedTab     *tab,
                          GAsyncResult *result,
                          gpointer      user_data)
{
    save_as_tab_finish (tab, result);
}

void
_xed_cmd_file_save_as (GtkAction *action,
                       XedWindow *window)
{
    XedTab *tab;

    xed_debug (DEBUG_COMMANDS);

    tab = xed_window_get_active_tab (window);
    if (tab != NULL)
    {
        save_as_tab_async (tab,
                           window,
                           NULL,
                           (GAsyncReadyCallback) _xed_cmd_file_save_as_cb,
                           NULL);
    }
}

static void
quit_if_needed (XedWindow *window)
{
    gboolean is_quitting;
    gboolean is_quitting_all;

    is_quitting = GPOINTER_TO_BOOLEAN (g_object_get_data (G_OBJECT (window), XED_IS_QUITTING));

    is_quitting_all = GPOINTER_TO_BOOLEAN (g_object_get_data (G_OBJECT (window), XED_IS_QUITTING_ALL));

    if (is_quitting)
    {
       gtk_widget_destroy (GTK_WIDGET (window));
    }

    if (is_quitting_all)
    {
        GtkApplication *app;

        app = GTK_APPLICATION (g_application_get_default ());

        if (gtk_application_get_windows (app) == NULL)
        {
            g_application_quit (G_APPLICATION (app));
        }
    }
}

static gboolean
really_close_tab (XedTab *tab)
{
    GtkWidget *toplevel;
    XedWindow *window;

    xed_debug (DEBUG_COMMANDS);

    g_return_val_if_fail (xed_tab_get_state (tab) == XED_TAB_STATE_CLOSING, FALSE);

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (tab));
    g_return_val_if_fail (XED_IS_WINDOW (toplevel), FALSE);

    window = XED_WINDOW (toplevel);

    xed_window_close_tab (window, tab);

    if (xed_window_get_active_tab (window) == NULL)
    {
        quit_if_needed (window);
    }

    return FALSE;
}

static void
close_tab (XedTab *tab)
{
    XedDocument *doc;

    doc = xed_tab_get_document (tab);
    g_return_if_fail (doc != NULL);

    /* If the user has modified again the document, do not close the tab. */
    if (_xed_document_needs_saving (doc))
    {
        return;
    }

    /* Close the document only if it has been succesfully saved.
     * Tab state is set to CLOSING (it is a state without exiting
     * transitions) and the tab is closed in an idle handler.
     */
    _xed_tab_mark_for_closing (tab);

    g_idle_add_full (G_PRIORITY_HIGH_IDLE,
                     (GSourceFunc) really_close_tab,
                     tab,
                     NULL);
}

typedef struct _SaveAsData SaveAsData;

struct _SaveAsData
{
    /* Reffed */
    XedWindow *window;

    /* List of reffed GeditTab's */
    GSList *tabs_to_save_as;

    guint close_tabs : 1;
};

static void save_as_documents_list (SaveAsData *data);

static void
save_as_documents_list_cb (XedTab       *tab,
                           GAsyncResult *result,
                           SaveAsData   *data)
{
    gboolean saved = save_as_tab_finish (tab, result);

    if (saved && data->close_tabs)
    {
        close_tab (tab);
    }

    g_return_if_fail (tab == XED_TAB (data->tabs_to_save_as->data));
    g_object_unref (data->tabs_to_save_as->data);
    data->tabs_to_save_as = g_slist_delete_link (data->tabs_to_save_as, data->tabs_to_save_as);

    if (data->tabs_to_save_as != NULL)
    {
        save_as_documents_list (data);
    }
    else
    {
       g_object_unref (data->window);
       g_slice_free (SaveAsData, data);
    }
}

static void
save_as_documents_list (SaveAsData *data)
{
    XedTab *next_tab = XED_TAB (data->tabs_to_save_as->data);

    xed_window_set_active_tab (data->window, next_tab);

    save_as_tab_async (next_tab,
                       data->window,
                       NULL,
                       (GAsyncReadyCallback) save_as_documents_list_cb,
                       data);
}

/*
 * The docs in the list must belong to the same XedWindow.
 */
static void
save_documents_list (XedWindow *window,
                     GList     *docs)
{
    SaveAsData *data = NULL;
    GList *l;

    xed_debug (DEBUG_COMMANDS);

    g_return_if_fail (!(xed_window_get_state (window) & (XED_WINDOW_STATE_PRINTING | XED_WINDOW_STATE_SAVING_SESSION)));

    l = docs;
    while (l != NULL)
    {
        XedDocument *doc;
        XedTab *t;
        XedTabState state;

        g_return_if_fail (XED_IS_DOCUMENT (l->data));

        doc = XED_DOCUMENT (l->data);
        t = xed_tab_get_from_document (doc);
        state = xed_tab_get_state (t);

        g_return_if_fail (state != XED_TAB_STATE_PRINTING);
        g_return_if_fail (state != XED_TAB_STATE_PRINT_PREVIEWING);
        g_return_if_fail (state != XED_TAB_STATE_CLOSING);

        if ((state == XED_TAB_STATE_NORMAL) ||
            (state == XED_TAB_STATE_SHOWING_PRINT_PREVIEW) ||
            (state == XED_TAB_STATE_GENERIC_NOT_EDITABLE))
        {
            /* FIXME: manage the case of local readonly files owned by the
               user is running xed - Paolo (Dec. 8, 2005) */
            if (xed_document_is_untitled (doc) || xed_document_get_readonly (doc))
            {
                if (_xed_document_needs_saving (doc))
                {
                    if (data == NULL)
                    {
                        data = g_slice_new (SaveAsData);
                        data->window = g_object_ref (window);
                        data->tabs_to_save_as = NULL;
                        data->close_tabs = FALSE;
                    }

                    data->tabs_to_save_as = g_slist_prepend (data->tabs_to_save_as, g_object_ref (t));
                }
            }
            else
            {
                save_tab (t, window);
            }
        }
        else
        {
            /* If the state is:
               - XED_TAB_STATE_LOADING: we do not save since we are sure the file is unmodified
               - XED_TAB_STATE_REVERTING: we do not save since the user wants
                 to return back to the version of the file she previously saved
               - XED_TAB_STATE_SAVING: well, we are already saving (no need to save again)
               - XED_TAB_STATE_PRINTING, XED_TAB_STATE_PRINT_PREVIEWING: there is not a
                 real reason for not saving in this case, we do not save to avoid to run
                 two operations using the message area at the same time (may be we can remove
                 this limitation in the future). Note that SaveAll, ClosAll
                 and Quit are unsensitive if the window state is PRINTING.
               - XED_TAB_STATE_GENERIC_ERROR: we do not save since the document contains
                 errors (I don't think this is a very frequent case, we should probably remove
                 this state)
               - XED_TAB_STATE_LOADING_ERROR: there is nothing to save
               - XED_TAB_STATE_REVERTING_ERROR: there is nothing to save and saving the current
                 document will overwrite the copy of the file the user wants to go back to
               - XED_TAB_STATE_SAVING_ERROR: we do not save since we just failed to save, so there is
                 no reason to automatically retry... we wait for user intervention
               - XED_TAB_STATE_CLOSING: this state is invalid in this case
            */

            gchar *uri_for_display;

            uri_for_display = xed_document_get_uri_for_display (doc);
            xed_debug_message (DEBUG_COMMANDS,
                               "File '%s' not saved. State: %d",
                               uri_for_display,
                               state);
            g_free (uri_for_display);
        }

        l = g_list_next (l);
    }

    if (data != NULL)
    {
        data->tabs_to_save_as = g_slist_reverse (data->tabs_to_save_as);
        save_as_documents_list (data);
    }
}

/**
 * xed_commands_save_all_documents:
 * @window: a #XedWindow.
 *
 * Asynchronously save all documents belonging to @window. The result of the
 * operation is not available, so it's difficult to know whether all the
 * documents are correctly saved.
 */
void
xed_commands_save_all_documents (XedWindow *window)
{
    GList *docs;

    g_return_if_fail (XED_IS_WINDOW (window));

    xed_debug (DEBUG_COMMANDS);

    docs = xed_window_get_documents (window);

    save_documents_list (window, docs);

    g_list_free (docs);
}

void
_xed_cmd_file_save_all (GtkAction *action,
                        XedWindow *window)
{
    xed_commands_save_all_documents (window);
}

/**
 * xed_commands_save_document:
 * @window: a #XedWindow.
 * @document: the #XedDocument to save.
 *
 * Asynchronously save @document. @document must belong to @window. If you need
 * the result of the operation, use xed_commands_save_document_async().
 */
void
xed_commands_save_document (XedWindow   *window,
                            XedDocument *document)
{
    XedTab *tab;

    g_return_if_fail (XED_IS_WINDOW (window));
    g_return_if_fail (XED_IS_DOCUMENT (document));

    xed_debug (DEBUG_COMMANDS);

    tab = xed_tab_get_from_document (document);
    save_tab (tab, window);
}

/* File revert */
static void
do_revert (XedWindow *window,
           XedTab    *tab)
{
    XedDocument *doc;
    gchar *docname;

    xed_debug (DEBUG_COMMANDS);

    doc = xed_tab_get_document (tab);
    docname = xed_document_get_short_name_for_display (doc);

    xed_statusbar_flash_message (XED_STATUSBAR (window->priv->statusbar),
                                 window->priv->generic_message_cid,
                                 _("Reverting the document '%s'\342\200\246"),
                                 docname);

    g_free (docname);

    _xed_tab_revert (tab);
}

static void
revert_dialog_response_cb (GtkDialog *dialog,
                           gint       response_id,
                           XedWindow *window)
{
    XedTab *tab;

    xed_debug (DEBUG_COMMANDS);

    /* FIXME: we are relying on the fact that the dialog is
       modal so the active tab can't be changed...
       not very nice - Paolo (Oct 11, 2005) */
    tab = xed_window_get_active_tab (window);
    if (tab == NULL)
    {
        return;
    }

    gtk_widget_destroy (GTK_WIDGET (dialog));

    if (response_id == GTK_RESPONSE_OK)
    {
        do_revert (window, tab);
    }
}

static GtkWidget *
revert_dialog (XedWindow   *window,
               XedDocument *doc)
{
    GtkWidget *dialog;
    gchar *docname;
    gchar *primary_msg;
    gchar *secondary_msg;
    glong seconds;

    xed_debug (DEBUG_COMMANDS);

    docname = xed_document_get_short_name_for_display (doc);
    primary_msg = g_strdup_printf (_("Revert unsaved changes to document '%s'?"), docname);
    g_free (docname);

    seconds = MAX (1, _xed_document_get_seconds_since_last_save_or_load (doc));

    if (seconds < 55)
    {
        secondary_msg = g_strdup_printf (ngettext ("Changes made to the document in the last %ld second "
                                                   "will be permanently lost.",
                                                   "Changes made to the document in the last %ld seconds "
                                                   "will be permanently lost.",
                                                   seconds),
                                                   seconds);
    }
    else if (seconds < 75) /* 55 <= seconds < 75 */
    {
        secondary_msg = g_strdup (_("Changes made to the document in the last minute "
                                  "will be permanently lost."));
    }
    else if (seconds < 110) /* 75 <= seconds < 110 */
    {
        secondary_msg = g_strdup_printf (ngettext ("Changes made to the document in the last minute and "
                                                   "%ld second will be permanently lost.",
                                                   "Changes made to the document in the last minute and "
                                                   "%ld seconds will be permanently lost.",
                                                   seconds - 60 ),
                                                   seconds - 60);
    }
    else if (seconds < 3600)
    {
        secondary_msg = g_strdup_printf (ngettext ("Changes made to the document in the last %ld minute "
                                                   "will be permanently lost.",
                                                   "Changes made to the document in the last %ld minutes "
                                                   "will be permanently lost.",
                                                   seconds / 60),
                                                   seconds / 60);
    }
    else if (seconds < 7200)
    {
        gint minutes;
        seconds -= 3600;

        minutes = seconds / 60;
        if (minutes < 5)
        {
            secondary_msg = g_strdup (_("Changes made to the document in the last hour "
                                      "will be permanently lost."));
        }
        else
        {
            secondary_msg = g_strdup_printf (ngettext ("Changes made to the document in the last hour and "
                                                       "%d minute will be permanently lost.",
                                                       "Changes made to the document in the last hour and "
                                                       "%d minutes will be permanently lost.",
                                                       minutes),
                                                       minutes);
        }
    }
    else
    {
        gint hours;

        hours = seconds / 3600;

        secondary_msg = g_strdup_printf (ngettext ("Changes made to the document in the last %d hour "
                                                   "will be permanently lost.",
                                                   "Changes made to the document in the last %d hours "
                                                   "will be permanently lost.",
                                                   hours),
                                                   hours);
    }

    dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                     GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_MESSAGE_QUESTION,
                                     GTK_BUTTONS_NONE,
                                     "%s", primary_msg);

    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s", secondary_msg);
    g_free (primary_msg);
    g_free (secondary_msg);

    gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

    gtk_dialog_add_button (GTK_DIALOG (dialog), _("_Cancel"), GTK_RESPONSE_CANCEL);
    gtk_dialog_add_button (GTK_DIALOG (dialog), _("_Revert"), GTK_RESPONSE_OK);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);

    return dialog;
}

void
_xed_cmd_file_revert (GtkAction   *action,
                      XedWindow *window)
{
    XedTab *tab;
    XedDocument *doc;
    GtkWidget *dialog;
    GtkWindowGroup *wg;

    xed_debug (DEBUG_COMMANDS);

    tab = xed_window_get_active_tab (window);
    g_return_if_fail (tab != NULL);

    /* If we are already displaying a notification
     * reverting will drop local modifications, do
     * not bug the user further */
    if (xed_tab_get_state (tab) == XED_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION || _xed_tab_get_can_close (tab))
    {
        do_revert (window, tab);
        return;
    }

    doc = xed_tab_get_document (tab);
    g_return_if_fail (doc != NULL);
    g_return_if_fail (!xed_document_is_untitled (doc));

    dialog = revert_dialog (window, doc);

    wg = xed_window_get_group (window);

    gtk_window_group_add_window (wg, GTK_WINDOW (dialog));

    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

    g_signal_connect (dialog, "response", G_CALLBACK (revert_dialog_response_cb), window);

    gtk_widget_show (dialog);
}

static void
tab_state_changed_while_saving (XedTab     *tab,
                                GParamSpec *pspec,
                                XedWindow  *window)
{
    XedTabState ts;

    ts = xed_tab_get_state (tab);

    xed_debug_message (DEBUG_COMMANDS, "State while saving: %d\n", ts);

    /* When the state become NORMAL, it means the saving operation is
       finished */
    if (ts == XED_TAB_STATE_NORMAL)
    {
        g_signal_handlers_disconnect_by_func (tab, G_CALLBACK (tab_state_changed_while_saving), window);

        close_tab (tab);
    }
}

static void
save_and_close (XedTab    *tab,
                XedWindow *window)
{
    xed_debug (DEBUG_COMMANDS);

    /* Trace tab state changes */
    g_signal_connect (tab, "notify::state", G_CALLBACK (tab_state_changed_while_saving), window);

    save_tab (tab, window);
}

static void
save_and_close_all_documents (const GList *docs,
                              XedWindow   *window)
{
    GList  *tabs;
    GList  *l;
    GSList *sl;
    SaveAsData *data = NULL;
    GSList *tabs_to_save_and_close = NULL;
    GList  *tabs_to_close = NULL;

    xed_debug (DEBUG_COMMANDS);

    g_return_if_fail (!(xed_window_get_state (window) & XED_WINDOW_STATE_PRINTING));

    tabs = gtk_container_get_children (GTK_CONTAINER (_xed_window_get_notebook (window)));

    l = tabs;
    while (l != NULL)
    {
        XedTab *t = XED_TAB (l->data);;
        XedTabState state;
        XedDocument *doc;

        state = xed_tab_get_state (t);
        doc = xed_tab_get_document (t);

        /* If the state is: ([*] invalid states)
           - XED_TAB_STATE_NORMAL: close (and if needed save)
           - XED_TAB_STATE_LOADING: close, we are sure the file is unmodified
           - XED_TAB_STATE_REVERTING: since the user wants
             to return back to the version of the file she previously saved, we can close
             without saving (CHECK: are we sure this is the right behavior, suppose the case
             the original file has been deleted)
           - [*] XED_TAB_STATE_SAVING: invalid, ClosAll
             and Quit are unsensitive if the window state is SAVING.
           - [*] XED_TAB_STATE_PRINTING, XED_TAB_STATE_PRINT_PREVIEWING: there is not a
             real reason for not closing in this case, we do not save to avoid to run
             two operations using the message area at the same time (may be we can remove
             this limitation in the future). Note that ClosAll
             and Quit are unsensitive if the window state is PRINTING.
           - XED_TAB_STATE_SHOWING_PRINT_PREVIEW: close (and if needed save)
           - XED_TAB_STATE_LOADING_ERROR: close without saving (if the state is LOADING_ERROR then the
             document is not modified)
           - XED_TAB_STATE_REVERTING_ERROR: we do not close since the document contains errors
           - XED_TAB_STATE_SAVING_ERROR: we do not close since the document contains errors
           - XED_TAB_STATE_GENERIC_ERROR: we do not close since the document contains
             errors (CHECK: we should problably remove this state)
           - [*] XED_TAB_STATE_CLOSING: this state is invalid in this case
        */

        g_return_if_fail (state != XED_TAB_STATE_PRINTING);
        g_return_if_fail (state != XED_TAB_STATE_PRINT_PREVIEWING);
        g_return_if_fail (state != XED_TAB_STATE_CLOSING);
        g_return_if_fail (state != XED_TAB_STATE_SAVING);

        if ((state != XED_TAB_STATE_SAVING_ERROR) &&
            (state != XED_TAB_STATE_GENERIC_ERROR) &&
            (state != XED_TAB_STATE_REVERTING_ERROR))
        {
            if ((g_list_index ((GList *)docs, doc) >= 0) &&
                (state != XED_TAB_STATE_LOADING) &&
                (state != XED_TAB_STATE_LOADING_ERROR) &&
                (state != XED_TAB_STATE_REVERTING)) /* CHECK: is this the right behavior with REVERTING ?*/
            {
                /* The document must be saved before closing */
                g_return_if_fail (_xed_document_needs_saving (doc));

                /* FIXME: manage the case of local readonly files owned by the
                   user is running xed - Paolo (Dec. 8, 2005) */
                if (xed_document_is_untitled (doc) || xed_document_get_readonly (doc))
                {
                    if (data == NULL)
                    {
                        data = g_slice_new (SaveAsData);
                        data->window = g_object_ref (window);
                        data->tabs_to_save_as = NULL;
                        data->close_tabs = TRUE;
                    }

                    data->tabs_to_save_as = g_slist_prepend (data->tabs_to_save_as, g_object_ref (t));
                }
                else
                {
                    tabs_to_save_and_close = g_slist_prepend (tabs_to_save_and_close, t);
                }
            }
            else
            {
                /* The document must be closed without saving */
                tabs_to_close = g_list_prepend (tabs_to_close, t);
            }
        }

        l = g_list_next (l);
    }

    g_list_free (tabs);

    /* Close all tabs to close (in a sync way) */
    xed_window_close_tabs (window, tabs_to_close);
    g_list_free (tabs_to_close);

    /* Save and close all the files in tabs_to_save_and_close */
    sl = tabs_to_save_and_close;
    while (sl != NULL)
    {
        save_and_close (XED_TAB (sl->data), window);
        sl = g_slist_next (sl);
    }
    g_slist_free (tabs_to_save_and_close);

    /* Save As and close all the files in data->tabs_to_save_as. */
    if (data != NULL)
    {
        data->tabs_to_save_as = g_slist_reverse (data->tabs_to_save_as);
        save_as_documents_list (data);
    }
}

static void
save_and_close_document (const GList *docs,
                         XedWindow   *window)
{
    XedTab *tab;

    xed_debug (DEBUG_COMMANDS);

    g_return_if_fail (docs->next == NULL);

    tab = xed_tab_get_from_document (XED_DOCUMENT (docs->data));
    g_return_if_fail (tab != NULL);

    save_and_close (tab, window);
}

static void
close_all_tabs (XedWindow *window)
{
    gboolean is_quitting;

    xed_debug (DEBUG_COMMANDS);

    /* There is no document to save -> close all tabs */
    xed_window_close_all_tabs (window);

    is_quitting = GPOINTER_TO_BOOLEAN (g_object_get_data (G_OBJECT (window), XED_IS_QUITTING));

    if (is_quitting)
    {
        gtk_widget_destroy (GTK_WIDGET (window));
    }

    return;
}

static void
close_document (XedWindow   *window,
                XedDocument *doc)
{
    XedTab *tab;

    xed_debug (DEBUG_COMMANDS);

    tab = xed_tab_get_from_document (doc);
    g_return_if_fail (tab != NULL);

    xed_window_close_tab (window, tab);
}

static void
close_confirmation_dialog_response_handler (XedCloseConfirmationDialog *dlg,
                                            gint                        response_id,
                                            XedWindow                  *window)
{
    GList *selected_documents;
    gboolean is_closing_all;

    xed_debug (DEBUG_COMMANDS);

    is_closing_all = GPOINTER_TO_BOOLEAN (g_object_get_data (G_OBJECT (window), XED_IS_CLOSING_ALL));

    gtk_widget_hide (GTK_WIDGET (dlg));

    switch (response_id)
    {
        case GTK_RESPONSE_YES: /* Save and Close */
            selected_documents = xed_close_confirmation_dialog_get_selected_documents (dlg);
            if (selected_documents == NULL)
            {
                if (is_closing_all)
                {
                    /* There is no document to save -> close all tabs */
                    /* We call gtk_widget_destroy before close_all_tabs
                     * because close_all_tabs could destroy the xed window */
                    gtk_widget_destroy (GTK_WIDGET (dlg));

                    close_all_tabs (window);

                    return;
                }
                else
                    g_return_if_reached ();
            }
            else
            {
                if (is_closing_all)
                {
                    save_and_close_all_documents (selected_documents, window);
                }
                else
                {
                    save_and_close_document (selected_documents, window);
                }
            }

            g_list_free (selected_documents);

            break;

        case GTK_RESPONSE_NO: /* Close without Saving */
            if (is_closing_all)
            {
                /* We call gtk_widget_destroy before close_all_tabs
                 * because close_all_tabs could destroy the xed window */
                gtk_widget_destroy (GTK_WIDGET (dlg));

                close_all_tabs (window);

                return;
            }
            else
            {
                const GList *unsaved_documents;

                unsaved_documents = xed_close_confirmation_dialog_get_unsaved_documents (dlg);
                g_return_if_fail (unsaved_documents->next == NULL);

                close_document (window, XED_DOCUMENT (unsaved_documents->data));
            }

            break;
        default: /* Do not close */

            /* Reset is_quitting flag */
            g_object_set_data (G_OBJECT (window), XED_IS_QUITTING, GBOOLEAN_TO_POINTER (FALSE));

            break;
    }

    gtk_widget_destroy (GTK_WIDGET (dlg));
}

/* Returns TRUE if the tab can be immediately closed */
static gboolean
tab_can_close (XedTab  *tab,
               GtkWindow *window)
{
    XedDocument *doc;

    xed_debug (DEBUG_COMMANDS);

    doc = xed_tab_get_document (tab);

    if (!_xed_tab_get_can_close (tab))
    {
        GtkWidget *dlg;

        xed_window_set_active_tab (XED_WINDOW (window), tab);

        dlg = xed_close_confirmation_dialog_new_single (window, doc, FALSE);

        g_signal_connect (dlg, "response",
                          G_CALLBACK (close_confirmation_dialog_response_handler), window);

        gtk_widget_show (dlg);

        return FALSE;
    }

    return TRUE;
}

/* CHECK: we probably need this one public for plugins...
 * maybe even a _list variant. Or maybe it's better make
 * xed_window_close_tab always run the confirm dialog?
 * we should not allow closing a tab without resetting the
 * XED_IS_CLOSING_ALL flag!
 */
void
_xed_cmd_file_close_tab (XedTab    *tab,
                         XedWindow *window)
{
    xed_debug (DEBUG_COMMANDS);

    g_return_if_fail (GTK_WIDGET (window) == gtk_widget_get_toplevel (GTK_WIDGET (tab)));

    g_object_set_data (G_OBJECT (window), XED_IS_CLOSING_ALL, GBOOLEAN_TO_POINTER (FALSE));
    g_object_set_data (G_OBJECT (window), XED_IS_QUITTING, GBOOLEAN_TO_POINTER (FALSE));
    g_object_set_data (G_OBJECT (window), XED_IS_QUITTING_ALL, GINT_TO_POINTER (FALSE));


    if (tab_can_close (tab, GTK_WINDOW (window)))
    {
        xed_window_close_tab (window, tab);
    }
}

void
_xed_cmd_file_close (GtkAction   *action,
                     XedWindow *window)
{
    XedTab *active_tab;

    xed_debug (DEBUG_COMMANDS);

    active_tab = xed_window_get_active_tab (window);

    if (active_tab == NULL)
    {
        return;
    }

    _xed_cmd_file_close_tab (active_tab, window);
}

/* Close all tabs */
static void
file_close_all (XedWindow *window,
                gboolean   is_quitting)
{
    GList     *unsaved_docs;
    GtkWidget *dlg;

    xed_debug (DEBUG_COMMANDS);

    g_return_if_fail (!(xed_window_get_state (window) &
                        (XED_WINDOW_STATE_SAVING |
                         XED_WINDOW_STATE_PRINTING |
                         XED_WINDOW_STATE_SAVING_SESSION)));

    g_object_set_data (G_OBJECT (window), XED_IS_CLOSING_ALL, GBOOLEAN_TO_POINTER (TRUE));
    g_object_set_data (G_OBJECT (window), XED_IS_QUITTING, GBOOLEAN_TO_POINTER (is_quitting));

    unsaved_docs = xed_window_get_unsaved_documents (window);

    if (unsaved_docs == NULL)
    {
        /* There is no document to save -> close all tabs */
        xed_window_close_all_tabs (window);

        if (is_quitting)
        {
            gtk_widget_destroy (GTK_WIDGET (window));
        }

        return;
    }

    if (unsaved_docs->next == NULL)
    {
        /* There is only one unsaved document */
        XedTab      *tab;
        XedDocument *doc;

        doc = XED_DOCUMENT (unsaved_docs->data);

        tab = xed_tab_get_from_document (doc);
        g_return_if_fail (tab != NULL);

        xed_window_set_active_tab (window, tab);

        dlg = xed_close_confirmation_dialog_new_single (GTK_WINDOW (window), doc, FALSE);
    }
    else
    {
        dlg = xed_close_confirmation_dialog_new (GTK_WINDOW (window), unsaved_docs, FALSE);
    }

    g_list_free (unsaved_docs);

    g_signal_connect (dlg, "response",
                      G_CALLBACK (close_confirmation_dialog_response_handler), window);

    gtk_widget_show (dlg);
}

void
_xed_cmd_file_close_all (GtkAction   *action,
                         XedWindow *window)
{
    xed_debug (DEBUG_COMMANDS);

    g_return_if_fail (!(xed_window_get_state (window) &
                        (XED_WINDOW_STATE_SAVING |
                        XED_WINDOW_STATE_PRINTING |
                        XED_WINDOW_STATE_SAVING_SESSION)));

    file_close_all (window, FALSE);
}

void
_xed_cmd_file_quit (GtkAction *action,
                    XedWindow *window)
{
    xed_debug (DEBUG_COMMANDS);

    g_return_if_fail (!(xed_window_get_state (window) &
                        (XED_WINDOW_STATE_SAVING |
                         XED_WINDOW_STATE_PRINTING |
                         XED_WINDOW_STATE_SAVING_SESSION)));

    file_close_all (window, TRUE);
}
