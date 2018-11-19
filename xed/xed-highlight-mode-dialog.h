/*
 * xed-highlight-mode-dialog.h
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


#ifndef XED_HIGHLIGHT_MODE_DIALOG_H
#define XED_HIGHLIGHT_MODE_DIALOG_H

#include <glib.h>
#include "xed-highlight-mode-selector.h"

G_BEGIN_DECLS

#define XED_TYPE_HIGHLIGHT_MODE_DIALOG (xed_highlight_mode_dialog_get_type ())

G_DECLARE_FINAL_TYPE (XedHighlightModeDialog, xed_highlight_mode_dialog, XED, HIGHLIGHT_MODE_DIALOG, GtkDialog)

GtkWidget                  *xed_highlight_mode_dialog_new             (GtkWindow *parent);

XedHighlightModeSelector *xed_highlight_mode_dialog_get_selector    (XedHighlightModeDialog *dlg);

G_END_DECLS

#endif /* XED_HIGHLIGHT_MODE_DIALOG_H */

/* ex:set ts=8 noet: */
