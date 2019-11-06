/*
 * xed-tab.h
 * This file is part of xed
 *
 * Copyright (C) 2005 - Paolo Maggi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/*
 * Modified by the xed Team, 2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __XED_TAB_H__
#define __XED_TAB_H__

#include <gtksourceview/gtksource.h>
#include <xed/xed-view.h>
#include <xed/xed-document.h>

G_BEGIN_DECLS

typedef enum
{
    XED_TAB_STATE_NORMAL = 0,
    XED_TAB_STATE_LOADING,
    XED_TAB_STATE_REVERTING,
    XED_TAB_STATE_SAVING,
    XED_TAB_STATE_PRINTING,
    XED_TAB_STATE_PRINT_PREVIEWING,
    XED_TAB_STATE_SHOWING_PRINT_PREVIEW,
    XED_TAB_STATE_GENERIC_NOT_EDITABLE,
    XED_TAB_STATE_LOADING_ERROR,
    XED_TAB_STATE_REVERTING_ERROR,
    XED_TAB_STATE_SAVING_ERROR,
    XED_TAB_STATE_GENERIC_ERROR,
    XED_TAB_STATE_CLOSING,
    XED_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION,
    XED_TAB_NUM_OF_STATES /* This is not a valid state */
} XedTabState;

#define XED_TYPE_TAB              (xed_tab_get_type())
#define XED_TAB(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_TAB, XedTab))
#define XED_TAB_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_TAB, XedTabClass))
#define XED_IS_TAB(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_TAB))
#define XED_IS_TAB_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_TAB))
#define XED_TAB_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_TAB, XedTabClass))

typedef struct _XedTab          XedTab;
typedef struct _XedTabPrivate   XedTabPrivate;
typedef struct _XedTabClass     XedTabClass;

struct _XedTab
{
    GtkBox vbox;

    /*< private > */
    XedTabPrivate *priv;
};

struct _XedTabClass
{
    GtkBoxClass parent_class;

};

GType xed_tab_get_type (void) G_GNUC_CONST;

XedView *xed_tab_get_view (XedTab *tab);

/* This is only an helper function */
XedDocument *xed_tab_get_document (XedTab *tab);
XedTab *xed_tab_get_from_document (XedDocument *doc);

XedTabState xed_tab_get_state (XedTab *tab);

gboolean xed_tab_get_auto_save_enabled (XedTab *tab);
void         xed_tab_set_auto_save_enabled (XedTab   *tab,
                                            gboolean  enable);

gint xed_tab_get_auto_save_interval (XedTab *tab);
void xed_tab_set_auto_save_interval (XedTab *tab,
                                     gint    interval);

void xed_tab_set_info_bar (XedTab    *tab,
                           GtkWidget *info_bar);
/*
 * Non exported methods
 */
GtkWidget *_xed_tab_new (void);

/* Whether create is TRUE, creates a new empty document if location does
   not refer to an existing file */
GtkWidget *_xed_tab_new_from_location  (GFile                   *location,
                                        const GtkSourceEncoding *encoding,
                                        gint                     line_pos,
                                        gboolean                 create);

GtkWidget *_xed_tab_new_from_stream (GInputStream            *stream,
                                     const GtkSourceEncoding *encoding,
                                     gint                     line_pos);

gchar *_xed_tab_get_name (XedTab *tab);

gchar *_xed_tab_get_tooltips (XedTab *tab);

GdkPixbuf *_xed_tab_get_icon (XedTab *tab);

void _xed_tab_load (XedTab                  *tab,
                    GFile                   *location,
                    const GtkSourceEncoding *encoding,
                    gint                     line_pos,
                    gboolean                 create);

void _xed_tab_load_stream     (XedTab                  *tab,
                               GInputStream            *location,
                               const GtkSourceEncoding *encoding,
                               gint                     line_pos);

void _xed_tab_revert (XedTab *tab);

void _xed_tab_save_async (XedTab              *tab,
                          GCancellable        *cancellable,
                          GAsyncReadyCallback  callback,
                          gpointer             user_data);


gboolean _xed_tab_save_finish (XedTab       *tab,
                               GAsyncResult *result);

void _xed_tab_save_as_async (XedTab                   *tab,
                             GFile                    *location,
                             const GtkSourceEncoding  *encoding,
                             GtkSourceNewlineType      newline_type,
                             GCancellable             *cancellable,
                             GAsyncReadyCallback       callback,
                             gpointer                  user_data);

void _xed_tab_print (XedTab *tab, gboolean show_dialog);
void _xed_tab_print_preview (XedTab *tab);
void _xed_tab_mark_for_closing (XedTab *tab);
gboolean _xed_tab_get_can_close (XedTab *tab);
GtkWidget *_xed_tab_get_view_frame (XedTab *tab);

G_END_DECLS

#endif  /* __XED_TAB_H__  */
