/*
 * xed-document.c
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

#include <string.h>
#include <stdlib.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "xed-document.h"
#include "xed-settings.h"
#include "xed-debug.h"
#include "xed-utils.h"
#include "xed-marshal.h"
#include "xed-enum-types.h"

#ifndef ENABLE_GVFS_METADATA
#include "xed-metadata-manager.h"
#else
#define METADATA_QUERY "metadata::*"
#endif

#define NO_LANGUAGE_NAME "_NORMAL_"

#define XED_DOCUMENT_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), XED_TYPE_DOCUMENT, XedDocumentPrivate))

static void xed_document_loaded_real (XedDocument  *doc);
static void xed_document_saved_real (XedDocument  *doc);

struct _XedDocumentPrivate
{
    GtkSourceFile *file;

    GSettings *editor_settings;

    gint   untitled_number;
    gchar *short_name;

    GFileInfo *metadata_info;

    gchar *content_type;

    GTimeVal mtime;
    GTimeVal time_of_last_save_or_load;

    GtkSourceSearchContext *search_context;

    guint readonly : 1;
    guint externally_modified : 1;
    guint deleted : 1;
    guint last_save_was_manually : 1;
    guint language_set_by_user : 1;
    guint stop_cursor_moved_emission : 1;
    guint mtime_set : 1;

    /* Create file if location points to a non existing file (for example
     * when opened from the command line).
     */
    guint create : 1;
};

enum
{
    PROP_0,
    PROP_SHORTNAME,
    PROP_CONTENT_TYPE,
    PROP_MIME_TYPE,
    PROP_READ_ONLY
};

enum
{
    CURSOR_MOVED,
    LOAD,
    LOADED,
    SAVE,
    SAVED,
    LAST_SIGNAL
};

static guint document_signals[LAST_SIGNAL] = { 0 };

static GHashTable *allocated_untitled_numbers = NULL;

G_DEFINE_TYPE(XedDocument, xed_document, GTK_SOURCE_TYPE_BUFFER)

static gint
get_untitled_number (void)
{
    gint i = 1;

    if (allocated_untitled_numbers == NULL)
    {
        allocated_untitled_numbers = g_hash_table_new (NULL, NULL);
    }

    g_return_val_if_fail (allocated_untitled_numbers != NULL, -1);

    while (TRUE)
    {
        if (g_hash_table_lookup (allocated_untitled_numbers, GINT_TO_POINTER (i)) == NULL)
        {
            g_hash_table_insert (allocated_untitled_numbers, GINT_TO_POINTER (i), GINT_TO_POINTER (i));

            return i;
        }

        ++i;
    }
}

static void
release_untitled_number (gint n)
{
    g_return_if_fail (allocated_untitled_numbers != NULL);

    g_hash_table_remove (allocated_untitled_numbers, GINT_TO_POINTER (n));
}

static const gchar *
get_language_string (XedDocument *doc)
{
    GtkSourceLanguage *lang = xed_document_get_language (doc);

    return lang != NULL ? gtk_source_language_get_id (lang) : NO_LANGUAGE_NAME;
}

static void
save_metadata (XedDocument *doc)
{
    const gchar *language = NULL;
    GtkTextIter iter;
    gchar *position;

    if (doc->priv->language_set_by_user)
    {
        language = get_language_string (doc);
    }

    gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (doc),
                                      &iter,
                                      gtk_text_buffer_get_insert (GTK_TEXT_BUFFER (doc)));

    position = g_strdup_printf ("%d", gtk_text_iter_get_offset (&iter));

    if (language == NULL)
    {
        xed_document_set_metadata (doc,
                                   XED_METADATA_ATTRIBUTE_POSITION, position,
                                   NULL);
    }
    else
    {
        xed_document_set_metadata (doc,
                                   XED_METADATA_ATTRIBUTE_POSITION, position,
                                   XED_METADATA_ATTRIBUTE_LANGUAGE, language,
                                   NULL);
    }

    g_free (position);
}

static void
xed_document_dispose (GObject *object)
{
    XedDocument *doc = XED_DOCUMENT (object);

    xed_debug (DEBUG_DOCUMENT);

    /* Metadata must be saved here and not in finalize because the language
    * is gone by the time finalize runs.
    */
    if (doc->priv->file != NULL)
    {
        save_metadata (doc);

        g_object_unref (doc->priv->file);
        doc->priv->file = NULL;
    }

    g_clear_object (&doc->priv->editor_settings);
    g_clear_object (&doc->priv->metadata_info);
    g_clear_object (&doc->priv->search_context);

    G_OBJECT_CLASS (xed_document_parent_class)->dispose (object);
}

static void
xed_document_finalize (GObject *object)
{
    XedDocument *doc = XED_DOCUMENT (object);

    xed_debug (DEBUG_DOCUMENT);

    if (doc->priv->untitled_number > 0)
    {
        release_untitled_number (doc->priv->untitled_number);
    }

    g_free (doc->priv->content_type);
    g_free (doc->priv->short_name);

    G_OBJECT_CLASS (xed_document_parent_class)->finalize (object);
}

