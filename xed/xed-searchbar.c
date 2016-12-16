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

/* Signals */
enum
{
    SHOW_REPLACE, LAST_SIGNAL
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
    GtkWidget *match_case_checkbutton;
    GtkWidget *entire_word_checkbutton;
    GtkWidget *wrap_around_checkbutton;
    GtkWidget *find_button;
    GtkWidget *find_prev_button;
    GtkWidget *replace_button;
    GtkWidget *replace_all_button;
    GtkWidget *close_button;
};

G_DEFINE_TYPE(XedSearchbar, xed_searchbar, GTK_TYPE_BOX)

static void
xed_searchbar_class_init (XedSearchbarClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkBindingSet *binding_set;
    g_type_class_add_private (object_class, sizeof(XedSearchbarPrivate));
    binding_set = gtk_binding_set_by_class (klass);
}

#define XED_SEARCHBAR_KEY "xed-searchbar-key"
#define MAX_MSG_LENGTH    40

/* Use occurrences only for Replace All */
static void
text_found (XedWindow *window,
            gint occurrences)
{
    if (occurrences > 1)
    {
        xed_statusbar_flash_message (XED_STATUSBAR(window->priv->statusbar), window->priv->generic_message_cid,
                        ngettext ("Found and replaced %d occurrence", "Found and replaced %d occurrences", occurrences),
                        occurrences);
    }
    else
    {
        if (occurrences == 1)
        {
            xed_statusbar_flash_message (XED_STATUSBAR(window->priv->statusbar), window->priv->generic_message_cid,
                                         _("Found and replaced one occurrence"));
        }
        else
        {
            xed_statusbar_flash_message (XED_STATUSBAR(window->priv->statusbar), window->priv->generic_message_cid,
                                         " ");
        }
    }
}

static void
text_not_found (XedWindow *window,
                const gchar *text)
{
    gchar *searched;

    searched = xed_utils_str_end_truncate (text, MAX_MSG_LENGTH);
    xed_statusbar_flash_message (XED_STATUSBAR(window->priv->statusbar), window->priv->generic_message_cid,
                                 _("\"%s\" not found"), searched);
    g_free (searched);
}

static gboolean
run_search (XedView *view,
            gboolean wrap_around,
            gboolean search_backwards,
            gboolean jump_to_next_result)
{
    XedDocument *doc;
    GtkTextIter start_iter;
    GtkTextIter end_iter;
    GtkTextIter match_start;
    GtkTextIter match_end;
    gboolean found = FALSE;

    doc = XED_DOCUMENT(gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)));
    gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER(doc), &start_iter, &end_iter);

    if (!search_backwards)
    {
        if (jump_to_next_result) {
            found = xed_document_search_forward (doc, &end_iter, NULL, &match_start, &match_end);
        }
        else {
            found = xed_document_search_forward (doc, &start_iter, NULL, &match_start, &match_end);
        }
    }
    else
    {
        found = xed_document_search_backward (doc, NULL, &start_iter, &match_start, &match_end);
    }

    if (!found && wrap_around)
    {
        if (!search_backwards)
        {
            /* FIXME: set the end_inter */
            found = xed_document_search_forward (doc, NULL, NULL, &match_start, &match_end);
        }
        else
        {
            /* FIXME: set the start_inter */
            found = xed_document_search_backward (doc, NULL, NULL, &match_start, &match_end);
        }
    }

    if (found)
    {
        gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER(doc), &match_start);
        gtk_text_buffer_move_mark_by_name (GTK_TEXT_BUFFER(doc), "selection_bound", &match_end);
        xed_view_scroll_to_cursor (view);
    }
    else
    {
        gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER(doc), &start_iter);
    }

    return found;
}

static void
do_find (XedSearchbar *searchbar,
         gboolean search_backwards,
         gboolean jump_to_next_result)
{
    XedView *active_view;
    XedDocument *doc;
    gchar *search_text;
    const gchar *entry_text;
    gboolean match_case;
    gboolean entire_word;
    gboolean wrap_around;
    guint flags = 0;
    guint old_flags = 0;
    gboolean found;

    /* TODO: make the searchbar insensitive when all the tabs are closed
     * and assert here that the view is not NULL */
    active_view = xed_window_get_active_view (searchbar->window);
    if (active_view == NULL)
    {
        return;
    }

    doc = XED_DOCUMENT(gtk_text_view_get_buffer (GTK_TEXT_VIEW (active_view)));

    match_case = xed_searchbar_get_match_case (searchbar);
    entire_word = xed_searchbar_get_entire_word (searchbar);
    wrap_around = xed_searchbar_get_wrap_around (searchbar);
    entry_text = xed_searchbar_get_search_text (searchbar);

    XED_SEARCH_SET_CASE_SENSITIVE(flags, match_case);
    XED_SEARCH_SET_ENTIRE_WORD(flags, entire_word);

    search_text = xed_document_get_search_text (doc, &old_flags);

    if ((search_text == NULL) || (strcmp (search_text, entry_text) != 0) || (flags != old_flags))
    {
        xed_document_set_search_text (doc, entry_text, flags);
    }

    g_free (search_text);

    found = run_search (active_view, wrap_around, search_backwards, jump_to_next_result);

    if (found)
    {
        text_found (searchbar->window, 0);
    }
    else
    {
        text_not_found (searchbar->window, entry_text);
    }
}

