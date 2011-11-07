/*
 * gedit-object-module.h
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */
 
/* This is a modified version of gedit-module.h from Epiphany source code.
 * Here the original copyright assignment:
 *
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003, 2004 Christian Persch
 *
 */

/*
 * Modified by the gedit Team, 2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id: gedit-module.h 6263 2008-05-05 10:52:10Z sfre $
 */
 
#ifndef __GEDIT_OBJECT_MODULE_H__
#define __GEDIT_OBJECT_MODULE_H__

#include <glib-object.h>
#include <gmodule.h>
#include <stdarg.h>

G_BEGIN_DECLS

#define GEDIT_TYPE_OBJECT_MODULE		(gedit_object_module_get_type ())
#define GEDIT_OBJECT_MODULE(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_OBJECT_MODULE, GeditObjectModule))
#define GEDIT_OBJECT_MODULE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_OBJECT_MODULE, GeditObjectModuleClass))
#define GEDIT_IS_OBJECT_MODULE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_OBJECT_MODULE))
#define GEDIT_IS_OBJECT_MODULE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_OBJECT_MODULE))
#define GEDIT_OBJECT_MODULE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GEDIT_TYPE_OBJECT_MODULE, GeditObjectModuleClass))

typedef struct _GeditObjectModule 		GeditObjectModule;
typedef struct _GeditObjectModulePrivate	GeditObjectModulePrivate;

struct _GeditObjectModule
{
	GTypeModule parent;

	GeditObjectModulePrivate *priv;
};

typedef struct _GeditObjectModuleClass GeditObjectModuleClass;

struct _GeditObjectModuleClass
{
	GTypeModuleClass parent_class;

	/* Virtual class methods */
	void		 (* garbage_collect)	();
};

GType		 gedit_object_module_get_type			(void) G_GNUC_CONST;

GeditObjectModule *gedit_object_module_new			(const gchar *module_name,
								 const gchar *path,
								 const gchar *type_registration,
								 gboolean     resident);

GObject		*gedit_object_module_new_object			(GeditObjectModule *module, 
								 const gchar	   *first_property_name,
								 ...);

GType		 gedit_object_module_get_object_type		(GeditObjectModule *module);
const gchar	*gedit_object_module_get_path			(GeditObjectModule *module);
const gchar	*gedit_object_module_get_module_name		(GeditObjectModule *module);
const gchar 	*gedit_object_module_get_type_registration 	(GeditObjectModule *module);

G_END_DECLS

#endif
