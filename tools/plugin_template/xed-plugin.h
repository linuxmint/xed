/*
 * ##(FILENAME) - ##(DESCRIPTION)
 *
 * Copyright (C) ##(DATE_YEAR) - ##(AUTHOR_FULLNAME)
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

#ifndef __##(PLUGIN_ID.upper)_PLUGIN_H__
#define __##(PLUGIN_ID.upper)_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <libpeas/peas-extension-base.h>
#include <libpeas/peas-object-module.h>

G_BEGIN_DECLS

#define ##(PLUGIN_ID.upper)_TYPE_PLUGIN     (##(PLUGIN_ID.lower)_plugin_get_type ())
G_DECLARE_FINAL_TYPE (##(PLUGIN_ID.camel)Plugin, ##(PLUGIN_ID.lower)_plugin, ##(PLUGIN_ID.upper), PLUGIN, PeasExtensionBase)

/* All the plugins must implement this function */
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* __##(PLUGIN_ID.upper)_PLUGIN_H__ */
