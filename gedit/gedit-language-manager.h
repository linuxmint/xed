/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gedit-languages-manager.h
 * This file is part of gedit
 *
 * Copyright (C) 2003-2005 - Paolo Maggi 
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */
 
/*
 * Modified by the gedit Team, 2003-2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __GEDIT_LANGUAGES_MANAGER_H__
#define __GEDIT_LANGUAGES_MANAGER_H__

#include <gtksourceview/gtksourcelanguagemanager.h>

G_BEGIN_DECLS

GtkSourceLanguageManager	*gedit_get_language_manager	(void);

GSList				*gedit_language_manager_list_languages_sorted
								(GtkSourceLanguageManager	*lm,
								 gboolean			 include_hidden);

G_END_DECLS

#endif /* __GEDIT_LANGUAGES_MANAGER_H__ */
