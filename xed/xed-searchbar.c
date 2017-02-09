#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "xed-searchbar.h"
#include "xed-statusbar.h"
#include "xed-history-entry.h"
#include "xed-utils.h"
#include "xed-marshal.h"
#include "xed-dirs.h"
#include "xed-commands.h"
#include "xed-window-private.h"

#define XED_SEARCHBAR_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), XED_TYPE_SEARCHBAR, XedSearchbarPrivate))

enum
{
    SHOW_REPLACE,
    LAST_SIGNAL
};

struct _XedSearchbarPrivate
{
    gboolean show_replace;

    GtkWidget *revealer;
    GtkWidget *grid;
    GtkWidget *search_label;
    GtkWidget *search_entry;
    GtkWidget *search_text_entry;
    GtkWidget *replace_label;
    GtkWidget *replace_entry;
    GtkWidget *replace_text_entry;
    GtkWidget *regex_checkbutton;
    GtkWidget *match_case_checkbutton;
    GtkWidget *entire_word_checkbutton;
    GtkWidget *wrap_around_checkbutton;
    GtkWidget *find_button;
    GtkWidget *find_prev_button;
    GtkWidget *replace_button;
    GtkWidget *replace_all_button;
    GtkWidget *close_button;

    GtkSourceSearchSettings *search_settings;
    XedSearchMode search_mode;

    guint update_occurrence_count_id;
};

G_DEFINE_TYPE(XedSearchbar, xed_searchbar, GTK_TYPE_BOX)

static void
xed_searchbar_dispose (GObject *object)
{
    XedSearchbar *searchbar = XED_SEARCHBAR (object);

    if (searchbar->priv->update_occurrence_count_id != 0)
    {
        g_source_remove (searchbar->priv->update_occurrence_count_id);
        searchbar->priv->update_occurrence_count_id = 0;
    }

    g_clear_object (&searchbar->priv->search_settings);

    G_OBJECT_CLASS (xed_searchbar_parent_class)->dispose (object);
}

static void
xed_searchbar_class_init (XedSearchbarClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = xed_searchbar_dispose;

    g_type_class_add_private (object_class, sizeof(XedSearchbarPrivate));
}

#define XED_SEARCHBAR_KEY "xed-searchbar-key"
#define MAX_MSG_LENGTH    40

/* Use occurrences only for Replace All */
static void
text_found (XedWindow *window,
            gint       occurrences)
{
    if (occurrences > 1)
    {
        xed_statusbar_flash_message (XED_STATUSBAR (window->priv->statusbar),
                                     window->priv->generic_message_cid,
                                     ngettext ("Found and replaced %d occurrence", "Found and replaced %d occurrences",
                                     occurrences),
                                     occurrences);
    }
    else
    {
        if (occurrences == 1)
        {
            xed_statusbar_flash_message (XED_STATUSBAR (window->priv->statusbar),
                                         window->priv->generic_message_cid,
                                         _("Found and replaced one occurrence"));
        }
        else
        {
            xed_statusbar_flash_message (XED_STATUSBAR (window->priv->statusbar),
                                         window->priv->generic_message_cid,
                                         " ");
        }
    }
}

static void
text_not_found (XedSearchbar *searchbar)
{
    const gchar *search_text;
    gchar *truncated_text;

    search_text = xed_searchbar_get_search_text (searchbar);
    truncated_text = xed_utils_str_end_truncate (search_text, MAX_MSG_LENGTH);

    xed_statusbar_flash_message (XED_STATUSBAR (searchbar->window->priv->statusbar),
                                 searchbar->window->priv->generic_message_cid,
                                 _("\"%s\" not found"), truncated_text);

    g_free (truncated_text);
}

static gboolean
forward_search_finished (GtkSourceSearchContext *search_context,
                         GAsyncResult           *result,
                         XedView                *view)
{
    gboolean found;
    GtkSourceBuffer *buffer;
    GtkTextIter match_start;
    GtkTextIter match_end;

    found = gtk_source_search_context_forward_finish (search_context, result, &match_start, &match_end, NULL);
    buffer = gtk_source_search_context_get_buffer (search_context);

    if (found)
    {
        gtk_text_buffer_select_range (GTK_TEXT_BUFFER (buffer), &match_start, &match_end);
        xed_view_scroll_to_cursor (view);
    }
    else
    {
        GtkTextIter end_selection;

        gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (buffer), NULL, &end_selection);
        gtk_text_buffer_select_range (GTK_TEXT_BUFFER (buffer), &end_selection, &end_selection);
    }

    return found;
}

