/*
 * xed-io-error-info-bar.c
 * This file is part of xed
 *
 * Copyright (C) 2005 - Paolo Maggi
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
 * Modified by the xed Team, 2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

/*
 * Verbose error reporting for file I/O operations (load, save, revert, create)
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <string.h>

#include <glib/gi18n.h>
#include <gio/gio.h>

#include "xed-settings.h"
#include "xed-utils.h"
#include "xed-document.h"
#include "xed-io-error-info-bar.h"
#include <xed/xed-encodings-combo-box.h>

#define MAX_URI_IN_DIALOG_LENGTH 50

static gboolean
is_recoverable_error (const GError *error)
{
    gboolean is_recoverable = FALSE;

    if (error->domain == G_IO_ERROR)
    {
        switch (error->code)
        {
            case G_IO_ERROR_PERMISSION_DENIED:
            case G_IO_ERROR_NOT_FOUND:
            case G_IO_ERROR_HOST_NOT_FOUND:
            case G_IO_ERROR_TIMED_OUT:
            case G_IO_ERROR_NOT_MOUNTABLE_FILE:
            case G_IO_ERROR_NOT_MOUNTED:
            case G_IO_ERROR_BUSY:
                is_recoverable = TRUE;
        }
    }

    return is_recoverable;
}

static gboolean
is_gio_error (const GError *error,
              gint          code)
{
    return error->domain == G_IO_ERROR && error->code == code;
}

static void
set_contents (GtkWidget *area,
              GtkWidget *contents)
{
    GtkWidget *content_area;

    content_area = gtk_info_bar_get_content_area (GTK_INFO_BAR (area));
    gtk_container_add (GTK_CONTAINER (content_area), contents);
}

static void
set_info_bar_text_and_icon (GtkWidget   *info_bar,
                            const gchar *icon_name,
                            const gchar *primary_text,
                            const gchar *secondary_text)
{
    GtkWidget *hbox_content;
    GtkWidget *image;
    GtkWidget *vbox;
    gchar *primary_markup;
    gchar *secondary_markup;
    GtkWidget *primary_label;
    GtkWidget *secondary_label;

    hbox_content = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);

    image = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start (GTK_BOX (hbox_content), image, FALSE, FALSE, 0);
    gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (image, GTK_ALIGN_START);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_box_pack_start (GTK_BOX (hbox_content), vbox, TRUE, TRUE, 0);

    primary_markup = g_strdup_printf ("<b>%s</b>", primary_text);
    primary_label = gtk_label_new (primary_markup);
    g_free (primary_markup);
    gtk_box_pack_start (GTK_BOX (vbox), primary_label, TRUE, TRUE, 0);
    gtk_label_set_use_markup (GTK_LABEL (primary_label), TRUE);
    gtk_label_set_line_wrap (GTK_LABEL (primary_label), TRUE);
    gtk_widget_set_halign (primary_label, GTK_ALIGN_START);
    gtk_widget_set_can_focus (primary_label, TRUE);
    gtk_label_set_selectable (GTK_LABEL (primary_label), TRUE);

    if (secondary_text != NULL)
    {
        secondary_markup = g_strdup_printf ("<small>%s</small>", secondary_text);
        secondary_label = gtk_label_new (secondary_markup);
        g_free (secondary_markup);
        gtk_box_pack_start (GTK_BOX (vbox), secondary_label, TRUE, TRUE, 0);
        gtk_widget_set_can_focus (secondary_label, TRUE);
        gtk_label_set_use_markup (GTK_LABEL (secondary_label), TRUE);
        gtk_label_set_line_wrap (GTK_LABEL (secondary_label), TRUE);
        gtk_label_set_selectable (GTK_LABEL (secondary_label), TRUE);
        gtk_widget_set_halign (secondary_label, GTK_ALIGN_START);
    }

    gtk_widget_show_all (hbox_content);
    set_contents (info_bar, hbox_content);
}

static GtkWidget *
create_io_loading_error_info_bar (const gchar *primary_text,
                                  const gchar *secondary_text,
                                  gboolean     recoverable_error)
{
    GtkWidget *info_bar;

    info_bar = gtk_info_bar_new_with_buttons (_("_Cancel"), GTK_RESPONSE_CANCEL, NULL);
    gtk_info_bar_set_message_type (GTK_INFO_BAR (info_bar), GTK_MESSAGE_ERROR);

    set_info_bar_text_and_icon (info_bar, "dialog-error-symbolic", primary_text, secondary_text);

    if (recoverable_error)
    {
        gtk_info_bar_add_button (GTK_INFO_BAR (info_bar), _("_Retry"), GTK_RESPONSE_OK);
    }

    return info_bar;
}

static gboolean
parse_gio_error (gint          code,
                 gchar       **error_message,
                 gchar       **message_details,
                 GFile        *location,
                 const gchar  *uri_for_display)
{
    gboolean ret = TRUE;

    switch (code)
    {
        case G_IO_ERROR_NOT_FOUND:
        case G_IO_ERROR_NOT_DIRECTORY:
            *error_message = g_strdup_printf (_("Could not find the file %s."), uri_for_display);
            *message_details = g_strdup (_("Please check that you typed the "
                                         "location correctly and try again."));
            break;
        case G_IO_ERROR_NOT_SUPPORTED:
            {
                gchar *scheme_string;
                gchar *scheme_markup;

                scheme_string = g_file_get_uri_scheme (location);

                if ((scheme_string != NULL) && g_utf8_validate (scheme_string, -1, NULL))
                {
                    scheme_markup = g_markup_printf_escaped ("<i>%s:</i>", scheme_string);

                    /* Translators: %s is a URI scheme (like for example http:, ftp:, etc.) */
                    *message_details = g_strdup_printf (_("xed cannot handle %s locations."), scheme_markup);
                    g_free (scheme_markup);
                }
                else
                {
                    *message_details = g_strdup (_("xed cannot handle this location."));
                }

                g_free (scheme_string);
            }
            break;
        case G_IO_ERROR_NOT_MOUNTABLE_FILE:
            *message_details = g_strdup (_("The location of the file cannot be mounted."));
            break;
        case G_IO_ERROR_NOT_MOUNTED:
            *message_details = g_strdup( _("The location of the file cannot be accessed because it is not mounted."));
            break;
        case G_IO_ERROR_IS_DIRECTORY:
            *error_message = g_strdup_printf (_("%s is a directory."), uri_for_display);
            *message_details = g_strdup (_("Please check that you typed the "
                                         "location correctly and try again."));
            break;
        case G_IO_ERROR_INVALID_FILENAME:
            *error_message = g_strdup_printf (_("%s is not a valid location."), uri_for_display);
            *message_details = g_strdup (_("Please check that you typed the "
                                         "location correctly and try again."));
            break;
        case G_IO_ERROR_HOST_NOT_FOUND:
            /* This case can be hit for user-typed strings like "foo" due to
             * the code that guesses web addresses when there's no initial "/".
             * But this case is also hit for legitimate web addresses when
             * the proxy is set up wrong.
             */
            {
                gchar *hn = NULL;
                gchar *uri;

                uri = g_file_get_uri (location);

                if (uri && xed_utils_decode_uri (uri, NULL, NULL, &hn, NULL, NULL))
                {
                    if (hn != NULL)
                    {
                        gchar *host_markup;
                        gchar *host_name;

                        host_name = xed_utils_make_valid_utf8 (hn);
                        g_free (hn);

                        host_markup = g_markup_printf_escaped ("<i>%s</i>", host_name);
                        g_free (host_name);

                        /* Translators: %s is a host name */
                        *message_details = g_strdup_printf (_("Host %s could not be found. "
                                                            "Please check that your proxy settings "
                                                            "are correct and try again."),
                                                            host_markup);

                        g_free (host_markup);
                    }
                }

                g_free (uri);

                if (!*message_details)
                {
                    /* use the same string as INVALID_HOST */
                    *message_details = g_strdup_printf (_("Hostname was invalid. "
                                                        "Please check that you typed the location "
                                                        "correctly and try again."));
                }
            }
            break;
        case G_IO_ERROR_NOT_REGULAR_FILE:
            *message_details = g_strdup_printf (_("%s is not a regular file."), uri_for_display);
            break;
        case G_IO_ERROR_TIMED_OUT:
            *message_details = g_strdup (_("Connection timed out. Please try again."));
            break;
        default:
            ret = FALSE;
            break;
    }

    return ret;
}

