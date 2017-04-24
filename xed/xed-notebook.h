/*
 * xed-notebook.h
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
 */

/* This file is a modified version of the epiphany file ephy-notebook.h
 * Here the relevant copyright:
 *
 *  Copyright (C) 2002 Christophe Fergeau
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003, 2004 Christian Persch
 *
 */

#ifndef XED_NOTEBOOK_H
#define XED_NOTEBOOK_H

#include <xed/xed-tab.h>

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XED_TYPE_NOTEBOOK		(xed_notebook_get_type ())
#define XED_NOTEBOOK(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), XED_TYPE_NOTEBOOK, XedNotebook))
#define XED_NOTEBOOK_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XED_TYPE_NOTEBOOK, XedNotebookClass))
#define XED_IS_NOTEBOOK(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), XED_TYPE_NOTEBOOK))
#define XED_IS_NOTEBOOK_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), XED_TYPE_NOTEBOOK))
#define XED_NOTEBOOK_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), XED_TYPE_NOTEBOOK, XedNotebookClass))

/* Private structure type */
typedef struct _XedNotebookPrivate	XedNotebookPrivate;

/*
 * Main object structure
 */
typedef struct _XedNotebook		XedNotebook;

struct _XedNotebook
{
	GtkNotebook notebook;

	/*< private >*/
        XedNotebookPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XedNotebookClass	XedNotebookClass;

struct _XedNotebookClass
{
        GtkNotebookClass parent_class;

	/* Signals */
	void	 (* tab_added)      (XedNotebook *notebook,
				     XedTab      *tab);
	void	 (* tab_removed)    (XedNotebook *notebook,
				     XedTab      *tab);
	void	 (* tab_detached)   (XedNotebook *notebook,
				     XedTab      *tab);
	void	 (* tabs_reordered) (XedNotebook *notebook);
	void	 (* tab_close_request)
				    (XedNotebook *notebook,
				     XedTab      *tab);
};

/*
 * Public methods
 */
GType		xed_notebook_get_type		(void) G_GNUC_CONST;

GtkWidget      *xed_notebook_new		(void);

void		xed_notebook_add_tab		(XedNotebook *nb,
						 XedTab      *tab,
						 gint           position,
						 gboolean       jump_to);

void		xed_notebook_remove_tab	(XedNotebook *nb,
						 XedTab      *tab);

void		xed_notebook_remove_all_tabs 	(XedNotebook *nb);

GList *xed_notebook_get_all_tabs (XedNotebook *nb);

void		xed_notebook_reorder_tab	(XedNotebook *src,
			    			 XedTab      *tab,
			    			 gint           dest_position);

void            xed_notebook_move_tab		(XedNotebook *src,
						 XedNotebook *dest,
						 XedTab      *tab,
						 gint           dest_position);

void		xed_notebook_set_close_buttons_sensitive
						(XedNotebook *nb,
						 gboolean       sensitive);

gboolean	xed_notebook_get_close_buttons_sensitive
						(XedNotebook *nb);

void		xed_notebook_set_tab_drag_and_drop_enabled
						(XedNotebook *nb,
						 gboolean       enable);

gboolean	xed_notebook_get_tab_drag_and_drop_enabled
						(XedNotebook *nb);

void        xed_notebook_set_tab_scrolling_enabled (XedNotebook *nb,
                                                    gboolean     enable);
gboolean    xed_notebook_get_tab_scrolling_enabled (XedNotebook *nb);

G_END_DECLS

#endif /* XED_NOTEBOOK_H */
