/*
 * xedit-view.h
 * This file is part of xedit
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi
 * Copyright (C) 2002-2005 Paolo Maggi  
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
 * Modified by the xedit Team, 1998-2005. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifndef __XEDIT_VIEW_H__
#define __XEDIT_VIEW_H__

#include <gtk/gtk.h>

#include <xedit/xedit-document.h>
#include <gtksourceview/gtksourceview.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XEDIT_TYPE_VIEW            (xedit_view_get_type ())
#define XEDIT_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), XEDIT_TYPE_VIEW, XeditView))
#define XEDIT_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), XEDIT_TYPE_VIEW, XeditViewClass))
#define XEDIT_IS_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), XEDIT_TYPE_VIEW))
#define XEDIT_IS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_VIEW))
#define XEDIT_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), XEDIT_TYPE_VIEW, XeditViewClass))

/* Private structure type */
typedef struct _XeditViewPrivate	XeditViewPrivate;

/*
 * Main object structure
 */
typedef struct _XeditView		XeditView;

struct _XeditView
{
	GtkSourceView view;

	/*< private > */
	XeditViewPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XeditViewClass		XeditViewClass;

struct _XeditViewClass
{
	GtkSourceViewClass parent_class;

	/* FIXME: Do we need placeholders ? */

	/* Key bindings */
	gboolean (* start_interactive_search)	(XeditView       *view);
	gboolean (* start_interactive_goto_line)(XeditView       *view);
	gboolean (* reset_searched_text)	(XeditView       *view);

	void	 (* drop_uris)			(XeditView	 *view,
						 gchar          **uri_list);
};

/*
 * Public methods
 */
GType		 xedit_view_get_type     	(void) G_GNUC_CONST;

GtkWidget	*xedit_view_new			(XeditDocument   *doc);

void		 xedit_view_cut_clipboard 	(XeditView       *view);
void		 xedit_view_copy_clipboard 	(XeditView       *view);
void		 xedit_view_paste_clipboard	(XeditView       *view);
void		 xedit_view_delete_selection	(XeditView       *view);
void		 xedit_view_select_all		(XeditView       *view);

void		 xedit_view_scroll_to_cursor 	(XeditView       *view);

void 		 xedit_view_set_font		(XeditView       *view,
						 gboolean         def,
						 const gchar     *font_name);

G_END_DECLS

#endif /* __XEDIT_VIEW_H__ */
