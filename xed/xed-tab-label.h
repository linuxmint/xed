/*
 * xed-tab-label.h
 * This file is part of xed
 *
 * Copyright (C) 2010 - Paolo Borelli
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

#ifndef __XED_TAB_LABEL_H__
#define __XED_TAB_LABEL_H__

#include <gtk/gtk.h>
#include <xed/xed-tab.h>

G_BEGIN_DECLS

#define XED_TYPE_TAB_LABEL		(xed_tab_label_get_type ())
#define XED_TAB_LABEL(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_TAB_LABEL, XedTabLabel))
#define XED_TAB_LABEL_CONST(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_TAB_LABEL, XedTabLabel const))
#define XED_TAB_LABEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_TAB_LABEL, XedTabLabelClass))
#define XED_IS_TAB_LABEL(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_TAB_LABEL))
#define XED_IS_TAB_LABEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_TAB_LABEL))
#define XED_TAB_LABEL_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), XED_TYPE_TAB_LABEL, XedTabLabelClass))

typedef struct _XedTabLabel		XedTabLabel;
typedef struct _XedTabLabelClass	XedTabLabelClass;
typedef struct _XedTabLabelPrivate	XedTabLabelPrivate;

struct _XedTabLabel {
#if GTK_CHECK_VERSION (3, 0, 0)
	GtkBox parent;
#else
	GtkHBox parent;
#endif
	
	XedTabLabelPrivate *priv;
};

struct _XedTabLabelClass {
#if GTK_CHECK_VERSION (3, 0, 0)
	GtkBoxClass parent_class;
#else
	GtkHBoxClass parent_class;
#endif

	void (* close_clicked)  (XedTabLabel *tab_label);
};

GType		 xed_tab_label_get_type (void) G_GNUC_CONST;

GtkWidget 	*xed_tab_label_new (XedTab *tab);

XedTab	*xed_tab_label_get_tab (XedTabLabel *tab_label);

void		xed_tab_label_set_close_button_sensitive (XedTabLabel *tab_label,
							    gboolean       sensitive);

G_END_DECLS

#endif /* __XED_TAB_LABEL_H__ */