static void
parse_error (const GError *error,
             gchar       **error_message,
             gchar       **message_details,
             GFile        *location,
             const gchar  *uri_for_display)
{
    gboolean ret = FALSE;

    if (error->domain == G_IO_ERROR)
    {
        ret = parse_gio_error (error->code, error_message, message_details, location, uri_for_display);
    }

    if (!ret)
    {
        g_warning ("Hit unhandled case %d (%s) in %s.", error->code, error->message, G_STRFUNC);
        *message_details = g_strdup_printf (_("Unexpected error: %s"), error->message);
    }
}

GtkWidget *
xed_unrecoverable_reverting_error_info_bar_new (GFile        *location,
                                                const GError *error)
{
    gchar *error_message = NULL;
    gchar *message_details = NULL;
    gchar *full_formatted_uri;
    gchar *uri_for_display;
    gchar *temp_uri_for_display;
    GtkWidget *info_bar;

    g_return_val_if_fail (G_IS_FILE (location), NULL);
    g_return_val_if_fail (error != NULL, NULL);
    g_return_val_if_fail ((error->domain == GTK_SOURCE_FILE_LOADER_ERROR) || error->domain == G_IO_ERROR, NULL);

    full_formatted_uri = g_file_get_parse_name (location);

    /* Truncate the URI so it doesn't get insanely wide. Note that even
     * though the dialog uses wrapped text, if the URI doesn't contain
     * white space then the text-wrapping code is too stupid to wrap it.
     */
    temp_uri_for_display = xed_utils_str_middle_truncate (full_formatted_uri, MAX_URI_IN_DIALOG_LENGTH);
    g_free (full_formatted_uri);

    uri_for_display = g_markup_printf_escaped ("<i>%s</i>", temp_uri_for_display);
    g_free (temp_uri_for_display);

    if (is_gio_error (error, G_IO_ERROR_NOT_FOUND))
    {
        message_details = g_strdup (_("xed cannot find the file. "
                                    "Perhaps it has recently been deleted."));
    }
    else
    {
        parse_error (error, &error_message, &message_details, location, uri_for_display);
    }

    if (error_message == NULL)
    {
        error_message = g_strdup_printf (_("Could not revert the file %s."), uri_for_display);
    }

    info_bar = create_io_loading_error_info_bar (error_message, message_details, FALSE);

    g_free (uri_for_display);
    g_free (error_message);
    g_free (message_details);

    return info_bar;
}

