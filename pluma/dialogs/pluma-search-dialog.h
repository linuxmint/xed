/*
 * pluma-search-dialog.h
 * This file is part of pluma
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
 * Modified by the pluma Team, 2005. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __PLUMA_SEARCH_DIALOG_H__
#define __PLUMA_SEARCH_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_SEARCH_DIALOG              (pluma_search_dialog_get_type())
#define PLUMA_SEARCH_DIALOG(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_SEARCH_DIALOG, PlumaSearchDialog))
#define PLUMA_SEARCH_DIALOG_CONST(obj)        (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_SEARCH_DIALOG, PlumaSearchDialog const))
#define PLUMA_SEARCH_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), PLUMA_TYPE_SEARCH_DIALOG, PlumaSearchDialogClass))
#define PLUMA_IS_SEARCH_DIALOG(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), PLUMA_TYPE_SEARCH_DIALOG))
#define PLUMA_IS_SEARCH_DIALOG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_SEARCH_DIALOG))
#define PLUMA_SEARCH_DIALOG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), PLUMA_TYPE_SEARCH_DIALOG, PlumaSearchDialogClass))

/* Private structure type */
typedef struct _PlumaSearchDialogPrivate PlumaSearchDialogPrivate;

/*
 * Main object structure
 */
typedef struct _PlumaSearchDialog PlumaSearchDialog;

struct _PlumaSearchDialog 
{
	GtkDialog dialog;

	/*< private > */
	PlumaSearchDialogPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _PlumaSearchDialogClass PlumaSearchDialogClass;

struct _PlumaSearchDialogClass 
{
	GtkDialogClass parent_class;
	
	/* Key bindings */
	gboolean (* show_replace) (PlumaSearchDialog *dlg);
};

enum
{
	PLUMA_SEARCH_DIALOG_FIND_RESPONSE = 100,
	PLUMA_SEARCH_DIALOG_REPLACE_RESPONSE,
	PLUMA_SEARCH_DIALOG_REPLACE_ALL_RESPONSE
};

/*
 * Public methods
 */
GType 		 pluma_search_dialog_get_type 		(void) G_GNUC_CONST;

GtkWidget	*pluma_search_dialog_new		(GtkWindow         *parent,
							 gboolean           show_replace);

void		 pluma_search_dialog_present_with_time	(PlumaSearchDialog *dialog,
							 guint32 timestamp);

gboolean	 pluma_search_dialog_get_show_replace	(PlumaSearchDialog *dialog);

void		 pluma_search_dialog_set_show_replace	(PlumaSearchDialog *dialog,
							 gboolean           show_replace);


void		 pluma_search_dialog_set_search_text	(PlumaSearchDialog *dialog,
							 const gchar       *text);
const gchar	*pluma_search_dialog_get_search_text	(PlumaSearchDialog *dialog);

void		 pluma_search_dialog_set_replace_text	(PlumaSearchDialog *dialog,
							 const gchar       *text);
const gchar	*pluma_search_dialog_get_replace_text	(PlumaSearchDialog *dialog);

void		 pluma_search_dialog_set_match_case	(PlumaSearchDialog *dialog,
							 gboolean           match_case);
gboolean	 pluma_search_dialog_get_match_case	(PlumaSearchDialog *dialog);

void		 pluma_search_dialog_set_entire_word	(PlumaSearchDialog *dialog,
							 gboolean           entire_word);
gboolean	 pluma_search_dialog_get_entire_word	(PlumaSearchDialog *dialog);

void		 pluma_search_dialog_set_backwards	(PlumaSearchDialog *dialog,
							 gboolean           backwards);
gboolean	 pluma_search_dialog_get_backwards	(PlumaSearchDialog *dialog);

void		 pluma_search_dialog_set_wrap_around	(PlumaSearchDialog *dialog,
							 gboolean           wrap_around);
gboolean	 pluma_search_dialog_get_wrap_around	(PlumaSearchDialog *dialog);
   

void		pluma_search_dialog_set_parse_escapes (PlumaSearchDialog *dialog,
                                    		       gboolean           parse_escapes);
gboolean	pluma_search_dialog_get_parse_escapes (PlumaSearchDialog *dialog);

G_END_DECLS

#endif  /* __PLUMA_SEARCH_DIALOG_H__  */
