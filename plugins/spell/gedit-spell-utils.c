/*
 * gedit-spell-utils.c
 * This file is part of gedit
 *
 * Copyright (C) 2010 - Jesse van den Kieboom
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
 * Boston, MA  02110-1301  USA
 */

#include <string.h>

#include "gedit-spell-utils.h"
#include <gtksourceview/gtksourcebuffer.h>

gboolean
gedit_spell_utils_is_digit (const char *text, gssize length)
{
	gunichar c;
	const gchar *p;
	const gchar *end;

	g_return_val_if_fail (text != NULL, FALSE);

	if (length < 0)
		length = strlen (text);

	p = text;
	end = text + length;

	while (p != end) {
		const gchar *next;
		next = g_utf8_next_char (p);

		c = g_utf8_get_char (p);

		if (!g_unichar_isdigit (c) && c != '.' && c != ',')
			return FALSE;

		p = next;
	}

	return TRUE;
}

gboolean
gedit_spell_utils_skip_no_spell_check (GtkTextIter *start,
                                       GtkTextIter *end)
{
	GtkSourceBuffer *buffer = GTK_SOURCE_BUFFER (gtk_text_iter_get_buffer (start));

	while (gtk_source_buffer_iter_has_context_class (buffer, start, "no-spell-check"))
	{
		GtkTextIter last = *start;

		if (!gtk_source_buffer_iter_forward_to_context_class_toggle (buffer, start, "no-spell-check"))
		{
			return FALSE;
		}

		if (gtk_text_iter_compare (start, &last) <= 0)
		{
			return FALSE;
		}

		gtk_text_iter_forward_word_end (start);
		gtk_text_iter_backward_word_start (start);

		if (gtk_text_iter_compare (start, &last) <= 0)
		{
			return FALSE;
		}

		if (gtk_text_iter_compare (start, end) >= 0)
		{
			return FALSE;
		}
	}

	return TRUE;
}

