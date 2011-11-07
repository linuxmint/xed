/*
 * gedit-app.h
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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

#ifndef __GEDIT_APP_H__
#define __GEDIT_APP_H__

#include <gtk/gtk.h>

#include <gedit/gedit-window.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GEDIT_TYPE_APP              (gedit_app_get_type())
#define GEDIT_APP(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GEDIT_TYPE_APP, GeditApp))
#define GEDIT_APP_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GEDIT_TYPE_APP, GeditAppClass))
#define GEDIT_IS_APP(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GEDIT_TYPE_APP))
#define GEDIT_IS_APP_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_APP))
#define GEDIT_APP_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GEDIT_TYPE_APP, GeditAppClass))

/* Private structure type */
typedef struct _GeditAppPrivate GeditAppPrivate;

/*
 * Main object structure
 */
typedef struct _GeditApp GeditApp;

struct _GeditApp 
{
	GObject object;

	/*< private > */
	GeditAppPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GeditAppClass GeditAppClass;

struct _GeditAppClass 
{
	GObjectClass parent_class;
};

/*
 * Lockdown mask definition
 */
typedef enum
{
	GEDIT_LOCKDOWN_COMMAND_LINE	= 1 << 0,
	GEDIT_LOCKDOWN_PRINTING		= 1 << 1,
	GEDIT_LOCKDOWN_PRINT_SETUP	= 1 << 2,
	GEDIT_LOCKDOWN_SAVE_TO_DISK	= 1 << 3,
	GEDIT_LOCKDOWN_ALL		= 0xF
} GeditLockdownMask;

/*
 * Public methods
 */
GType 		 gedit_app_get_type 			(void) G_GNUC_CONST;

GeditApp 	*gedit_app_get_default			(void);

GeditWindow	*gedit_app_create_window		(GeditApp  *app,
							 GdkScreen *screen);

const GList	*gedit_app_get_windows			(GeditApp *app);
GeditWindow	*gedit_app_get_active_window		(GeditApp *app);

/* Returns a newly allocated list with all the documents */
GList		*gedit_app_get_documents		(GeditApp *app);

/* Returns a newly allocated list with all the views */
GList		*gedit_app_get_views			(GeditApp *app);

/* Lockdown state */
GeditLockdownMask gedit_app_get_lockdown		(GeditApp *app);

/*
 * Non exported functions
 */
GeditWindow	*_gedit_app_restore_window		(GeditApp    *app,
							 const gchar *role);
GeditWindow	*_gedit_app_get_window_in_viewport	(GeditApp     *app,
							 GdkScreen    *screen,
							 gint          workspace,
							 gint          viewport_x,
							 gint          viewport_y);
void		 _gedit_app_set_lockdown		(GeditApp          *app,
							 GeditLockdownMask  lockdown);
void		 _gedit_app_set_lockdown_bit		(GeditApp          *app,
							 GeditLockdownMask  bit,
							 gboolean           value);
/*
 * This one is a gedit-window function, but we declare it here to avoid
 * #include headaches since it needs the GeditLockdownMask declaration.
 */
void		 _gedit_window_set_lockdown		(GeditWindow         *window,
							 GeditLockdownMask    lockdown);

/* global print config */
GtkPageSetup		*_gedit_app_get_default_page_setup	(GeditApp         *app);
void			 _gedit_app_set_default_page_setup	(GeditApp         *app,
								 GtkPageSetup     *page_setup);
GtkPrintSettings	*_gedit_app_get_default_print_settings	(GeditApp         *app);
void			 _gedit_app_set_default_print_settings	(GeditApp         *app,
								 GtkPrintSettings *settings);

G_END_DECLS

#endif  /* __GEDIT_APP_H__  */
