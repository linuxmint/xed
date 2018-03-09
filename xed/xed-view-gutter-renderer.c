#include "xed-view-gutter-renderer.h"

#define XED_VIEW_GUTTER_RENDERER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), XED_VIEW_GUTTER_RENDERER, XedViewGutterRendererPrivate))

G_DEFINE_TYPE (XedViewGutterRenderer, xed_view_gutter_renderer, GTK_SOURCE_TYPE_GUTTER_RENDERER)

static void
xed_view_gutter_renderer_class_init (XedViewGutterRendererClass *klass)
{
}

static void
xed_view_gutter_renderer_init (XedViewGutterRenderer *self)
{
}
