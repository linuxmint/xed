#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <libpeas/peas-extension-set.h>
#include <glib/gi18n.h>

#include "xed-view.h"
#include "xed-view-gutter-renderer.h"
#include "xed-view-activatable.h"
#include "xed-plugins-engine.h"
#include "xed-debug.h"
#include "xed-marshal.h"
#include "xed-utils.h"
#include "xed-settings.h"
#include "xed-app.h"

#define XED_VIEW_SCROLL_MARGIN 0.02

#define XED_VIEW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), XED_TYPE_VIEW, XedViewPrivate))

enum
{
    TARGET_URI_LIST = 100
};

struct _XedViewPrivate
{
    GSettings *editor_settings;
    GtkTextBuffer *current_buffer;
    PeasExtensionSet *extensions;
    GtkSourceGutterRenderer *renderer;
    guint view_realized : 1;
};

G_DEFINE_TYPE (XedView, xed_view, GTK_SOURCE_TYPE_VIEW)

/* Signals */
enum
{
    DROP_URIS,
    LAST_SIGNAL
};

static guint view_signals[LAST_SIGNAL] = { 0 };

static void
document_read_only_notify_handler (XedDocument *document,
                                   GParamSpec *pspec,
                                   XedView *view)
{
    xed_debug (DEBUG_VIEW);
    gtk_text_view_set_editable (GTK_TEXT_VIEW (view), !xed_document_get_readonly (document));
}

static void
current_buffer_removed (XedView *view)
{
    if (view->priv->current_buffer != NULL)
    {
        g_signal_handlers_disconnect_by_func(view->priv->current_buffer, document_read_only_notify_handler, view);
        g_object_unref (view->priv->current_buffer);
        view->priv->current_buffer = NULL;
    }
}

static void
extension_added (PeasExtensionSet *extensions,
                 PeasPluginInfo   *info,
                 PeasExtension    *exten,
                 XedView          *view)
{
    peas_extension_call (exten, "activate");
}

static void
extension_removed (PeasExtensionSet *extensions,
                   PeasPluginInfo   *info,
                   PeasExtension    *exten,
                   XedView          *view)
{
    peas_extension_call (exten, "deactivate");
}

static void
on_notify_buffer_cb (XedView *view,
                     GParamSpec *arg1,
                     gpointer userdata)
{
    GtkTextBuffer *buffer;

    current_buffer_removed (view);
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

    if (buffer == NULL || !XED_IS_DOCUMENT (buffer))
    {
        return;
    }

    view->priv->current_buffer = g_object_ref (buffer);
    g_signal_connect(buffer, "notify::read-only", G_CALLBACK (document_read_only_notify_handler), view);

    gtk_text_view_set_editable (GTK_TEXT_VIEW (view), !xed_document_get_readonly (XED_DOCUMENT(buffer)));
}

static void
xed_view_init (XedView *view)
{
    GtkTargetList *tl;

    xed_debug (DEBUG_VIEW);

    view->priv = XED_VIEW_GET_PRIVATE (view);

    view->priv->editor_settings = g_settings_new ("org.x.editor.preferences.editor");

    /* Drag and drop support */
    tl = gtk_drag_dest_get_target_list (GTK_WIDGET(view));

    if (tl != NULL)
    {
        gtk_target_list_add_uri_targets (tl, TARGET_URI_LIST);
    }

    view->priv->extensions = peas_extension_set_new (PEAS_ENGINE (xed_plugins_engine_get_default ()),
                                                     XED_TYPE_VIEW_ACTIVATABLE, "view", view, NULL);

    g_signal_connect (view->priv->extensions, "extension-added",
                      G_CALLBACK (extension_added), view);
    g_signal_connect (view->priv->extensions, "extension-removed",
                      G_CALLBACK (extension_removed), view);

    /* Act on buffer change */
    g_signal_connect(view, "notify::buffer", G_CALLBACK (on_notify_buffer_cb), NULL);
}

