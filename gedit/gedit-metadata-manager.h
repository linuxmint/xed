/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gedit-metadata-manager.h
 * This file is part of gedit
 *
 * Copyright (C) 2003  Paolo Maggi 
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
 * Modified by the gedit Team, 2003. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifndef __GEDIT_METADATA_MANAGER_H__
#define __GEDIT_METADATA_MANAGER_H__

#include <glib.h>

G_BEGIN_DECLS


/* This function must be called before exiting gedit */
void		 gedit_metadata_manager_shutdown 	(void);


gchar		*gedit_metadata_manager_get 		(const gchar *uri,
					     		 const gchar *key);
void		 gedit_metadata_manager_set		(const gchar *uri,
							 const gchar *key,
							 const gchar *value);

G_END_DECLS

#endif /* __GEDIT_METADATA_MANAGER_H__ */