static void
create_combo_box (GtkWidget *info_bar,
                  GtkWidget *vbox)
{
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *menu;
    gchar *label_markup;

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);

    label_markup = g_strdup_printf ("<small>%s</small>", _("Ch_aracter Encoding:"));
    label = gtk_label_new_with_mnemonic (label_markup);
    g_free (label_markup);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    menu = xed_encodings_combo_box_new (TRUE);
    g_object_set_data (G_OBJECT (info_bar), "xed-info-bar-encoding-menu", menu);

    gtk_label_set_mnemonic_widget (GTK_LABEL (label), menu);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), menu, FALSE, FALSE, 0);

    gtk_widget_show_all (hbox);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
}

static GtkWidget *
create_conversion_error_info_bar (const gchar *primary_text,
                                  const gchar *secondary_text,
                                  gboolean     edit_anyway)
{
    GtkWidget *info_bar;
    GtkWidget *hbox_content;
    GtkWidget *image;
    GtkWidget *vbox;
    gchar *primary_markup;
    gchar *secondary_markup;
    GtkWidget *primary_label;
    GtkWidget *secondary_label;

    info_bar = gtk_info_bar_new ();

    gtk_info_bar_add_button (GTK_INFO_BAR (info_bar), _("_Retry"), GTK_RESPONSE_OK);

    if (edit_anyway)
    {
        gtk_info_bar_add_button (GTK_INFO_BAR (info_bar),
        /* Translators: the access key chosen for this string should be
         different from other main menu access keys (Open, Edit, View...) */
                     _("Edit Any_way"),
                     GTK_RESPONSE_YES);
        gtk_info_bar_set_message_type (GTK_INFO_BAR (info_bar), GTK_MESSAGE_WARNING);
    }
    else
    {
        gtk_info_bar_set_message_type (GTK_INFO_BAR (info_bar), GTK_MESSAGE_ERROR);
    }

    gtk_info_bar_add_button (GTK_INFO_BAR (info_bar), _("_Cancel"), GTK_RESPONSE_CANCEL);

    hbox_content = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);

    image = gtk_image_new_from_icon_name ("dialog-error-symbolic", GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start (GTK_BOX (hbox_content), image, FALSE, FALSE, 0);
    gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (image, GTK_ALIGN_START);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_box_pack_start (GTK_BOX (hbox_content), vbox, TRUE, TRUE, 0);

    primary_markup = g_strdup_printf ("<b>%s</b>", primary_text);
    primary_label = gtk_label_new (primary_markup);
    g_free (primary_markup);
    gtk_box_pack_start (GTK_BOX (vbox), primary_label, TRUE, TRUE, 0);
    gtk_label_set_use_markup (GTK_LABEL (primary_label), TRUE);
    gtk_label_set_line_wrap (GTK_LABEL (primary_label), TRUE);
    gtk_widget_set_halign (primary_label, GTK_ALIGN_START);
    gtk_widget_set_can_focus (primary_label, TRUE);
    gtk_label_set_selectable (GTK_LABEL (primary_label), TRUE);

    if (secondary_text != NULL)
    {
        secondary_markup = g_strdup_printf ("<small>%s</small>", secondary_text);
        secondary_label = gtk_label_new (secondary_markup);
        g_free (secondary_markup);
        gtk_box_pack_start (GTK_BOX (vbox), secondary_label, TRUE, TRUE, 0);
        gtk_widget_set_can_focus (secondary_label, TRUE);
        gtk_label_set_use_markup (GTK_LABEL (secondary_label), TRUE);
        gtk_label_set_line_wrap (GTK_LABEL (secondary_label), TRUE);
        gtk_label_set_selectable (GTK_LABEL (secondary_label), TRUE);
        gtk_widget_set_halign (secondary_label, GTK_ALIGN_START);
    }

    create_combo_box (info_bar, vbox);
    gtk_widget_show_all (hbox_content);
    set_contents (info_bar, hbox_content);

    return info_bar;
}