static void
xed_document_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    XedDocument *doc = XED_DOCUMENT (object);

    switch (prop_id)
    {
        case PROP_SHORTNAME:
            g_value_take_string (value, xed_document_get_short_name_for_display (doc));
            break;
        case PROP_CONTENT_TYPE:
            g_value_take_string (value, xed_document_get_content_type (doc));
            break;
        case PROP_MIME_TYPE:
            g_value_take_string (value, xed_document_get_mime_type (doc));
            break;
        case PROP_READ_ONLY:
            g_value_set_boolean (value, doc->priv->readonly);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_document_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    XedDocument *doc = XED_DOCUMENT (object);

    switch (prop_id)
    {
        case PROP_SHORTNAME:
            xed_document_set_short_name_for_display (doc, g_value_get_string (value));
            break;
        case PROP_CONTENT_TYPE:
            xed_document_set_content_type (doc, g_value_get_string (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
emit_cursor_moved (XedDocument *doc)
{
    if (!doc->priv->stop_cursor_moved_emission)
    {
        g_signal_emit (doc, document_signals[CURSOR_MOVED], 0);
    }
}

static void
xed_document_mark_set (GtkTextBuffer     *buffer,
                       const GtkTextIter *iter,
                       GtkTextMark       *mark)
{
    XedDocument *doc = XED_DOCUMENT (buffer);

    if (GTK_TEXT_BUFFER_CLASS (xed_document_parent_class)->mark_set != NULL)
    {
        GTK_TEXT_BUFFER_CLASS (xed_document_parent_class)->mark_set (buffer, iter, mark);
    }

    if (mark == gtk_text_buffer_get_insert (buffer))
    {
        emit_cursor_moved (doc);
    }
}

static void
xed_document_changed (GtkTextBuffer *buffer)
{
    emit_cursor_moved (XED_DOCUMENT (buffer));

    GTK_TEXT_BUFFER_CLASS (xed_document_parent_class)->changed (buffer);
}

static void
xed_document_constructed (GObject *object)
{
    XedDocument *doc = XED_DOCUMENT (object);

    g_settings_bind (doc->priv->editor_settings,
                     XED_SETTINGS_ENSURE_TRAILING_NEWLINE,
                     doc,
                     "implicit-trailing-newline",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_NO_SENSITIVITY);

    G_OBJECT_CLASS (xed_document_parent_class)->constructed (object);
}

static void
xed_document_class_init (XedDocumentClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkTextBufferClass *buf_class = GTK_TEXT_BUFFER_CLASS (klass);

    object_class->dispose = xed_document_dispose;
    object_class->finalize = xed_document_finalize;
    object_class->get_property = xed_document_get_property;
    object_class->set_property = xed_document_set_property;
    object_class->constructed = xed_document_constructed;

    buf_class->mark_set = xed_document_mark_set;
    buf_class->changed = xed_document_changed;

    klass->loaded = xed_document_loaded_real;
    klass->saved = xed_document_saved_real;

    g_object_class_install_property (object_class, PROP_SHORTNAME,
                                     g_param_spec_string ("shortname",
                                                          "Short Name",
                                                          "The document's short name",
                                                          NULL,
                                                          G_PARAM_READWRITE |
                                                          G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (object_class, PROP_CONTENT_TYPE,
                                     g_param_spec_string ("content-type",
                                                          "Content Type",
                                                          "The document's Content Type",
                                                          NULL,
                                                          G_PARAM_READWRITE |
                                                          G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (object_class, PROP_MIME_TYPE,
                                     g_param_spec_string ("mime-type",
                                                          "MIME Type",
                                                          "The document's MIME Type",
                                                          "text/plain",
                                                          G_PARAM_READABLE |
                                                          G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (object_class, PROP_READ_ONLY,
                                     g_param_spec_boolean ("read-only",
                                                           "Read Only",
                                                           "Whether the document is read only or not",
                                                           FALSE,
                                                           G_PARAM_READABLE |
                                                           G_PARAM_STATIC_STRINGS));

    /* This signal is used to update the cursor position is the statusbar,
     * it's emitted either when the insert mark is moved explicitely or
     * when the buffer changes (insert/delete).
     * We prevent the emission of the signal during replace_all to
     * improve performance.
     */
    document_signals[CURSOR_MOVED] =
        g_signal_new ("cursor-moved",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedDocumentClass, cursor_moved),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE,
                      0);

    /**
     * XedDocument::load:
     * @document: the #XedDocument.
     *
     * The "load" signal is emitted at the beginning of file loading.
     */
    document_signals[LOAD] =
        g_signal_new ("load",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedDocumentClass, load),
                      NULL, NULL, NULL,
                      G_TYPE_NONE, 0);

    /**
     * XedDocument::loaded:
     * @document: the #XedDocument.
     *
     * The "loaded" signal is emitted at the end of a successful loading.
     */
    document_signals[LOADED] =
        g_signal_new ("loaded",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedDocumentClass, loaded),
                      NULL, NULL, NULL,
                      G_TYPE_NONE, 0);

    /**
     * XedDocument::save:
     * @document: the #XedDocument.
     *
     * The "save" signal is emitted at the beginning of file saving.
     */
    document_signals[SAVE] =
        g_signal_new ("save",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedDocumentClass, save),
                      NULL, NULL, NULL,
                      G_TYPE_NONE, 0);

    /**
     * XedDocument::saved:
     * @document: the #XedDocument.
     *
     * The "saved" signal is emitted at the end of a successful file saving.
     */
    document_signals[SAVED] =
        g_signal_new ("saved",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedDocumentClass, saved),
                      NULL, NULL, NULL,
                      G_TYPE_NONE, 0);

    g_type_class_add_private (object_class, sizeof (XedDocumentPrivate));
}

static void
set_language (XedDocument       *doc,
              GtkSourceLanguage *lang,
              gboolean           set_by_user)
{
    GtkSourceLanguage *old_lang;

    xed_debug (DEBUG_DOCUMENT);

    old_lang = gtk_source_buffer_get_language (GTK_SOURCE_BUFFER (doc));

    if (old_lang == lang)
    {
        return;
    }

    gtk_source_buffer_set_language (GTK_SOURCE_BUFFER (doc), lang);

    if (lang != NULL)
    {
        gboolean syntax_hl;

        syntax_hl = g_settings_get_boolean (doc->priv->editor_settings,
                                            XED_SETTINGS_SYNTAX_HIGHLIGHTING);
        gtk_source_buffer_set_highlight_syntax (GTK_SOURCE_BUFFER (doc), syntax_hl);
    }
    else
    {
        gtk_source_buffer_set_highlight_syntax (GTK_SOURCE_BUFFER (doc), FALSE);
    }

    if (set_by_user)
    {
        const gchar *language = get_language_string (doc);

        xed_document_set_metadata (doc, XED_METADATA_ATTRIBUTE_LANGUAGE, language, NULL);
    }

    doc->priv->language_set_by_user = set_by_user;
}

static void
save_encoding_metadata (XedDocument *doc)
{
    const GtkSourceEncoding *encoding;
    const gchar *charset;

    xed_debug (DEBUG_DOCUMENT);

    encoding = gtk_source_file_get_encoding (doc->priv->file);

    if (encoding == NULL)
    {
        encoding = gtk_source_encoding_get_utf8 ();
    }

    charset = gtk_source_encoding_get_charset (encoding);

    xed_document_set_metadata (doc, XED_METADATA_ATTRIBUTE_ENCODING, charset, NULL);
}

static GtkSourceStyleScheme *
get_default_style_scheme (GSettings *editor_settings)
{
    GtkSourceStyleSchemeManager *manager;
    gchar *scheme_id;
    GtkSourceStyleScheme *def_style;

    manager = gtk_source_style_scheme_manager_get_default ();
    scheme_id = g_settings_get_string (editor_settings, XED_SETTINGS_SCHEME);
    def_style = gtk_source_style_scheme_manager_get_scheme (manager, scheme_id);

    if (def_style == NULL)
    {
        g_warning ("Default style scheme '%s' cannot be found, falling back to 'classic' style scheme ", scheme_id);

        def_style = gtk_source_style_scheme_manager_get_scheme (manager, "classic");
        if (def_style == NULL)
        {
            g_warning ("Style scheme 'classic' cannot be found, check your GtkSourceView installation.");
        }
    }

    g_free (scheme_id);

    return def_style;
}

static GtkSourceLanguage *
guess_language (XedDocument *doc)
{
    gchar *data;
    GtkSourceLanguageManager *manager = gtk_source_language_manager_get_default ();
    GtkSourceLanguage *language = NULL;

    data = xed_document_get_metadata (doc, XED_METADATA_ATTRIBUTE_LANGUAGE);

    if (data != NULL)
    {
        xed_debug_message (DEBUG_DOCUMENT, "Language from metadata: %s", data);

        if (!g_str_equal (data, NO_LANGUAGE_NAME))
        {
            language = gtk_source_language_manager_get_language (manager, data);
        }

        g_free (data);
    }
    else
    {
        GFile *location;
        gchar *basename = NULL;

        location = gtk_source_file_get_location (doc->priv->file);
        xed_debug_message (DEBUG_DOCUMENT, "Sniffing Language");

        if (location != NULL)
        {
            basename = g_file_get_basename (location);
        }
        else if (doc->priv->short_name != NULL)
        {
            basename = g_strdup (doc->priv->short_name);
        }

        language = gtk_source_language_manager_guess_language (manager, basename, doc->priv->content_type);

        g_free (basename);
    }

    return language;
}

static void
on_content_type_changed (XedDocument *doc,
                         GParamSpec  *pspec,
                         gpointer     useless)
{
    if (!doc->priv->language_set_by_user)
    {
        GtkSourceLanguage *language = guess_language (doc);

        xed_debug_message (DEBUG_DOCUMENT, "Language: %s",
                           language != NULL ? gtk_source_language_get_name (language) : "None");

        set_language (doc, language, FALSE);
    }
}

static gchar *
get_default_content_type (void)
{
    return g_content_type_from_mime_type ("text/plain");
}

static void
on_location_changed (GtkSourceFile *file,
                     GParamSpec    *pspec,
                     XedDocument   *doc)
{
    GFile *location;

    xed_debug (DEBUG_DOCUMENT);

    location = gtk_source_file_get_location (file);

    if (location != NULL && doc->priv->untitled_number > 0)
    {
        release_untitled_number (doc->priv->untitled_number);
        doc->priv->untitled_number = 0;
    }

    if (doc->priv->short_name == NULL)
    {
        g_object_notify (G_OBJECT (doc), "shortname");
    }

#ifdef ENABLE_GVFS_METADATA

    /* load metadata for this location: we load sync since metadata is
    * always local so it should be fast and we need the information
    * right after the location was set.
    */
    if (location != NULL)
    {
        GError *error = NULL;

        if (doc->priv->metadata_info != NULL)
        {
            g_object_unref (doc->priv->metadata_info);
        }

        doc->priv->metadata_info = g_file_query_info (location,
                                                      METADATA_QUERY,
                                                      G_FILE_QUERY_INFO_NONE,
                                                      NULL,
                                                      &error);

        if (error != NULL)
        {
            /* TODO document why the warning is not displayed in
             * certain cases.
             */
            if (error->domain != G_FILE_ERROR ||
                (error->code != G_FILE_ERROR_ISDIR &&
                 error->code != G_FILE_ERROR_NOTDIR &&
                 error->code != G_FILE_ERROR_NOENT))
             {
                 g_warning ("%s", error->message);
             }

            g_error_free (error);
        }
    }
#endif
}

static void
xed_document_init (XedDocument *doc)
{
    XedDocumentPrivate *priv;
    GtkSourceStyleScheme *style_scheme;

    xed_debug (DEBUG_DOCUMENT);

    doc->priv = XED_DOCUMENT_GET_PRIVATE (doc);
    priv = doc->priv;

    priv->editor_settings = g_settings_new ("org.x.editor.preferences.editor");

    priv->untitled_number = get_untitled_number ();

    priv->content_type = get_default_content_type ();

    priv->readonly = FALSE;

    priv->stop_cursor_moved_emission = FALSE;

    priv->last_save_was_manually = TRUE;
    priv->language_set_by_user = FALSE;

    g_get_current_time (&doc->priv->time_of_last_save_or_load);

    priv->file = gtk_source_file_new ();

    g_signal_connect_object (priv->file, "notify::location",
                             G_CALLBACK (on_location_changed), doc, 0);

    g_settings_bind (priv->editor_settings,
                     XED_SETTINGS_MAX_UNDO_ACTIONS,
                     doc,
                     "max-undo-levels",
                     G_SETTINGS_BIND_GET);

    g_settings_bind (priv->editor_settings,
                     XED_SETTINGS_BRACKET_MATCHING,
                     doc,
                     "highlight-matching-brackets",
                     G_SETTINGS_BIND_GET);

    style_scheme = get_default_style_scheme (priv->editor_settings);
    if (style_scheme != NULL)
    {
        gtk_source_buffer_set_style_scheme (GTK_SOURCE_BUFFER (doc), style_scheme);
    }

    g_signal_connect (doc, "notify::content-type", G_CALLBACK (on_content_type_changed), NULL);
}

XedDocument *
xed_document_new (void)
{
    return g_object_new (XED_TYPE_DOCUMENT, NULL);
}

static void
set_content_type_no_guess (XedDocument *doc,
                           const gchar *content_type)
{
    xed_debug (DEBUG_DOCUMENT);

    if (doc->priv->content_type != NULL &&
        content_type != NULL &&
        g_str_equal (doc->priv->content_type, content_type))
    {
        return;
    }

    g_free (doc->priv->content_type);

    if (content_type == NULL || g_content_type_is_unknown (content_type))
    {
        doc->priv->content_type = get_default_content_type ();
    }
    else
    {
        doc->priv->content_type = g_strdup (content_type);
    }

    g_object_notify (G_OBJECT (doc), "content-type");
}

/**
 * xed_document_set_content_type:
 * @doc:
 * @content_type: (allow-none):
 */
void
xed_document_set_content_type (XedDocument *doc,
                               const gchar *content_type)
{
    g_return_if_fail (XED_IS_DOCUMENT (doc));

    xed_debug (DEBUG_DOCUMENT);

    if (content_type == NULL)
    {
        GFile *location;
        gchar *guessed_type = NULL;

        /* If content type is null, we guess from the filename */
        location = gtk_source_file_get_location (doc->priv->file);
        if (location != NULL)
        {
            gchar *basename;

            basename = g_file_get_basename (location);
            guessed_type = g_content_type_guess (basename, NULL, 0, NULL);

            g_free (basename);
        }

        set_content_type_no_guess (doc, guessed_type);
        g_free (guessed_type);
    }
    else
    {
        set_content_type_no_guess (doc, content_type);
    }
}

/**
 * xed_document_get_location:
 * @doc: a #XedDocument
 *
 * Returns: (allow-none) (transfer full): a new #GFile
 */
GFile *
xed_document_get_location (XedDocument *doc)
{
    GFile *location;

    g_return_val_if_fail (XED_IS_DOCUMENT (doc), NULL);

    location = gtk_source_file_get_location (doc->priv->file);

    return location != NULL ? g_object_ref (location) : NULL;
}

void
xed_document_set_location (XedDocument *doc,
                           GFile       *location)
{
    g_return_if_fail (XED_IS_DOCUMENT (doc));
    g_return_if_fail (G_IS_FILE (location));

    gtk_source_file_set_location (doc->priv->file, location);
    xed_document_set_content_type (doc, NULL);
}

/**
 * xed_document_get_uri_for_display:
 * @doc:
 *
 * Note: this never returns %NULL.
 **/
gchar *
xed_document_get_uri_for_display (XedDocument *doc)
{
    GFile *location;

    g_return_val_if_fail (XED_IS_DOCUMENT (doc), g_strdup (""));

    location = gtk_source_file_get_location (doc->priv->file);

    if (location == NULL)
    {
        return g_strdup_printf (_("Unsaved Document %d"), doc->priv->untitled_number);
    }
    else
    {
        return g_file_get_parse_name (location);
    }
}

/**
 * xed_document_get_short_name_for_display:
 * @doc:
 *
 * Note: this never returns %NULL.
 **/
gchar *
xed_document_get_short_name_for_display (XedDocument *doc)
{
    GFile *location;

    g_return_val_if_fail (XED_IS_DOCUMENT (doc), g_strdup (""));

    location = gtk_source_file_get_location (doc->priv->file);

    if (doc->priv->short_name != NULL)
    {
        return g_strdup (doc->priv->short_name);
    }
    else if (location == NULL)
    {
        return g_strdup_printf (_("Unsaved Document %d"), doc->priv->untitled_number);
    }
    else
    {
        return xed_utils_basename_for_display (location);
    }
}

/**
 * xed_document_set_short_name_for_display:
 * @doc:
 * @short_name: (allow-none):
 */
void
xed_document_set_short_name_for_display (XedDocument *doc,
                                         const gchar *short_name)
{
    g_return_if_fail (XED_IS_DOCUMENT (doc));

    g_free (doc->priv->short_name);
    doc->priv->short_name = g_strdup (short_name);

    g_object_notify (G_OBJECT (doc), "shortname");
}

gchar *
xed_document_get_content_type (XedDocument *doc)
{
    g_return_val_if_fail (XED_IS_DOCUMENT (doc), NULL);

    return g_strdup (doc->priv->content_type);
}

/**
 * xed_document_get_mime_type:
 * @doc:
 *
 * Note: this never returns %NULL.
 **/
gchar *
xed_document_get_mime_type (XedDocument *doc)
{
    g_return_val_if_fail (XED_IS_DOCUMENT (doc), g_strdup ("text/plain"));

    if (doc->priv->content_type != NULL &&
        !g_content_type_is_unknown (doc->priv->content_type))
    {
        return g_content_type_get_mime_type (doc->priv->content_type);
    }

    return g_strdup ("text/plain");
}

static void
set_readonly (XedDocument *doc,
              gboolean     readonly)
{
    xed_debug (DEBUG_DOCUMENT);

    g_return_if_fail (XED_IS_DOCUMENT (doc));

    readonly = readonly != FALSE;

    if (doc->priv->readonly != readonly)
    {
        doc->priv->readonly = readonly;
        g_object_notify (G_OBJECT (doc), "read-only");
    }
}

gboolean
xed_document_get_readonly (XedDocument *doc)
{
    g_return_val_if_fail (XED_IS_DOCUMENT (doc), TRUE);

    return doc->priv->readonly;
}

static void
loaded_query_info_cb (GFile        *location,
                      GAsyncResult *result,
                      XedDocument  *doc)
{
    GFileInfo *info;
    GError *error = NULL;

    info = g_file_query_info_finish (location, result, &error);

    if (error != NULL)
    {
        /* Ignore not found error as it can happen when opening a
         * non-existent file from the command line.
         */
        if (error->domain != G_IO_ERROR || error->code != G_IO_ERROR_NOT_FOUND)
        {
            g_warning ("Document loading: query info error: %s", error->message);
        }

        g_error_free (error);
        error = NULL;
    }

    if (info != NULL)
    {
        if (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE))
        {
            const gchar *content_type;

            content_type = g_file_info_get_attribute_string (info, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE);
            xed_document_set_content_type (doc, content_type);
        }

        if (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE))
        {
            gboolean read_only;

            read_only = !g_file_info_get_attribute_boolean (info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
            set_readonly (doc, read_only);
        }

        if (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_TIME_MODIFIED))
        {
            g_file_info_get_modification_time (info, &doc->priv->mtime);
            doc->priv->mtime_set = TRUE;
        }

        g_object_unref (info);
    }

    /* Async operation finished. */
    g_object_unref (doc);
}

static void
xed_document_loaded_real (XedDocument *doc)
{
   GFile *location;

    if (!doc->priv->language_set_by_user)
    {
        GtkSourceLanguage *language = guess_language (doc);

        xed_debug_message (DEBUG_DOCUMENT, "Language: %s",
                           language != NULL ? gtk_source_language_get_name (language) : "None");

        set_language (doc, language, FALSE);
    }

    doc->priv->mtime_set = FALSE;
    doc->priv->externally_modified = FALSE;
    doc->priv->deleted = FALSE;

    g_get_current_time (&doc->priv->time_of_last_save_or_load);

    set_readonly (doc, FALSE);

    xed_document_set_content_type (doc, NULL);

    location = gtk_source_file_get_location (doc->priv->file);

    if (location != NULL)
    {
        /* Keep the doc alive during the async operation. */
        g_object_ref (doc);

        g_file_query_info_async (location,
                                 G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE ","
                                 G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE ","
                                 G_FILE_ATTRIBUTE_TIME_MODIFIED,
                                 G_FILE_QUERY_INFO_NONE,
                                 G_PRIORITY_DEFAULT,
                                 NULL,
                                 (GAsyncReadyCallback) loaded_query_info_cb,
                                 doc);
    }
}

static void
saved_query_info_cb (GFile        *location,
                     GAsyncResult *result,
                     XedDocument  *doc)
{
    GFileInfo *info;
    const gchar *content_type = NULL;
    GError *error = NULL;

    info = g_file_query_info_finish (location, result, &error);

    if (error != NULL)
    {
        g_warning ("Document saving: query info error: %s", error->message);
        g_error_free (error);
        error = NULL;
    }

    doc->priv->mtime_set = FALSE;

    if (info != NULL)
    {
        if (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE))
        {
            content_type = g_file_info_get_attribute_string (info, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE);
        }

        if (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_TIME_MODIFIED))
        {
            g_file_info_get_modification_time (info, &doc->priv->mtime);
            doc->priv->mtime_set = TRUE;
        }
    }

    xed_document_set_content_type (doc, content_type);

    if (info != NULL)
    {
        g_object_unref (info);
    }

    g_get_current_time (&doc->priv->time_of_last_save_or_load);

    doc->priv->externally_modified = FALSE;
    doc->priv->deleted = FALSE;
    doc->priv->create = FALSE;

    set_readonly (doc, FALSE);

    gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (doc), FALSE);

    save_encoding_metadata (doc);

    /* Async operation finished. */
    g_object_unref (doc);
}

