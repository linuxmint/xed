/*
 * xed-view-frame.h
 * This file is part of xed
 *
 * Copyright (C) 2010 - Ignacio Casal Quinteiro
 *
 * xed is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xed is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xed; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef __XED_VIEW_FRAME_H__
#define __XED_VIEW_FRAME_H__

#include <gtk/gtk.h>
#include "xed-document.h"
#include "xed-view.h"

G_BEGIN_DECLS

#define XED_TYPE_VIEW_FRAME               (xed_view_frame_get_type ())
#define XED_VIEW_FRAME(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_VIEW_FRAME, XedViewFrame))
#define XED_VIEW_FRAME_CONST(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_VIEW_FRAME, XedViewFrame const))
#define XED_VIEW_FRAME_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_VIEW_FRAME, XedViewFrameClass))
#define XED_IS_VIEW_FRAME(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_VIEW_FRAME))
#define XED_IS_VIEW_FRAME_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_VIEW_FRAME))
#define XED_VIEW_FRAME_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), XED_TYPE_VIEW_FRAME, XedViewFrameClass))

typedef struct _XedViewFrame        XedViewFrame;
typedef struct _XedViewFrameClass   XedViewFrameClass;
typedef struct _XedViewFramePrivate XedViewFramePrivate;

struct _XedViewFrame
{
    GtkOverlay parent;

    XedViewFramePrivate *priv;
};

struct _XedViewFrameClass
{
    GtkOverlayClass parent_class;
};

GType xed_view_frame_get_type (void) G_GNUC_CONST;

XedViewFrame *xed_view_frame_new (void);

XedDocument *xed_view_frame_get_document (XedViewFrame *frame);

XedView *xed_view_frame_get_view (XedViewFrame *frame);

GtkFrame *xed_view_frame_get_map_frame (XedViewFrame *frame);

void xed_view_frame_popup_goto_line (XedViewFrame *frame);

gboolean xed_view_frame_get_search_popup_visible (XedViewFrame *frame);

G_END_DECLS

#endif /* __XED_VIEW_FRAME_H__ */
