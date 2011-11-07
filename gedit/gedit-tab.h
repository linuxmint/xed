/*
 * gedit-tab.h
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

#ifndef __GEDIT_TAB_H__
#define __GEDIT_TAB_H__

#include <gtk/gtk.h>

#include <gedit/gedit-view.h>
#include <gedit/gedit-document.h>

G_BEGIN_DECLS

typedef enum
{
	GEDIT_TAB_STATE_NORMAL = 0,
	GEDIT_TAB_STATE_LOADING,
	GEDIT_TAB_STATE_REVERTING,
	GEDIT_TAB_STATE_SAVING,	
	GEDIT_TAB_STATE_PRINTING,
	GEDIT_TAB_STATE_PRINT_PREVIEWING,
	GEDIT_TAB_STATE_SHOWING_PRINT_PREVIEW,
	GEDIT_TAB_STATE_GENERIC_NOT_EDITABLE,
	GEDIT_TAB_STATE_LOADING_ERROR,
	GEDIT_TAB_STATE_REVERTING_ERROR,	
	GEDIT_TAB_STATE_SAVING_ERROR,
	GEDIT_TAB_STATE_GENERIC_ERROR,
	GEDIT_TAB_STATE_CLOSING,
	GEDIT_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION,
	GEDIT_TAB_NUM_OF_STATES /* This is not a valid state */
} GeditTabState;

/*
 * Type checking and casting macros
 */
#define GEDIT_TYPE_TAB              (gedit_tab_get_type())
#define GEDIT_TAB(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GEDIT_TYPE_TAB, GeditTab))
#define GEDIT_TAB_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GEDIT_TYPE_TAB, GeditTabClass))
#define GEDIT_IS_TAB(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GEDIT_TYPE_TAB))
#define GEDIT_IS_TAB_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_TAB))
#define GEDIT_TAB_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GEDIT_TYPE_TAB, GeditTabClass))

/* Private structure type */
typedef struct _GeditTabPrivate GeditTabPrivate;

/*
 * Main object structure
 */
typedef struct _GeditTab GeditTab;

struct _GeditTab 
{
	GtkVBox vbox;

	/*< private > */
	GeditTabPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GeditTabClass GeditTabClass;

struct _GeditTabClass 
{
	GtkVBoxClass parent_class;
};

/*
 * Public methods
 */
GType 		 gedit_tab_get_type 		(void) G_GNUC_CONST;

GeditView	*gedit_tab_get_view		(GeditTab            *tab);

/* This is only an helper function */
GeditDocument	*gedit_tab_get_document		(GeditTab            *tab);

GeditTab	*gedit_tab_get_from_document	(GeditDocument       *doc);

GeditTabState	 gedit_tab_get_state		(GeditTab	     *tab);

gboolean	 gedit_tab_get_auto_save_enabled	
						(GeditTab            *tab); 

void		 gedit_tab_set_auto_save_enabled	
						(GeditTab            *tab, 
						 gboolean            enable);

gint		 gedit_tab_get_auto_save_interval 
						(GeditTab            *tab);

void		 gedit_tab_set_auto_save_interval 
						(GeditTab            *tab, 
						 gint                interval);

void		 gedit_tab_set_info_bar		(GeditTab            *tab,
						 GtkWidget           *info_bar);
/*
 * Non exported methods
 */
GtkWidget 	*_gedit_tab_new 		(void);

/* Whether create is TRUE, creates a new empty document if location does 
   not refer to an existing file */
GtkWidget	*_gedit_tab_new_from_uri	(const gchar         *uri,
						 const GeditEncoding *encoding,
						 gint                 line_pos,
						 gboolean             create);
gchar 		*_gedit_tab_get_name		(GeditTab            *tab);
gchar 		*_gedit_tab_get_tooltips	(GeditTab            *tab);
GdkPixbuf 	*_gedit_tab_get_icon		(GeditTab            *tab);
void		 _gedit_tab_load		(GeditTab            *tab,
						 const gchar         *uri,
						 const GeditEncoding *encoding,
						 gint                 line_pos,
						 gboolean             create);
void		 _gedit_tab_revert		(GeditTab            *tab);
void		 _gedit_tab_save		(GeditTab            *tab);
void		 _gedit_tab_save_as		(GeditTab            *tab,
						 const gchar         *uri,
						 const GeditEncoding *encoding,
						 GeditDocumentNewlineType newline_type);

void		 _gedit_tab_print		(GeditTab            *tab);
void		 _gedit_tab_print_preview	(GeditTab            *tab);

void		 _gedit_tab_mark_for_closing	(GeditTab	     *tab);

gboolean	 _gedit_tab_can_close		(GeditTab	     *tab);

#if !GTK_CHECK_VERSION (2, 17, 4)
void		 _gedit_tab_page_setup		(GeditTab            *tab);
#endif

G_END_DECLS

#endif  /* __GEDIT_TAB_H__  */
