/*
 * pluma-tab.h
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

#ifndef __PLUMA_TAB_H__
#define __PLUMA_TAB_H__

#include <gtk/gtk.h>

#include <pluma/pluma-view.h>
#include <pluma/pluma-document.h>

G_BEGIN_DECLS

typedef enum
{
	PLUMA_TAB_STATE_NORMAL = 0,
	PLUMA_TAB_STATE_LOADING,
	PLUMA_TAB_STATE_REVERTING,
	PLUMA_TAB_STATE_SAVING,	
	PLUMA_TAB_STATE_PRINTING,
	PLUMA_TAB_STATE_PRINT_PREVIEWING,
	PLUMA_TAB_STATE_SHOWING_PRINT_PREVIEW,
	PLUMA_TAB_STATE_GENERIC_NOT_EDITABLE,
	PLUMA_TAB_STATE_LOADING_ERROR,
	PLUMA_TAB_STATE_REVERTING_ERROR,	
	PLUMA_TAB_STATE_SAVING_ERROR,
	PLUMA_TAB_STATE_GENERIC_ERROR,
	PLUMA_TAB_STATE_CLOSING,
	PLUMA_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION,
	PLUMA_TAB_NUM_OF_STATES /* This is not a valid state */
} PlumaTabState;

/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_TAB              (pluma_tab_get_type())
#define PLUMA_TAB(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_TAB, PlumaTab))
#define PLUMA_TAB_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), PLUMA_TYPE_TAB, PlumaTabClass))
#define PLUMA_IS_TAB(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), PLUMA_TYPE_TAB))
#define PLUMA_IS_TAB_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_TAB))
#define PLUMA_TAB_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), PLUMA_TYPE_TAB, PlumaTabClass))

/* Private structure type */
typedef struct _PlumaTabPrivate PlumaTabPrivate;

/*
 * Main object structure
 */
typedef struct _PlumaTab PlumaTab;

struct _PlumaTab 
{
	GtkVBox vbox;

	/*< private > */
	PlumaTabPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _PlumaTabClass PlumaTabClass;

struct _PlumaTabClass 
{
	GtkVBoxClass parent_class;
};

/*
 * Public methods
 */
GType 		 pluma_tab_get_type 		(void) G_GNUC_CONST;

PlumaView	*pluma_tab_get_view		(PlumaTab            *tab);

/* This is only an helper function */
PlumaDocument	*pluma_tab_get_document		(PlumaTab            *tab);

PlumaTab	*pluma_tab_get_from_document	(PlumaDocument       *doc);

PlumaTabState	 pluma_tab_get_state		(PlumaTab	     *tab);

gboolean	 pluma_tab_get_auto_save_enabled	
						(PlumaTab            *tab); 

void		 pluma_tab_set_auto_save_enabled	
						(PlumaTab            *tab, 
						 gboolean            enable);

gint		 pluma_tab_get_auto_save_interval 
						(PlumaTab            *tab);

void		 pluma_tab_set_auto_save_interval 
						(PlumaTab            *tab, 
						 gint                interval);

void		 pluma_tab_set_info_bar		(PlumaTab            *tab,
						 GtkWidget           *info_bar);
/*
 * Non exported methods
 */
GtkWidget 	*_pluma_tab_new 		(void);

/* Whether create is TRUE, creates a new empty document if location does 
   not refer to an existing file */
GtkWidget	*_pluma_tab_new_from_uri	(const gchar         *uri,
						 const PlumaEncoding *encoding,
						 gint                 line_pos,
						 gboolean             create);
gchar 		*_pluma_tab_get_name		(PlumaTab            *tab);
gchar 		*_pluma_tab_get_tooltips	(PlumaTab            *tab);
GdkPixbuf 	*_pluma_tab_get_icon		(PlumaTab            *tab);
void		 _pluma_tab_load		(PlumaTab            *tab,
						 const gchar         *uri,
						 const PlumaEncoding *encoding,
						 gint                 line_pos,
						 gboolean             create);
void		 _pluma_tab_revert		(PlumaTab            *tab);
void		 _pluma_tab_save		(PlumaTab            *tab);
void		 _pluma_tab_save_as		(PlumaTab            *tab,
						 const gchar         *uri,
						 const PlumaEncoding *encoding,
						 PlumaDocumentNewlineType newline_type);

void		 _pluma_tab_print		(PlumaTab            *tab);
void		 _pluma_tab_print_preview	(PlumaTab            *tab);

void		 _pluma_tab_mark_for_closing	(PlumaTab	     *tab);

gboolean	 _pluma_tab_can_close		(PlumaTab	     *tab);

#if !GTK_CHECK_VERSION (2, 17, 4)
void		 _pluma_tab_page_setup		(PlumaTab            *tab);
#endif

G_END_DECLS

#endif  /* __PLUMA_TAB_H__  */
