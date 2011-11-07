/*
 * gedit-spinner.h
 * This file is part of gedit
 *
 * Copyright (C) 2005 - Paolo Maggi 
 * Copyright (C) 2000 - Eazel, Inc. 
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
 * This widget was originally written by Andy Hertzfeld <andy@eazel.com> for
 * Caja.
 *
 * Modified by the gedit Team, 2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __GEDIT_SPINNER_H__
#define __GEDIT_SPINNER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GEDIT_TYPE_SPINNER		(gedit_spinner_get_type ())
#define GEDIT_SPINNER(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), GEDIT_TYPE_SPINNER, GeditSpinner))
#define GEDIT_SPINNER_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), GEDIT_TYPE_SPINNER, GeditSpinnerClass))
#define GEDIT_IS_SPINNER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GEDIT_TYPE_SPINNER))
#define GEDIT_IS_SPINNER_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), GEDIT_TYPE_SPINNER))
#define GEDIT_SPINNER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), GEDIT_TYPE_SPINNER, GeditSpinnerClass))


/* Private structure type */
typedef struct _GeditSpinnerPrivate	GeditSpinnerPrivate;

/*
 * Main object structure
 */
typedef struct _GeditSpinner		GeditSpinner;

struct _GeditSpinner
{
	GtkWidget parent;

	/*< private >*/
	GeditSpinnerPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GeditSpinnerClass	GeditSpinnerClass;

struct _GeditSpinnerClass
{
	GtkWidgetClass parent_class;
};

/*
 * Public methods
 */
GType		gedit_spinner_get_type	(void) G_GNUC_CONST;

GtkWidget      *gedit_spinner_new	(void);

void		gedit_spinner_start	(GeditSpinner *throbber);

void		gedit_spinner_stop	(GeditSpinner *throbber);

void		gedit_spinner_set_size	(GeditSpinner *spinner,
					 GtkIconSize   size);

G_END_DECLS

#endif /* __GEDIT_SPINNER_H__ */