static void
xed_view_dispose (GObject *object)
{
    XedView *view = XED_VIEW (object);

    g_clear_object (&view->priv->extensions);
    g_clear_object (&view->priv->editor_settings);
    g_clear_object (&view->priv->renderer);

    current_buffer_removed (view);

    /* Disconnect notify buffer because the destroy of the textview will set
     * the buffer to NULL, and we call get_buffer in the notify which would
     * reinstate a buffer which we don't want.
     * There is no problem calling g_signal_handlers_disconnect_by_func()
     * several times (if dispose() is called several times).
     */
    g_signal_handlers_disconnect_by_func (view, on_notify_buffer_cb, NULL);

    G_OBJECT_CLASS (xed_view_parent_class)->dispose (object);
}

static void
xed_view_constructed (GObject *object)
{
    XedView *view;
    XedViewPrivate *priv;
    gboolean use_default_font;
    GtkSourceGutter *gutter;

    view = XED_VIEW (object);
    priv = view->priv;

    /* Get setting values */
    use_default_font = g_settings_get_boolean (view->priv->editor_settings, XED_SETTINGS_USE_DEFAULT_FONT);

    /*
     *  Set tab, fonts, wrap mode, colors, etc. according to preferences
     */
    if (!use_default_font)
    {
        gchar *editor_font;

        editor_font = g_settings_get_string (view->priv->editor_settings, XED_SETTINGS_EDITOR_FONT);

        xed_view_set_font (view, FALSE, editor_font);

        g_free (editor_font);
    }
    else
    {
        xed_view_set_font (view, TRUE, NULL);
    }

    g_settings_bind (priv->editor_settings,
                     XED_SETTINGS_DISPLAY_LINE_NUMBERS,
                     view,
                     "show-line-numbers",
                     G_SETTINGS_BIND_GET);

    g_settings_bind (priv->editor_settings,
                     XED_SETTINGS_AUTO_INDENT,
                     view,
                     "auto-indent",
                     G_SETTINGS_BIND_GET);

    g_settings_bind (priv->editor_settings,
                     XED_SETTINGS_TABS_SIZE,
                     view,
                     "tab-width",
                     G_SETTINGS_BIND_GET);

    g_settings_bind (priv->editor_settings,
                     XED_SETTINGS_INSERT_SPACES,
                     view,
                     "insert-spaces-instead-of-tabs",
                     G_SETTINGS_BIND_GET);

    g_settings_bind (priv->editor_settings,
                     XED_SETTINGS_DISPLAY_RIGHT_MARGIN,
                     view,
                     "show-right-margin",
                     G_SETTINGS_BIND_GET);

    g_settings_bind (priv->editor_settings,
                     XED_SETTINGS_RIGHT_MARGIN_POSITION,
                     view,
                     "right-margin-position",
                     G_SETTINGS_BIND_GET);

    g_settings_bind (priv->editor_settings,
                     XED_SETTINGS_HIGHLIGHT_CURRENT_LINE,
                     view,
                     "highlight-current-line",
                     G_SETTINGS_BIND_GET);

    g_settings_bind (priv->editor_settings,
                     XED_SETTINGS_WRAP_MODE,
                     view,
                     "wrap-mode",
                     G_SETTINGS_BIND_GET);

    g_settings_bind (priv->editor_settings,
                     XED_SETTINGS_SMART_HOME_END,
                     view,
                     "smart-home-end",
                     G_SETTINGS_BIND_GET);

    g_object_set (G_OBJECT (view),
                  "indent_on_tab", TRUE,
                  NULL);

    gutter = gtk_source_view_get_gutter (GTK_SOURCE_VIEW (view), GTK_TEXT_WINDOW_LEFT);
    priv->renderer = g_object_new (XED_TYPE_VIEW_GUTTER_RENDERER,
                                   "size", 2,
                                   NULL);
    g_object_ref (priv->renderer);
    gtk_source_gutter_insert (gutter, priv->renderer, 0);

#if GTK_CHECK_VERSION (3, 18, 0)
    gtk_text_view_set_top_margin (GTK_TEXT_VIEW (view), 2);
#endif

    G_OBJECT_CLASS (xed_view_parent_class)->constructed (object);
}

static gint
xed_view_focus_out (GtkWidget     *widget,
                    GdkEventFocus *event)
{
    gtk_widget_queue_draw (widget);

    GTK_WIDGET_CLASS (xed_view_parent_class)->focus_out_event (widget, event);

    return FALSE;
}

