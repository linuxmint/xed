/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xedit-spell-language-dialog.h
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

#ifndef __XEDIT_SPELL_LANGUAGE_DIALOG_H__
#define __XEDIT_SPELL_LANGUAGE_DIALOG_H__

#include <gtk/gtk.h>
#include "xedit-spell-checker-language.h"

G_BEGIN_DECLS

#define XEDIT_TYPE_SPELL_LANGUAGE_DIALOG              (xedit_spell_language_dialog_get_type())
#define XEDIT_SPELL_LANGUAGE_DIALOG(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XEDIT_TYPE_SPELL_LANGUAGE_DIALOG, XeditSpellLanguageDialog))
#define XEDIT_SPELL_LANGUAGE_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XEDIT_TYPE_SPELL_LANGUAGE_DIALOG, XeditSpellLanguageDialogClass))
#define XEDIT_IS_SPELL_LANGUAGE_DIALOG(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XEDIT_TYPE_SPELL_LANGUAGE_DIALOG))
#define XEDIT_IS_SPELL_LANGUAGE_DIALOG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_SPELL_LANGUAGE_DIALOG))
#define XEDIT_SPELL_LANGUAGE_DIALOG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XEDIT_TYPE_SPELL_LANGUAGE_DIALOG, XeditSpellLanguageDialogClass))


typedef struct _XeditSpellLanguageDialog XeditSpellLanguageDialog;

typedef struct _XeditSpellLanguageDialogClass XeditSpellLanguageDialogClass;

struct _XeditSpellLanguageDialogClass 
{
	GtkDialogClass parent_class;
};

GType		 xedit_spell_language_dialog_get_type		(void) G_GNUC_CONST;

GtkWidget	*xedit_spell_language_dialog_new		(GtkWindow			 *parent,
								 const XeditSpellCheckerLanguage *cur_lang,
								 const gchar *data_dir);

const XeditSpellCheckerLanguage *
		 xedit_spell_language_get_selected_language	(XeditSpellLanguageDialog *dlg);

G_END_DECLS

#endif  /* __XEDIT_SPELL_LANGUAGE_DIALOG_H__ */

