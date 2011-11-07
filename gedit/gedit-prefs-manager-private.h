/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gedit-prefs-manager-private.h
 * This file is part of gedit
 *
 * Copyright (C) 2002  Paolo Maggi 
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
 * Modified by the gedit Team, 2002. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifndef __GEDIT_PREFS_MANAGER_PRIVATE_H__
#define __GEDIT_PREFS_MANAGER_PRIVATE_H__

#include <mateconf/mateconf-client.h>

typedef struct _GeditPrefsManager 	GeditPrefsManager;

struct _GeditPrefsManager {
	MateConfClient *mateconf_client;
};

extern GeditPrefsManager *gedit_prefs_manager;

#endif /* __GEDIT_PREFS_MANAGER_PRIVATE_H__ */


