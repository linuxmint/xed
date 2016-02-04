/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xed-spell-checker-dialog.h
 * This file is part of xed
 *
 * Copyright (C) 2002 Paolo Maggi 
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
 * Modified by the xed Team, 2002. See the AUTHORS file for a 
 * list of people on the xed Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifndef __XED_SPELL_CHECKER_DIALOG_H__
#define __XED_SPELL_CHECKER_DIALOG_H__

#include <gtk/gtk.h>
#include "xed-spell-checker.h"

G_BEGIN_DECLS

#define XED_TYPE_SPELL_CHECKER_DIALOG            (xed_spell_checker_dialog_get_type ())
#define XED_SPELL_CHECKER_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_SPELL_CHECKER_DIALOG, XedSpellCheckerDialog))
#define XED_SPELL_CHECKER_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_SPELL_CHECKER_DIALOG, XedSpellCheckerDialog))
#define XED_IS_SPELL_CHECKER_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_SPELL_CHECKER_DIALOG))
#define XED_IS_SPELL_CHECKER_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_SPELL_CHECKER_DIALOG))
#define XED_SPELL_CHECKER_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XED_TYPE_SPELL_CHECKER_DIALOG, XedSpellCheckerDialog))


typedef struct _XedSpellCheckerDialog XedSpellCheckerDialog;

typedef struct _XedSpellCheckerDialogClass XedSpellCheckerDialogClass;

struct _XedSpellCheckerDialogClass 
{
	GtkWindowClass parent_class;

	/* Signals */
	void		(*ignore)		(XedSpellCheckerDialog *dlg,
						 const gchar *word);
	void		(*ignore_all)		(XedSpellCheckerDialog *dlg,
						 const gchar *word);
	void		(*change)		(XedSpellCheckerDialog *dlg,
						 const gchar *word, 
						 const gchar *change_to);
	void		(*change_all)		(XedSpellCheckerDialog *dlg,
						 const gchar *word, 
						 const gchar *change_to);
	void		(*add_word_to_personal)	(XedSpellCheckerDialog *dlg,
						 const gchar *word);

};

GType        		 xed_spell_checker_dialog_get_type	(void) G_GNUC_CONST;

/* Constructors */
GtkWidget		*xed_spell_checker_dialog_new		(const gchar *data_dir);
GtkWidget		*xed_spell_checker_dialog_new_from_spell_checker 
								(XedSpellChecker *spell,
								 const gchar *data_dir);

void 			 xed_spell_checker_dialog_set_spell_checker
								(XedSpellCheckerDialog *dlg,
								 XedSpellChecker *spell);
void			 xed_spell_checker_dialog_set_misspelled_word 
								(XedSpellCheckerDialog *dlg, 
								 const gchar* word, 
								 gint len);

void 			 xed_spell_checker_dialog_set_completed 
								(XedSpellCheckerDialog *dlg);
								
G_END_DECLS

#endif  /* __XED_SPELL_CHECKER_DIALOG_H__ */

