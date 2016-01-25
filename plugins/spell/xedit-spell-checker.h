/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xedit-spell-checker.h
 * This file is part of xedit
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
 * Modified by the xedit Team, 2002. See the AUTHORS file for a
 * list of people on the xedit Team.
 * See the ChangeLog files for a list of changes.
 */

#ifndef __XEDIT_SPELL_CHECKER_H__
#define __XEDIT_SPELL_CHECKER_H__

#include <glib.h>
#include <glib-object.h>

#include "xedit-spell-checker-language.h"

G_BEGIN_DECLS

#define XEDIT_TYPE_SPELL_CHECKER            (xedit_spell_checker_get_type ())
#define XEDIT_SPELL_CHECKER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_SPELL_CHECKER, XeditSpellChecker))
#define XEDIT_SPELL_CHECKER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_SPELL_CHECKER, XeditSpellChecker))
#define XEDIT_IS_SPELL_CHECKER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XEDIT_TYPE_SPELL_CHECKER))
#define XEDIT_IS_SPELL_CHECKER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_SPELL_CHECKER))
#define XEDIT_SPELL_CHECKER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XEDIT_TYPE_SPELL_CHECKER, XeditSpellChecker))

typedef struct _XeditSpellChecker XeditSpellChecker;

typedef struct _XeditSpellCheckerClass XeditSpellCheckerClass;

struct _XeditSpellCheckerClass
{
	GObjectClass parent_class;

	/* Signals */
	void (*add_word_to_personal) (XeditSpellChecker               *spell,
				      const gchar                     *word,
				      gint                             len);

	void (*add_word_to_session)  (XeditSpellChecker               *spell,
				      const gchar                     *word,
				      gint                             len);

	void (*set_language)         (XeditSpellChecker               *spell,
				      const XeditSpellCheckerLanguage *lang);

	void (*clear_session)	     (XeditSpellChecker               *spell);
};


GType        		 xedit_spell_checker_get_type		(void) G_GNUC_CONST;

/* Constructors */
XeditSpellChecker	*xedit_spell_checker_new		(void);

gboolean		 xedit_spell_checker_set_language 	(XeditSpellChecker               *spell,
								 const XeditSpellCheckerLanguage *lang);
const XeditSpellCheckerLanguage
			*xedit_spell_checker_get_language 	(XeditSpellChecker               *spell);

gboolean		 xedit_spell_checker_check_word 	(XeditSpellChecker               *spell,
								 const gchar                     *word,
								 gssize                           len);

GSList 			*xedit_spell_checker_get_suggestions 	(XeditSpellChecker               *spell,
								 const gchar                     *word,
								 gssize                           len);

gboolean		 xedit_spell_checker_add_word_to_personal
								(XeditSpellChecker               *spell,
								 const gchar                     *word,
								 gssize                           len);

gboolean		 xedit_spell_checker_add_word_to_session
								(XeditSpellChecker               *spell,
								 const gchar                     *word,
								 gssize                           len);

gboolean		 xedit_spell_checker_clear_session 	(XeditSpellChecker               *spell);

gboolean		 xedit_spell_checker_set_correction 	(XeditSpellChecker               *spell,
								 const gchar                     *word,
								 gssize                           w_len,
								 const gchar                     *replacement,
								 gssize                           r_len);
G_END_DECLS

#endif  /* __XEDIT_SPELL_CHECKER_H__ */

