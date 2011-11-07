/*
 * gedit-window.h
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */
 
/*
 * Modified by the gedit Team, 2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __GEDIT_WINDOW_H__
#define __GEDIT_WINDOW_H__

#include <gio/gio.h>
#include <gtk/gtk.h>

#include <gedit/gedit-tab.h>
#include <gedit/gedit-panel.h>
#include <gedit/gedit-message-bus.h>

G_BEGIN_DECLS

typedef enum
{
	GEDIT_WINDOW_STATE_NORMAL		= 0,
	GEDIT_WINDOW_STATE_SAVING		= 1 << 1,
	GEDIT_WINDOW_STATE_PRINTING		= 1 << 2,
	GEDIT_WINDOW_STATE_LOADING		= 1 << 3,
	GEDIT_WINDOW_STATE_ERROR		= 1 << 4,
	GEDIT_WINDOW_STATE_SAVING_SESSION	= 1 << 5
} GeditWindowState;
	
/*
 * Type checking and casting macros
 */
#define GEDIT_TYPE_WINDOW              (gedit_window_get_type())
#define GEDIT_WINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GEDIT_TYPE_WINDOW, GeditWindow))
#define GEDIT_WINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GEDIT_TYPE_WINDOW, GeditWindowClass))
#define GEDIT_IS_WINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GEDIT_TYPE_WINDOW))
#define GEDIT_IS_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_WINDOW))
#define GEDIT_WINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GEDIT_TYPE_WINDOW, GeditWindowClass))

/* Private structure type */
typedef struct _GeditWindowPrivate GeditWindowPrivate;

/*
 * Main object structure
 */
typedef struct _GeditWindow GeditWindow;

struct _GeditWindow 
{
	GtkWindow window;

	/*< private > */
	GeditWindowPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GeditWindowClass GeditWindowClass;

struct _GeditWindowClass 
{
	GtkWindowClass parent_class;
	
	/* Signals */
	void	 (* tab_added)      	(GeditWindow *window,
				     	 GeditTab    *tab);
	void	 (* tab_removed)    	(GeditWindow *window,
				     	 GeditTab    *tab);
	void	 (* tabs_reordered) 	(GeditWindow *window);
	void	 (* active_tab_changed)	(GeditWindow *window,
				     	 GeditTab    *tab);
	void	 (* active_tab_state_changed)	
					(GeditWindow *window);
};

/*
 * Public methods
 */
GType 		 gedit_window_get_type 			(void) G_GNUC_CONST;

GeditTab	*gedit_window_create_tab		(GeditWindow         *window,
							 gboolean             jump_to);
							 
GeditTab	*gedit_window_create_tab_from_uri	(GeditWindow         *window,
							 const gchar         *uri,
							 const GeditEncoding *encoding,
							 gint                 line_pos,
							 gboolean             create,
							 gboolean             jump_to);
							 
void		 gedit_window_close_tab			(GeditWindow         *window,
							 GeditTab            *tab);
							 
void		 gedit_window_close_all_tabs		(GeditWindow         *window);

void		 gedit_window_close_tabs		(GeditWindow         *window,
							 const GList         *tabs);
							 
GeditTab	*gedit_window_get_active_tab		(GeditWindow         *window);

void		 gedit_window_set_active_tab		(GeditWindow         *window,
							 GeditTab            *tab);

/* Helper functions */
GeditView	*gedit_window_get_active_view		(GeditWindow         *window);
GeditDocument	*gedit_window_get_active_document	(GeditWindow         *window);

/* Returns a newly allocated list with all the documents in the window */
GList		*gedit_window_get_documents		(GeditWindow         *window);

/* Returns a newly allocated list with all the documents that need to be 
   saved before closing the window */
GList		*gedit_window_get_unsaved_documents 	(GeditWindow         *window);

/* Returns a newly allocated list with all the views in the window */
GList		*gedit_window_get_views			(GeditWindow         *window);

GtkWindowGroup  *gedit_window_get_group			(GeditWindow         *window);

GeditPanel	*gedit_window_get_side_panel		(GeditWindow         *window);

GeditPanel	*gedit_window_get_bottom_panel		(GeditWindow         *window);

GtkWidget	*gedit_window_get_statusbar		(GeditWindow         *window);

GtkUIManager	*gedit_window_get_ui_manager		(GeditWindow         *window);

GeditWindowState gedit_window_get_state 		(GeditWindow         *window);

GeditTab        *gedit_window_get_tab_from_location	(GeditWindow         *window,
							 GFile               *location);

GeditTab        *gedit_window_get_tab_from_uri		(GeditWindow         *window,
							 const gchar         *uri);

/* Message bus */
GeditMessageBus	*gedit_window_get_message_bus		(GeditWindow         *window);

/*
 * Non exported functions
 */
GtkWidget	*_gedit_window_get_notebook		(GeditWindow         *window);

GeditWindow	*_gedit_window_move_tab_to_new_window	(GeditWindow         *window,
							 GeditTab            *tab);
gboolean	 _gedit_window_is_removing_tabs		(GeditWindow         *window);

GFile		*_gedit_window_get_default_location 	(GeditWindow         *window);

void		 _gedit_window_set_default_location 	(GeditWindow         *window,
							 GFile               *location);

void		 _gedit_window_set_saving_session_state	(GeditWindow         *window,
							 gboolean             saving_session);

void		 _gedit_window_fullscreen		(GeditWindow         *window);

void		 _gedit_window_unfullscreen		(GeditWindow         *window);

gboolean	 _gedit_window_is_fullscreen		(GeditWindow         *window);

/* these are in gedit-window because of screen safety */
void		 _gedit_recent_add			(GeditWindow	     *window,
							 const gchar         *uri,
							 const gchar         *mime);
void		 _gedit_recent_remove			(GeditWindow         *window,
							 const gchar         *uri);

G_END_DECLS

#endif  /* __GEDIT_WINDOW_H__  */
