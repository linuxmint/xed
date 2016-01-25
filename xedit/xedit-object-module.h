/*
 * xedit-object-module.h
 * This file is part of xedit
 *
 * Copyright (C) 2005 - Paolo Maggi
 * Copyright (C) 2008 - Jesse van den Kieboom
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
 
/* This is a modified version of xedit-module.h from Epiphany source code.
 * Here the original copyright assignment:
 *
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003, 2004 Christian Persch
 *
 */

/*
 * Modified by the xedit Team, 2005. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id: xedit-module.h 6263 2008-05-05 10:52:10Z sfre $
 */
 
#ifndef __XEDIT_OBJECT_MODULE_H__
#define __XEDIT_OBJECT_MODULE_H__

#include <glib-object.h>
#include <gmodule.h>
#include <stdarg.h>

G_BEGIN_DECLS

#define XEDIT_TYPE_OBJECT_MODULE		(xedit_object_module_get_type ())
#define XEDIT_OBJECT_MODULE(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_OBJECT_MODULE, XeditObjectModule))
#define XEDIT_OBJECT_MODULE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_OBJECT_MODULE, XeditObjectModuleClass))
#define XEDIT_IS_OBJECT_MODULE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XEDIT_TYPE_OBJECT_MODULE))
#define XEDIT_IS_OBJECT_MODULE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_OBJECT_MODULE))
#define XEDIT_OBJECT_MODULE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XEDIT_TYPE_OBJECT_MODULE, XeditObjectModuleClass))

typedef struct _XeditObjectModule 		XeditObjectModule;
typedef struct _XeditObjectModulePrivate	XeditObjectModulePrivate;

struct _XeditObjectModule
{
	GTypeModule parent;

	XeditObjectModulePrivate *priv;
};

typedef struct _XeditObjectModuleClass XeditObjectModuleClass;

struct _XeditObjectModuleClass
{
	GTypeModuleClass parent_class;

	/* Virtual class methods */
	void		 (* garbage_collect)	();
};

GType		 xedit_object_module_get_type			(void) G_GNUC_CONST;

XeditObjectModule *xedit_object_module_new			(const gchar *module_name,
								 const gchar *path,
								 const gchar *type_registration,
								 gboolean     resident);

GObject		*xedit_object_module_new_object			(XeditObjectModule *module, 
								 const gchar	   *first_property_name,
								 ...);

GType		 xedit_object_module_get_object_type		(XeditObjectModule *module);
const gchar	*xedit_object_module_get_path			(XeditObjectModule *module);
const gchar	*xedit_object_module_get_module_name		(XeditObjectModule *module);
const gchar 	*xedit_object_module_get_type_registration 	(XeditObjectModule *module);

G_END_DECLS

#endif
