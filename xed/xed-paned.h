#ifndef __XED_PANED_H__
#define __XED_PANED_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XED_TYPE_PANED          (xed_paned_get_type ())
#define XED_PANED(o)            (G_TYPE_CHECK_INSTANCE_CAST ((o), XED_TYPE_PANED, XedPaned))
#define XED_PANED_CLASS(k)      (G_TYPE_CHECK_CLASS_CAST((k), XED_TYPE_PANED, XedPanedClass))
#define XED_IS_PANED(o)         (G_TYPE_CHECK_INSTANCE_TYPE ((o), XED_TYPE_PANED))
#define XED_IS_PANED_CLASS(k)   (G_TYPE_CHECK_CLASS_TYPE ((k), XED_TYPE_PANED))
#define XED_PANED_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), XED_TYPE_PANED, XedPanedClass))

typedef struct _XedPaned        XedPaned;
typedef struct _XedPanedPrivate XedPanedPrivate;
typedef struct _XedPanedClass   XedPanedClass;

struct _XedPaned
{
    GtkPaned parent;

    /* <private/> */
    XedPanedPrivate *priv;
};

struct _XedPanedClass
{
    GtkPanedClass parent_class;
};

GType xed_paned_get_type (void) G_GNUC_CONST;

GtkWidget *xed_paned_new (GtkOrientation orientation);

void xed_paned_close (XedPaned *paned,
                      gint      pane_number);
void xed_paned_open (XedPaned *paned,
                     gint      pane_number,
                     gint      pos);

gboolean xed_paned_get_is_animating (XedPaned *paned);

G_END_DECLS

#endif /* __XED_PANED_H__ */
