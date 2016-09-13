/*
 * xed-window.h
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
 * MERCHANWINDOWILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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
 *
 * $Id$
 */

#ifndef __XED_WINDOW_H__
#define __XED_WINDOW_H__

#include <gio/gio.h>
#include <gtk/gtk.h>

#include <xed/xed-tab.h>
#include <xed/xed-panel.h>
#include <xed/xed-message-bus.h>

G_BEGIN_DECLS

typedef enum
{
	XED_WINDOW_STATE_NORMAL		= 0,
	XED_WINDOW_STATE_SAVING		= 1 << 1,
	XED_WINDOW_STATE_PRINTING		= 1 << 2,
	XED_WINDOW_STATE_LOADING		= 1 << 3,
	XED_WINDOW_STATE_ERROR		= 1 << 4,
	XED_WINDOW_STATE_SAVING_SESSION	= 1 << 5
} XedWindowState;
	
/*
 * Type checking and casting macros
 */
#define XED_TYPE_WINDOW              (xed_window_get_type())
#define XED_WINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_WINDOW, XedWindow))
#define XED_WINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_WINDOW, XedWindowClass))
#define XED_IS_WINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_WINDOW))
#define XED_IS_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_WINDOW))
#define XED_WINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_WINDOW, XedWindowClass))

/* Private structure type */
typedef struct _XedWindowPrivate XedWindowPrivate;

/*
 * Main object structure
 */
typedef struct _XedWindow XedWindow;

struct _XedWindow 
{
	GtkWindow window;

	/*< private > */
	XedWindowPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XedWindowClass XedWindowClass;

struct _XedWindowClass 
{
	GtkWindowClass parent_class;
	
	/* Signals */
	void	 (* tab_added)      	(XedWindow *window,
				     	 XedTab    *tab);
	void	 (* tab_removed)    	(XedWindow *window,
				     	 XedTab    *tab);
	void	 (* tabs_reordered) 	(XedWindow *window);
	void	 (* active_tab_changed)	(XedWindow *window,
				     	 XedTab    *tab);
	void	 (* active_tab_state_changed)	
					(XedWindow *window);
};

/*
 * Public methods
 */
GType 		 xed_window_get_type 			(void) G_GNUC_CONST;

XedTab	*xed_window_create_tab		(XedWindow         *window,
							 gboolean             jump_to);
							 
XedTab	*xed_window_create_tab_from_uri	(XedWindow         *window,
							 const gchar         *uri,
							 const XedEncoding *encoding,
							 gint                 line_pos,
							 gboolean             create,
							 gboolean             jump_to);
							 
void		 xed_window_close_tab			(XedWindow         *window,
							 XedTab            *tab);
							 
void		 xed_window_close_all_tabs		(XedWindow         *window);

void		 xed_window_close_tabs		(XedWindow         *window,
							 const GList         *tabs);
							 
XedTab	*xed_window_get_active_tab		(XedWindow         *window);

void		 xed_window_set_active_tab		(XedWindow         *window,
							 XedTab            *tab);

/* Helper functions */
XedView	*xed_window_get_active_view		(XedWindow         *window);
XedDocument	*xed_window_get_active_document	(XedWindow         *window);

/* Returns a newly allocated list with all the documents in the window */
GList		*xed_window_get_documents		(XedWindow         *window);

/* Returns a newly allocated list with all the documents that need to be 
   saved before closing the window */
GList		*xed_window_get_unsaved_documents 	(XedWindow         *window);

/* Returns a newly allocated list with all the views in the window */
GList		*xed_window_get_views			(XedWindow         *window);

GtkWindowGroup  *xed_window_get_group			(XedWindow         *window);

XedPanel	*xed_window_get_side_panel		(XedWindow         *window);

XedPanel	*xed_window_get_bottom_panel		(XedWindow         *window);

GtkWidget	*xed_window_get_statusbar		(XedWindow         *window);

GtkWidget	*xed_window_get_searchbar		(XedWindow         *window);

GtkUIManager	*xed_window_get_ui_manager		(XedWindow         *window);

XedWindowState xed_window_get_state 		(XedWindow         *window);

XedTab        *xed_window_get_tab_from_location	(XedWindow         *window,
							 GFile               *location);

XedTab        *xed_window_get_tab_from_uri		(XedWindow         *window,
							 const gchar         *uri);

/* Message bus */
XedMessageBus	*xed_window_get_message_bus		(XedWindow         *window);

/*
 * Non exported functions
 */
GtkWidget	*_xed_window_get_notebook		(XedWindow         *window);

XedWindow	*_xed_window_move_tab_to_new_window	(XedWindow         *window,
							 XedTab            *tab);
gboolean	 _xed_window_is_removing_tabs		(XedWindow         *window);

GFile		*_xed_window_get_default_location 	(XedWindow         *window);

void		 _xed_window_set_default_location 	(XedWindow         *window,
							 GFile               *location);

void		 _xed_window_set_saving_session_state	(XedWindow         *window,
							 gboolean             saving_session);

void		 _xed_window_fullscreen		(XedWindow         *window);

void		 _xed_window_unfullscreen		(XedWindow         *window);

gboolean	 _xed_window_is_fullscreen		(XedWindow         *window);

/* these are in xed-window because of screen safety */
void		 _xed_recent_add			(XedWindow	     *window,
							 const gchar         *uri,
							 const gchar         *mime);
void		 _xed_recent_remove			(XedWindow         *window,
							 const gchar         *uri);

G_END_DECLS

#endif  /* __XED_WINDOW_H__  */
