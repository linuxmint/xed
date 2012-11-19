/*
 * pluma-view.h
 * This file is part of pluma
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
 * Modified by the pluma Team, 1998-2005. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifndef __PLUMA_VIEW_H__
#define __PLUMA_VIEW_H__

#include <gtk/gtk.h>

#include <pluma/pluma-document.h>
#include <gtksourceview/gtksourceview.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_VIEW            (pluma_view_get_type ())
#define PLUMA_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_VIEW, PlumaView))
#define PLUMA_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PLUMA_TYPE_VIEW, PlumaViewClass))
#define PLUMA_IS_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PLUMA_TYPE_VIEW))
#define PLUMA_IS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_VIEW))
#define PLUMA_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PLUMA_TYPE_VIEW, PlumaViewClass))

/* Private structure type */
typedef struct _PlumaViewPrivate	PlumaViewPrivate;

/*
 * Main object structure
 */
typedef struct _PlumaView		PlumaView;

struct _PlumaView
{
	GtkSourceView view;

	/*< private > */
	PlumaViewPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _PlumaViewClass		PlumaViewClass;

struct _PlumaViewClass
{
	GtkSourceViewClass parent_class;

	/* FIXME: Do we need placeholders ? */

	/* Key bindings */
	gboolean (* start_interactive_search)	(PlumaView       *view);
	gboolean (* start_interactive_goto_line)(PlumaView       *view);
	gboolean (* reset_searched_text)	(PlumaView       *view);

	void	 (* drop_uris)			(PlumaView	 *view,
						 gchar          **uri_list);
};

/*
 * Public methods
 */
GType		 pluma_view_get_type     	(void) G_GNUC_CONST;

GtkWidget	*pluma_view_new			(PlumaDocument   *doc);

void		 pluma_view_cut_clipboard 	(PlumaView       *view);
void		 pluma_view_copy_clipboard 	(PlumaView       *view);
void		 pluma_view_paste_clipboard	(PlumaView       *view);
void		 pluma_view_delete_selection	(PlumaView       *view);
void		 pluma_view_select_all		(PlumaView       *view);

void		 pluma_view_scroll_to_cursor 	(PlumaView       *view);

void 		 pluma_view_set_font		(PlumaView       *view,
						 gboolean         def,
						 const gchar     *font_name);

G_END_DECLS

#endif /* __PLUMA_VIEW_H__ */