static void
run_forward_search (XedWindow *window,
                    gboolean   jump_to_next_result)
{
    XedView *view;
    GtkTextBuffer *buffer;
    GtkTextIter start_at;
    GtkTextIter end_at;
    GtkSourceSearchContext *search_context;

    view = xed_window_get_active_view (window);

    if (view == NULL)
    {
        return;
    }

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    search_context = xed_document_get_search_context (XED_DOCUMENT (buffer));

    if (search_context == NULL)
    {
        return;
    }

    gtk_text_buffer_get_selection_bounds (buffer, &start_at, &end_at);

    if (jump_to_next_result)
    {
        gtk_source_search_context_forward_async (search_context,
                                                 &end_at,
                                                 NULL,
                                                 (GAsyncReadyCallback)forward_search_finished,
                                                 view);
    }
    else
    {
        gtk_source_search_context_forward_async (search_context,
                                                 &start_at,
                                                 NULL,
                                                 (GAsyncReadyCallback)forward_search_finished,
                                                 view);
    }
}

static gboolean
backward_search_finished (GtkSourceSearchContext *search_context,
                          GAsyncResult           *result,
                          XedView                *view)
{
    gboolean found;
    GtkTextIter match_start;
    GtkTextIter match_end;
    GtkSourceBuffer *buffer;

    found = gtk_source_search_context_backward_finish (search_context, result, &match_start, &match_end, NULL);
    buffer = gtk_source_search_context_get_buffer (search_context);

    if (found)
    {
        gtk_text_buffer_select_range (GTK_TEXT_BUFFER (buffer), &match_start, &match_end);
        xed_view_scroll_to_cursor (view);
    }
    else
    {
        GtkTextIter start_selection;

        gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (buffer), &start_selection, NULL);
        gtk_text_buffer_select_range (GTK_TEXT_BUFFER (buffer), &start_selection, &start_selection);
    }

    return found;
}

static void
run_backward_search (XedWindow *window)
{
    XedView *view;
    GtkTextBuffer *buffer;
    GtkTextIter start_at;
    GtkSourceSearchContext *search_context;

    view = xed_window_get_active_view (window);

    if (view == NULL)
    {
        return;
    }

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    search_context = xed_document_get_search_context (XED_DOCUMENT (buffer));

    if (search_context == NULL)
    {
        return;
    }

    gtk_text_buffer_get_selection_bounds (buffer, &start_at, NULL);
    gtk_source_search_context_backward_async (search_context,
                                              &start_at,
                                              NULL,
                                              (GAsyncReadyCallback)backward_search_finished,
                                              view);
}

static void
update_occurrence_count (XedSearchbar *searchbar)
{
    XedDocument *doc;
    GtkSourceSearchContext *search_context;
    GtkTextIter match_start;
    GtkTextIter match_end;
    gint count;
    gint pos;

    if (searchbar->priv->search_mode == XED_SEARCH_MODE_REPLACE)
    {
        return;
    }

    searchbar->priv->update_occurrence_count_id = 0;
    doc = xed_window_get_active_document (searchbar->window);
    search_context = xed_document_get_search_context (doc);

    if (search_context == NULL)
    {
        return;
    }

    count = gtk_source_search_context_get_occurrences_count (search_context);

    gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (doc), &match_start, &match_end);
    pos = gtk_source_search_context_get_occurrence_position (search_context, &match_start, &match_end);

    if (count == -1 || pos == -1)
    {
        /* Wait for the buffer to be fully scanned */
        return;
    }

    if (count == 0)
    {
        xed_statusbar_flash_message (XED_STATUSBAR (searchbar->window->priv->statusbar),
                                     searchbar->window->priv->generic_message_cid,
                                     _("No matches found"));
        return;
    }

    if (pos == 0)
    {
        xed_statusbar_flash_message (XED_STATUSBAR (searchbar->window->priv->statusbar),
                                     searchbar->window->priv->generic_message_cid,
                                     ngettext ("%d match", "%d matches", count), count);
        return;
    }

    xed_statusbar_flash_message (XED_STATUSBAR (searchbar->window->priv->statusbar),
                                 searchbar->window->priv->generic_message_cid,
                                 ngettext ("%d of %d match", "%d of %d matches",
                                 pos),
                                 pos, count);
}

