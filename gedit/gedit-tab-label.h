/*
 * gedit-tab-label.h
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GEDIT_TAB_LABEL_H__
#define __GEDIT_TAB_LABEL_H__

#include <gtk/gtk.h>
#include <gedit/gedit-tab.h>

G_BEGIN_DECLS

#define GEDIT_TYPE_TAB_LABEL		(gedit_tab_label_get_type ())
#define GEDIT_TAB_LABEL(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_TAB_LABEL, GeditTabLabel))
#define GEDIT_TAB_LABEL_CONST(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_TAB_LABEL, GeditTabLabel const))
#define GEDIT_TAB_LABEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_TAB_LABEL, GeditTabLabelClass))
#define GEDIT_IS_TAB_LABEL(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_TAB_LABEL))
#define GEDIT_IS_TAB_LABEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_TAB_LABEL))
#define GEDIT_TAB_LABEL_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_TYPE_TAB_LABEL, GeditTabLabelClass))

typedef struct _GeditTabLabel		GeditTabLabel;
typedef struct _GeditTabLabelClass	GeditTabLabelClass;
typedef struct _GeditTabLabelPrivate	GeditTabLabelPrivate;

struct _GeditTabLabel {
	GtkHBox parent;
	
	GeditTabLabelPrivate *priv;
};

struct _GeditTabLabelClass {
	GtkHBoxClass parent_class;

	void (* close_clicked)  (GeditTabLabel *tab_label);
};

GType		 gedit_tab_label_get_type (void) G_GNUC_CONST;

GtkWidget 	*gedit_tab_label_new (GeditTab *tab);

GeditTab	*gedit_tab_label_get_tab (GeditTabLabel *tab_label);

void		gedit_tab_label_set_close_button_sensitive (GeditTabLabel *tab_label,
							    gboolean       sensitive);

G_END_DECLS

#endif /* __GEDIT_TAB_LABEL_H__ */