static void
xed_document_saved_real (XedDocument  *doc)
{
    GFile *location = gtk_source_file_get_location (doc->priv->file);

    /* Keep the doc alive during the async operation. */
    g_object_ref (doc);

    g_file_query_info_async (location,
                             G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE ","
                             G_FILE_ATTRIBUTE_TIME_MODIFIED,
                             G_FILE_QUERY_INFO_NONE,
                             G_PRIORITY_DEFAULT,
                             NULL,
                             (GAsyncReadyCallback) saved_query_info_cb,
                             doc);
}

gboolean
xed_document_is_untouched (XedDocument *doc)
{
    GFile *location;

    g_return_val_if_fail (XED_IS_DOCUMENT (doc), TRUE);

    location = gtk_source_file_get_location (doc->priv->file);

    return location == NULL && !gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (doc));
}

gboolean
xed_document_is_untitled (XedDocument *doc)
{
    g_return_val_if_fail (XED_IS_DOCUMENT (doc), TRUE);

    return gtk_source_file_get_location (doc->priv->file) == NULL;
}

gboolean
xed_document_is_local (XedDocument *doc)
{
    GFile *location;

    g_return_val_if_fail (XED_IS_DOCUMENT (doc), FALSE);

    location = gtk_source_file_get_location (doc->priv->file);

    if (location == NULL)
    {
        return FALSE;
    }

    return g_file_has_uri_scheme (location, "file");
}

