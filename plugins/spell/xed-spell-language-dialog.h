/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xed-spell-language-dialog.h
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

#ifndef __XED_SPELL_LANGUAGE_DIALOG_H__
#define __XED_SPELL_LANGUAGE_DIALOG_H__

#include <gtk/gtk.h>
#include "xed-spell-checker-language.h"

G_BEGIN_DECLS

#define XED_TYPE_SPELL_LANGUAGE_DIALOG              (xed_spell_language_dialog_get_type())
#define XED_SPELL_LANGUAGE_DIALOG(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_SPELL_LANGUAGE_DIALOG, XedSpellLanguageDialog))
#define XED_SPELL_LANGUAGE_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_SPELL_LANGUAGE_DIALOG, XedSpellLanguageDialogClass))
#define XED_IS_SPELL_LANGUAGE_DIALOG(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_SPELL_LANGUAGE_DIALOG))
#define XED_IS_SPELL_LANGUAGE_DIALOG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_SPELL_LANGUAGE_DIALOG))
#define XED_SPELL_LANGUAGE_DIALOG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_SPELL_LANGUAGE_DIALOG, XedSpellLanguageDialogClass))


typedef struct _XedSpellLanguageDialog XedSpellLanguageDialog;

typedef struct _XedSpellLanguageDialogClass XedSpellLanguageDialogClass;

struct _XedSpellLanguageDialogClass 
{
	GtkDialogClass parent_class;
};

GType		 xed_spell_language_dialog_get_type		(void) G_GNUC_CONST;

GtkWidget	*xed_spell_language_dialog_new		(GtkWindow			 *parent,
								 const XedSpellCheckerLanguage *cur_lang,
								 const gchar *data_dir);

const XedSpellCheckerLanguage *
		 xed_spell_language_get_selected_language	(XedSpellLanguageDialog *dlg);

G_END_DECLS

#endif  /* __XED_SPELL_LANGUAGE_DIALOG_H__ */

