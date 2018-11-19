/*
 * xed-highlight-mode-selector.h
 * This file is part of xed
 *
 * Copyright (C) 2013 - Ignacio Casal Quinteiro
 *
 * xed is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xed is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xed. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef XED_HIGHLIGHT_MODE_SELECTOR_H
#define XED_HIGHLIGHT_MODE_SELECTOR_H

#include <glib-object.h>
#include <gtksourceview/gtksource.h>
#include "xed-window.h"

G_BEGIN_DECLS

#define XED_TYPE_HIGHLIGHT_MODE_SELECTOR (xed_highlight_mode_selector_get_type ())

G_DECLARE_FINAL_TYPE (XedHighlightModeSelector, xed_highlight_mode_selector, XED, HIGHLIGHT_MODE_SELECTOR, GtkGrid)

XedHighlightModeSelector *xed_highlight_mode_selector_new             (void);

void                        xed_highlight_mode_selector_select_language (XedHighlightModeSelector *selector,
                                                                           GtkSourceLanguage          *language);

void                        xed_highlight_mode_selector_activate_selected_language
                                                                          (XedHighlightModeSelector *selector);

G_END_DECLS

#endif /* XED_HIGHLIGHT_MODE_SELECTOR_H */

/* ex:set ts=8 noet: */