void
xed_searchbar_find_again (XedSearchbar *searchbar,
                          gboolean backward)
{
    XedView *active_view;
    gboolean wrap_around = TRUE;
    gpointer data;

    active_view = xed_window_get_active_view (searchbar->window);
    g_return_if_fail(active_view != NULL);

    data = g_object_get_data (G_OBJECT(searchbar->window), XED_SEARCHBAR_KEY);

    if (data != NULL)
    {
        wrap_around = xed_searchbar_get_wrap_around (XED_SEARCHBAR(data));
    }

    run_search (active_view, wrap_around, backward, TRUE);
}

static void
search_buttons_set_sensitive (XedSearchbar *searchbar,
                              gboolean sensitive)
{
    gtk_widget_set_sensitive (searchbar->priv->find_button, sensitive);
    gtk_widget_set_sensitive (searchbar->priv->find_prev_button, sensitive);
    gtk_widget_set_sensitive (searchbar->priv->replace_button, sensitive);
    gtk_widget_set_sensitive (searchbar->priv->replace_all_button, sensitive);
}

/* FIXME: move in xed-document.c and share it with xed-view */
static gboolean
get_selected_text (GtkTextBuffer *doc,
                   gchar **selected_text,
                   gint *len)
{
    GtkTextIter start, end;

    g_return_val_if_fail(selected_text != NULL, FALSE);
    g_return_val_if_fail(*selected_text == NULL, FALSE);

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
replace_selected_text (GtkTextBuffer *buffer,
                       const gchar *replace)
{
    g_return_if_fail(gtk_text_buffer_get_selection_bounds (buffer, NULL, NULL));
    g_return_if_fail(replace != NULL);

    gtk_text_buffer_begin_user_action (buffer);
    gtk_text_buffer_delete_selection (buffer, FALSE, TRUE);
    gtk_text_buffer_insert_at_cursor (buffer, replace, strlen (replace));
    gtk_text_buffer_end_user_action (buffer);
}

static void
do_replace (XedSearchbar *searchbar)
{
    XedDocument *doc;
    const gchar *search_entry_text;
    const gchar *replace_entry_text;
    gchar *unescaped_search_text;
    gchar *unescaped_replace_text;
    gchar *selected_text = NULL;
    gboolean match_case;

    doc = xed_window_get_active_document (searchbar->window);
    if (doc == NULL)
    {
        return;
    }

    search_entry_text = xed_searchbar_get_search_text (searchbar);
    g_return_if_fail((search_entry_text) != NULL);
    g_return_if_fail((*search_entry_text) != '\0');

    /* replace text may be "", we just delete */
    replace_entry_text = xed_searchbar_get_replace_text (searchbar);
    g_return_if_fail((replace_entry_text) != NULL);

    unescaped_search_text = xed_utils_unescape_search_text (search_entry_text);

    get_selected_text (GTK_TEXT_BUFFER(doc), &selected_text, NULL);

    match_case = xed_searchbar_get_match_case (searchbar);

    if ((selected_text == NULL)
        || (match_case && (strcmp (selected_text, unescaped_search_text) != 0))
        || (!match_case && !g_utf8_caselessnmatch (selected_text,
                                   unescaped_search_text,
                                   strlen (selected_text),
                                   strlen (unescaped_search_text)) != 0))
    {
        do_find (searchbar, FALSE, TRUE );
        g_free (unescaped_search_text);
        g_free (selected_text);

        return;
    }

    unescaped_replace_text = xed_utils_unescape_search_text (replace_entry_text);
    replace_selected_text (GTK_TEXT_BUFFER(doc), unescaped_replace_text);

    g_free (unescaped_search_text);
    g_free (selected_text);
    g_free (unescaped_replace_text);

    do_find (searchbar, FALSE, TRUE);
}

static void
do_replace_all (XedSearchbar *searchbar)
{
    XedView *active_view;
    XedDocument *doc;
    const gchar *search_entry_text;
    const gchar *replace_entry_text;
    gboolean match_case;
    gboolean entire_word;
    guint flags = 0;
    gint count;

    active_view = xed_window_get_active_view (searchbar->window);
    if (active_view == NULL)
    {
        return;
    }

    doc = XED_DOCUMENT(gtk_text_view_get_buffer (GTK_TEXT_VIEW (active_view)));

    search_entry_text = xed_searchbar_get_search_text (searchbar);
    g_return_if_fail((search_entry_text) != NULL);
    g_return_if_fail((*search_entry_text) != '\0');

    /* replace text may be "", we just delete all occurrences */
    replace_entry_text = xed_searchbar_get_replace_text (searchbar);
    g_return_if_fail((replace_entry_text) != NULL);

    match_case = xed_searchbar_get_match_case (searchbar);
    entire_word = xed_searchbar_get_entire_word (searchbar);

    XED_SEARCH_SET_CASE_SENSITIVE(flags, match_case);
    XED_SEARCH_SET_ENTIRE_WORD(flags, entire_word);

    count = xed_document_replace_all (doc, search_entry_text, replace_entry_text, flags);

    if (count > 0)
    {
        text_found (searchbar->window, count);
    }
    else
    {
        text_not_found (searchbar->window, search_entry_text);
    }

}

static void
insert_text_handler (GtkEditable *editable,
                     const gchar *text,
                     gint length,
                     gint *position,
                     gpointer data)
{
    static gboolean insert_text = FALSE;
    gchar *escaped_text;
    gint new_len;

    /* To avoid recursive behavior */
    if (insert_text)
    {
        return;
    }

    escaped_text = xed_utils_escape_search_text (text);

    new_len = strlen (escaped_text);

    if (new_len == length)
    {
        g_free (escaped_text);
        return;
    }

    insert_text = TRUE;

    g_signal_stop_emission_by_name (editable, "insert_text");

    gtk_editable_insert_text (editable, escaped_text, new_len, position);

    insert_text = FALSE;

    g_free (escaped_text);
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
        do_find (searchbar, FALSE, FALSE);
    }
    else
    {
        search_buttons_set_sensitive (searchbar, FALSE);
        do_find (searchbar, FALSE, FALSE);
    }
}