static gboolean
update_occurrence_count_id_cb (XedSearchbar *searchbar)
{
    searchbar->priv->update_occurrence_count_id = 0;
    update_occurrence_count (searchbar);

    return G_SOURCE_REMOVE;
}

static void
install_occurrence_count_idle (XedSearchbar *searchbar)
{
    if (searchbar->priv->update_occurrence_count_id == 0)
    {
        searchbar->priv->update_occurrence_count_id = g_idle_add ((GSourceFunc)update_occurrence_count_id_cb, searchbar);
    }
}

static void
mark_set_cb (GtkTextBuffer *buffer,
             GtkTextIter   *location,
             GtkTextMark   *mark,
             XedSearchbar  *searchbar)
{
    GtkTextMark *insert;
    GtkTextMark *selection_bound;

    insert = gtk_text_buffer_get_insert (buffer);
    selection_bound = gtk_text_buffer_get_selection_bound (buffer);

    if (mark == insert || mark == selection_bound)
    {
        install_occurrence_count_idle (searchbar);
    }
}

static void
do_find (XedSearchbar *searchbar,
         gboolean      search_backwards,
         gboolean      jump_to_next_result)
{
    XedDocument *doc;
    GtkSourceSearchContext *search_context;
    GtkSourceSearchSettings *search_settings;

    search_settings = xed_searchbar_get_search_settings (searchbar);
    doc = xed_window_get_active_document (searchbar->window);
    search_context = xed_document_get_search_context (doc);
    searchbar->priv->search_mode = XED_SEARCH_MODE_SEARCH;

    if (search_context == NULL || search_settings != gtk_source_search_context_get_settings (search_context))
    {
        search_context = gtk_source_search_context_new (GTK_SOURCE_BUFFER (doc), search_settings);

        xed_document_set_search_context (doc, search_context);

        g_signal_connect (GTK_TEXT_BUFFER (doc), "mark-set",
                          G_CALLBACK (mark_set_cb), searchbar);

        g_signal_connect_swapped (search_context, "notify::occurrences-count",
                                  G_CALLBACK (install_occurrence_count_idle), searchbar);

        g_object_unref (search_context);
    }

    if (search_backwards)
    {
        run_backward_search (searchbar->window);
    }
    else
    {
        run_forward_search (searchbar->window, jump_to_next_result);
    }
}

void
xed_searchbar_find_again (XedSearchbar *searchbar,
                          gboolean      backward)
{
    if (backward)
    {
        do_find (searchbar, TRUE, TRUE);
    }
    else
    {
        do_find (searchbar, FALSE, TRUE);
    }
}

static void
search_buttons_set_sensitive (XedSearchbar *searchbar,
                              gboolean      sensitive)
{
    gtk_widget_set_sensitive (searchbar->priv->find_button, sensitive);
    gtk_widget_set_sensitive (searchbar->priv->find_prev_button, sensitive);
    gtk_widget_set_sensitive (searchbar->priv->replace_button, sensitive);
    gtk_widget_set_sensitive (searchbar->priv->replace_all_button, sensitive);
}

/* FIXME: move in xed-document.c and share it with xed-view */
static gboolean
get_selected_text (GtkTextBuffer  *doc,
                   gchar         **selected_text,
                   gint           *len)
{
    GtkTextIter start, end;

    g_return_val_if_fail (selected_text != NULL, FALSE);
    g_return_val_if_fail (*selected_text == NULL, FALSE);

    if (!gtk_text_buffer_get_selection_bounds (doc, &start, &end))
    {
        if (len != NULL)
        {
            len = 0;
        }
        return FALSE;
    }

    *selected_text = gtk_text_buffer_get_slice (doc, &start, &end, TRUE);

    if (len != NULL)
    {
        *len = g_utf8_strlen (*selected_text, -1);
    }

    return TRUE;
}