static void
check_file_on_disk (XedDocument *doc)
{
    GFile *location;
    GFileInfo *info;

    location = gtk_source_file_get_location (doc->priv->file);

    if (location == NULL)
    {
        return;
    }

    info = g_file_query_info (location,
                              G_FILE_ATTRIBUTE_TIME_MODIFIED "," \
                              G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE,
                              G_FILE_QUERY_INFO_NONE,
                              NULL, NULL);

    if (info != NULL)
    {
        /* While at it also check if permissions changed */
        if (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE))
        {
            gboolean read_only;

            read_only = !g_file_info_get_attribute_boolean (info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);

            set_readonly (doc, read_only);
        }

        if (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_TIME_MODIFIED) && doc->priv->mtime_set)
        {
            GTimeVal timeval;

            g_file_info_get_modification_time (info, &timeval);

            /* Note that mtime can even go backwards if the
             * user is copying over a file with an old mtime
             */
            if (timeval.tv_sec != doc->priv->mtime.tv_sec ||
                timeval.tv_usec != doc->priv->mtime.tv_usec)
            {
                doc->priv->externally_modified = TRUE;
            }
        }

        g_object_unref (info);
    }
    else
    {
        doc->priv->deleted = TRUE;
    }
}

gboolean
_xed_document_check_externally_modified (XedDocument *doc)
{
    g_return_val_if_fail (XED_IS_DOCUMENT (doc), FALSE);

    if (!doc->priv->externally_modified)
    {
        check_file_on_disk (doc);
    }

    return doc->priv->externally_modified;
}

