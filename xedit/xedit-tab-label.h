/*
 * xedit-tab-label.h
 * This file is part of xedit
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

#ifndef __XEDIT_TAB_LABEL_H__
#define __XEDIT_TAB_LABEL_H__

#include <gtk/gtk.h>
#include <xedit/xedit-tab.h>

G_BEGIN_DECLS

#define XEDIT_TYPE_TAB_LABEL		(xedit_tab_label_get_type ())
#define XEDIT_TAB_LABEL(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_TAB_LABEL, XeditTabLabel))
#define XEDIT_TAB_LABEL_CONST(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_TAB_LABEL, XeditTabLabel const))
#define XEDIT_TAB_LABEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_TAB_LABEL, XeditTabLabelClass))
#define XEDIT_IS_TAB_LABEL(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XEDIT_TYPE_TAB_LABEL))
#define XEDIT_IS_TAB_LABEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_TAB_LABEL))
#define XEDIT_TAB_LABEL_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), XEDIT_TYPE_TAB_LABEL, XeditTabLabelClass))

typedef struct _XeditTabLabel		XeditTabLabel;
typedef struct _XeditTabLabelClass	XeditTabLabelClass;
typedef struct _XeditTabLabelPrivate	XeditTabLabelPrivate;

struct _XeditTabLabel {
#if GTK_CHECK_VERSION (3, 0, 0)
	GtkBox parent;
#else
	GtkHBox parent;
#endif
	
	XeditTabLabelPrivate *priv;
};

struct _XeditTabLabelClass {
#if GTK_CHECK_VERSION (3, 0, 0)
	GtkBoxClass parent_class;
#else
	GtkHBoxClass parent_class;
#endif

	void (* close_clicked)  (XeditTabLabel *tab_label);
};

GType		 xedit_tab_label_get_type (void) G_GNUC_CONST;

GtkWidget 	*xedit_tab_label_new (XeditTab *tab);

XeditTab	*xedit_tab_label_get_tab (XeditTabLabel *tab_label);

void		xedit_tab_label_set_close_button_sensitive (XeditTabLabel *tab_label,
							    gboolean       sensitive);

G_END_DECLS

#endif /* __XEDIT_TAB_LABEL_H__ */