static void
do_replace (XedSearchbar *searchbar)
{
    XedDocument *doc;
    GtkSourceSearchContext *search_context;
    const gchar *replace_entry_text;
    gchar *unescaped_replace_text;
    GtkTextIter start;
    GtkTextIter end;

    doc = xed_window_get_active_document (searchbar->window);

    if (doc == NULL)
    {
        return;
    }

    search_context = xed_document_get_search_context (doc);

    if (search_context == NULL)
    {
        return;
    }

    /* replace text may be "", we just delete */
    replace_entry_text = xed_searchbar_get_replace_text (searchbar);
    g_return_if_fail ((replace_entry_text) != NULL);

    unescaped_replace_text = gtk_source_utils_unescape_search_text (replace_entry_text);
    gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (doc), &start, &end);
    searchbar->priv->search_mode = XED_SEARCH_MODE_REPLACE;

    gtk_source_search_context_replace (search_context,
                                       &start,
                                       &end,
                                       unescaped_replace_text,
                                       -1,
                                       NULL);

    g_free (unescaped_replace_text);

    do_find (searchbar, FALSE, TRUE);
}

static void
do_replace_all (XedSearchbar *searchbar)
{
    XedDocument *doc;
    GtkSourceSearchContext *search_context;
    const gchar *replace_entry_text;
    gchar *unescaped_replace_text;
    gint count;

    doc = xed_window_get_active_document (searchbar->window);

    if (doc == NULL)
    {
        return;
    }

    search_context = xed_document_get_search_context (doc);

    if (search_context == NULL)
    {
        return;
    }

    /* replace text may be "", we just delete all occurrences */
    replace_entry_text = xed_searchbar_get_replace_text (searchbar);
    g_return_if_fail ((replace_entry_text) != NULL);

    unescaped_replace_text = gtk_source_utils_unescape_search_text (replace_entry_text);
    count = gtk_source_search_context_replace_all (search_context, unescaped_replace_text, -1, NULL);
    searchbar->priv->search_mode = XED_SEARCH_MODE_REPLACE;

    g_free (unescaped_replace_text);

    if (count > 0)
    {
        text_found (searchbar->window, count);
    }
    else
    {
        text_not_found (searchbar);
    }

}

static void
search_text_entry_changed (GtkEditable  *editable,
                           XedSearchbar *searchbar)
{
    const gchar *search_string;

    search_string = gtk_entry_get_text (GTK_ENTRY (editable));
    g_return_if_fail (search_string != NULL);

    if (*search_string != '\0')
    {
        search_buttons_set_sensitive (searchbar, TRUE);
    }
    else
    {
        search_buttons_set_sensitive (searchbar, FALSE);
    }

    if (gtk_source_search_settings_get_regex_enabled (searchbar->priv->search_settings))
    {
        gtk_source_search_settings_set_search_text (searchbar->priv->search_settings, search_string);
    }
    else
    {
        gchar *unescaped_search_string;

        unescaped_search_string = gtk_source_utils_unescape_search_text (search_string);
        gtk_source_search_settings_set_search_text (searchbar->priv->search_settings, unescaped_search_string);

        g_free (unescaped_search_string);
    }

    do_find (searchbar, FALSE, FALSE);
}

static void
remember_search_entry (XedSearchbar *searchbar)
{
    const gchar *str;

    str = gtk_entry_get_text (GTK_ENTRY(searchbar->priv->search_text_entry));
    if (*str != '\0')
    {
        xed_history_entry_prepend_text (XED_HISTORY_ENTRY (searchbar->priv->search_entry), str);
    }
}

static void
remember_replace_entry (XedSearchbar *searchbar)
{
    const gchar *str;

    str = gtk_entry_get_text (GTK_ENTRY(searchbar->priv->replace_text_entry));
    if (*str != '\0')
    {
        xed_history_entry_prepend_text (XED_HISTORY_ENTRY(searchbar->priv->replace_entry), str);
    }
}