GtkWidget *
xed_io_loading_error_info_bar_new (GFile                   *location,
                                   const GtkSourceEncoding *encoding,
                                   const GError            *error)
{
    gchar *error_message = NULL;
    gchar *message_details = NULL;
    gchar *full_formatted_uri;
    gchar *uri_for_display;
    gchar *temp_uri_for_display;
    GtkWidget *info_bar;
    gboolean edit_anyway = FALSE;
    gboolean convert_error = FALSE;

    g_return_val_if_fail (G_IS_FILE (location), NULL);
    g_return_val_if_fail (error != NULL, NULL);
    g_return_val_if_fail (error->domain == GTK_SOURCE_FILE_LOADER_ERROR ||
                          error->domain == G_IO_ERROR ||
                          error->domain == G_CONVERT_ERROR, NULL);

    if (location != NULL)
    {
        full_formatted_uri = g_file_get_parse_name (location);
    }
    else
    {
        full_formatted_uri = g_strdup ("stdin");
    }

    /* Truncate the URI so it doesn't get insanely wide. Note that even
     * though the dialog uses wrapped text, if the URI doesn't contain
     * white space then the text-wrapping code is too stupid to wrap it.
     */
    temp_uri_for_display = xed_utils_str_middle_truncate (full_formatted_uri, MAX_URI_IN_DIALOG_LENGTH);
    g_free (full_formatted_uri);

    uri_for_display = g_markup_printf_escaped ("<i>%s</i>", temp_uri_for_display);
    g_free (temp_uri_for_display);

    if (is_gio_error (error, G_IO_ERROR_TOO_MANY_LINKS))
    {
        message_details = g_strdup (_("The number of followed links is limited and the actual file could not be found within this limit."));
    }
    else if (is_gio_error (error, G_IO_ERROR_PERMISSION_DENIED))
    {
        message_details = g_strdup (_("You do not have the permissions necessary to open the file."));
    }
    else if ((is_gio_error (error, G_IO_ERROR_INVALID_DATA) && encoding == NULL) ||
             (error->domain == GTK_SOURCE_FILE_LOADER_ERROR &&
              error->code == GTK_SOURCE_FILE_LOADER_ERROR_ENCODING_AUTO_DETECTION_FAILED))
    {
        message_details = g_strconcat (_("xed has not been able to detect "
                                         "the character encoding."), "\n",
                                       _("Please check that you are not trying to open a binary file."), "\n",
                                       _("Select a character encoding from the menu and try again."), NULL);
        convert_error = TRUE;
    }
    else if (error->domain == GTK_SOURCE_FILE_LOADER_ERROR &&
             error->code == GTK_SOURCE_FILE_LOADER_ERROR_CONVERSION_FALLBACK)
    {
        error_message = g_strdup_printf (_("There was a problem opening the file %s."), uri_for_display);
        message_details = g_strconcat (_("The file you opened has some invalid characters. "
                                       "If you continue editing this file you could corrupt this "
                                       "document."), "\n",
                                       _("You can also choose another character encoding and try again."),
                                       NULL);
        edit_anyway = TRUE;
        convert_error = TRUE;
    }
    else if (is_gio_error (error, G_IO_ERROR_INVALID_DATA) && encoding != NULL)
    {
        gchar *encoding_name = gtk_source_encoding_to_string (encoding);

        error_message = g_strdup_printf (_("Could not open the file %s using the %s character encoding."),
                                         uri_for_display,
                                         encoding_name);
        message_details = g_strconcat (_("Please check that you are not trying to open a binary file."), "\n",
                                       _("Select a different character encoding from the menu and try again."), NULL);
        convert_error = TRUE;

        g_free (encoding_name);
    }
    else
    {
        parse_error (error, &error_message, &message_details, location, uri_for_display);
    }

    if (error_message == NULL)
    {
        error_message = g_strdup_printf (_("Could not open the file %s."), uri_for_display);
    }

    if (convert_error)
    {
        info_bar = create_conversion_error_info_bar (error_message, message_details, edit_anyway);
    }
    else
    {
        info_bar = create_io_loading_error_info_bar (error_message, message_details, is_recoverable_error (error));
    }

    g_free (uri_for_display);
    g_free (error_message);
    g_free (message_details);

    return info_bar;
}

GtkWidget *
xed_conversion_error_while_saving_info_bar_new (GFile                   *location,
                                                const GtkSourceEncoding *encoding,
                                                const GError            *error)
{
    gchar *error_message = NULL;
    gchar *message_details = NULL;
    gchar *full_formatted_uri;
    gchar *encoding_name;
    gchar *uri_for_display;
    gchar *temp_uri_for_display;
    GtkWidget *info_bar;

    g_return_val_if_fail (G_IS_FILE (location), NULL);
    g_return_val_if_fail (error != NULL, NULL);
    g_return_val_if_fail (error->domain == G_CONVERT_ERROR ||
                          error->domain == G_IO_ERROR, NULL);
    g_return_val_if_fail (encoding != NULL, NULL);

    full_formatted_uri = g_file_get_parse_name (location);

    /* Truncate the URI so it doesn't get insanely wide. Note that even
     * though the dialog uses wrapped text, if the URI doesn't contain
     * white space then the text-wrapping code is too stupid to wrap it.
     */
    temp_uri_for_display = xed_utils_str_middle_truncate (full_formatted_uri, MAX_URI_IN_DIALOG_LENGTH);
    g_free (full_formatted_uri);

    uri_for_display = g_markup_printf_escaped ("<i>%s</i>", temp_uri_for_display);
    g_free (temp_uri_for_display);

    encoding_name = gtk_source_encoding_to_string (encoding);

    error_message = g_strdup_printf (_("Could not save the file %s using the %s character encoding."),
                                     uri_for_display,
                                     encoding_name);
    message_details = g_strconcat (_("The document contains one or more characters that cannot be encoded "
                                     "using the specified character encoding."), "\n",
                                     _("Select a different character encoding from the menu and try again."), NULL);

    info_bar = create_conversion_error_info_bar (error_message, message_details, FALSE);

    g_free (uri_for_display);
    g_free (encoding_name);
    g_free (error_message);
    g_free (message_details);

    return info_bar;
}

const GtkSourceEncoding *
xed_conversion_error_info_bar_get_encoding (GtkWidget *info_bar)
{
    gpointer menu;

    g_return_val_if_fail (GTK_IS_INFO_BAR (info_bar), NULL);

    menu = g_object_get_data (G_OBJECT (info_bar), "xed-info-bar-encoding-menu");
    g_return_val_if_fail (menu, NULL);

    return xed_encodings_combo_box_get_selected_encoding (XED_ENCODINGS_COMBO_BOX (menu));
}