static GdkAtom
drag_get_uri_target (GtkWidget      *widget,
                     GdkDragContext *context)
{
    GdkAtom target;
    GtkTargetList *tl;

    tl = gtk_target_list_new (NULL, 0);
    gtk_target_list_add_uri_targets (tl, 0);

    target = gtk_drag_dest_find_target (widget, context, tl);
    gtk_target_list_unref (tl);

    return target;
}

static gboolean
xed_view_drag_motion (GtkWidget      *widget,
                      GdkDragContext *context,
                      gint            x,
                      gint            y,
                      guint           timestamp)
{
    gboolean result;

    /* Chain up to allow textview to scroll and position dnd mark, note
     * that this needs to be checked if gtksourceview or gtktextview
     * changes drag_motion behaviour */
    result = GTK_WIDGET_CLASS (xed_view_parent_class)->drag_motion (widget, context, x, y, timestamp);

    /* If this is a URL, deal with it here */
    if (drag_get_uri_target (widget, context) != GDK_NONE)
    {
        gdk_drag_status (context, gdk_drag_context_get_suggested_action (context), timestamp);
        result = TRUE;
    }

    return result;
}

static void
xed_view_drag_data_received (GtkWidget        *widget,
                             GdkDragContext   *context,
                             gint              x,
                             gint              y,
                             GtkSelectionData *selection_data,
                             guint             info,
                             guint             timestamp)
{
    gchar **uri_list;

    /* If this is an URL emit DROP_URIS, otherwise chain up the signal */
    if (info == TARGET_URI_LIST)
    {
        uri_list = xed_utils_drop_get_uris (selection_data);

        if (uri_list != NULL)
        {
            g_signal_emit (widget, view_signals[DROP_URIS], 0, uri_list);
            g_strfreev (uri_list);
            gtk_drag_finish (context, TRUE, FALSE, timestamp);
        }
    }
    else
    {
        GTK_WIDGET_CLASS (xed_view_parent_class)->drag_data_received (widget, context, x, y, selection_data, info,
                                                                      timestamp);
    }
}

static gboolean
xed_view_drag_drop (GtkWidget      *widget,
                    GdkDragContext *context,
                    gint            x,
                    gint            y,
                    guint           timestamp)
{
    gboolean result;
    GdkAtom target;

    /* If this is a URL, just get the drag data */
    target = drag_get_uri_target (widget, context);

    if (target != GDK_NONE)
    {
        gtk_drag_get_data (widget, context, target, timestamp);
        result = TRUE;
    }
    else
    {
        /* Chain up */
        result = GTK_WIDGET_CLASS (xed_view_parent_class)->drag_drop (widget, context, x, y, timestamp);
    }

    return result;
}

static GtkWidget *
create_line_numbers_menu (GtkWidget *view)
{
    GtkWidget *menu;
    GtkWidget *item;

    menu = gtk_menu_new ();

    item = gtk_check_menu_item_new_with_mnemonic (_("_Display line numbers"));
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item),
                                    gtk_source_view_get_show_line_numbers (GTK_SOURCE_VIEW (view)));

    g_settings_bind (XED_VIEW (view)->priv->editor_settings,
                     XED_SETTINGS_DISPLAY_LINE_NUMBERS,
                     item,
                     "active",
                     G_SETTINGS_BIND_SET);

    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    gtk_widget_show_all (menu);

    return menu;
}

static void
show_line_numbers_menu (GtkWidget      *view,
                        GdkEventButton *event)
{
    GtkWidget *menu;

    menu = create_line_numbers_menu (view);
    gtk_menu_popup (GTK_MENU(menu), NULL, NULL, NULL, NULL, event->button, event->time);
}

static gboolean
xed_view_button_press_event (GtkWidget      *widget,
                             GdkEventButton *event)
{
    if ((event->type == GDK_BUTTON_PRESS) &&
        (event->button == 3) &&
        (event->window == gtk_text_view_get_window (GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_LEFT)))
    {
        show_line_numbers_menu (widget, event);
        return TRUE;
    }

    return GTK_WIDGET_CLASS (xed_view_parent_class)->button_press_event (widget, event);
}