static void
find_button_clicked_callback (GtkWidget    *button,
                              XedSearchbar *searchbar)
{
    remember_search_entry (searchbar);
    do_find (searchbar, FALSE, TRUE);
}

static void
toggle_button_clicked_callback (GtkWidget    *button,
                                XedSearchbar *searchbar)
{
    remember_search_entry (searchbar);
    do_find (searchbar, FALSE, FALSE);
}

static void
find_prev_button_clicked_callback (GtkWidget    *button,
                                   XedSearchbar *searchbar)
{
    remember_search_entry (searchbar);
    do_find (searchbar, TRUE, TRUE);
}

static void
replace_button_clicked_callback (GtkWidget    *button,
                                 XedSearchbar *searchbar)
{
    remember_search_entry (searchbar);
    remember_replace_entry (searchbar);
    do_replace (searchbar);
}

static void
replace_all_button_clicked_callback (GtkWidget    *button,
                                     XedSearchbar *searchbar)
{
    remember_search_entry (searchbar);
    remember_replace_entry (searchbar);
    do_replace_all (searchbar);
}

static void
on_search_text_entry_activated (GtkEntry     *widget,
                                XedSearchbar *searchbar)
{
    remember_search_entry (searchbar);
    do_find (searchbar, FALSE, TRUE);
}

static void
close_button_clicked_callback (GtkWidget    *button,
                               XedSearchbar *searchbar)
{
    xed_searchbar_hide (searchbar);
}

