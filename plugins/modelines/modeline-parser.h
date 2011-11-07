/*
 * modelie-parser.h
 * Emacs, Kate and Vim-style modelines support for gedit.
 * 
 * Copyright (C) 2005-2007 - Steve Frécinaux <code@istique.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __MODELINE_PARSER_H__
#define __MODELINE_PARSER_H__

#include <glib.h>
#include <gtksourceview/gtksourceview.h>

G_BEGIN_DECLS

void	modeline_parser_init		(const gchar *data_dir);
void	modeline_parser_shutdown	(void);
void	modeline_parser_apply_modeline	(GtkSourceView *view);
void	modeline_parser_deactivate	(GtkSourceView *view);

G_END_DECLS

#endif /* __MODELINE_PARSER_H__ */