GtkWidget *
xed_file_already_open_warning_info_bar_new (GFile *location)
{
    GtkWidget *info_bar;
    GtkWidget *hbox_content;
    GtkWidget *image;
    GtkWidget *vbox;
    gchar *primary_markup;
    gchar *secondary_markup;
    GtkWidget *primary_label;
    GtkWidget *secondary_label;
    gchar *primary_text;
    const gchar *secondary_text;
    gchar *full_formatted_uri;
    gchar *uri_for_display;
    gchar *temp_uri_for_display;

    g_return_val_if_fail (G_IS_FILE (location), NULL);

    full_formatted_uri = g_file_get_parse_name (location);

    /* Truncate the URI so it doesn't get insanely wide. Note that even
     * though the dialog uses wrapped text, if the URI doesn't contain
     * white space then the text-wrapping code is too stupid to wrap it.
     */
    temp_uri_for_display = xed_utils_str_middle_truncate (full_formatted_uri, MAX_URI_IN_DIALOG_LENGTH);
    g_free (full_formatted_uri);

    uri_for_display = g_markup_printf_escaped ("<i>%s</i>", temp_uri_for_display);
    g_free (temp_uri_for_display);

    info_bar = gtk_info_bar_new ();
    gtk_info_bar_add_button (GTK_INFO_BAR (info_bar),
    /* Translators: the access key chosen for this string should be
     different from other main menu access keys (Open, Edit, View...) */
                 _("Edit Any_way"),
                 GTK_RESPONSE_YES);
    gtk_info_bar_add_button (GTK_INFO_BAR (info_bar),
    /* Translators: the access key chosen for this string should be
     different from other main menu access keys (Open, Edit, View...) */
                 _("D_on't Edit"),
                 GTK_RESPONSE_CANCEL);
    gtk_info_bar_set_message_type (GTK_INFO_BAR (info_bar), GTK_MESSAGE_WARNING);

    hbox_content = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);

    image = gtk_image_new_from_icon_name ("dialog-warning-symbolic", GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start (GTK_BOX (hbox_content), image, FALSE, FALSE, 0);
    gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (image, GTK_ALIGN_START);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_box_pack_start (GTK_BOX (hbox_content), vbox, TRUE, TRUE, 0);

    primary_text = g_strdup_printf (_("This file (%s) is already open in another xed window."), uri_for_display);
    g_free (uri_for_display);

    primary_markup = g_strdup_printf ("<b>%s</b>", primary_text);
    g_free (primary_text);
    primary_label = gtk_label_new (primary_markup);
    g_free (primary_markup);
    gtk_box_pack_start (GTK_BOX (vbox), primary_label, TRUE, TRUE, 0);
    gtk_label_set_use_markup (GTK_LABEL (primary_label), TRUE);
    gtk_label_set_line_wrap (GTK_LABEL (primary_label), TRUE);
    gtk_widget_set_halign (primary_label, GTK_ALIGN_START);
    gtk_widget_set_can_focus (primary_label, TRUE);
    gtk_label_set_selectable (GTK_LABEL (primary_label), TRUE);

    secondary_text = _("xed opened this instance of the file in a non-editable way. "
                     "Do you want to edit it anyway?");
    secondary_markup = g_strdup_printf ("<small>%s</small>", secondary_text);
    secondary_label = gtk_label_new (secondary_markup);
    g_free (secondary_markup);
    gtk_box_pack_start (GTK_BOX (vbox), secondary_label, TRUE, TRUE, 0);
    gtk_widget_set_can_focus (secondary_label, TRUE);
    gtk_label_set_use_markup (GTK_LABEL (secondary_label), TRUE);
    gtk_label_set_line_wrap (GTK_LABEL (secondary_label), TRUE);
    gtk_label_set_selectable (GTK_LABEL (secondary_label), TRUE);
    gtk_widget_set_halign (secondary_label, GTK_ALIGN_START);

    gtk_widget_show_all (hbox_content);
    set_contents (info_bar, hbox_content);

    return info_bar;
}

GtkWidget *
xed_externally_modified_saving_error_info_bar_new (GFile        *location,
                                                   const GError *error)
{
    GtkWidget *info_bar;
    GtkWidget *hbox_content;
    GtkWidget *image;
    GtkWidget *vbox;
    gchar *primary_markup;
    gchar *secondary_markup;
    GtkWidget *primary_label;
    GtkWidget *secondary_label;
    gchar *primary_text;
    const gchar *secondary_text;
    gchar *full_formatted_uri;
    gchar *uri_for_display;
    gchar *temp_uri_for_display;

    g_return_val_if_fail (G_IS_FILE (location), NULL);
    g_return_val_if_fail (error != NULL, NULL);
    g_return_val_if_fail (error->domain == GTK_SOURCE_FILE_SAVER_ERROR, NULL);
    g_return_val_if_fail (error->code == GTK_SOURCE_FILE_SAVER_ERROR_EXTERNALLY_MODIFIED, NULL);

    full_formatted_uri = g_file_get_parse_name (location);

    /* Truncate the URI so it doesn't get insanely wide. Note that even
     * though the dialog uses wrapped text, if the URI doesn't contain
     * white space then the text-wrapping code is too stupid to wrap it.
     */
    temp_uri_for_display = xed_utils_str_middle_truncate (full_formatted_uri, MAX_URI_IN_DIALOG_LENGTH);
    g_free (full_formatted_uri);

    uri_for_display = g_markup_printf_escaped ("<i>%s</i>", temp_uri_for_display);
    g_free (temp_uri_for_display);

    info_bar = gtk_info_bar_new ();

    gtk_info_bar_add_button (GTK_INFO_BAR (info_bar), _("S_ave Anyway"), GTK_RESPONSE_YES);
    gtk_info_bar_add_button (GTK_INFO_BAR (info_bar), _("D_on't Save"), GTK_RESPONSE_CANCEL);
    gtk_info_bar_set_message_type (GTK_INFO_BAR (info_bar), GTK_MESSAGE_WARNING);

    hbox_content = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);

    image = gtk_image_new_from_icon_name ("dialog-warning-symbolic", GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start (GTK_BOX (hbox_content), image, FALSE, FALSE, 0);
    gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (image, GTK_ALIGN_START);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_box_pack_start (GTK_BOX (hbox_content), vbox, TRUE, TRUE, 0);

    /* FIXME: review this message, it's not clear since for the user the "modification"
     * could be interpreted as the changes he made in the document. beside "reading" is
     * not accurate (since last load/save)
     */
    primary_text = g_strdup_printf (_("The file %s has been modified since reading it."), uri_for_display);
    g_free (uri_for_display);

    primary_markup = g_strdup_printf ("<b>%s</b>", primary_text);
    g_free (primary_text);
    primary_label = gtk_label_new (primary_markup);
    g_free (primary_markup);
    gtk_box_pack_start (GTK_BOX (vbox), primary_label, TRUE, TRUE, 0);
    gtk_label_set_use_markup (GTK_LABEL (primary_label), TRUE);
    gtk_label_set_line_wrap (GTK_LABEL (primary_label), TRUE);
    gtk_widget_set_halign (primary_label, GTK_ALIGN_START);
    gtk_widget_set_can_focus (primary_label, TRUE);
    gtk_label_set_selectable (GTK_LABEL (primary_label), TRUE);

    secondary_text = _("If you save it, all the external changes could be lost. Save it anyway?");
    secondary_markup = g_strdup_printf ("<small>%s</small>", secondary_text);
    secondary_label = gtk_label_new (secondary_markup);
    g_free (secondary_markup);
    gtk_box_pack_start (GTK_BOX (vbox), secondary_label, TRUE, TRUE, 0);
    gtk_widget_set_can_focus (secondary_label, TRUE);
    gtk_label_set_use_markup (GTK_LABEL (secondary_label), TRUE);
    gtk_label_set_line_wrap (GTK_LABEL (secondary_label), TRUE);
    gtk_label_set_selectable (GTK_LABEL (secondary_label), TRUE);
    gtk_widget_set_halign (secondary_label, GTK_ALIGN_START);

    gtk_widget_show_all (hbox_content);
    set_contents (info_bar, hbox_content);

    return info_bar;
}

