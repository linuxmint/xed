/*
 * xedit-app.h
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
 *
 * $Id$
 */

#ifndef __XEDIT_APP_H__
#define __XEDIT_APP_H__

#include <gtk/gtk.h>

#include <xedit/xedit-window.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XEDIT_TYPE_APP              (xedit_app_get_type())
#define XEDIT_APP(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XEDIT_TYPE_APP, XeditApp))
#define XEDIT_APP_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XEDIT_TYPE_APP, XeditAppClass))
#define XEDIT_IS_APP(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XEDIT_TYPE_APP))
#define XEDIT_IS_APP_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_APP))
#define XEDIT_APP_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XEDIT_TYPE_APP, XeditAppClass))

/* Private structure type */
typedef struct _XeditAppPrivate XeditAppPrivate;

/*
 * Main object structure
 */
typedef struct _XeditApp XeditApp;

struct _XeditApp 
{
	GObject object;

	/*< private > */
	XeditAppPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XeditAppClass XeditAppClass;

struct _XeditAppClass 
{
	GObjectClass parent_class;
};

/*
 * Lockdown mask definition
 */
typedef enum
{
	XEDIT_LOCKDOWN_COMMAND_LINE	= 1 << 0,
	XEDIT_LOCKDOWN_PRINTING		= 1 << 1,
	XEDIT_LOCKDOWN_PRINT_SETUP	= 1 << 2,
	XEDIT_LOCKDOWN_SAVE_TO_DISK	= 1 << 3,
	XEDIT_LOCKDOWN_ALL		= 0xF
} XeditLockdownMask;

/*
 * Public methods
 */
GType 		 xedit_app_get_type 			(void) G_GNUC_CONST;

XeditApp 	*xedit_app_get_default			(void);

XeditWindow	*xedit_app_create_window		(XeditApp  *app,
							 GdkScreen *screen);

const GList	*xedit_app_get_windows			(XeditApp *app);
XeditWindow	*xedit_app_get_active_window		(XeditApp *app);

/* Returns a newly allocated list with all the documents */
GList		*xedit_app_get_documents		(XeditApp *app);

/* Returns a newly allocated list with all the views */
GList		*xedit_app_get_views			(XeditApp *app);

/* Lockdown state */
XeditLockdownMask xedit_app_get_lockdown		(XeditApp *app);

/*
 * Non exported functions
 */
XeditWindow	*_xedit_app_restore_window		(XeditApp    *app,
							 const gchar *role);
XeditWindow	*_xedit_app_get_window_in_viewport	(XeditApp     *app,
							 GdkScreen    *screen,
							 gint          workspace,
							 gint          viewport_x,
							 gint          viewport_y);
void		 _xedit_app_set_lockdown		(XeditApp          *app,
							 XeditLockdownMask  lockdown);
void		 _xedit_app_set_lockdown_bit		(XeditApp          *app,
							 XeditLockdownMask  bit,
							 gboolean           value);
/*
 * This one is a xedit-window function, but we declare it here to avoid
 * #include headaches since it needs the XeditLockdownMask declaration.
 */
void		 _xedit_window_set_lockdown		(XeditWindow         *window,
							 XeditLockdownMask    lockdown);

/* global print config */
GtkPageSetup		*_xedit_app_get_default_page_setup	(XeditApp         *app);
void			 _xedit_app_set_default_page_setup	(XeditApp         *app,
								 GtkPageSetup     *page_setup);
GtkPrintSettings	*_xedit_app_get_default_print_settings	(XeditApp         *app);
void			 _xedit_app_set_default_print_settings	(XeditApp         *app,
								 GtkPrintSettings *settings);

G_END_DECLS

#endif  /* __XEDIT_APP_H__  */
