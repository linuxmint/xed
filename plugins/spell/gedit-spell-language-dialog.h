/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gedit-spell-language-dialog.h
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */
 
/*
 * Modified by the gedit Team, 2002. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifndef __GEDIT_SPELL_LANGUAGE_DIALOG_H__
#define __GEDIT_SPELL_LANGUAGE_DIALOG_H__

#include <gtk/gtk.h>
#include "gedit-spell-checker-language.h"

G_BEGIN_DECLS

#define GEDIT_TYPE_SPELL_LANGUAGE_DIALOG              (gedit_spell_language_dialog_get_type())
#define GEDIT_SPELL_LANGUAGE_DIALOG(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GEDIT_TYPE_SPELL_LANGUAGE_DIALOG, GeditSpellLanguageDialog))
#define GEDIT_SPELL_LANGUAGE_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GEDIT_TYPE_SPELL_LANGUAGE_DIALOG, GeditSpellLanguageDialogClass))
#define GEDIT_IS_SPELL_LANGUAGE_DIALOG(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GEDIT_TYPE_SPELL_LANGUAGE_DIALOG))
#define GEDIT_IS_SPELL_LANGUAGE_DIALOG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_SPELL_LANGUAGE_DIALOG))
#define GEDIT_SPELL_LANGUAGE_DIALOG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GEDIT_TYPE_SPELL_LANGUAGE_DIALOG, GeditSpellLanguageDialogClass))


typedef struct _GeditSpellLanguageDialog GeditSpellLanguageDialog;

typedef struct _GeditSpellLanguageDialogClass GeditSpellLanguageDialogClass;

struct _GeditSpellLanguageDialogClass 
{
	GtkDialogClass parent_class;
};

GType		 gedit_spell_language_dialog_get_type		(void) G_GNUC_CONST;

GtkWidget	*gedit_spell_language_dialog_new		(GtkWindow			 *parent,
								 const GeditSpellCheckerLanguage *cur_lang,
								 const gchar *data_dir);

const GeditSpellCheckerLanguage *
		 gedit_spell_language_get_selected_language	(GeditSpellLanguageDialog *dlg);

G_END_DECLS

#endif  /* __GEDIT_SPELL_LANGUAGE_DIALOG_H__ */

