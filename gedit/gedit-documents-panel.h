/*
 * gedit-documents-panel.h
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

#ifndef __GEDIT_DOCUMENTS_PANEL_H__
#define __GEDIT_DOCUMENTS_PANEL_H__

#include <gtk/gtk.h>

#include <gedit/gedit-window.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GEDIT_TYPE_DOCUMENTS_PANEL              (gedit_documents_panel_get_type())
#define GEDIT_DOCUMENTS_PANEL(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GEDIT_TYPE_DOCUMENTS_PANEL, GeditDocumentsPanel))
#define GEDIT_DOCUMENTS_PANEL_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GEDIT_TYPE_DOCUMENTS_PANEL, GeditDocumentsPanelClass))
#define GEDIT_IS_DOCUMENTS_PANEL(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GEDIT_TYPE_DOCUMENTS_PANEL))
#define GEDIT_IS_DOCUMENTS_PANEL_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_DOCUMENTS_PANEL))
#define GEDIT_DOCUMENTS_PANEL_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GEDIT_TYPE_DOCUMENTS_PANEL, GeditDocumentsPanelClass))

/* Private structure type */
typedef struct _GeditDocumentsPanelPrivate GeditDocumentsPanelPrivate;

/*
 * Main object structure
 */
typedef struct _GeditDocumentsPanel GeditDocumentsPanel;

struct _GeditDocumentsPanel 
{
	GtkVBox vbox;

	/*< private > */
	GeditDocumentsPanelPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GeditDocumentsPanelClass GeditDocumentsPanelClass;

struct _GeditDocumentsPanelClass 
{
	GtkVBoxClass parent_class;
};

/*
 * Public methods
 */
GType 		 gedit_documents_panel_get_type	(void) G_GNUC_CONST;

GtkWidget	*gedit_documents_panel_new 	(GeditWindow *window);

G_END_DECLS

#endif  /* __GEDIT_DOCUMENTS_PANEL_H__  */
