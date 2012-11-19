/*
 * pluma-spinner.h
 * This file is part of pluma
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA 02110-1301, USA.
 */
 
/*
 * This widget was originally written by Andy Hertzfeld <andy@eazel.com> for
 * Caja.
 *
 * Modified by the pluma Team, 2005. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __PLUMA_SPINNER_H__
#define __PLUMA_SPINNER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_SPINNER		(pluma_spinner_get_type ())
#define PLUMA_SPINNER(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), PLUMA_TYPE_SPINNER, PlumaSpinner))
#define PLUMA_SPINNER_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), PLUMA_TYPE_SPINNER, PlumaSpinnerClass))
#define PLUMA_IS_SPINNER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), PLUMA_TYPE_SPINNER))
#define PLUMA_IS_SPINNER_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), PLUMA_TYPE_SPINNER))
#define PLUMA_SPINNER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), PLUMA_TYPE_SPINNER, PlumaSpinnerClass))


/* Private structure type */
typedef struct _PlumaSpinnerPrivate	PlumaSpinnerPrivate;

/*
 * Main object structure
 */
typedef struct _PlumaSpinner		PlumaSpinner;

struct _PlumaSpinner
{
	GtkWidget parent;

	/*< private >*/
	PlumaSpinnerPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _PlumaSpinnerClass	PlumaSpinnerClass;

struct _PlumaSpinnerClass
{
	GtkWidgetClass parent_class;
};

/*
 * Public methods
 */
GType		pluma_spinner_get_type	(void) G_GNUC_CONST;

GtkWidget      *pluma_spinner_new	(void);

void		pluma_spinner_start	(PlumaSpinner *throbber);

void		pluma_spinner_stop	(PlumaSpinner *throbber);

void		pluma_spinner_set_size	(PlumaSpinner *spinner,
					 GtkIconSize   size);

G_END_DECLS

#endif /* __PLUMA_SPINNER_H__ */
