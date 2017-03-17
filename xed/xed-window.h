#ifndef __XED_WINDOW_H__
#define __XED_WINDOW_H__

#include <gtksourceview/gtksource.h>

#include <xed/xed-tab.h>
#include <xed/xed-panel.h>
#include <xed/xed-message-bus.h>
#include <xed/xed-paned.h>

G_BEGIN_DECLS

typedef enum
{
    XED_WINDOW_STATE_NORMAL = 0,
    XED_WINDOW_STATE_SAVING = 1 << 1,
    XED_WINDOW_STATE_PRINTING = 1 << 2,
    XED_WINDOW_STATE_LOADING = 1 << 3,
    XED_WINDOW_STATE_ERROR = 1 << 4,
    XED_WINDOW_STATE_SAVING_SESSION = 1 << 5
} XedWindowState;

#define XED_TYPE_WINDOW             (xed_window_get_type())
#define XED_WINDOW(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_WINDOW, XedWindow))
#define XED_WINDOW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_WINDOW, XedWindowClass))
#define XED_IS_WINDOW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_WINDOW))
#define XED_IS_WINDOW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_WINDOW))
#define XED_WINDOW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_WINDOW, XedWindowClass))

typedef struct _XedWindow           XedWindow;
typedef struct _XedWindowPrivate    XedWindowPrivate;
typedef struct _XedWindowClass      XedWindowClass;

struct _XedWindow
{
    GtkApplicationWindow window;

    /*< private > */
    XedWindowPrivate *priv;
};

struct _XedWindowClass
{
    GtkApplicationWindowClass parent_class;

    /* Signals */
    void (* tab_added) (XedWindow *window, XedTab *tab);
    void (* tab_removed) (XedWindow *window, XedTab *tab);
    void (* tabs_reordered) (XedWindow *window);
    void (* active_tab_changed) (XedWindow *window, XedTab *tab);
    void (* active_tab_state_changed) (XedWindow *window);
};

/* Public methods */
GType   xed_window_get_type (void) G_GNUC_CONST;
XedTab *xed_window_create_tab (XedWindow *window, gboolean jump_to);
XedTab *xed_window_create_tab_from_location (XedWindow *window, GFile *location, const GtkSourceEncoding *encoding,
                                             gint line_pos, gboolean create, gboolean jump_to);
XedTab *xed_window_create_tab_from_stream (XedWindow *window, GInputStream *stream, const GtkSourceEncoding *encoding,
                                           gint line_pos, gboolean jump_to);
void    xed_window_close_tab (XedWindow *window, XedTab *tab);
void    xed_window_close_all_tabs (XedWindow *window);
void    xed_window_close_tabs (XedWindow *window, const GList *tabs);
XedTab *xed_window_get_active_tab (XedWindow *window);
void    xed_window_set_active_tab (XedWindow *window, XedTab *tab);

/* Helper functions */
XedView     *xed_window_get_active_view (XedWindow *window);
XedDocument *xed_window_get_active_document (XedWindow *window);

/* Returns a newly allocated list with all the documents in the window */
GList *xed_window_get_documents (XedWindow *window);

/* Returns a newly allocated list with all the documents that need to be
 saved before closing the window */
GList *xed_window_get_unsaved_documents (XedWindow *window);

/* Returns a newly allocated list with all the views in the window */
GList          *xed_window_get_views (XedWindow *window);
GtkWindowGroup *xed_window_get_group (XedWindow *window);
XedPanel       *xed_window_get_side_panel (XedWindow *window);
XedPanel       *xed_window_get_bottom_panel (XedWindow *window);
GtkWidget      *xed_window_get_statusbar (XedWindow *window);
GtkWidget      *xed_window_get_searchbar (XedWindow *window);
GtkUIManager   *xed_window_get_ui_manager (XedWindow *window);
XedWindowState  xed_window_get_state (XedWindow *window);
XedTab         *xed_window_get_tab_from_location (XedWindow *window, GFile *location);

/* Message bus */
XedMessageBus *xed_window_get_message_bus (XedWindow *window);

/*
 * Non exported functions
 */
GtkWidget *_xed_window_get_notebook (XedWindow *window);
XedWindow *_xed_window_move_tab_to_new_window (XedWindow *window, XedTab *tab);
gboolean   _xed_window_is_removing_tabs (XedWindow *window);
GFile     *_xed_window_get_default_location (XedWindow *window);
void       _xed_window_set_default_location (XedWindow *window, GFile *location);
void       _xed_window_set_saving_session_state (XedWindow *window, gboolean saving_session);
void       _xed_window_fullscreen (XedWindow *window);
void       _xed_window_unfullscreen (XedWindow *window);
gboolean   _xed_window_is_fullscreen (XedWindow *window);

/* these are in xed-window because of screen safety */
void _xed_recent_add (XedWindow *window, GFile *location, const gchar *mime);
void _xed_recent_remove (XedWindow *window, GFile *location);

void _xed_window_get_default_size (gint *width, gint *height);
gint _xed_window_get_side_panel_size (XedWindow *window);
gint _xed_window_get_bottom_panel_size (XedWindow *window);

XedPaned *_xed_window_get_hpaned (XedWindow *window);
XedPaned *_xed_window_get_vpaned (XedWindow *window);

G_END_DECLS

#endif /* __XED_WINDOW_H__ */
