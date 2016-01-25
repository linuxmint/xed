/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xedit-automatic-spell-checker.h
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

/* This is a modified version of gtkspell 2.0.2  (gtkspell.sf.net) */
/* gtkspell - a spell-checking addon for GTK's TextView widget
 * Copyright (c) 2002 Evan Martin.
 */

#ifndef __XEDIT_AUTOMATIC_SPELL_CHECKER_H__
#define __XEDIT_AUTOMATIC_SPELL_CHECKER_H__

#include <xedit/xedit-document.h>
#include <xedit/xedit-view.h>

#include "xedit-spell-checker.h"

typedef struct _XeditAutomaticSpellChecker XeditAutomaticSpellChecker;

XeditAutomaticSpellChecker	*xedit_automatic_spell_checker_new (
							XeditDocument 			*doc,
							XeditSpellChecker		*checker);

XeditAutomaticSpellChecker	*xedit_automatic_spell_checker_get_from_document (
							const XeditDocument 		*doc);
		
void				 xedit_automatic_spell_checker_free (
							XeditAutomaticSpellChecker 	*spell);

void 				 xedit_automatic_spell_checker_attach_view (
							XeditAutomaticSpellChecker 	*spell,
							XeditView 			*view);

void 				 xedit_automatic_spell_checker_detach_view (
							XeditAutomaticSpellChecker 	*spell,
							XeditView 			*view);

void				 xedit_automatic_spell_checker_recheck_all (
							XeditAutomaticSpellChecker 	*spell);

#endif  /* __XEDIT_AUTOMATIC_SPELL_CHECKER_H__ */

