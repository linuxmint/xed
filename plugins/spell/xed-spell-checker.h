/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xed-spell-checker.h
 * This file is part of xed
 *
 * Copyright (C) 2002-2006 Paolo Maggi
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

#ifndef __XED_SPELL_CHECKER_H__
#define __XED_SPELL_CHECKER_H__

#include <glib.h>
#include <glib-object.h>

#include "xed-spell-checker-language.h"

G_BEGIN_DECLS

#define XED_TYPE_SPELL_CHECKER            (xed_spell_checker_get_type ())
#define XED_SPELL_CHECKER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XED_TYPE_SPELL_CHECKER, XedSpellChecker))
#define XED_SPELL_CHECKER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), XED_TYPE_SPELL_CHECKER, XedSpellChecker))
#define XED_IS_SPELL_CHECKER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XED_TYPE_SPELL_CHECKER))
#define XED_IS_SPELL_CHECKER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_SPELL_CHECKER))
#define XED_SPELL_CHECKER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XED_TYPE_SPELL_CHECKER, XedSpellChecker))

typedef struct _XedSpellChecker XedSpellChecker;

typedef struct _XedSpellCheckerClass XedSpellCheckerClass;

struct _XedSpellCheckerClass
{
	GObjectClass parent_class;

	/* Signals */
	void (*add_word_to_personal) (XedSpellChecker               *spell,
				      const gchar                     *word,
				      gint                             len);

	void (*add_word_to_session)  (XedSpellChecker               *spell,
				      const gchar                     *word,
				      gint                             len);

	void (*set_language)         (XedSpellChecker               *spell,
				      const XedSpellCheckerLanguage *lang);

	void (*clear_session)	     (XedSpellChecker               *spell);
};


GType        		 xed_spell_checker_get_type		(void) G_GNUC_CONST;

/* Constructors */
XedSpellChecker	*xed_spell_checker_new		(void);

gboolean		 xed_spell_checker_set_language 	(XedSpellChecker               *spell,
								 const XedSpellCheckerLanguage *lang);
const XedSpellCheckerLanguage
			*xed_spell_checker_get_language 	(XedSpellChecker               *spell);

gboolean		 xed_spell_checker_check_word 	(XedSpellChecker               *spell,
								 const gchar                     *word,
								 gssize                           len);

GSList 			*xed_spell_checker_get_suggestions 	(XedSpellChecker               *spell,
								 const gchar                     *word,
								 gssize                           len);

gboolean		 xed_spell_checker_add_word_to_personal
								(XedSpellChecker               *spell,
								 const gchar                     *word,
								 gssize                           len);

gboolean		 xed_spell_checker_add_word_to_session
								(XedSpellChecker               *spell,
								 const gchar                     *word,
								 gssize                           len);

gboolean		 xed_spell_checker_clear_session 	(XedSpellChecker               *spell);

gboolean		 xed_spell_checker_set_correction 	(XedSpellChecker               *spell,
								 const gchar                     *word,
								 gssize                           w_len,
								 const gchar                     *replacement,
								 gssize                           r_len);
G_END_DECLS

#endif  /* __XED_SPELL_CHECKER_H__ */

