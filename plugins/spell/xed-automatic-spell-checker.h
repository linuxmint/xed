/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xed-automatic-spell-checker.h
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

/* This is a modified version of gtkspell 2.0.2  (gtkspell.sf.net) */
/* gtkspell - a spell-checking addon for GTK's TextView widget
 * Copyright (c) 2002 Evan Martin.
 */

#ifndef __XED_AUTOMATIC_SPELL_CHECKER_H__
#define __XED_AUTOMATIC_SPELL_CHECKER_H__

#include <xed/xed-document.h>
#include <xed/xed-view.h>

#include "xed-spell-checker.h"

typedef struct _XedAutomaticSpellChecker XedAutomaticSpellChecker;

XedAutomaticSpellChecker	*xed_automatic_spell_checker_new (
							XedDocument 			*doc,
							XedSpellChecker		*checker);

XedAutomaticSpellChecker	*xed_automatic_spell_checker_get_from_document (
							const XedDocument 		*doc);
		
void				 xed_automatic_spell_checker_free (
							XedAutomaticSpellChecker 	*spell);

void 				 xed_automatic_spell_checker_attach_view (
							XedAutomaticSpellChecker 	*spell,
							XedView 			*view);

void 				 xed_automatic_spell_checker_detach_view (
							XedAutomaticSpellChecker 	*spell,
							XedView 			*view);

void				 xed_automatic_spell_checker_recheck_all (
							XedAutomaticSpellChecker 	*spell);

#endif  /* __XED_AUTOMATIC_SPELL_CHECKER_H__ */