GtkWidget *
xed_no_backup_saving_error_info_bar_new (GFile        *location,
                                         const GError *error)
{
    GtkWidget *info_bar;
    GtkWidget *hbox_content;
    GtkWidget *image;
    GtkWidget *vbox;
    gchar *primary_markup;
    gchar *secondary_markup;
    GtkWidget *primary_label;
    GtkWidget *secondary_label;
    gchar *primary_text;
    const gchar *secondary_text;
    gchar *full_formatted_uri;
    gchar *uri_for_display;
    gchar *temp_uri_for_display;
    gboolean create_backup_copy;
    GSettings *editor_settings;

    g_return_val_if_fail (G_IS_FILE (location), NULL);
    g_return_val_if_fail (error != NULL, NULL);
    g_return_val_if_fail (error->domain == G_IO_ERROR &&
                          error->code == G_IO_ERROR_CANT_CREATE_BACKUP, NULL);

    full_formatted_uri = g_file_get_parse_name (location);

    /* Truncate the URI so it doesn't get insanely wide. Note that even
     * though the dialog uses wrapped text, if the URI doesn't contain
     * white space then the text-wrapping code is too stupid to wrap it.
     */
    temp_uri_for_display = xed_utils_str_middle_truncate (full_formatted_uri, MAX_URI_IN_DIALOG_LENGTH);
    g_free (full_formatted_uri);

    uri_for_display = g_markup_printf_escaped ("<i>%s</i>", temp_uri_for_display);
    g_free (temp_uri_for_display);

    info_bar = gtk_info_bar_new ();

    gtk_info_bar_add_button (GTK_INFO_BAR (info_bar), _("S_ave Anyway"), GTK_RESPONSE_YES);
    gtk_info_bar_add_button (GTK_INFO_BAR (info_bar), _("D_on't Save"), GTK_RESPONSE_CANCEL);
    gtk_info_bar_set_message_type (GTK_INFO_BAR (info_bar), GTK_MESSAGE_WARNING);

    hbox_content = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);

    image = gtk_image_new_from_icon_name ("dialog-warning-symbolic", GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start (GTK_BOX (hbox_content), image, FALSE, FALSE, 0);
    gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (image, GTK_ALIGN_START);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_box_pack_start (GTK_BOX (hbox_content), vbox, TRUE, TRUE, 0);

    editor_settings = g_settings_new ("org.x.editor.preferences.editor");
    create_backup_copy = g_settings_get_boolean (editor_settings, XED_SETTINGS_CREATE_BACKUP_COPY);
    g_object_unref (editor_settings);

    // FIXME: review this messages
    if (create_backup_copy)
    {
        primary_text = g_strdup_printf (_("Could not create a backup file while saving %s"), uri_for_display);
    }
    else
    {
        primary_text = g_strdup_printf (_("Could not create a temporary backup file while saving %s"), uri_for_display);
    }

    g_free (uri_for_display);

    primary_markup = g_strdup_printf ("<b>%s</b>", primary_text);
    g_free (primary_text);
    primary_label = gtk_label_new (primary_markup);
    g_free (primary_markup);
    gtk_box_pack_start (GTK_BOX (vbox), primary_label, TRUE, TRUE, 0);
    gtk_label_set_use_markup (GTK_LABEL (primary_label), TRUE);
    gtk_label_set_line_wrap (GTK_LABEL (primary_label), TRUE);
    gtk_widget_set_halign (primary_label, GTK_ALIGN_START);
    gtk_widget_set_can_focus (primary_label, TRUE);
    gtk_label_set_selectable (GTK_LABEL (primary_label), TRUE);

    secondary_text = _("xed could not back up the old copy of the file before saving the new one. "
                    "You can ignore this warning and save the file anyway, but if an error "
                    "occurs while saving, you could lose the old copy of the file. Save anyway?");
    secondary_markup = g_strdup_printf ("<small>%s</small>", secondary_text);
    secondary_label = gtk_label_new (secondary_markup);
    g_free (secondary_markup);
    gtk_box_pack_start (GTK_BOX (vbox), secondary_label, TRUE, TRUE, 0);
    gtk_widget_set_can_focus (secondary_label, TRUE);
    gtk_label_set_use_markup (GTK_LABEL (secondary_label), TRUE);
    gtk_label_set_line_wrap (GTK_LABEL (secondary_label), TRUE);
    gtk_label_set_selectable (GTK_LABEL (secondary_label), TRUE);
    gtk_widget_set_halign (secondary_label, GTK_ALIGN_START);

    gtk_widget_show_all (hbox_content);
    set_contents (info_bar, hbox_content);

    return info_bar;
}

