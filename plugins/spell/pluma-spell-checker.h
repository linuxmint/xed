/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pluma-spell-checker.h
 * This file is part of pluma
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
 * Modified by the pluma Team, 2002. See the AUTHORS file for a
 * list of people on the pluma Team.
 * See the ChangeLog files for a list of changes.
 */

#ifndef __PLUMA_SPELL_CHECKER_H__
#define __PLUMA_SPELL_CHECKER_H__

#include <glib.h>
#include <glib-object.h>

#include "pluma-spell-checker-language.h"

G_BEGIN_DECLS

#define PLUMA_TYPE_SPELL_CHECKER            (pluma_spell_checker_get_type ())
#define PLUMA_SPELL_CHECKER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_SPELL_CHECKER, PlumaSpellChecker))
#define PLUMA_SPELL_CHECKER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PLUMA_TYPE_SPELL_CHECKER, PlumaSpellChecker))
#define PLUMA_IS_SPELL_CHECKER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUMA_TYPE_SPELL_CHECKER))
#define PLUMA_IS_SPELL_CHECKER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_SPELL_CHECKER))
#define PLUMA_SPELL_CHECKER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PLUMA_TYPE_SPELL_CHECKER, PlumaSpellChecker))

typedef struct _PlumaSpellChecker PlumaSpellChecker;

typedef struct _PlumaSpellCheckerClass PlumaSpellCheckerClass;

struct _PlumaSpellCheckerClass
{
	GObjectClass parent_class;

	/* Signals */
	void (*add_word_to_personal) (PlumaSpellChecker               *spell,
				      const gchar                     *word,
				      gint                             len);

	void (*add_word_to_session)  (PlumaSpellChecker               *spell,
				      const gchar                     *word,
				      gint                             len);

	void (*set_language)         (PlumaSpellChecker               *spell,
				      const PlumaSpellCheckerLanguage *lang);

	void (*clear_session)	     (PlumaSpellChecker               *spell);
};


GType        		 pluma_spell_checker_get_type		(void) G_GNUC_CONST;

/* Constructors */
PlumaSpellChecker	*pluma_spell_checker_new		(void);

gboolean		 pluma_spell_checker_set_language 	(PlumaSpellChecker               *spell,
								 const PlumaSpellCheckerLanguage *lang);
const PlumaSpellCheckerLanguage
			*pluma_spell_checker_get_language 	(PlumaSpellChecker               *spell);

gboolean		 pluma_spell_checker_check_word 	(PlumaSpellChecker               *spell,
								 const gchar                     *word,
								 gssize                           len);

GSList 			*pluma_spell_checker_get_suggestions 	(PlumaSpellChecker               *spell,
								 const gchar                     *word,
								 gssize                           len);

gboolean		 pluma_spell_checker_add_word_to_personal
								(PlumaSpellChecker               *spell,
								 const gchar                     *word,
								 gssize                           len);

gboolean		 pluma_spell_checker_add_word_to_session
								(PlumaSpellChecker               *spell,
								 const gchar                     *word,
								 gssize                           len);

gboolean		 pluma_spell_checker_clear_session 	(PlumaSpellChecker               *spell);

gboolean		 pluma_spell_checker_set_correction 	(PlumaSpellChecker               *spell,
								 const gchar                     *word,
								 gssize                           w_len,
								 const gchar                     *replacement,
								 gssize                           r_len);
G_END_DECLS

#endif  /* __PLUMA_SPELL_CHECKER_H__ */

