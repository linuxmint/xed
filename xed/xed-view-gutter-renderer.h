#ifndef __XED_VIEW_GUTTER_RENDERER_H__
#define __XED_VIEW_GUTTER_RENDERER_H__

#include <gtksourceview/gtksource.h>

G_BEGIN_DECLS

#define XED_TYPE_VIEW_GUTTER_RENDERER (xed_view_gutter_renderer_get_type ())
#define XED_VIEW_GUTTER_RENDERER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_VIEW_GUTTER_RENDERER, XedViewGutterRenderer))
#define XED_VIEW_GUTTER_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_VIEW_GUTTER_RENDERER, XedViewGutterRendererClass))
#define XED_IS_VIEW_GUTTER_RENDERER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_VIEW_GUTTER_RENDERER))
#define XED_IS_VIEW_GUTTER_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_VIEW_GUTTER_RENDERER))
#define XED_VIEW_GUTTER_RENDERER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_VIEW_GUTTER_RENDERER, XedViewGutterRendererClass))

typedef struct _XedViewGutterRenderer XedViewGutterRenderer;
typedef struct _XedViewGutterRendererClass XedViewGutterRendererClass;
typedef struct _XedViewGutterRendererPrivate XedViewGutterRendererPrivate;

struct _XedViewGutterRenderer
{
    GtkSourceGutterRenderer parent;

    XedViewGutterRendererPrivate *priv;
};

struct _XedViewGutterRendererClass
{
    GtkSourceGutterRendererClass parent_instance;
};

GType xed_view_gutter_renderer_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __XED_VIEW_GUTTER_RENDERER_H__ */