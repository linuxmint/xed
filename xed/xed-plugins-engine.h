/*
 * xed-plugins-engine.h
 * This file is part of xed
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
 * Modified by the xed Team, 2002-2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __XED_PLUGINS_ENGINE_H__
#define __XED_PLUGINS_ENGINE_H__

#include <glib.h>
#include <libpeas/peas-engine.h>

G_BEGIN_DECLS

#define XED_TYPE_PLUGINS_ENGINE              (xed_plugins_engine_get_type ())
#define XED_PLUGINS_ENGINE(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_PLUGINS_ENGINE, XedPluginsEngine))
#define XED_PLUGINS_ENGINE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_PLUGINS_ENGINE, XedPluginsEngineClass))
#define XED_IS_PLUGINS_ENGINE(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_PLUGINS_ENGINE))
#define XED_IS_PLUGINS_ENGINE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_PLUGINS_ENGINE))
#define XED_PLUGINS_ENGINE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_PLUGINS_ENGINE, XedPluginsEngineClass))

typedef struct _XedPluginsEngine        XedPluginsEngine;
typedef struct _XedPluginsEnginePrivate XedPluginsEnginePrivate;

struct _XedPluginsEngine
{
    PeasEngine parent;
    XedPluginsEnginePrivate *priv;
};

typedef struct _XedPluginsEngineClass       XedPluginsEngineClass;

struct _XedPluginsEngineClass
{
    PeasEngineClass parent_class;
};

GType             xed_plugins_engine_get_type    (void) G_GNUC_CONST;

XedPluginsEngine *xed_plugins_engine_get_default (void);

G_END_DECLS

#endif  /* __XED_PLUGINS_ENGINE_H__ */
