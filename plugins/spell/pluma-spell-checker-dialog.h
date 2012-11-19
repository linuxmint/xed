/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pluma-spell-checker-dialog.h
 * This file is part of pluma
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
 * Modified by the pluma Team, 2002. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifndef __PLUMA_SPELL_CHECKER_DIALOG_H__
#define __PLUMA_SPELL_CHECKER_DIALOG_H__

#include <gtk/gtk.h>
#include "pluma-spell-checker.h"

G_BEGIN_DECLS

#define PLUMA_TYPE_SPELL_CHECKER_DIALOG            (pluma_spell_checker_dialog_get_type ())
#define PLUMA_SPELL_CHECKER_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_SPELL_CHECKER_DIALOG, PlumaSpellCheckerDialog))
#define PLUMA_SPELL_CHECKER_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PLUMA_TYPE_SPELL_CHECKER_DIALOG, PlumaSpellCheckerDialog))
#define PLUMA_IS_SPELL_CHECKER_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUMA_TYPE_SPELL_CHECKER_DIALOG))
#define PLUMA_IS_SPELL_CHECKER_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_SPELL_CHECKER_DIALOG))
#define PLUMA_SPELL_CHECKER_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PLUMA_TYPE_SPELL_CHECKER_DIALOG, PlumaSpellCheckerDialog))


typedef struct _PlumaSpellCheckerDialog PlumaSpellCheckerDialog;

typedef struct _PlumaSpellCheckerDialogClass PlumaSpellCheckerDialogClass;

struct _PlumaSpellCheckerDialogClass 
{
	GtkWindowClass parent_class;

	/* Signals */
	void		(*ignore)		(PlumaSpellCheckerDialog *dlg,
						 const gchar *word);
	void		(*ignore_all)		(PlumaSpellCheckerDialog *dlg,
						 const gchar *word);
	void		(*change)		(PlumaSpellCheckerDialog *dlg,
						 const gchar *word, 
						 const gchar *change_to);
	void		(*change_all)		(PlumaSpellCheckerDialog *dlg,
						 const gchar *word, 
						 const gchar *change_to);
	void		(*add_word_to_personal)	(PlumaSpellCheckerDialog *dlg,
						 const gchar *word);

};

GType        		 pluma_spell_checker_dialog_get_type	(void) G_GNUC_CONST;

/* Constructors */
GtkWidget		*pluma_spell_checker_dialog_new		(const gchar *data_dir);
GtkWidget		*pluma_spell_checker_dialog_new_from_spell_checker 
								(PlumaSpellChecker *spell,
								 const gchar *data_dir);

void 			 pluma_spell_checker_dialog_set_spell_checker
								(PlumaSpellCheckerDialog *dlg,
								 PlumaSpellChecker *spell);
void			 pluma_spell_checker_dialog_set_misspelled_word 
								(PlumaSpellCheckerDialog *dlg, 
								 const gchar* word, 
								 gint len);

void 			 pluma_spell_checker_dialog_set_completed 
								(PlumaSpellCheckerDialog *dlg);
								
G_END_DECLS

#endif  /* __PLUMA_SPELL_CHECKER_DIALOG_H__ */

