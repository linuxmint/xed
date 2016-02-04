/*
 * xed-search-dialog.h
 * This file is part of xed
 *
 * Copyright (C) 2005 Paolo Maggi
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
 * Modified by the xed Team, 2005. See the AUTHORS file for a 
 * list of people on the xed Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __XED_SEARCH_DIALOG_H__
#define __XED_SEARCH_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XED_TYPE_SEARCH_DIALOG              (xed_search_dialog_get_type())
#define XED_SEARCH_DIALOG(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_SEARCH_DIALOG, XedSearchDialog))
#define XED_SEARCH_DIALOG_CONST(obj)        (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_SEARCH_DIALOG, XedSearchDialog const))
#define XED_SEARCH_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_SEARCH_DIALOG, XedSearchDialogClass))
#define XED_IS_SEARCH_DIALOG(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_SEARCH_DIALOG))
#define XED_IS_SEARCH_DIALOG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_SEARCH_DIALOG))
#define XED_SEARCH_DIALOG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_SEARCH_DIALOG, XedSearchDialogClass))

/* Private structure type */
typedef struct _XedSearchDialogPrivate XedSearchDialogPrivate;

/*
 * Main object structure
 */
typedef struct _XedSearchDialog XedSearchDialog;

struct _XedSearchDialog 
{
	GtkDialog dialog;

	/*< private > */
	XedSearchDialogPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XedSearchDialogClass XedSearchDialogClass;

struct _XedSearchDialogClass 
{
	GtkDialogClass parent_class;
	
	/* Key bindings */
	gboolean (* show_replace) (XedSearchDialog *dlg);
};

enum
{
	XED_SEARCH_DIALOG_FIND_RESPONSE = 100,
	XED_SEARCH_DIALOG_REPLACE_RESPONSE,
	XED_SEARCH_DIALOG_REPLACE_ALL_RESPONSE
};

/*
 * Public methods
 */
GType 		 xed_search_dialog_get_type 		(void) G_GNUC_CONST;

GtkWidget	*xed_search_dialog_new		(GtkWindow         *parent,
							 gboolean           show_replace);

void		 xed_search_dialog_present_with_time	(XedSearchDialog *dialog,
							 guint32 timestamp);

gboolean	 xed_search_dialog_get_show_replace	(XedSearchDialog *dialog);

void		 xed_search_dialog_set_show_replace	(XedSearchDialog *dialog,
							 gboolean           show_replace);


void		 xed_search_dialog_set_search_text	(XedSearchDialog *dialog,
							 const gchar       *text);
const gchar	*xed_search_dialog_get_search_text	(XedSearchDialog *dialog);

void		 xed_search_dialog_set_replace_text	(XedSearchDialog *dialog,
							 const gchar       *text);
const gchar	*xed_search_dialog_get_replace_text	(XedSearchDialog *dialog);

void		 xed_search_dialog_set_match_case	(XedSearchDialog *dialog,
							 gboolean           match_case);
gboolean	 xed_search_dialog_get_match_case	(XedSearchDialog *dialog);

void		 xed_search_dialog_set_entire_word	(XedSearchDialog *dialog,
							 gboolean           entire_word);
gboolean	 xed_search_dialog_get_entire_word	(XedSearchDialog *dialog);

void		 xed_search_dialog_set_backwards	(XedSearchDialog *dialog,
							 gboolean           backwards);
gboolean	 xed_search_dialog_get_backwards	(XedSearchDialog *dialog);

void		 xed_search_dialog_set_wrap_around	(XedSearchDialog *dialog,
							 gboolean           wrap_around);
gboolean	 xed_search_dialog_get_wrap_around	(XedSearchDialog *dialog);
   

void		xed_search_dialog_set_parse_escapes (XedSearchDialog *dialog,
                                    		       gboolean           parse_escapes);
gboolean	xed_search_dialog_get_parse_escapes (XedSearchDialog *dialog);

G_END_DECLS

#endif  /* __XED_SEARCH_DIALOG_H__  */