gboolean
xed_document_get_deleted (XedDocument *doc)
{
    g_return_val_if_fail (XED_IS_DOCUMENT (doc), FALSE);

    if (!doc->priv->deleted)
    {
        check_file_on_disk (doc);
    }

    return doc->priv->deleted;
}

/*
 * Deletion and external modification is only checked for local files.
 */
gboolean
_xed_document_needs_saving (XedDocument *doc)
{
    g_return_val_if_fail (XED_IS_DOCUMENT (doc), FALSE);

    if (gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (doc)))
    {
        return TRUE;
    }

    if (doc->priv->externally_modified || doc->priv->deleted)
    {
        return TRUE;
    }

    if (xed_document_is_local (doc))
    {
        check_file_on_disk (doc);

        if (doc->priv->externally_modified || doc->priv->deleted)
        {
        return TRUE;
        }
    }

    return FALSE;
}

/*
 * If @line is bigger than the lines of the document, the cursor is moved
 * to the last line and FALSE is returned.
 */
gboolean
xed_document_goto_line (XedDocument *doc,
                        gint         line)
{
    gboolean ret = TRUE;
    guint line_count;
    GtkTextIter iter;

    xed_debug (DEBUG_DOCUMENT);

    g_return_val_if_fail (XED_IS_DOCUMENT (doc), FALSE);
    g_return_val_if_fail (line >= -1, FALSE);

    line_count = gtk_text_buffer_get_line_count (GTK_TEXT_BUFFER (doc));

    if (line >= line_count)
    {
        ret = FALSE;
        gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (doc), &iter);
    }
    else
    {
        gtk_text_buffer_get_iter_at_line (GTK_TEXT_BUFFER (doc), &iter, line);
    }

    gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (doc), &iter);

    return ret;
}

