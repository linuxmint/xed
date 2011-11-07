/*
 * gedit-debug.h
 * This file is part of gedit
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi
 * Copyright (C) 2002 - 2005 Paolo Maggi  
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
 * Modified by the gedit Team, 1998-2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __GEDIT_DEBUG_H__
#define __GEDIT_DEBUG_H__

#include <glib.h>

/*
 * Set an environmental var of the same name to turn on
 * debugging output. Setting GEDIT_DEBUG will turn on all
 * sections.
 */
typedef enum {
	GEDIT_NO_DEBUG       = 0,
	GEDIT_DEBUG_VIEW     = 1 << 0,
	GEDIT_DEBUG_SEARCH   = 1 << 1,
	GEDIT_DEBUG_PRINT    = 1 << 2,
	GEDIT_DEBUG_PREFS    = 1 << 3,
	GEDIT_DEBUG_PLUGINS  = 1 << 4,
	GEDIT_DEBUG_TAB      = 1 << 5,
	GEDIT_DEBUG_DOCUMENT = 1 << 6,
	GEDIT_DEBUG_COMMANDS = 1 << 7,
	GEDIT_DEBUG_APP      = 1 << 8,
	GEDIT_DEBUG_SESSION  = 1 << 9,
	GEDIT_DEBUG_UTILS    = 1 << 10,
	GEDIT_DEBUG_METADATA = 1 << 11,
	GEDIT_DEBUG_WINDOW   = 1 << 12,
	GEDIT_DEBUG_LOADER   = 1 << 13,
	GEDIT_DEBUG_SAVER    = 1 << 14
} GeditDebugSection;


#define	DEBUG_VIEW	GEDIT_DEBUG_VIEW,    __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_SEARCH	GEDIT_DEBUG_SEARCH,  __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_PRINT	GEDIT_DEBUG_PRINT,   __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_PREFS	GEDIT_DEBUG_PREFS,   __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_PLUGINS	GEDIT_DEBUG_PLUGINS, __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_TAB	GEDIT_DEBUG_TAB,     __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_DOCUMENT	GEDIT_DEBUG_DOCUMENT,__FILE__, __LINE__, G_STRFUNC
#define	DEBUG_COMMANDS	GEDIT_DEBUG_COMMANDS,__FILE__, __LINE__, G_STRFUNC
#define	DEBUG_APP	GEDIT_DEBUG_APP,     __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_SESSION	GEDIT_DEBUG_SESSION, __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_UTILS	GEDIT_DEBUG_UTILS,   __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_METADATA	GEDIT_DEBUG_METADATA,__FILE__, __LINE__, G_STRFUNC
#define	DEBUG_WINDOW	GEDIT_DEBUG_WINDOW,  __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_LOADER	GEDIT_DEBUG_LOADER,  __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_SAVER	GEDIT_DEBUG_SAVER,   __FILE__, __LINE__, G_STRFUNC

void gedit_debug_init (void);

void gedit_debug (GeditDebugSection  section,
		  const gchar       *file,
		  gint               line,
		  const gchar       *function);

void gedit_debug_message (GeditDebugSection  section,
			  const gchar       *file,
			  gint               line,
			  const gchar       *function,
			  const gchar       *format, ...) G_GNUC_PRINTF(5, 6);


#endif /* __GEDIT_DEBUG_H__ */
