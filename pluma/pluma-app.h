/*
 * pluma-app.h
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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

#ifndef __PLUMA_APP_H__
#define __PLUMA_APP_H__

#include <gtk/gtk.h>

#include <pluma/pluma-window.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_APP              (pluma_app_get_type())
#define PLUMA_APP(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_APP, PlumaApp))
#define PLUMA_APP_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), PLUMA_TYPE_APP, PlumaAppClass))
#define PLUMA_IS_APP(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), PLUMA_TYPE_APP))
#define PLUMA_IS_APP_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_APP))
#define PLUMA_APP_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), PLUMA_TYPE_APP, PlumaAppClass))

/* Private structure type */
typedef struct _PlumaAppPrivate PlumaAppPrivate;

/*
 * Main object structure
 */
typedef struct _PlumaApp PlumaApp;

struct _PlumaApp 
{
	GObject object;

	/*< private > */
	PlumaAppPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _PlumaAppClass PlumaAppClass;

struct _PlumaAppClass 
{
	GObjectClass parent_class;
};

/*
 * Lockdown mask definition
 */
typedef enum
{
	PLUMA_LOCKDOWN_COMMAND_LINE	= 1 << 0,
	PLUMA_LOCKDOWN_PRINTING		= 1 << 1,
	PLUMA_LOCKDOWN_PRINT_SETUP	= 1 << 2,
	PLUMA_LOCKDOWN_SAVE_TO_DISK	= 1 << 3,
	PLUMA_LOCKDOWN_ALL		= 0xF
} PlumaLockdownMask;

/*
 * Public methods
 */
GType 		 pluma_app_get_type 			(void) G_GNUC_CONST;

PlumaApp 	*pluma_app_get_default			(void);

PlumaWindow	*pluma_app_create_window		(PlumaApp  *app,
							 GdkScreen *screen);

const GList	*pluma_app_get_windows			(PlumaApp *app);
PlumaWindow	*pluma_app_get_active_window		(PlumaApp *app);

/* Returns a newly allocated list with all the documents */
GList		*pluma_app_get_documents		(PlumaApp *app);

/* Returns a newly allocated list with all the views */
GList		*pluma_app_get_views			(PlumaApp *app);

/* Lockdown state */
PlumaLockdownMask pluma_app_get_lockdown		(PlumaApp *app);

/*
 * Non exported functions
 */
PlumaWindow	*_pluma_app_restore_window		(PlumaApp    *app,
							 const gchar *role);
PlumaWindow	*_pluma_app_get_window_in_viewport	(PlumaApp     *app,
							 GdkScreen    *screen,
							 gint          workspace,
							 gint          viewport_x,
							 gint          viewport_y);
void		 _pluma_app_set_lockdown		(PlumaApp          *app,
							 PlumaLockdownMask  lockdown);
void		 _pluma_app_set_lockdown_bit		(PlumaApp          *app,
							 PlumaLockdownMask  bit,
							 gboolean           value);
/*
 * This one is a pluma-window function, but we declare it here to avoid
 * #include headaches since it needs the PlumaLockdownMask declaration.
 */
void		 _pluma_window_set_lockdown		(PlumaWindow         *window,
							 PlumaLockdownMask    lockdown);

/* global print config */
GtkPageSetup		*_pluma_app_get_default_page_setup	(PlumaApp         *app);
void			 _pluma_app_set_default_page_setup	(PlumaApp         *app,
								 GtkPageSetup     *page_setup);
GtkPrintSettings	*_pluma_app_get_default_print_settings	(PlumaApp         *app);
void			 _pluma_app_set_default_print_settings	(PlumaApp         *app,
								 GtkPrintSettings *settings);

G_END_DECLS

#endif  /* __PLUMA_APP_H__  */