static void
remember_search_entry (XedSearchbar *searchbar)
{
    const gchar *str;
    str = gtk_entry_get_text (GTK_ENTRY(searchbar->priv->search_text_entry));
    if (*str != '\0')
    {
        gchar *text;
        text = xed_utils_unescape_search_text (str);
        xed_history_entry_prepend_text (XED_HISTORY_ENTRY(searchbar->priv->search_entry), text);
        g_free (text);
    }
}

static void
remember_replace_entry (XedSearchbar *searchbar)
{
    const gchar *str;
    str = gtk_entry_get_text (GTK_ENTRY(searchbar->priv->replace_text_entry));
    if (*str != '\0')
    {
        gchar *text;
        text = xed_utils_unescape_search_text (str);
        xed_history_entry_prepend_text (XED_HISTORY_ENTRY(searchbar->priv->replace_entry), text);
        g_free (text);
    }
}

static void
find_button_clicked_callback (GtkWidget *button,
                              XedSearchbar *searchbar)
{
    remember_search_entry (searchbar);
    do_find (searchbar, FALSE, TRUE);
}

static void
toggle_button_clicked_callback (GtkWidget *button,
                              XedSearchbar *searchbar)
{
    remember_search_entry (searchbar);
    do_find (searchbar, FALSE, FALSE);
}

static void
find_prev_button_clicked_callback (GtkWidget *button,
                                   XedSearchbar *searchbar)
{
    remember_search_entry (searchbar);
    do_find (searchbar, TRUE, TRUE);
}

static void
replace_button_clicked_callback (GtkWidget *button,
                                 XedSearchbar *searchbar)
{
    remember_search_entry (searchbar);
    remember_replace_entry (searchbar);
    do_replace (searchbar);
}

static void
replace_all_button_clicked_callback (GtkWidget *button,
                                     XedSearchbar *searchbar)
{
    remember_search_entry (searchbar);
    remember_replace_entry (searchbar);
    do_replace_all (searchbar);
}

static void
on_search_text_entry_activated (GtkEntry *widget,
                                XedSearchbar *searchbar)
{
    remember_search_entry (searchbar);
    do_find (searchbar, FALSE, TRUE);
}