static void
xed_view_realize (GtkWidget *widget)
{
    XedView *view = XED_VIEW (widget);

    if (!view->priv->view_realized)
    {
        peas_extension_set_call (view->priv->extensions, "activate");
        view->priv->view_realized = TRUE;
    }

    GTK_WIDGET_CLASS (xed_view_parent_class)->realize (widget);
}

static void
delete_line (GtkTextView *text_view,
             gint         count)
{
    GtkTextIter start;
    GtkTextIter end;
    GtkTextBuffer *buffer;

    buffer = gtk_text_view_get_buffer (text_view);

    gtk_text_view_reset_im_context (text_view);

    /* If there is a selection delete the selected lines and
     * ignore count */
    if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
    {
        gtk_text_iter_order (&start, &end);
        if (gtk_text_iter_starts_line (&end))
        {
            /* Do no delete the line with the cursor if the cursor
             * is at the beginning of the line */
            count = 0;
        }
        else
        {
            count = 1;
        }
    }

    gtk_text_iter_set_line_offset (&start, 0);

    if (count > 0)
    {
        gtk_text_iter_forward_lines (&end, count);
        if (gtk_text_iter_is_end (&end))
        {
            if (gtk_text_iter_backward_line (&start) && !gtk_text_iter_ends_line (&start))
            {
                gtk_text_iter_forward_to_line_end (&start);
            }
        }
    }
    else if (count < 0)
    {
        if (!gtk_text_iter_ends_line (&end))
        {
            gtk_text_iter_forward_to_line_end (&end);
        }

        while (count < 0)
        {
            if (!gtk_text_iter_backward_line (&start))
            {
                break;
            }
            ++count;
        }

        if (count == 0)
        {
            if (!gtk_text_iter_ends_line (&start))
            {
                gtk_text_iter_forward_to_line_end (&start);
            }
        }
        else
        {
            gtk_text_iter_forward_line (&end);
        }
    }

    if (!gtk_text_iter_equal (&start, &end))
    {
        GtkTextIter cur = start;
        gtk_text_iter_set_line_offset (&cur, 0);
        gtk_text_buffer_begin_user_action (buffer);
        gtk_text_buffer_place_cursor (buffer, &cur);
        gtk_text_buffer_delete_interactive (buffer, &start, &end, gtk_text_view_get_editable (text_view));
        gtk_text_buffer_end_user_action (buffer);
        gtk_text_view_scroll_mark_onscreen (text_view, gtk_text_buffer_get_insert (buffer));
    }
    else
    {
        gtk_widget_error_bell (GTK_WIDGET(text_view));
    }
}

static void
xed_view_delete_from_cursor (GtkTextView  *text_view,
                             GtkDeleteType type,
                             gint          count)
{
    /* We override the standard handler for delete_from_cursor since
     the GTK_DELETE_PARAGRAPHS case is not implemented as we like (i.e. it
     does not remove the carriage return in the previous line)
     */
    switch (type)
    {
        case GTK_DELETE_PARAGRAPHS:
            delete_line (text_view, count);
            break;
        default:
            GTK_TEXT_VIEW_CLASS (xed_view_parent_class)->delete_from_cursor (text_view, type, count);
            break;
    }
}

static GtkTextBuffer *
xed_view_create_buffer (GtkTextView *text_view)
{
    return GTK_TEXT_BUFFER (xed_document_new ());
}

