/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xedit-spell-checker-dialog.h
 * This file is part of xedit
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
 * Modified by the xedit Team, 2002. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifndef __XEDIT_SPELL_CHECKER_DIALOG_H__
#define __XEDIT_SPELL_CHECKER_DIALOG_H__

#include <gtk/gtk.h>
#include "xedit-spell-checker.h"

G_BEGIN_DECLS

#define XEDIT_TYPE_SPELL_CHECKER_DIALOG            (xedit_spell_checker_dialog_get_type ())
#define XEDIT_SPELL_CHECKER_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_SPELL_CHECKER_DIALOG, XeditSpellCheckerDialog))
#define XEDIT_SPELL_CHECKER_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_SPELL_CHECKER_DIALOG, XeditSpellCheckerDialog))
#define XEDIT_IS_SPELL_CHECKER_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XEDIT_TYPE_SPELL_CHECKER_DIALOG))
#define XEDIT_IS_SPELL_CHECKER_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_SPELL_CHECKER_DIALOG))
#define XEDIT_SPELL_CHECKER_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XEDIT_TYPE_SPELL_CHECKER_DIALOG, XeditSpellCheckerDialog))


typedef struct _XeditSpellCheckerDialog XeditSpellCheckerDialog;

typedef struct _XeditSpellCheckerDialogClass XeditSpellCheckerDialogClass;

struct _XeditSpellCheckerDialogClass 
{
	GtkWindowClass parent_class;

	/* Signals */
	void		(*ignore)		(XeditSpellCheckerDialog *dlg,
						 const gchar *word);
	void		(*ignore_all)		(XeditSpellCheckerDialog *dlg,
						 const gchar *word);
	void		(*change)		(XeditSpellCheckerDialog *dlg,
						 const gchar *word, 
						 const gchar *change_to);
	void		(*change_all)		(XeditSpellCheckerDialog *dlg,
						 const gchar *word, 
						 const gchar *change_to);
	void		(*add_word_to_personal)	(XeditSpellCheckerDialog *dlg,
						 const gchar *word);

};

GType        		 xedit_spell_checker_dialog_get_type	(void) G_GNUC_CONST;

/* Constructors */
GtkWidget		*xedit_spell_checker_dialog_new		(const gchar *data_dir);
GtkWidget		*xedit_spell_checker_dialog_new_from_spell_checker 
								(XeditSpellChecker *spell,
								 const gchar *data_dir);

void 			 xedit_spell_checker_dialog_set_spell_checker
								(XeditSpellCheckerDialog *dlg,
								 XeditSpellChecker *spell);
void			 xedit_spell_checker_dialog_set_misspelled_word 
								(XeditSpellCheckerDialog *dlg, 
								 const gchar* word, 
								 gint len);

void 			 xedit_spell_checker_dialog_set_completed 
								(XeditSpellCheckerDialog *dlg);
								
G_END_DECLS

#endif  /* __XEDIT_SPELL_CHECKER_DIALOG_H__ */