static void
xed_searchbar_init (XedSearchbar *searchbar)
{
    GtkWidget *content;
    GtkSizeGroup *size_group;
    GtkStyleContext *context;
    GtkCssProvider *provider;
    GtkBuilder *builder;
    gchar *root_objects[] = { "searchbar_content", NULL };
    const gchar *data = ".button {padding: 0;}";

    searchbar->priv = XED_SEARCHBAR_GET_PRIVATE (searchbar);

    builder = gtk_builder_new ();
    gtk_builder_add_objects_from_resource (builder, "/org/x/editor/ui/xed-searchbar.ui", root_objects, NULL);
    content = GTK_WIDGET (gtk_builder_get_object (builder, "searchbar_content"));
    g_object_ref (content);
    searchbar->priv->revealer = GTK_WIDGET (gtk_builder_get_object (builder, "revealer"));
    searchbar->priv->grid = GTK_WIDGET (gtk_builder_get_object (builder, "grid"));
    searchbar->priv->search_label = GTK_WIDGET (gtk_builder_get_object (builder, "search_label"));
    searchbar->priv->replace_label = GTK_WIDGET (gtk_builder_get_object (builder, "replace_with_label"));
    searchbar->priv->regex_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "regex_checkbutton"));
    searchbar->priv->match_case_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "match_case_checkbutton"));
    searchbar->priv->entire_word_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "entire_word_checkbutton"));
    searchbar->priv->wrap_around_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "wrap_around_checkbutton"));
    searchbar->priv->find_button = GTK_WIDGET (gtk_builder_get_object (builder, "find_button"));
    searchbar->priv->find_prev_button = GTK_WIDGET (gtk_builder_get_object (builder, "find_prev_button"));
    searchbar->priv->replace_button = GTK_WIDGET (gtk_builder_get_object (builder, "replace_button"));
    searchbar->priv->replace_all_button = GTK_WIDGET (gtk_builder_get_object (builder, "replace_all_button"));
    searchbar->priv->close_button = GTK_WIDGET (gtk_builder_get_object (builder, "close_button"));
    g_object_unref (builder);

    gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET (searchbar)), "xed-searchbar");

    searchbar->priv->search_entry = xed_history_entry_new ("history-search-for", FALSE);
    gtk_widget_set_hexpand (searchbar->priv->search_entry, TRUE);

    searchbar->priv->search_text_entry = xed_history_entry_get_entry (XED_HISTORY_ENTRY (searchbar->priv->search_entry));
    gtk_entry_set_activates_default (GTK_ENTRY (searchbar->priv->search_text_entry), TRUE);

    gtk_widget_show (searchbar->priv->search_entry);
    gtk_grid_attach (GTK_GRID (searchbar->priv->grid), searchbar->priv->search_entry, 2, 0, 1, 1);

    searchbar->priv->replace_entry = xed_history_entry_new ("history-replace-with", FALSE);

    searchbar->priv->replace_text_entry = xed_history_entry_get_entry (
                    XED_HISTORY_ENTRY (searchbar->priv->replace_entry));
    gtk_entry_set_activates_default (GTK_ENTRY (searchbar->priv->replace_text_entry), TRUE);

    gtk_widget_show (searchbar->priv->replace_entry);
    gtk_grid_attach (GTK_GRID (searchbar->priv->grid), searchbar->priv->replace_entry, 2, 1, 1, 1);

    gtk_label_set_mnemonic_widget (GTK_LABEL (searchbar->priv->search_label), searchbar->priv->search_entry);
    gtk_label_set_mnemonic_widget (GTK_LABEL (searchbar->priv->replace_label), searchbar->priv->replace_entry);

    provider = gtk_css_provider_new ();
    context = gtk_widget_get_style_context (searchbar->priv->close_button);
    gtk_css_provider_load_from_data (provider, data, -1, NULL);
    gtk_style_context_add_provider (context, GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
    gtk_size_group_add_widget (size_group, GTK_WIDGET (searchbar->priv->find_button));
    gtk_size_group_add_widget (size_group, GTK_WIDGET (searchbar->priv->find_prev_button));
    gtk_size_group_add_widget (size_group, GTK_WIDGET (searchbar->priv->replace_button));
    gtk_size_group_add_widget (size_group, GTK_WIDGET (searchbar->priv->replace_all_button));

    /* insensitive by default */
    search_buttons_set_sensitive (searchbar, FALSE);

    xed_searchbar_hide (searchbar);

    gtk_box_pack_start (GTK_BOX (searchbar), content, TRUE, TRUE, 0);
    gtk_widget_show (GTK_WIDGET (searchbar));

    g_object_unref (content);

    g_signal_connect (searchbar->priv->search_text_entry, "changed",
                      G_CALLBACK (search_text_entry_changed), searchbar);

    g_signal_connect (searchbar->priv->search_text_entry, "activate",
                      G_CALLBACK (on_search_text_entry_activated), searchbar);

    g_signal_connect (searchbar->priv->find_button, "clicked",
                      G_CALLBACK (find_button_clicked_callback), searchbar);

    g_signal_connect (searchbar->priv->find_prev_button, "clicked",
                      G_CALLBACK (find_prev_button_clicked_callback), searchbar);

    g_signal_connect (searchbar->priv->replace_button, "clicked",
                      G_CALLBACK (replace_button_clicked_callback), searchbar);

    g_signal_connect (searchbar->priv->replace_all_button, "clicked",
                      G_CALLBACK (replace_all_button_clicked_callback), searchbar);

    g_signal_connect (searchbar->priv->close_button, "clicked",
                      G_CALLBACK (close_button_clicked_callback), searchbar);

    // Start a search when match-case or entire-word buttons are clicked
    g_signal_connect (searchbar->priv->entire_word_checkbutton, "clicked",
                      G_CALLBACK (toggle_button_clicked_callback), searchbar);

    g_signal_connect (searchbar->priv->match_case_checkbutton, "clicked",
                      G_CALLBACK (toggle_button_clicked_callback), searchbar);

    searchbar->priv->search_settings = gtk_source_search_settings_new ();

    g_object_bind_property (searchbar->priv->regex_checkbutton, "active",
                            searchbar->priv->search_settings, "regex-enabled",
                            G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

    g_object_bind_property (searchbar->priv->match_case_checkbutton, "active",
                            searchbar->priv->search_settings, "case-sensitive",
                            G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

    g_object_bind_property (searchbar->priv->entire_word_checkbutton, "active",
                            searchbar->priv->search_settings, "at-word-boundaries",
                            G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

    g_object_bind_property (searchbar->priv->wrap_around_checkbutton, "active",
                            searchbar->priv->search_settings, "wrap-around",
                            G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
}

GtkWidget *
xed_searchbar_new (GtkWindow *parent)
{
    XedSearchbar *searchbar;

    searchbar = g_object_new (XED_TYPE_SEARCHBAR, NULL);
    searchbar->window = XED_WINDOW (parent);

    return GTK_WIDGET (searchbar);
}

void
xed_searchbar_show (XedSearchbar  *searchbar,
                    XedSearchMode  search_mode)
{
    XedDocument *doc;
    gboolean selection_exists;
    gchar *find_text = NULL;
    gint sel_len = 0;

    doc = xed_window_get_active_document (searchbar->window);
    g_return_if_fail (doc != NULL);

    selection_exists = get_selected_text (GTK_TEXT_BUFFER (doc), &find_text, &sel_len);

    if (selection_exists && find_text != NULL && sel_len < 80)
    {
        gchar *escaped_find_text;

        if (gtk_source_search_settings_get_regex_enabled (searchbar->priv->search_settings))
        {
            escaped_find_text = g_regex_escape_string (find_text, -1);
        }
        else
        {
            escaped_find_text = gtk_source_utils_escape_search_text (find_text);
        }

        xed_searchbar_set_search_text (XED_SEARCHBAR (searchbar), escaped_find_text);

        g_free (escaped_find_text);
    }

    g_free (find_text);

    gtk_revealer_set_transition_type (GTK_REVEALER (searchbar->priv->revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_UP);
    gtk_revealer_set_reveal_child (GTK_REVEALER (searchbar->priv->revealer), TRUE);

    if (search_mode == XED_SEARCH_MODE_REPLACE)
    {
        gtk_widget_show (searchbar->priv->replace_label);
        gtk_widget_show (searchbar->priv->replace_entry);
        gtk_widget_show (searchbar->priv->replace_all_button);
        gtk_widget_show (searchbar->priv->replace_button);
        gtk_grid_set_row_spacing (GTK_GRID (searchbar->priv->grid), 10);
    }
    else
    {
        gtk_widget_hide (searchbar->priv->replace_label);
        gtk_widget_hide (searchbar->priv->replace_entry);
        gtk_widget_hide (searchbar->priv->replace_all_button);
        gtk_widget_hide (searchbar->priv->replace_button);
        gtk_grid_set_row_spacing (GTK_GRID (searchbar->priv->grid), 0);
    }

    gtk_widget_show (searchbar->priv->find_button);
    gtk_widget_grab_focus (searchbar->priv->search_text_entry);
}

void
xed_searchbar_hide (XedSearchbar *searchbar)
{
    XedView *active_view;

    gtk_revealer_set_transition_type (GTK_REVEALER (searchbar->priv->revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
    gtk_revealer_set_reveal_child (GTK_REVEALER (searchbar->priv->revealer), FALSE);

    // focus document
    active_view = xed_window_get_active_view (searchbar->window);

    if (active_view != NULL)
    {
        gtk_widget_grab_focus (GTK_WIDGET (active_view));
    }

    // remove highlighting
    _xed_cmd_search_clear_highlight (searchbar->window);
}

const gchar *
xed_searchbar_get_replace_text (XedSearchbar *searchbar)
{
    g_return_val_if_fail (XED_IS_SEARCHBAR (searchbar), NULL);

    return gtk_entry_get_text (GTK_ENTRY (searchbar->priv->replace_text_entry));
}

GtkSourceSearchSettings *
xed_searchbar_get_search_settings (XedSearchbar *searchbar)
{
    g_return_val_if_fail (XED_IS_SEARCHBAR (searchbar), NULL);

    return searchbar->priv->search_settings;
}

const gchar *
xed_searchbar_get_search_text (XedSearchbar *searchbar)
{
    g_return_val_if_fail (XED_IS_SEARCHBAR (searchbar), NULL);

    return gtk_entry_get_text (GTK_ENTRY (searchbar->priv->search_text_entry));
}

void
xed_searchbar_set_search_text (XedSearchbar *searchbar,
                               const gchar  *search_text)
{
    g_return_if_fail (XED_IS_SEARCHBAR (searchbar));

    gtk_entry_set_text (GTK_ENTRY (searchbar->priv->search_text_entry), search_text);
}