static void
xed_view_class_init (XedViewClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GtkTextViewClass *text_view_class = GTK_TEXT_VIEW_CLASS (klass);
    GtkBindingSet *binding_set;

    object_class->dispose = xed_view_dispose;
    object_class->constructed = xed_view_constructed;

    widget_class->focus_out_event = xed_view_focus_out;

    /*
     * Override the gtk_text_view_drag_motion and drag_drop
     * functions to get URIs
     *
     * If the mime type is text/uri-list, then we will accept
     * the potential drop, or request the data (depending on the
     * function).
     *
     * If the drag context has any other mime type, then pass the
     * information onto the GtkTextView's standard handlers.
     * (widget_class->function_name).
     *
     * See bug #89881 for details
     */
    widget_class->drag_motion = xed_view_drag_motion;
    widget_class->drag_data_received = xed_view_drag_data_received;
    widget_class->drag_drop = xed_view_drag_drop;
    widget_class->button_press_event = xed_view_button_press_event;
    widget_class->realize = xed_view_realize;

    text_view_class->delete_from_cursor = xed_view_delete_from_cursor;
    text_view_class->create_buffer = xed_view_create_buffer;

    /* A new signal DROP_URIS has been added to allow plugins to intercept
     * the default dnd behaviour of 'text/uri-list'. XedView now handles
     * dnd in the default handlers of drag_drop, drag_motion and
     * drag_data_received. The view emits drop_uris from drag_data_received
     * if valid uris have been dropped. Plugins should connect to
     * drag_motion, drag_drop and drag_data_received to change this
     * default behaviour. They should _NOT_ use this signal because this
     * will not prevent xed from loading the uri
     */
    view_signals[DROP_URIS] =
        g_signal_new ("drop_uris",
                      G_TYPE_FROM_CLASS (object_class),
                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (XedViewClass, drop_uris),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__BOXED,
                      G_TYPE_NONE, 1, G_TYPE_STRV);

    g_type_class_add_private (klass, sizeof (XedViewPrivate));

    binding_set = gtk_binding_set_by_class (klass);

    gtk_binding_entry_add_signal (binding_set,
                                  GDK_KEY_d,
                                  GDK_CONTROL_MASK,
                                  "delete_from_cursor", 2,
                                  G_TYPE_ENUM, GTK_DELETE_PARAGRAPHS,
                                  G_TYPE_INT, 1);

    gtk_binding_entry_add_signal (binding_set,
                                  GDK_KEY_u,
                                  GDK_CONTROL_MASK,
                                  "change_case", 1,
                                  G_TYPE_ENUM, GTK_SOURCE_CHANGE_CASE_UPPER);

    gtk_binding_entry_add_signal (binding_set,
                                  GDK_KEY_l,
                                  GDK_CONTROL_MASK,
                                  "change_case", 1,
                                  G_TYPE_ENUM, GTK_SOURCE_CHANGE_CASE_LOWER);

    gtk_binding_entry_add_signal (binding_set,
                                  GDK_KEY_asciitilde,
                                  GDK_CONTROL_MASK,
                                  "change_case", 1,
                                  G_TYPE_ENUM, GTK_SOURCE_CHANGE_CASE_TOGGLE);

    gtk_binding_entry_add_signal (binding_set,
                                  GDK_KEY_t,
                                  GDK_CONTROL_MASK,
                                  "change_case", 1,
                                  G_TYPE_ENUM, GTK_SOURCE_CHANGE_CASE_TITLE);
}

/**
 * xed_view_new:
 * @doc: a #XedDocument
 *
 * Creates a new #XedView object displaying the @doc document.
 * @doc cannot be %NULL.
 *
 * Return value: a new #XedView
 **/
GtkWidget *
xed_view_new (XedDocument *doc)
{
    GtkWidget *view;

    xed_debug_message (DEBUG_VIEW, "START");

    g_return_val_if_fail(XED_IS_DOCUMENT (doc), NULL);

    view = GTK_WIDGET(g_object_new (XED_TYPE_VIEW, "buffer", doc, NULL));

    xed_debug_message (DEBUG_VIEW, "END: %d", G_OBJECT (view)->ref_count);

    gtk_widget_show_all (view);

    return view;
}

void
xed_view_cut_clipboard (XedView *view)
{
    GtkTextBuffer *buffer;
    GtkClipboard *clipboard;

    xed_debug (DEBUG_VIEW);
    g_return_if_fail(XED_IS_VIEW (view));

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(view));
    g_return_if_fail(buffer != NULL);

    clipboard = gtk_widget_get_clipboard (GTK_WIDGET(view), GDK_SELECTION_CLIPBOARD);

    /* FIXME: what is default editability of a buffer? */
    gtk_text_buffer_cut_clipboard (buffer, clipboard, !xed_document_get_readonly (XED_DOCUMENT(buffer)));

    gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW(view), gtk_text_buffer_get_insert (buffer), XED_VIEW_SCROLL_MARGIN,
                                  FALSE, 0.0, 0.0);
}

