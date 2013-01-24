/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pluma-prefs-manager-private.h
 * This file is part of pluma
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA 02110-1301, USA. 
 */
 
/*
 * Modified by the pluma Team, 2002. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifndef __PLUMA_PREFS_MANAGER_PRIVATE_H__
#define __PLUMA_PREFS_MANAGER_PRIVATE_H__

#include <gio/gio.h>

typedef struct _PlumaPrefsManager 	PlumaPrefsManager;

struct _PlumaPrefsManager {
	GSettings *settings;
	GSettings *lockdown_settings;
	GSettings *interface_settings;
};

extern PlumaPrefsManager *pluma_prefs_manager;

#endif /* __PLUMA_PREFS_MANAGER_PRIVATE_H__ */


