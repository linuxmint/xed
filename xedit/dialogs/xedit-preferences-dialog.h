/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xedit-preferences-dialog.c
 * This file is part of xedit
 *
 * Copyright (C) 2001-2005 Paolo Maggi 
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
 * Modified by the xedit Team, 2003. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __XEDIT_PREFERENCES_DIALOG_H__
#define __XEDIT_PREFERENCES_DIALOG_H__

#include "xedit-window.h"

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XEDIT_TYPE_PREFERENCES_DIALOG              (xedit_preferences_dialog_get_type())
#define XEDIT_PREFERENCES_DIALOG(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XEDIT_TYPE_PREFERENCES_DIALOG, XeditPreferencesDialog))
#define XEDIT_PREFERENCES_DIALOG_CONST(obj)        (G_TYPE_CHECK_INSTANCE_CAST((obj), XEDIT_TYPE_PREFERENCES_DIALOG, XeditPreferencesDialog const))
#define XEDIT_PREFERENCES_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XEDIT_TYPE_PREFERENCES_DIALOG, XeditPreferencesDialogClass))
#define XEDIT_IS_PREFERENCES_DIALOG(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XEDIT_TYPE_PREFERENCES_DIALOG))
#define XEDIT_IS_PREFERENCES_DIALOG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_PREFERENCES_DIALOG))
#define XEDIT_PREFERENCES_DIALOG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XEDIT_TYPE_PREFERENCES_DIALOG, XeditPreferencesDialogClass))


/* Private structure type */
typedef struct _XeditPreferencesDialogPrivate XeditPreferencesDialogPrivate;

/*
 * Main object structure
 */
typedef struct _XeditPreferencesDialog XeditPreferencesDialog;

struct _XeditPreferencesDialog 
{
	GtkDialog dialog;
	
	/*< private > */
	XeditPreferencesDialogPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XeditPreferencesDialogClass XeditPreferencesDialogClass;

struct _XeditPreferencesDialogClass 
{
	GtkDialogClass parent_class;
};

/*
 * Public methods
 */
GType		 xedit_preferences_dialog_get_type	(void) G_GNUC_CONST;

void		 xedit_show_preferences_dialog		(XeditWindow *parent);

G_END_DECLS

#endif /* __XEDIT_PREFERENCES_DIALOG_H__ */

