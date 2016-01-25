/*
 * xedit-notebook.h
 * This file is part of xedit
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
 * Modified by the xedit Team, 2005. See the AUTHORS file for a 
 * list of people on the xedit Team.  
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
 
#ifndef XEDIT_NOTEBOOK_H
#define XEDIT_NOTEBOOK_H

#include <xedit/xedit-tab.h>

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XEDIT_TYPE_NOTEBOOK		(xedit_notebook_get_type ())
#define XEDIT_NOTEBOOK(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), XEDIT_TYPE_NOTEBOOK, XeditNotebook))
#define XEDIT_NOTEBOOK_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XEDIT_TYPE_NOTEBOOK, XeditNotebookClass))
#define XEDIT_IS_NOTEBOOK(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), XEDIT_TYPE_NOTEBOOK))
#define XEDIT_IS_NOTEBOOK_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), XEDIT_TYPE_NOTEBOOK))
#define XEDIT_NOTEBOOK_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), XEDIT_TYPE_NOTEBOOK, XeditNotebookClass))

/* Private structure type */
typedef struct _XeditNotebookPrivate	XeditNotebookPrivate;

/*
 * Main object structure
 */
typedef struct _XeditNotebook		XeditNotebook;
 
struct _XeditNotebook
{
	GtkNotebook notebook;

	/*< private >*/
        XeditNotebookPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XeditNotebookClass	XeditNotebookClass;

struct _XeditNotebookClass
{
        GtkNotebookClass parent_class;

	/* Signals */
	void	 (* tab_added)      (XeditNotebook *notebook,
				     XeditTab      *tab);
	void	 (* tab_removed)    (XeditNotebook *notebook,
				     XeditTab      *tab);
	void	 (* tab_detached)   (XeditNotebook *notebook,
				     XeditTab      *tab);
	void	 (* tabs_reordered) (XeditNotebook *notebook);
	void	 (* tab_close_request)
				    (XeditNotebook *notebook,
				     XeditTab      *tab);
};

/*
 * Public methods
 */
GType		xedit_notebook_get_type		(void) G_GNUC_CONST;

GtkWidget      *xedit_notebook_new		(void);

void		xedit_notebook_add_tab		(XeditNotebook *nb,
						 XeditTab      *tab,
						 gint           position,
						 gboolean       jump_to);

void		xedit_notebook_remove_tab	(XeditNotebook *nb,
						 XeditTab      *tab);

void		xedit_notebook_remove_all_tabs 	(XeditNotebook *nb);

void		xedit_notebook_reorder_tab	(XeditNotebook *src,
			    			 XeditTab      *tab,
			    			 gint           dest_position);
			    			 
void            xedit_notebook_move_tab		(XeditNotebook *src,
						 XeditNotebook *dest,
						 XeditTab      *tab,
						 gint           dest_position);

/* FIXME: do we really need this function ? */
void		xedit_notebook_set_always_show_tabs	
						(XeditNotebook *nb,
						 gboolean       show_tabs);

void		xedit_notebook_set_close_buttons_sensitive
						(XeditNotebook *nb,
						 gboolean       sensitive);

gboolean	xedit_notebook_get_close_buttons_sensitive
						(XeditNotebook *nb);

void		xedit_notebook_set_tab_drag_and_drop_enabled
						(XeditNotebook *nb,
						 gboolean       enable);

gboolean	xedit_notebook_get_tab_drag_and_drop_enabled
						(XeditNotebook *nb);

G_END_DECLS

#endif /* XEDIT_NOTEBOOK_H */
