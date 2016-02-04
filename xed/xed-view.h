/*
 * xed-view.h
 * This file is part of xed
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
 * Modified by the xed Team, 1998-2005. See the AUTHORS file for a 
 * list of people on the xed Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifndef __XED_VIEW_H__
#define __XED_VIEW_H__

#include <gtk/gtk.h>

#include <xed/xed-document.h>
#include <gtksourceview/gtksourceview.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XED_TYPE_VIEW            (xed_view_get_type ())
#define XED_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_VIEW, XedView))
#define XED_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_VIEW, XedViewClass))
#define XED_IS_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_VIEW))
#define XED_IS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_VIEW))
#define XED_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_VIEW, XedViewClass))

/* Private structure type */
typedef struct _XedViewPrivate	XedViewPrivate;

/*
 * Main object structure
 */
typedef struct _XedView		XedView;

struct _XedView
{
	GtkSourceView view;

	/*< private > */
	XedViewPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XedViewClass		XedViewClass;

struct _XedViewClass
{
	GtkSourceViewClass parent_class;

	/* FIXME: Do we need placeholders ? */

	/* Key bindings */
	gboolean (* start_interactive_search)	(XedView       *view);
	gboolean (* start_interactive_goto_line)(XedView       *view);
	gboolean (* reset_searched_text)	(XedView       *view);

	void	 (* drop_uris)			(XedView	 *view,
						 gchar          **uri_list);
};

/*
 * Public methods
 */
GType		 xed_view_get_type     	(void) G_GNUC_CONST;

GtkWidget	*xed_view_new			(XedDocument   *doc);

void		 xed_view_cut_clipboard 	(XedView       *view);
void		 xed_view_copy_clipboard 	(XedView       *view);
void		 xed_view_paste_clipboard	(XedView       *view);
void		 xed_view_delete_selection	(XedView       *view);
void		 xed_view_select_all		(XedView       *view);

void		 xed_view_scroll_to_cursor 	(XedView       *view);

void 		 xed_view_set_font		(XedView       *view,
						 gboolean         def,
						 const gchar     *font_name);

G_END_DECLS

#endif /* __XED_VIEW_H__ */
