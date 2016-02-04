/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xed-preferences-dialog.c
 * This file is part of xed
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
 * Modified by the xed Team, 2003. See the AUTHORS file for a 
 * list of people on the xed Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __XED_PREFERENCES_DIALOG_H__
#define __XED_PREFERENCES_DIALOG_H__

#include "xed-window.h"

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XED_TYPE_PREFERENCES_DIALOG              (xed_preferences_dialog_get_type())
#define XED_PREFERENCES_DIALOG(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_PREFERENCES_DIALOG, XedPreferencesDialog))
#define XED_PREFERENCES_DIALOG_CONST(obj)        (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_PREFERENCES_DIALOG, XedPreferencesDialog const))
#define XED_PREFERENCES_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_PREFERENCES_DIALOG, XedPreferencesDialogClass))
#define XED_IS_PREFERENCES_DIALOG(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_PREFERENCES_DIALOG))
#define XED_IS_PREFERENCES_DIALOG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_PREFERENCES_DIALOG))
#define XED_PREFERENCES_DIALOG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_PREFERENCES_DIALOG, XedPreferencesDialogClass))


/* Private structure type */
typedef struct _XedPreferencesDialogPrivate XedPreferencesDialogPrivate;

/*
 * Main object structure
 */
typedef struct _XedPreferencesDialog XedPreferencesDialog;

struct _XedPreferencesDialog 
{
	GtkDialog dialog;
	
	/*< private > */
	XedPreferencesDialogPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XedPreferencesDialogClass XedPreferencesDialogClass;

struct _XedPreferencesDialogClass 
{
	GtkDialogClass parent_class;
};

/*
 * Public methods
 */
GType		 xed_preferences_dialog_get_type	(void) G_GNUC_CONST;

void		 xed_show_preferences_dialog		(XedWindow *parent);

G_END_DECLS

#endif /* __XED_PREFERENCES_DIALOG_H__ */