GtkWidget *
xed_unrecoverable_saving_error_info_bar_new (GFile        *location,
                                             const GError *error)
{
    gchar *error_message = NULL;
    gchar *message_details = NULL;
    gchar *full_formatted_uri;
    gchar *scheme_string;
    gchar *scheme_markup;
    gchar *uri_for_display;
    gchar *temp_uri_for_display;
    GtkWidget *info_bar;

    g_return_val_if_fail (G_IS_FILE (location), NULL);
    g_return_val_if_fail (error != NULL, NULL);
    g_return_val_if_fail (error->domain == GTK_SOURCE_FILE_SAVER_ERROR || error->domain == G_IO_ERROR, NULL);

    full_formatted_uri = g_file_get_parse_name (location);

    /* Truncate the URI so it doesn't get insanely wide. Note that even
     * though the dialog uses wrapped text, if the URI doesn't contain
     * white space then the text-wrapping code is too stupid to wrap it.
     */
    temp_uri_for_display = xed_utils_str_middle_truncate (full_formatted_uri, MAX_URI_IN_DIALOG_LENGTH);
    g_free (full_formatted_uri);

    uri_for_display = g_markup_printf_escaped ("<i>%s</i>", temp_uri_for_display);
    g_free (temp_uri_for_display);

    if (is_gio_error (error, G_IO_ERROR_NOT_SUPPORTED))
    {
        scheme_string = g_file_get_uri_scheme (location);

        if ((scheme_string != NULL) && g_utf8_validate (scheme_string, -1, NULL))
        {
            scheme_markup = g_markup_printf_escaped ("<i>%s:</i>", scheme_string);

            /* Translators: %s is a URI scheme (like for example http:, ftp:, etc.) */
            message_details = g_strdup_printf (_("xed cannot handle %s locations in write mode. "
                                               "Please check that you typed the "
                                               "location correctly and try again."),
                                               scheme_markup);
            g_free (scheme_markup);
        }
        else
        {
            message_details = g_strdup (_("xed cannot handle this location in write mode. "
                                        "Please check that you typed the "
                                        "location correctly and try again."));
        }

        g_free (scheme_string);
    }
    else if (is_gio_error (error, G_IO_ERROR_INVALID_FILENAME))
    {
        message_details = g_strdup (_("%s is not a valid location. "
                                    "Please check that you typed the "
                                    "location correctly and try again."));
    }
    else if (is_gio_error (error, G_IO_ERROR_PERMISSION_DENIED))
    {
        message_details = g_strdup (_("You do not have the permissions necessary to save the file. "
                                    "Please check that you typed the "
                                    "location correctly and try again."));
    }
    else if (is_gio_error (error, G_IO_ERROR_NO_SPACE))
    {
        message_details = g_strdup (_("There is not enough disk space to save the file. "
                                    "Please free some disk space and try again."));
    }
    else if (is_gio_error (error, G_IO_ERROR_READ_ONLY))
    {
        message_details = g_strdup (_("You are trying to save the file on a read-only disk. "
                                    "Please check that you typed the location "
                                    "correctly and try again."));
    }
    else if (is_gio_error (error, G_IO_ERROR_EXISTS))
    {
        message_details = g_strdup (_("A file with the same name already exists. "
                                    "Please use a different name."));
    }
    else if (is_gio_error (error, G_IO_ERROR_FILENAME_TOO_LONG))
    {
        message_details = g_strdup (_("The disk where you are trying to save the file has "
                                    "a limitation on length of the file names. "
                                    "Please use a shorter name."));
    }
#if 0
    /* FIXME this error can not occur for a file saving. Either remove the
     * code here, or improve the GtkSourceFileSaver so this error can occur.
     */
    else if (error->domain == XED_DOCUMENT_ERROR && error->code == XED_DOCUMENT_ERROR_TOO_BIG)
    {
        message_details = g_strdup (_("The disk where you are trying to save the file has "
                                    "a limitation on file sizes. Please try saving "
                                    "a smaller file or saving it to a disk that does not "
                                    "have this limitation."));
    }
#endif
    else
    {
        parse_error (error, &error_message, &message_details, location, uri_for_display);
    }

    if (error_message == NULL)
    {
        error_message = g_strdup_printf (_("Could not save the file %s."), uri_for_display);
    }

    info_bar = create_io_loading_error_info_bar (error_message, message_details, FALSE);

    g_free (uri_for_display);
    g_free (error_message);
    g_free (message_details);

    return info_bar;
}

