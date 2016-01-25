/*
 * xedit-session.h - Basic session management for xedit
 * This file is part of xedit
 *
 * Copyright (C) 2002 Ximian, Inc.
 * Copyright (C) 2005 - Paolo Maggi 
 *
 * Author: Federico Mena-Quintero <federico@ximian.com>
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
 * Modified by the xedit Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes.
 *
 * $Id 
 */

#ifndef __XEDIT_SESSION_H__
#define __XEDIT_SESSION_H__

#include <glib.h>

G_BEGIN_DECLS

void		xedit_session_init 		(void);
gboolean	xedit_session_is_restored 	(void);
gboolean 	xedit_session_load 		(void);

G_END_DECLS

#endif /* __XEDIT_SESSION_H__ */
