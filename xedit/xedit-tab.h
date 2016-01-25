/*
 * xedit-tab.h
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

#ifndef __XEDIT_TAB_H__
#define __XEDIT_TAB_H__

#include <gtk/gtk.h>

#include <xedit/xedit-view.h>
#include <xedit/xedit-document.h>

G_BEGIN_DECLS

typedef enum
{
	XEDIT_TAB_STATE_NORMAL = 0,
	XEDIT_TAB_STATE_LOADING,
	XEDIT_TAB_STATE_REVERTING,
	XEDIT_TAB_STATE_SAVING,	
	XEDIT_TAB_STATE_PRINTING,
	XEDIT_TAB_STATE_PRINT_PREVIEWING,
	XEDIT_TAB_STATE_SHOWING_PRINT_PREVIEW,
	XEDIT_TAB_STATE_GENERIC_NOT_EDITABLE,
	XEDIT_TAB_STATE_LOADING_ERROR,
	XEDIT_TAB_STATE_REVERTING_ERROR,	
	XEDIT_TAB_STATE_SAVING_ERROR,
	XEDIT_TAB_STATE_GENERIC_ERROR,
	XEDIT_TAB_STATE_CLOSING,
	XEDIT_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION,
	XEDIT_TAB_NUM_OF_STATES /* This is not a valid state */
} XeditTabState;

/*
 * Type checking and casting macros
 */
#define XEDIT_TYPE_TAB              (xedit_tab_get_type())
#define XEDIT_TAB(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XEDIT_TYPE_TAB, XeditTab))
#define XEDIT_TAB_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XEDIT_TYPE_TAB, XeditTabClass))
#define XEDIT_IS_TAB(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XEDIT_TYPE_TAB))
#define XEDIT_IS_TAB_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_TAB))
#define XEDIT_TAB_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XEDIT_TYPE_TAB, XeditTabClass))

/* Private structure type */
typedef struct _XeditTabPrivate XeditTabPrivate;

/*
 * Main object structure
 */
typedef struct _XeditTab XeditTab;

struct _XeditTab 
{
#if GTK_CHECK_VERSION (3, 0, 0)
	GtkBox vbox;
#else
	GtkVBox vbox;
#endif

	/*< private > */
	XeditTabPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XeditTabClass XeditTabClass;

struct _XeditTabClass 
{
#if GTK_CHECK_VERSION (3, 0, 0)
	GtkBoxClass parent_class;
#else
	GtkVBoxClass parent_class;
#endif
};

/*
 * Public methods
 */
GType 		 xedit_tab_get_type 		(void) G_GNUC_CONST;

XeditView	*xedit_tab_get_view		(XeditTab            *tab);

/* This is only an helper function */
XeditDocument	*xedit_tab_get_document		(XeditTab            *tab);

XeditTab	*xedit_tab_get_from_document	(XeditDocument       *doc);

XeditTabState	 xedit_tab_get_state		(XeditTab	     *tab);

gboolean	 xedit_tab_get_auto_save_enabled	
						(XeditTab            *tab); 

void		 xedit_tab_set_auto_save_enabled	
						(XeditTab            *tab, 
						 gboolean            enable);

gint		 xedit_tab_get_auto_save_interval 
						(XeditTab            *tab);

void		 xedit_tab_set_auto_save_interval 
						(XeditTab            *tab, 
						 gint                interval);

void		 xedit_tab_set_info_bar		(XeditTab            *tab,
						 GtkWidget           *info_bar);
/*
 * Non exported methods
 */
GtkWidget 	*_xedit_tab_new 		(void);

/* Whether create is TRUE, creates a new empty document if location does 
   not refer to an existing file */
GtkWidget	*_xedit_tab_new_from_uri	(const gchar         *uri,
						 const XeditEncoding *encoding,
						 gint                 line_pos,
						 gboolean             create);
gchar 		*_xedit_tab_get_name		(XeditTab            *tab);
gchar 		*_xedit_tab_get_tooltips	(XeditTab            *tab);
GdkPixbuf 	*_xedit_tab_get_icon		(XeditTab            *tab);
void		 _xedit_tab_load		(XeditTab            *tab,
						 const gchar         *uri,
						 const XeditEncoding *encoding,
						 gint                 line_pos,
						 gboolean             create);
void		 _xedit_tab_revert		(XeditTab            *tab);
void		 _xedit_tab_save		(XeditTab            *tab);
void		 _xedit_tab_save_as		(XeditTab            *tab,
						 const gchar         *uri,
						 const XeditEncoding *encoding,
						 XeditDocumentNewlineType newline_type);

void		 _xedit_tab_print		(XeditTab            *tab);
void		 _xedit_tab_print_preview	(XeditTab            *tab);

void		 _xedit_tab_mark_for_closing	(XeditTab	     *tab);

gboolean	 _xedit_tab_can_close		(XeditTab	     *tab);

G_END_DECLS

#endif  /* __XEDIT_TAB_H__  */
