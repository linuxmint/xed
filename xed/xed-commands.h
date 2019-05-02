#ifndef __XED_COMMANDS_H__
#define __XED_COMMANDS_H__

#include <gtksourceview/gtksource.h>
#include <xed/xed-window.h>

G_BEGIN_DECLS

/* Do nothing if URI does not exist */
void xed_commands_load_location (XedWindow *window, GFile *location, const GtkSourceEncoding *encoding, gint line_pos);

/* Ignore non-existing URIs */
GSList *xed_commands_load_locations (XedWindow *window, const GSList *locations, const GtkSourceEncoding *encoding, gint line_pos);
void xed_commands_save_document (XedWindow *window, XedDocument *document);
void xed_commands_save_document_async (XedDocument *document, XedWindow *window, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean xed_commands_save_document_finish (XedDocument *document, GAsyncResult *result);
void xed_commands_save_all_documents (XedWindow *window);

/*
 * Non-exported functions
 */

/* Create titled documens for non-existing URIs */
GSList *_xed_cmd_load_files_from_prompt (XedWindow *window, GSList *files, const GtkSourceEncoding *encoding, gint line_pos);
void _xed_cmd_file_new (GtkAction *action, XedWindow *window);
void _xed_cmd_file_open (GtkAction *action, XedWindow *window);
void _xed_cmd_file_save (GtkAction *action, XedWindow *window);
void _xed_cmd_file_save_as (GtkAction *action, XedWindow *window);
void _xed_cmd_file_save_all (GtkAction *action, XedWindow *window);
void _xed_cmd_file_revert (GtkAction *action, XedWindow *window);
void _xed_cmd_file_print_preview (GtkAction *action, XedWindow *window);
void _xed_cmd_file_print (GtkAction *action, XedWindow *window);
void _xed_cmd_file_close (GtkAction *action, XedWindow *window);
void _xed_cmd_file_close_all (GtkAction *action, XedWindow *window);
void _xed_cmd_file_quit (GtkAction *action, XedWindow *window);

void _xed_cmd_edit_undo (GtkAction *action, XedWindow *window);
void _xed_cmd_edit_redo (GtkAction *action, XedWindow *window);
void _xed_cmd_edit_cut (GtkAction *action, XedWindow *window);
void _xed_cmd_edit_copy (GtkAction *action, XedWindow *window);
void _xed_cmd_edit_paste (GtkAction *action, XedWindow *window);
void _xed_cmd_edit_delete (GtkAction *action, XedWindow *window);
void _xed_cmd_edit_select_all (GtkAction *action, XedWindow *window);
void _xed_cmd_edit_preferences (GtkAction *action, XedWindow *window);
void _xed_cmd_edit_toggle_comment (GtkAction *action, XedWindow *window);
void _xed_cmd_edit_toggle_comment_block (GtkAction *action, XedWindow *window);

void _xed_cmd_view_show_toolbar (GtkAction *action, XedWindow *window);
void _xed_cmd_view_show_statusbar (GtkAction *action, XedWindow *window);
void _xed_cmd_view_show_side_pane (GtkAction *action, XedWindow *window);
void _xed_cmd_view_show_bottom_pane (GtkAction *action, XedWindow *window);
void _xed_cmd_view_toggle_overview_map (GtkAction *action, XedWindow *window);
void _xed_cmd_view_toggle_fullscreen_mode (GtkAction *action, XedWindow *window);
void _xed_cmd_view_toggle_word_wrap (GtkAction *action, XedWindow *window);
void _xed_cmd_view_leave_fullscreen_mode (GtkAction *action, XedWindow *window);
void _xed_cmd_view_change_highlight_mode (GtkAction *action, XedWindow *window);

void _xed_cmd_search_find (GtkAction *action, XedWindow *window);

void _xed_cmd_search_find_next (GtkAction *action, XedWindow *window);
void _xed_cmd_search_find_prev (GtkAction *action, XedWindow *window);
void _xed_cmd_search_replace (GtkAction *action, XedWindow *window);
void _xed_cmd_search_clear_highlight (XedWindow *window);
void _xed_cmd_search_goto_line (GtkAction *action, XedWindow *window);

void _xed_cmd_documents_previous_document (GtkAction *action, XedWindow *window);
void _xed_cmd_documents_next_document (GtkAction *action, XedWindow *window);
void _xed_cmd_documents_move_to_new_window (GtkAction *action, XedWindow *window);

void _xed_cmd_help_contents (GtkAction *action, XedWindow *window);
void _xed_cmd_help_about (GtkAction *action, XedWindow *window);
void _xed_cmd_help_keyboard_shortcuts (GtkAction *action, XedWindow *window);

void _xed_cmd_file_close_tab (XedTab *tab, XedWindow *window);

G_END_DECLS

#endif /* __XED_COMMANDS_H__ */