static void
close_button_clicked_callback (GtkWidget *button,
                               XedSearchbar *searchbar)
{
    xed_searchbar_hide (searchbar);
}

static void
xed_searchbar_init (XedSearchbar *searchbar)
{
    GtkWidget *content;
    GtkSizeGroup *size_group;
    GtkWidget *error_widget;
    GtkStyleContext *context;
    GtkCssProvider *provider;
    gchar *file;
    gchar *root_objects[] = { "searchbar_content", NULL };
    const gchar *data = ".button {padding: 0;}";

    searchbar->priv = XED_SEARCHBAR_GET_PRIVATE (searchbar);

    file = xed_dirs_get_ui_file ("xed-searchbar.ui");
    xed_utils_get_ui_objects (file,
                              root_objects,
                              &error_widget,
                              "searchbar_content", &content,
                              "revealer", &searchbar->priv->revealer,
                              "grid", &searchbar->priv->grid,
                              "search_label", &searchbar->priv->search_label,
                              "replace_with_label", &searchbar->priv->replace_label,
                              "match_case_checkbutton", &searchbar->priv->match_case_checkbutton,
                              "entire_word_checkbutton", &searchbar->priv->entire_word_checkbutton,
                              "wrap_around_checkbutton", &searchbar->priv->wrap_around_checkbutton,
                              "find_button", &searchbar->priv->find_button,
                              "find_prev_button", &searchbar->priv->find_prev_button,
                              "replace_button", &searchbar->priv->replace_button,
                              "replace_all_button", &searchbar->priv->replace_all_button,
                              "close_button", &searchbar->priv->close_button,
                              NULL);
    g_free (file);

    searchbar->priv->search_entry = xed_history_entry_new ("history-search-for", TRUE);
    gtk_widget_set_hexpand (searchbar->priv->search_entry, TRUE);
    xed_history_entry_set_escape_func (XED_HISTORY_ENTRY (searchbar->priv->search_entry),
                                       (XedHistoryEntryEscapeFunc) xed_utils_escape_search_text);

    searchbar->priv->search_text_entry = xed_history_entry_get_entry (XED_HISTORY_ENTRY (searchbar->priv->search_entry));
    gtk_entry_set_activates_default (GTK_ENTRY (searchbar->priv->search_text_entry), TRUE);

    gtk_widget_show (searchbar->priv->search_entry);
    gtk_grid_attach (GTK_GRID (searchbar->priv->grid), searchbar->priv->search_entry, 2, 0, 1, 1);

    searchbar->priv->replace_entry = xed_history_entry_new ("history-replace-with", TRUE);
    xed_history_entry_set_escape_func (XED_HISTORY_ENTRY(searchbar->priv->replace_entry),
                                       (XedHistoryEntryEscapeFunc) xed_utils_escape_search_text);

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

    g_signal_connect (searchbar->priv->search_text_entry, "insert_text",
                      G_CALLBACK (insert_text_handler), NULL);

    g_signal_connect (searchbar->priv->replace_text_entry, "insert_text",
                      G_CALLBACK (insert_text_handler), NULL);

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
}

GtkWidget *
xed_searchbar_new (GtkWindow *parent,
                   gboolean show_replace)
{
    XedSearchbar *searchbar;
    searchbar = g_object_new (XED_TYPE_SEARCHBAR, NULL);
    searchbar->window = parent;
    return GTK_WIDGET(searchbar);
}

