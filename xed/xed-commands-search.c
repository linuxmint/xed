#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "xed-commands.h"
#include "xed-debug.h"
#include "xed-window.h"
#include "xed-utils.h"
#include "xed-searchbar.h"
#include "xed-view-frame.h"

// void
// _xed_cmd_search_find (GtkAction *action,
//                       XedWindow *window)
// {
//     xed_searchbar_show (XED_SEARCHBAR (xed_window_get_searchbar (window)), FALSE);
// }

void
_xed_cmd_search_find (GtkAction *action,
                      XedWindow *window)
{
    xed_searchbar_show (XED_SEARCHBAR (xed_window_get_searchbar (window)), SEARCH_MODE_SEARCH);
}

void
_xed_cmd_search_replace (GtkAction *action,
                         XedWindow *window)
{
    xed_searchbar_show (XED_SEARCHBAR (xed_window_get_searchbar (window)), SEARCH_MODE_REPLACE);
}

void
_xed_cmd_search_find_next (GtkAction *action,
                           XedWindow *window)
{
    xed_debug (DEBUG_COMMANDS);
    xed_searchbar_find_again (XED_SEARCHBAR (xed_window_get_searchbar (window)), FALSE);
}

void
_xed_cmd_search_find_prev (GtkAction *action,
                           XedWindow *window)
{
    xed_debug (DEBUG_COMMANDS);
    xed_searchbar_find_again (XED_SEARCHBAR (xed_window_get_searchbar (window)), TRUE);
}

void
_xed_cmd_search_clear_highlight (XedWindow *window)
{
    XedDocument *doc;

    xed_debug (DEBUG_COMMANDS);

    doc = xed_window_get_active_document (window);
    if (doc != NULL)
    {
        _xed_document_set_search_context (doc, NULL);
    }
}

void
_xed_cmd_search_goto_line (GtkAction *action,
                           XedWindow *window)
{
    XedTab *active_tab;
    XedViewFrame *frame;

    xed_debug (DEBUG_COMMANDS);

    active_tab = xed_window_get_active_tab (window);
    if (active_tab == NULL)
    {
        return;
    }

    /* Focus the view if needed: we need to focus the view otherwise
     activating the binding for goto line has no effect */
    // gtk_widget_grab_focus (GTK_WIDGET(active_view));

    /* Goto line is builtin in XedView, just activate the corresponding binding. */
    frame = XED_VIEW_FRAME (_xed_tab_get_view_frame (active_tab));
    xed_view_frame_popup_goto_line (frame);
}