GtkWidget *
xed_externally_modified_info_bar_new (GFile    *location,
                                      gboolean  document_modified)
{
    gchar *full_formatted_uri;
    gchar *uri_for_display;
    gchar *temp_uri_for_display;
    const gchar *primary_text;
    const gchar *secondary_text;
    GtkWidget *info_bar;

    g_return_val_if_fail (G_IS_FILE (location), NULL);

    full_formatted_uri = g_file_get_parse_name (location);

    /* Truncate the URI so it doesn't get insanely wide. Note that even
     * though the dialog uses wrapped text, if the URI doesn't contain
     * white space then the text-wrapping code is too stupid to wrap it.
     */
    temp_uri_for_display = xed_utils_str_middle_truncate (full_formatted_uri, MAX_URI_IN_DIALOG_LENGTH);
    g_free (full_formatted_uri);

    uri_for_display = g_markup_printf_escaped ("<i>%s</i>", temp_uri_for_display);
    g_free (temp_uri_for_display);

    // FIXME: review this message, it's not clear since for the user the "modification"
    // could be interpreted as the changes he made in the document. beside "reading" is
    // not accurate (since last load/save)
    primary_text = g_strdup_printf (_("The file %s changed on disk."), uri_for_display);
    g_free (uri_for_display);

    if (document_modified)
    {
        secondary_text = _("Do you want to drop your changes and reload the file?");
    }
    else
    {
        secondary_text = _("Do you want to reload the file?");
    }

    info_bar = gtk_info_bar_new ();

    gtk_info_bar_add_button (GTK_INFO_BAR (info_bar), _("_Reload"), GTK_RESPONSE_OK);
    gtk_info_bar_add_button (GTK_INFO_BAR (info_bar), _("_Cancel"), GTK_RESPONSE_CANCEL);
    gtk_info_bar_set_message_type (GTK_INFO_BAR (info_bar), GTK_MESSAGE_WARNING);

    set_info_bar_text_and_icon (info_bar, "dialog-warning-symbolic", primary_text, secondary_text);

    return info_bar;
}

GtkWidget *
xed_invalid_character_info_bar_new (GFile *location)
{
    GtkWidget *info_bar;
    GtkWidget *hbox_content;
    GtkWidget *image;
    GtkWidget *vbox;
    GtkWidget *primary_label;
    GtkWidget *secondary_label;
    gchar *primary_markup;
    gchar *secondary_markup;
    gchar *primary_text;
    gchar *full_formatted_uri;
    gchar *uri_for_display;
    gchar *temp_uri_for_display;
    const gchar *secondary_text;

    g_return_val_if_fail (G_IS_FILE (location), NULL);

    full_formatted_uri = g_file_get_parse_name (location);

    /* Truncate the URI so it doesn't get insanely wide. Note that even
    * though the dialog uses wrapped text, if the URI doesn't contain
    * white space then the text-wrapping code is too stupid to wrap it.
    */
    temp_uri_for_display = xed_utils_str_middle_truncate (full_formatted_uri, MAX_URI_IN_DIALOG_LENGTH);
    g_free (full_formatted_uri);

    uri_for_display = g_markup_printf_escaped ("<i>%s</i>", temp_uri_for_display);
    g_free (temp_uri_for_display);

    info_bar = gtk_info_bar_new ();

    gtk_info_bar_add_button (GTK_INFO_BAR (info_bar), _("S_ave Anyway"), GTK_RESPONSE_YES);
    gtk_info_bar_add_button (GTK_INFO_BAR (info_bar),
                            _("D_on't Save"),
                            GTK_RESPONSE_CANCEL);
    gtk_info_bar_set_message_type (GTK_INFO_BAR (info_bar), GTK_MESSAGE_WARNING);

    hbox_content = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);

    image = gtk_image_new_from_icon_name ("dialog-warning-symbolic", GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start (GTK_BOX (hbox_content), image, FALSE, FALSE, 0);
    gtk_widget_set_valign (image, GTK_ALIGN_START);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_box_pack_start (GTK_BOX (hbox_content), vbox, TRUE, TRUE, 0);

    primary_text = g_strdup_printf (_("Some invalid chars have been detected while saving %s"), uri_for_display);

    g_free (uri_for_display);

    primary_markup = g_strdup_printf ("<b>%s</b>", primary_text);
    g_free (primary_text);
    primary_label = gtk_label_new (primary_markup);
    g_free (primary_markup);
    gtk_box_pack_start (GTK_BOX (vbox), primary_label, TRUE, TRUE, 0);
    gtk_label_set_use_markup (GTK_LABEL (primary_label), TRUE);
    gtk_label_set_line_wrap (GTK_LABEL (primary_label), TRUE);
    gtk_widget_set_halign (primary_label, GTK_ALIGN_START);
    gtk_widget_set_can_focus (primary_label, TRUE);
    gtk_label_set_selectable (GTK_LABEL (primary_label), TRUE);

    secondary_text = _("If you continue saving this file you can corrupt the document. "
                      " Save anyway?");
    secondary_markup = g_strdup_printf ("<small>%s</small>", secondary_text);
    secondary_label = gtk_label_new (secondary_markup);
    g_free (secondary_markup);
    gtk_box_pack_start (GTK_BOX (vbox), secondary_label, TRUE, TRUE, 0);
    gtk_widget_set_can_focus (secondary_label, TRUE);
    gtk_label_set_use_markup (GTK_LABEL (secondary_label), TRUE);
    gtk_label_set_line_wrap (GTK_LABEL (secondary_label), TRUE);
    gtk_label_set_selectable (GTK_LABEL (secondary_label), TRUE);
    gtk_widget_set_halign (secondary_label, GTK_ALIGN_START);

    gtk_widget_show_all (hbox_content);
    set_contents (info_bar, hbox_content);

    return info_bar;
}