gboolean
xed_document_goto_line_offset (XedDocument *doc,
                               gint         line,
                               gint         line_offset)
{
    gboolean ret = TRUE;
    guint offset_count;
    GtkTextIter iter;

    g_return_val_if_fail (XED_IS_DOCUMENT (doc), FALSE);
    g_return_val_if_fail (line >= -1, FALSE);
    g_return_val_if_fail (line_offset >= -1, FALSE);

    gtk_text_buffer_get_iter_at_line (GTK_TEXT_BUFFER (doc), &iter, line);

    offset_count = gtk_text_iter_get_chars_in_line (&iter);
    if (line_offset > offset_count)
    {
        ret = FALSE;
    }
    else
    {
        gtk_text_iter_set_line_offset (&iter, line_offset);
    }

    gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (doc), &iter);

    return ret;
}

/**
 * xed_document_set_language:
 * @doc:
 * @lang: (allow-none):
 **/
void
xed_document_set_language (XedDocument       *doc,
                           GtkSourceLanguage *lang)
{
    g_return_if_fail (XED_IS_DOCUMENT (doc));

    set_language (doc, lang, TRUE);
}

/**
 * xed_document_get_language:
 * @doc:
 *
 * Return value: (transfer none):
 */
GtkSourceLanguage *
xed_document_get_language (XedDocument *doc)
{
    g_return_val_if_fail (XED_IS_DOCUMENT (doc), NULL);

    return gtk_source_buffer_get_language (GTK_SOURCE_BUFFER (doc));
}

