/*
 * pluma-taglist-plugin-parser.h
 * This file is part of pluma
 *
 * Copyright (C) 2002-2005 - Paolo Maggi 
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
 * Modified by the pluma Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __PLUMA_TAGLIST_PLUGIN_PARSER_H__
#define __PLUMA_TAGLIST_PLUGIN_PARSER_H__

#include <libxml/tree.h>
#include <glib.h>

typedef struct _TagList TagList;
typedef struct _TagGroup TagGroup;
typedef struct _Tag Tag;

struct _TagList {
	GList* tag_groups;
};

struct _TagGroup {
	xmlChar* name;

	GList* tags;
};

struct _Tag {
	xmlChar* name;
	xmlChar* begin;
	xmlChar* end;
};

/* Note that the taglist is ref counted */
extern TagList *taglist;

TagList* create_taglist(const gchar* data_dir);

void free_taglist(void);

#endif /* __PLUMA_TAGLIST_PLUGIN_PARSER_H__ */

