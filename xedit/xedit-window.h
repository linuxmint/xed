/*
 * xedit-window.h
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
 * MERCHANWINDOWILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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
 *
 * $Id$
 */

#ifndef __XEDIT_WINDOW_H__
#define __XEDIT_WINDOW_H__

#include <gio/gio.h>
#include <gtk/gtk.h>

#include <xedit/xedit-tab.h>
#include <xedit/xedit-panel.h>
#include <xedit/xedit-message-bus.h>

G_BEGIN_DECLS

typedef enum
{
	XEDIT_WINDOW_STATE_NORMAL		= 0,
	XEDIT_WINDOW_STATE_SAVING		= 1 << 1,
	XEDIT_WINDOW_STATE_PRINTING		= 1 << 2,
	XEDIT_WINDOW_STATE_LOADING		= 1 << 3,
	XEDIT_WINDOW_STATE_ERROR		= 1 << 4,
	XEDIT_WINDOW_STATE_SAVING_SESSION	= 1 << 5
} XeditWindowState;
	
/*
 * Type checking and casting macros
 */
#define XEDIT_TYPE_WINDOW              (xedit_window_get_type())
#define XEDIT_WINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XEDIT_TYPE_WINDOW, XeditWindow))
#define XEDIT_WINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XEDIT_TYPE_WINDOW, XeditWindowClass))
#define XEDIT_IS_WINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XEDIT_TYPE_WINDOW))
#define XEDIT_IS_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_WINDOW))
#define XEDIT_WINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XEDIT_TYPE_WINDOW, XeditWindowClass))

/* Private structure type */
typedef struct _XeditWindowPrivate XeditWindowPrivate;

/*
 * Main object structure
 */
typedef struct _XeditWindow XeditWindow;

struct _XeditWindow 
{
	GtkWindow window;

	/*< private > */
	XeditWindowPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XeditWindowClass XeditWindowClass;

struct _XeditWindowClass 
{
	GtkWindowClass parent_class;
	
	/* Signals */
	void	 (* tab_added)      	(XeditWindow *window,
				     	 XeditTab    *tab);
	void	 (* tab_removed)    	(XeditWindow *window,
				     	 XeditTab    *tab);
	void	 (* tabs_reordered) 	(XeditWindow *window);
	void	 (* active_tab_changed)	(XeditWindow *window,
				     	 XeditTab    *tab);
	void	 (* active_tab_state_changed)	
					(XeditWindow *window);
};

/*
 * Public methods
 */
GType 		 xedit_window_get_type 			(void) G_GNUC_CONST;

XeditTab	*xedit_window_create_tab		(XeditWindow         *window,
							 gboolean             jump_to);
							 
XeditTab	*xedit_window_create_tab_from_uri	(XeditWindow         *window,
							 const gchar         *uri,
							 const XeditEncoding *encoding,
							 gint                 line_pos,
							 gboolean             create,
							 gboolean             jump_to);
							 
void		 xedit_window_close_tab			(XeditWindow         *window,
							 XeditTab            *tab);
							 
void		 xedit_window_close_all_tabs		(XeditWindow         *window);

void		 xedit_window_close_tabs		(XeditWindow         *window,
							 const GList         *tabs);
							 
XeditTab	*xedit_window_get_active_tab		(XeditWindow         *window);

void		 xedit_window_set_active_tab		(XeditWindow         *window,
							 XeditTab            *tab);

/* Helper functions */
XeditView	*xedit_window_get_active_view		(XeditWindow         *window);
XeditDocument	*xedit_window_get_active_document	(XeditWindow         *window);

/* Returns a newly allocated list with all the documents in the window */
GList		*xedit_window_get_documents		(XeditWindow         *window);

/* Returns a newly allocated list with all the documents that need to be 
   saved before closing the window */
GList		*xedit_window_get_unsaved_documents 	(XeditWindow         *window);

/* Returns a newly allocated list with all the views in the window */
GList		*xedit_window_get_views			(XeditWindow         *window);

GtkWindowGroup  *xedit_window_get_group			(XeditWindow         *window);

XeditPanel	*xedit_window_get_side_panel		(XeditWindow         *window);

XeditPanel	*xedit_window_get_bottom_panel		(XeditWindow         *window);

GtkWidget	*xedit_window_get_statusbar		(XeditWindow         *window);

GtkUIManager	*xedit_window_get_ui_manager		(XeditWindow         *window);

XeditWindowState xedit_window_get_state 		(XeditWindow         *window);

XeditTab        *xedit_window_get_tab_from_location	(XeditWindow         *window,
							 GFile               *location);

XeditTab        *xedit_window_get_tab_from_uri		(XeditWindow         *window,
							 const gchar         *uri);

/* Message bus */
XeditMessageBus	*xedit_window_get_message_bus		(XeditWindow         *window);

/*
 * Non exported functions
 */
GtkWidget	*_xedit_window_get_notebook		(XeditWindow         *window);

XeditWindow	*_xedit_window_move_tab_to_new_window	(XeditWindow         *window,
							 XeditTab            *tab);
gboolean	 _xedit_window_is_removing_tabs		(XeditWindow         *window);

GFile		*_xedit_window_get_default_location 	(XeditWindow         *window);

void		 _xedit_window_set_default_location 	(XeditWindow         *window,
							 GFile               *location);

void		 _xedit_window_set_saving_session_state	(XeditWindow         *window,
							 gboolean             saving_session);

void		 _xedit_window_fullscreen		(XeditWindow         *window);

void		 _xedit_window_unfullscreen		(XeditWindow         *window);

gboolean	 _xedit_window_is_fullscreen		(XeditWindow         *window);

/* these are in xedit-window because of screen safety */
void		 _xedit_recent_add			(XeditWindow	     *window,
							 const gchar         *uri,
							 const gchar         *mime);
void		 _xedit_recent_remove			(XeditWindow         *window,
							 const gchar         *uri);

G_END_DECLS

#endif  /* __XEDIT_WINDOW_H__  */
