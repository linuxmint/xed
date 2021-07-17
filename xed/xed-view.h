#ifndef __XED_VIEW_H__
#define __XED_VIEW_H__

#include <gtk/gtk.h>

#include <xed/xed-document.h>
#include <gtksourceview/gtksourceview.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XED_TYPE_VIEW (xed_view_get_type ())
#define XED_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_VIEW, XedView))
#define XED_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_VIEW, XedViewClass))
#define XED_IS_VIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_VIEW))
#define XED_IS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_VIEW))
#define XED_VIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_VIEW, XedViewClass))

/* Private structure type */
typedef struct _XedViewPrivate XedViewPrivate;

/*
 * Main object structure
 */
typedef struct _XedView XedView;

struct _XedView
{
    GtkSourceView view;

    /*< private > */
    XedViewPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XedViewClass XedViewClass;

struct _XedViewClass
{
    GtkSourceViewClass parent_class;

    /* FIXME: Do we need placeholders ? */

    /* Key bindings */
    void     (* drop_uris) (XedView *view, gchar **uri_list);
};

/*
 * Public methods
 */
GType      xed_view_get_type (void) G_GNUC_CONST;
GtkWidget *xed_view_new (XedDocument *doc);
void       xed_view_cut_clipboard (XedView *view);
void       xed_view_copy_clipboard (XedView *view);
void       xed_view_paste_clipboard (XedView *view);
void       xed_view_delete_selection (XedView *view);
void       xed_view_select_all (XedView *view);
void       xed_view_scroll_to_cursor (XedView *view);
void       xed_view_set_font (XedView *view, gboolean def, const gchar *font_name);
void       xed_view_set_draw_whitespace (XedView *view, gboolean enable);
void       xed_view_update_draw_whitespace_locations_and_types (XedView *view);

G_END_DECLS

#endif /* __XED_VIEW_H__ */