void
xed_view_copy_clipboard (XedView *view)
{
    GtkTextBuffer *buffer;
    GtkClipboard *clipboard;

    xed_debug (DEBUG_VIEW);
    g_return_if_fail(XED_IS_VIEW (view));

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(view));
    g_return_if_fail(buffer != NULL);

    clipboard = gtk_widget_get_clipboard (GTK_WIDGET(view), GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_copy_clipboard (buffer, clipboard);
}

void
xed_view_paste_clipboard (XedView *view)
{
    GtkTextBuffer *buffer;
    GtkClipboard *clipboard;

    xed_debug (DEBUG_VIEW);

    g_return_if_fail(XED_IS_VIEW (view));

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(view));
    g_return_if_fail(buffer != NULL);

    clipboard = gtk_widget_get_clipboard (GTK_WIDGET(view), GDK_SELECTION_CLIPBOARD);

    /* FIXME: what is default editability of a buffer? */
    gtk_text_buffer_paste_clipboard (buffer, clipboard, NULL, !xed_document_get_readonly (XED_DOCUMENT(buffer)));

    gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW(view), gtk_text_buffer_get_insert (buffer), XED_VIEW_SCROLL_MARGIN,
                                  FALSE, 0.0, 0.0);
}

/**
 * xed_view_delete_selection:
 * @view: a #XedView
 *
 * Deletes the text currently selected in the #GtkTextBuffer associated
 * to the view and scroll to the cursor position.
 **/
void
xed_view_delete_selection (XedView *view)
{
    GtkTextBuffer *buffer = NULL;

    xed_debug (DEBUG_VIEW);

    g_return_if_fail(XED_IS_VIEW (view));

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(view));
    g_return_if_fail(buffer != NULL);

    /* FIXME: what is default editability of a buffer? */
    gtk_text_buffer_delete_selection (buffer, TRUE, !xed_document_get_readonly (XED_DOCUMENT(buffer)));

    gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW(view), gtk_text_buffer_get_insert (buffer), XED_VIEW_SCROLL_MARGIN,
                                  FALSE, 0.0, 0.0);
}

/**
 * xed_view_select_all:
 * @view: a #XedView
 *
 * Selects all the text displayed in the @view.
 **/
void
xed_view_select_all (XedView *view)
{
    GtkTextBuffer *buffer = NULL;
    GtkTextIter start, end;

    xed_debug (DEBUG_VIEW);

    g_return_if_fail(XED_IS_VIEW (view));

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(view));
    g_return_if_fail(buffer != NULL);

    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_select_range (buffer, &start, &end);
}

/**
 * xed_view_scroll_to_cursor:
 * @view: a #XedView
 *
 * Scrolls the @view to the cursor position.
 **/
void
xed_view_scroll_to_cursor (XedView *view)
{
    GtkTextBuffer* buffer = NULL;

    xed_debug (DEBUG_VIEW);

    g_return_if_fail(XED_IS_VIEW (view));

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(view));
    g_return_if_fail(buffer != NULL);

    gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW(view), gtk_text_buffer_get_insert (buffer), 0.25, FALSE, 0.0, 0.0);
}

/* FIXME this is an issue for introspection */
/**
 * xed_view_set_font:
 * @view: a #XedView
 * @def: whether to reset the default font
 * @font_name: the name of the font to use
 *
 * If @def is #TRUE, resets the font of the @view to the default font
 * otherwise sets it to @font_name.
 **/
void
xed_view_set_font (XedView     *view,
                   gboolean     def,
                   const gchar *font_name)
{
    PangoFontDescription *font_desc = NULL;

    xed_debug (DEBUG_VIEW);

    g_return_if_fail(XED_IS_VIEW (view));

    if (def)
    {
        GObject *settings;
        gchar *font;

        settings = _xed_app_get_settings (XED_APP (g_application_get_default ()));
        font = xed_settings_get_system_font (XED_SETTINGS (settings));
        font_desc = pango_font_description_from_string (font);

        g_free (font);
    }
    else
    {
        g_return_if_fail (font_name != NULL);
        font_desc = pango_font_description_from_string (font_name);
    }

    g_return_if_fail (font_desc != NULL);
    gtk_widget_modify_font (GTK_WIDGET (view), font_desc);
    pango_font_description_free (font_desc);
}
