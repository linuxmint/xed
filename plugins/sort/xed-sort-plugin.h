/*
 * xed-sort-plugin.h
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
 */

#ifndef __XED_SORT_PLUGIN_H__
#define __XED_SORT_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <libpeas/peas-extension-base.h>
#include <libpeas/peas-object-module.h>

G_BEGIN_DECLS

#define XED_TYPE_SORT_PLUGIN            (xed_sort_plugin_get_type ())
#define XED_SORT_PLUGIN(o)              (G_TYPE_CHECK_INSTANCE_CAST ((o), XED_TYPE_SORT_PLUGIN, XedSortPlugin))
#define XED_SORT_PLUGIN_CLASS(k)        (G_TYPE_CHECK_CLASS_CAST((k), XED_TYPE_SORT_PLUGIN, XedSortPluginClass))
#define XED_IS_SORT_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_TYPE ((o), XED_TYPE_SORT_PLUGIN))
#define XED_IS_SORT_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_TYPE ((k), XED_TYPE_SORT_PLUGIN))
#define XED_SORT_PLUGIN_GET_CLASS(o)    (G_TYPE_INSTANCE_GET_CLASS ((o), XED_TYPE_SORT_PLUGIN, XedSortPluginClass))

typedef struct _XedSortPlugin           XedSortPlugin;
typedef struct _XedSortPluginPrivate    XedSortPluginPrivate;
typedef struct _XedSortPluginClass      XedSortPluginClass;

struct _XedSortPlugin
{
    PeasExtensionBase parent;

    /*< private >*/
    XedSortPluginPrivate *priv;
};

struct _XedSortPluginClass
{
    PeasExtensionBaseClass parent_class;
};

GType xed_sort_plugin_get_type (void) G_GNUC_CONST;

G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* __XED_SORT_PLUGIN_H__ */