const GtkSourceEncoding *
xed_document_get_encoding (XedDocument *doc)
{
    g_return_val_if_fail (XED_IS_DOCUMENT (doc), NULL);

    return gtk_source_file_get_encoding (doc->priv->file);
}

glong
_xed_document_get_seconds_since_last_save_or_load (XedDocument *doc)
{
    GTimeVal current_time;

    xed_debug (DEBUG_DOCUMENT);

    g_return_val_if_fail (XED_IS_DOCUMENT (doc), -1);

    g_get_current_time (&current_time);

    return (current_time.tv_sec - doc->priv->time_of_last_save_or_load.tv_sec);
}

GtkSourceNewlineType
xed_document_get_newline_type (XedDocument *doc)
{
    g_return_val_if_fail (XED_IS_DOCUMENT (doc), 0);

    return gtk_source_file_get_newline_type (doc->priv->file);
}

#ifndef ENABLE_GVFS_METADATA
gchar *
xed_document_get_metadata (XedDocument *doc,
                           const gchar *key)
{
    GFile *location;

    g_return_val_if_fail (XED_IS_DOCUMENT (doc), NULL);
    g_return_val_if_fail (key != NULL, NULL);

    location = gtk_source_file_get_location (doc->priv->file);

    if (location != NULL)
    {
        return xed_metadata_manager_get (location, key);
    }

    return NULL;
}

void
xed_document_set_metadata (XedDocument *doc,
                           const gchar *first_key,
                           ...)
{
    GFile *location;
    const gchar *key;
    const gchar *value;
    va_list var_args;

    g_return_if_fail (XED_IS_DOCUMENT (doc));
    g_return_if_fail (first_key != NULL);

    location = gtk_source_file_get_location (doc->priv->file);

    if (location == NULL)
    {
        /* Can't set metadata for untitled documents */
        return;
    }

    va_start (var_args, first_key);

    for (key = first_key; key; key = va_arg (var_args, const gchar *))
    {
        value = va_arg (var_args, const gchar *);

        xed_metadata_manager_set (location, key, value);
    }

    va_end (var_args);
}

