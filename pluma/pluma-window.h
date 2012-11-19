/*
 * pluma-window.h
 * This file is part of pluma
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
 * MERCHANWINDOWILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA 02110-1301, USA.
 */
 
/*
 * Modified by the pluma Team, 2005. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __PLUMA_WINDOW_H__
#define __PLUMA_WINDOW_H__

#include <gio/gio.h>
#include <gtk/gtk.h>

#include <pluma/pluma-tab.h>
#include <pluma/pluma-panel.h>
#include <pluma/pluma-message-bus.h>

G_BEGIN_DECLS

typedef enum
{
	PLUMA_WINDOW_STATE_NORMAL		= 0,
	PLUMA_WINDOW_STATE_SAVING		= 1 << 1,
	PLUMA_WINDOW_STATE_PRINTING		= 1 << 2,
	PLUMA_WINDOW_STATE_LOADING		= 1 << 3,
	PLUMA_WINDOW_STATE_ERROR		= 1 << 4,
	PLUMA_WINDOW_STATE_SAVING_SESSION	= 1 << 5
} PlumaWindowState;
	
/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_WINDOW              (pluma_window_get_type())
#define PLUMA_WINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_WINDOW, PlumaWindow))
#define PLUMA_WINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), PLUMA_TYPE_WINDOW, PlumaWindowClass))
#define PLUMA_IS_WINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), PLUMA_TYPE_WINDOW))
#define PLUMA_IS_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_WINDOW))
#define PLUMA_WINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), PLUMA_TYPE_WINDOW, PlumaWindowClass))

/* Private structure type */
typedef struct _PlumaWindowPrivate PlumaWindowPrivate;

/*
 * Main object structure
 */
typedef struct _PlumaWindow PlumaWindow;

struct _PlumaWindow 
{
	GtkWindow window;

	/*< private > */
	PlumaWindowPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _PlumaWindowClass PlumaWindowClass;

struct _PlumaWindowClass 
{
	GtkWindowClass parent_class;
	
	/* Signals */
	void	 (* tab_added)      	(PlumaWindow *window,
				     	 PlumaTab    *tab);
	void	 (* tab_removed)    	(PlumaWindow *window,
				     	 PlumaTab    *tab);
	void	 (* tabs_reordered) 	(PlumaWindow *window);
	void	 (* active_tab_changed)	(PlumaWindow *window,
				     	 PlumaTab    *tab);
	void	 (* active_tab_state_changed)	
					(PlumaWindow *window);
};

/*
 * Public methods
 */
GType 		 pluma_window_get_type 			(void) G_GNUC_CONST;

PlumaTab	*pluma_window_create_tab		(PlumaWindow         *window,
							 gboolean             jump_to);
							 
PlumaTab	*pluma_window_create_tab_from_uri	(PlumaWindow         *window,
							 const gchar         *uri,
							 const PlumaEncoding *encoding,
							 gint                 line_pos,
							 gboolean             create,
							 gboolean             jump_to);
							 
void		 pluma_window_close_tab			(PlumaWindow         *window,
							 PlumaTab            *tab);
							 
void		 pluma_window_close_all_tabs		(PlumaWindow         *window);

void		 pluma_window_close_tabs		(PlumaWindow         *window,
							 const GList         *tabs);
							 
PlumaTab	*pluma_window_get_active_tab		(PlumaWindow         *window);

void		 pluma_window_set_active_tab		(PlumaWindow         *window,
							 PlumaTab            *tab);

/* Helper functions */
PlumaView	*pluma_window_get_active_view		(PlumaWindow         *window);
PlumaDocument	*pluma_window_get_active_document	(PlumaWindow         *window);

/* Returns a newly allocated list with all the documents in the window */
GList		*pluma_window_get_documents		(PlumaWindow         *window);

/* Returns a newly allocated list with all the documents that need to be 
   saved before closing the window */
GList		*pluma_window_get_unsaved_documents 	(PlumaWindow         *window);

/* Returns a newly allocated list with all the views in the window */
GList		*pluma_window_get_views			(PlumaWindow         *window);

GtkWindowGroup  *pluma_window_get_group			(PlumaWindow         *window);

PlumaPanel	*pluma_window_get_side_panel		(PlumaWindow         *window);

PlumaPanel	*pluma_window_get_bottom_panel		(PlumaWindow         *window);

GtkWidget	*pluma_window_get_statusbar		(PlumaWindow         *window);

GtkUIManager	*pluma_window_get_ui_manager		(PlumaWindow         *window);

PlumaWindowState pluma_window_get_state 		(PlumaWindow         *window);

PlumaTab        *pluma_window_get_tab_from_location	(PlumaWindow         *window,
							 GFile               *location);

PlumaTab        *pluma_window_get_tab_from_uri		(PlumaWindow         *window,
							 const gchar         *uri);

/* Message bus */
PlumaMessageBus	*pluma_window_get_message_bus		(PlumaWindow         *window);

/*
 * Non exported functions
 */
GtkWidget	*_pluma_window_get_notebook		(PlumaWindow         *window);

PlumaWindow	*_pluma_window_move_tab_to_new_window	(PlumaWindow         *window,
							 PlumaTab            *tab);
gboolean	 _pluma_window_is_removing_tabs		(PlumaWindow         *window);

GFile		*_pluma_window_get_default_location 	(PlumaWindow         *window);

void		 _pluma_window_set_default_location 	(PlumaWindow         *window,
							 GFile               *location);

void		 _pluma_window_set_saving_session_state	(PlumaWindow         *window,
							 gboolean             saving_session);

void		 _pluma_window_fullscreen		(PlumaWindow         *window);

void		 _pluma_window_unfullscreen		(PlumaWindow         *window);

gboolean	 _pluma_window_is_fullscreen		(PlumaWindow         *window);

/* these are in pluma-window because of screen safety */
void		 _pluma_recent_add			(PlumaWindow	     *window,
							 const gchar         *uri,
							 const gchar         *mime);
void		 _pluma_recent_remove			(PlumaWindow         *window,
							 const gchar         *uri);

G_END_DECLS

#endif  /* __PLUMA_WINDOW_H__  */
