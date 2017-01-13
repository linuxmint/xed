
#ifndef __XED_SEARCHBAR_H__
#define __XED_SEARCHBAR_H__

#include <gtk/gtk.h>
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

enum
{
    XED_SEARCHBAR_FIND_RESPONSE = 100,
    XED_SEARCHBAR_REPLACE_RESPONSE,
    XED_SEARCHBAR_REPLACE_ALL_RESPONSE
};

GType        xed_searchbar_get_type (void) G_GNUC_CONST;

GtkWidget   *xed_searchbar_new (GtkWindow *parent, gboolean show_replace);

void         xed_searchbar_hide (XedSearchbar *searchbar);
void         xed_searchbar_show (XedSearchbar *searchbar, gboolean show_replace);
void         xed_searchbar_find_again (XedSearchbar *searchbar, gboolean     backward);

void         xed_searchbar_set_search_text  (XedSearchbar *searchbar, const gchar *text);
const gchar *xed_searchbar_get_search_text  (XedSearchbar *searchbar);

void         xed_searchbar_set_replace_text (XedSearchbar *searchbar, const gchar *text);
const gchar *xed_searchbar_get_replace_text (XedSearchbar *searchbar);

void         xed_searchbar_set_match_case   (XedSearchbar *searchbar, gboolean match_case);
gboolean     xed_searchbar_get_match_case   (XedSearchbar *searchbar);

void         xed_searchbar_set_entire_word  (XedSearchbar *searchbar, gboolean entire_word);
gboolean     xed_searchbar_get_entire_word  (XedSearchbar *searchbar);

void         xed_searchbar_set_backwards    (XedSearchbar *searchbar, gboolean backwards);
gboolean     xed_searchbar_get_backwards    (XedSearchbar *searchbar);

void         xed_searchbar_set_wrap_around  (XedSearchbar *searchbar, gboolean wrap_around);
gboolean     xed_searchbar_get_wrap_around  (XedSearchbar *searchbar);

void         xed_searchbar_set_parse_escapes (XedSearchbar *searchbar, gboolean parse_escapes);
gboolean     xed_searchbar_get_parse_escapes (XedSearchbar *searchbar);

G_END_DECLS

#endif  /* __XED_SEARCHBAR_H__  */