#else

/**
 * xed_document_get_metadata:
 * @doc: a #XedDocument
 * @key: name of the key
 *
 * Gets the metadata assigned to @key.
 *
 * Returns: the value assigned to @key.
 */
gchar *
xed_document_get_metadata (XedDocument *doc,
                           const gchar *key)
{
    g_return_val_if_fail (XED_IS_DOCUMENT (doc), NULL);
    g_return_val_if_fail (key != NULL, NULL);

    if (doc->priv->metadata_info && g_file_info_has_attribute (doc->priv->metadata_info, key))
    {
        return g_strdup (g_file_info_get_attribute_string (doc->priv->metadata_info, key));
    }

    return NULL;
}

static void
set_attributes_cb (GFile        *location,
                   GAsyncResult *result)
{
    GError *error = NULL;

    g_file_set_attributes_finish (location, result, NULL, &error);

    if (error != NULL)
    {
        g_warning ("Set document metadata failed: %s", error->message);
        g_error_free (error);
    }
}

/**
 * xed_document_set_metadata:
 * @doc: a #XedDocument
 * @first_key: name of the first key to set
 * @...: value for the first key, followed optionally by more key/value pairs,
 * followed by %NULL.
 *
 * Sets metadata on a document.
 */
void
xed_document_set_metadata (XedDocument *doc,
                           const gchar   *first_key,
                           ...)
{
    const gchar *key;
    const gchar *value;
    va_list var_args;
    GFileInfo *info;
    GFile *location;

    g_return_if_fail (XED_IS_DOCUMENT (doc));
    g_return_if_fail (first_key != NULL);

    info = g_file_info_new ();

    va_start (var_args, first_key);

    for (key = first_key; key; key = va_arg (var_args, const gchar *))
    {
        value = va_arg (var_args, const gchar *);

        if (value != NULL)
        {
            g_file_info_set_attribute_string (info, key, value);
        }
        else
        {
            /* Unset the key */
            g_file_info_remove_attribute (info, key);
        }
    }

    va_end (var_args);

    if (doc->priv->metadata_info != NULL)
    {
        g_file_info_copy_into (info, doc->priv->metadata_info);
    }

    location = gtk_source_file_get_location (doc->priv->file);

    if (location != NULL)
    {
        g_file_set_attributes_async (location,
                                     info,
                                     G_FILE_QUERY_INFO_NONE,
                                     G_PRIORITY_DEFAULT,
                                     NULL,
                                     (GAsyncReadyCallback) set_attributes_cb,
                                     NULL);
    }

    g_object_unref (info);
}
#endif

/**
 * xed_document_set_search_context:
 * @doc: a #XedDocument
 * @search_context: (allow-none): the new #GtkSourceSearchContext
 *
 * Sets the new search context for the document.
 */
void
xed_document_set_search_context (XedDocument            *doc,
                                 GtkSourceSearchContext *search_context)
{
   g_return_if_fail (XED_IS_DOCUMENT (doc));

   g_clear_object (&doc->priv->search_context);
   doc->priv->search_context = search_context;

   if (search_context != NULL)
   {
        gboolean highlight = g_settings_get_boolean (doc->priv->editor_settings, XED_SETTINGS_SEARCH_HIGHLIGHTING);

        gtk_source_search_context_set_highlight (search_context, highlight);

        g_object_ref (search_context);
   }
}

/**
 * xed_document_get_search_context:
 * @doc: a #XedDocument
 *
 * Returns: the current search context of the document,
 * or NULL if there is no search context
 */
GtkSourceSearchContext *
xed_document_get_search_context (XedDocument *doc)
{
    g_return_val_if_fail (XED_IS_DOCUMENT (doc), NULL);

    return doc->priv->search_context;
}

/**
 * xed_document_get_file:
 * @doc: a #XedDocument.
 *
 * Gets the associated #GtkSourceFile. You should use it only for reading
 * purposes, not for creating a #GtkSourceFileLoader or #GtkSourceFileSaver,
 * because xed does some extra work when loading or saving a file and
 * maintains an internal state. If you use in a plugin a file loader or saver on
 * the returned #GtkSourceFile, the internal state of xed won't be updated.
 *
 * If you want to save the #GeditDocument to a secondary file, you can create a
 * new #GtkSourceFile and use a #GtkSourceFileSaver.
 *
 * Returns: (transfer none): the associated #GtkSourceFile.
 */
GtkSourceFile *
xed_document_get_file (XedDocument *doc)
{
   g_return_val_if_fail (XED_IS_DOCUMENT (doc), NULL);

   return doc->priv->file;
}

void
_xed_document_set_create (XedDocument *doc,
                          gboolean     create)
{
    g_return_if_fail (XED_IS_DOCUMENT (doc));

    doc->priv->create = create != FALSE;
}

gboolean
_xed_document_get_create (XedDocument *doc)
{
   g_return_val_if_fail (XED_IS_DOCUMENT (doc), FALSE);

   return doc->priv->create;
}
