/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xedit-spell-checker-language.h
 * This file is part of xedit
 *
 * Copyright (C) 2006 Paolo Maggi 
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
 * Modified by the xedit Team, 2006. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifndef __XEDIT_SPELL_CHECKER_LANGUAGE_H__
#define __XEDIT_SPELL_CHECKER_LANGUAGE_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _XeditSpellCheckerLanguage XeditSpellCheckerLanguage;

const gchar			*xedit_spell_checker_language_to_string	(const XeditSpellCheckerLanguage *lang);

const gchar			*xedit_spell_checker_language_to_key	(const XeditSpellCheckerLanguage *lang);

const XeditSpellCheckerLanguage	*xedit_spell_checker_language_from_key	(const gchar *key);

/* GSList contains "XeditSpellCheckerLanguage*" items */
const GSList 			*xedit_spell_checker_get_available_languages
									(void);

G_END_DECLS

#endif /* __XEDIT_SPELL_CHECKER_LANGUAGE_H__ */