void
xed_searchbar_show (XedSearchbar *searchbar,
                    gboolean show_replace)
{
    XedDocument *doc;
    gboolean selection_exists;
    gchar *find_text = NULL;
    const gchar *search_text = NULL;
    gint sel_len;

    doc = xed_window_get_active_document (searchbar->window);
    g_return_if_fail(doc != NULL);

    selection_exists = get_selected_text (GTK_TEXT_BUFFER(doc), &find_text, &sel_len);

    if (selection_exists && find_text != NULL && sel_len < 80)
    {
        /*
         * Special case: if the currently selected text
         * is the same as the unescaped search text, use the
         * same old search text. (Without this, if you e.g.
         * search for '\n' and then open the search searchbar again,
         * you'll get an unprintable single-character literal '\n' in the "search for" box).
         */
        search_text = xed_searchbar_get_search_text (XED_SEARCHBAR(searchbar));
        if (!(search_text != NULL && !strcmp (xed_utils_unescape_search_text (search_text), find_text)))
        {
            /* General case */
            xed_searchbar_set_search_text (XED_SEARCHBAR(searchbar), find_text);
        }
        g_free (find_text);
    }
    else
    {
        g_free (find_text);
    }
    gtk_revealer_set_transition_type (GTK_REVEALER(searchbar->priv->revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_UP);
    gtk_revealer_set_reveal_child (GTK_REVEALER(searchbar->priv->revealer), TRUE);
    if (show_replace)
    {
        gtk_widget_show (searchbar->priv->replace_label);
        gtk_widget_show (searchbar->priv->replace_entry);
        gtk_widget_show (searchbar->priv->replace_all_button);
        gtk_widget_show (searchbar->priv->replace_button);
        gtk_grid_set_row_spacing (GTK_GRID(searchbar->priv->grid), 10);
    }
    else
    {
        gtk_widget_hide (searchbar->priv->replace_label);
        gtk_widget_hide (searchbar->priv->replace_entry);
        gtk_widget_hide (searchbar->priv->replace_all_button);
        gtk_widget_hide (searchbar->priv->replace_button);
        gtk_grid_set_row_spacing (GTK_GRID(searchbar->priv->grid), 0);
    }

    gtk_widget_show (searchbar->priv->find_button);
    gtk_widget_grab_focus (searchbar->priv->search_text_entry);
}

void
xed_searchbar_hide (XedSearchbar *searchbar)
{
    gtk_revealer_set_transition_type (GTK_REVEALER(searchbar->priv->revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
    gtk_revealer_set_reveal_child (GTK_REVEALER(searchbar->priv->revealer), FALSE);

    // focus document
    XedView *active_view = xed_window_get_active_view (searchbar->window);
    if (active_view != NULL)
    {
        gtk_widget_grab_focus (GTK_WIDGET(active_view));
    }

    // remove highlighting
    _xed_cmd_search_clear_highlight (searchbar->window);
}

void
xed_searchbar_set_search_text (XedSearchbar *searchbar,
                               const gchar *text)
{
    g_return_if_fail(XED_IS_SEARCHBAR (searchbar));
    g_return_if_fail(text != NULL);
    gtk_entry_set_text (GTK_ENTRY(searchbar->priv->search_text_entry), text);
    search_buttons_set_sensitive (searchbar, (text != '\0'));
}

/*
 * The text must be unescaped before searching.
 */
const gchar *
xed_searchbar_get_search_text (XedSearchbar *searchbar)
{
    g_return_val_if_fail(XED_IS_SEARCHBAR (searchbar), NULL);
    return gtk_entry_get_text (GTK_ENTRY(searchbar->priv->search_text_entry));
}

void
xed_searchbar_set_replace_text (XedSearchbar *searchbar,
                                const gchar *text)
{
    g_return_if_fail(XED_IS_SEARCHBAR (searchbar));
    g_return_if_fail(text != NULL);

    gtk_entry_set_text (GTK_ENTRY(searchbar->priv->replace_text_entry), text);
}

const gchar *
xed_searchbar_get_replace_text (XedSearchbar *searchbar)
{
    g_return_val_if_fail(XED_IS_SEARCHBAR (searchbar), NULL);
    return gtk_entry_get_text (GTK_ENTRY(searchbar->priv->replace_text_entry));
}

void
xed_searchbar_set_match_case (XedSearchbar *searchbar,
                              gboolean match_case)
{
    g_return_if_fail(XED_IS_SEARCHBAR (searchbar));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(searchbar->priv->match_case_checkbutton), match_case);
}

gboolean
xed_searchbar_get_match_case (XedSearchbar *searchbar)
{
    g_return_val_if_fail(XED_IS_SEARCHBAR (searchbar), FALSE);
    return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(searchbar->priv->match_case_checkbutton));
}

void
xed_searchbar_set_entire_word (XedSearchbar *searchbar,
                               gboolean entire_word)
{
    g_return_if_fail(XED_IS_SEARCHBAR (searchbar));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(searchbar->priv->entire_word_checkbutton), entire_word);
}

gboolean
xed_searchbar_get_entire_word (XedSearchbar *searchbar)
{
    g_return_val_if_fail(XED_IS_SEARCHBAR (searchbar), FALSE);
    return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(searchbar->priv->entire_word_checkbutton));
}

void
xed_searchbar_set_wrap_around (XedSearchbar *searchbar,
                               gboolean wrap_around)
{
    g_return_if_fail(XED_IS_SEARCHBAR (searchbar));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(searchbar->priv->wrap_around_checkbutton), wrap_around);
}

gboolean
xed_searchbar_get_wrap_around (XedSearchbar *searchbar)
{
    g_return_val_if_fail(XED_IS_SEARCHBAR (searchbar), FALSE);
    return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(searchbar->priv->wrap_around_checkbutton));
}
