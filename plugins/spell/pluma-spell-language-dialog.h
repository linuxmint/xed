/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pluma-spell-language-dialog.h
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

#ifndef __PLUMA_SPELL_LANGUAGE_DIALOG_H__
#define __PLUMA_SPELL_LANGUAGE_DIALOG_H__

#include <gtk/gtk.h>
#include "pluma-spell-checker-language.h"

G_BEGIN_DECLS

#define PLUMA_TYPE_SPELL_LANGUAGE_DIALOG              (pluma_spell_language_dialog_get_type())
#define PLUMA_SPELL_LANGUAGE_DIALOG(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_SPELL_LANGUAGE_DIALOG, PlumaSpellLanguageDialog))
#define PLUMA_SPELL_LANGUAGE_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), PLUMA_TYPE_SPELL_LANGUAGE_DIALOG, PlumaSpellLanguageDialogClass))
#define PLUMA_IS_SPELL_LANGUAGE_DIALOG(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), PLUMA_TYPE_SPELL_LANGUAGE_DIALOG))
#define PLUMA_IS_SPELL_LANGUAGE_DIALOG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_SPELL_LANGUAGE_DIALOG))
#define PLUMA_SPELL_LANGUAGE_DIALOG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), PLUMA_TYPE_SPELL_LANGUAGE_DIALOG, PlumaSpellLanguageDialogClass))


typedef struct _PlumaSpellLanguageDialog PlumaSpellLanguageDialog;

typedef struct _PlumaSpellLanguageDialogClass PlumaSpellLanguageDialogClass;

struct _PlumaSpellLanguageDialogClass 
{
	GtkDialogClass parent_class;
};

GType		 pluma_spell_language_dialog_get_type		(void) G_GNUC_CONST;

GtkWidget	*pluma_spell_language_dialog_new		(GtkWindow			 *parent,
								 const PlumaSpellCheckerLanguage *cur_lang,
								 const gchar *data_dir);

const PlumaSpellCheckerLanguage *
		 pluma_spell_language_get_selected_language	(PlumaSpellLanguageDialog *dlg);

G_END_DECLS

#endif  /* __PLUMA_SPELL_LANGUAGE_DIALOG_H__ */

