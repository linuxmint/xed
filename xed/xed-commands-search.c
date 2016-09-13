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

void
_xed_cmd_search_find (GtkAction *action, XedWindow *window)
{
    xed_searchbar_show (xed_window_get_searchbar (window), FALSE);
}

void
_xed_cmd_search_replace (GtkAction *action, XedWindow *window)
{
    xed_searchbar_show (xed_window_get_searchbar (window), TRUE);
}

void
_xed_cmd_search_find_next (GtkAction *action, XedWindow *window)
{
    xed_debug (DEBUG_COMMANDS);
    xed_searchbar_find_again (xed_window_get_searchbar (window), FALSE);
}

void
_xed_cmd_search_find_prev (GtkAction *action, XedWindow *window)
{
    xed_debug (DEBUG_COMMANDS);
    xed_searchbar_find_again (xed_window_get_searchbar (window), TRUE);
}

void
_xed_cmd_search_clear_highlight (XedWindow *window)
{
    XedDocument *doc;
    xed_debug (DEBUG_COMMANDS);
    doc = xed_window_get_active_document (window);
    if (doc != NULL) {
        xed_document_set_search_text (XED_DOCUMENT (doc), "", XED_SEARCH_DONT_SET_FLAGS);
    }
}

void
_xed_cmd_search_goto_line (GtkAction *action, XedWindow *window)
{
    XedView *active_view;
    xed_debug (DEBUG_COMMANDS);

    active_view = xed_window_get_active_view (window);
    if (active_view == NULL) {
        return;
    }

    /* Focus the view if needed: we need to focus the view otherwise
       activating the binding for goto line has no effect */
    gtk_widget_grab_focus (GTK_WIDGET (active_view));


    /* Goto line is builtin in XedView, just activate the corresponding binding. */
    gtk_bindings_activate (G_OBJECT (active_view), GDK_KEY_i, GDK_CONTROL_MASK);
}
