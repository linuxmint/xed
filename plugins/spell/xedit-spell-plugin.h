/*
 * xedit-spell-plugin.h
 * 
 * Copyright (C) 2002-2005 Paolo Maggi 
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Id$
 */

#ifndef __XEDIT_SPELL_PLUGIN_H__
#define __XEDIT_SPELL_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <xedit/xedit-plugin.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XEDIT_TYPE_SPELL_PLUGIN		(xedit_spell_plugin_get_type ())
#define XEDIT_SPELL_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), XEDIT_TYPE_SPELL_PLUGIN, XeditSpellPlugin))
#define XEDIT_SPELL_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), XEDIT_TYPE_SPELL_PLUGIN, XeditSpellPluginClass))
#define XEDIT_IS_SPELL_PLUGIN(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), XEDIT_TYPE_SPELL_PLUGIN))
#define XEDIT_IS_SPELL_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), XEDIT_TYPE_SPELL_PLUGIN))
#define XEDIT_SPELL_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), XEDIT_TYPE_SPELL_PLUGIN, XeditSpellPluginClass))

/* Private structure type */
typedef struct _XeditSpellPluginPrivate	XeditSpellPluginPrivate;

/*
 * Main object structure
 */
typedef struct _XeditSpellPlugin	XeditSpellPlugin;

struct _XeditSpellPlugin
{
	XeditPlugin parent_instance;

	XeditSpellPluginPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XeditSpellPluginClass	XeditSpellPluginClass;

struct _XeditSpellPluginClass
{
	XeditPluginClass parent_class;
};

/*
 * Public methods
 */
GType	xedit_spell_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT GType register_xedit_plugin (GTypeModule *module);

G_END_DECLS

#endif /* __XEDIT_SPELL_PLUGIN_H__ */
