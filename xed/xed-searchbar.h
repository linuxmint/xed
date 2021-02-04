
#ifndef __XED_SEARCHBAR_H__
#define __XED_SEARCHBAR_H__

#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include "xed-window.h"

G_BEGIN_DECLS

#define XED_TYPE_SEARCHBAR              (xed_searchbar_get_type())
#define XED_SEARCHBAR(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_SEARCHBAR, XedSearchbar))
#define XED_SEARCHBAR_CONST(obj)        (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_SEARCHBAR, XedSearchbar const))
#define XED_SEARCHBAR_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_SEARCHBAR, XedSearchbarClass))
#define XED_IS_SEARCHBAR(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_SEARCHBAR))
#define XED_IS_SEARCHBAR_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_SEARCHBAR))
#define XED_SEARCHBAR_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_SEARCHBAR, XedSearchbarClass))

typedef struct _XedSearchbar        XedSearchbar;
typedef struct _XedSearchbarPrivate XedSearchbarPrivate;
typedef struct _XedSearchbarClass   XedSearchbarClass;

struct _XedSearchbar
{
    GtkBox parent;
    XedWindow *window;

    /*< private > */
    XedSearchbarPrivate *priv;
};

struct _XedSearchbarClass
{
    GtkBoxClass parent_class;

    /* Key bindings */
    gboolean (* show_replace) (XedSearchbar *dlg);
};

typedef enum
{
    XED_SEARCH_MODE_SEARCH,
    XED_SEARCH_MODE_REPLACE
} XedSearchMode;

GType        xed_searchbar_get_type (void) G_GNUC_CONST;

GtkWidget   *xed_searchbar_new (GtkWindow *parent);

void         xed_searchbar_hide (XedSearchbar *searchbar);
void         xed_searchbar_show (XedSearchbar *searchbar, XedSearchMode search_mode);
void         xed_searchbar_find_again (XedSearchbar *searchbar, gboolean backward);

const gchar *xed_searchbar_get_replace_text (XedSearchbar *searchbar);
const gchar *xed_searchbar_get_search_text (XedSearchbar *searchbar);

gboolean     xed_searchbar_get_backwards    (XedSearchbar *searchbar);

GtkSourceSearchSettings *xed_searchbar_get_search_settings (XedSearchbar *searchbar);

void xed_searchbar_set_search_text (XedSearchbar *searchbar,
                                    const gchar  *search_text);

void         xed_searchbar_set_parse_escapes (XedSearchbar *searchbar, gboolean parse_escapes);
gboolean     xed_searchbar_get_parse_escapes (XedSearchbar *searchbar);

G_END_DECLS

#endif  /* __XED_SEARCHBAR_H__  */
