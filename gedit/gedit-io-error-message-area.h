/*
 * gedit-io-error-message-area.h
 * This file is part of gedit
 *
 * Copyright (C) 2005 - Paolo Maggi 
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
 * Modified by the gedit Team, 2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __GEDIT_IO_ERROR_MESSAGE_AREA_H__
#define __GEDIT_IO_ERROR_MESSAGE_AREA_H__

#include <glib.h>

G_BEGIN_DECLS

GtkWidget	*gedit_io_loading_error_message_area_new		 (const gchar         *uri,
									  const GeditEncoding *encoding,
									  const GError        *error);

GtkWidget	*gedit_unrecoverable_reverting_error_message_area_new	 (const gchar         *uri,
									  const GError        *error);

GtkWidget	*gedit_conversion_error_while_saving_message_area_new	 (const gchar         *uri,
									  const GeditEncoding *encoding,
									  const GError        *error);

const GeditEncoding 
		*gedit_conversion_error_message_area_get_encoding	 (GtkWidget           *message_area);

GtkWidget	*gedit_file_already_open_warning_message_area_new	 (const gchar         *uri);

GtkWidget	*gedit_externally_modified_saving_error_message_area_new (const gchar         *uri,
									  const GError        *error);

GtkWidget	*gedit_no_backup_saving_error_message_area_new		 (const gchar         *uri,
									  const GError        *error);

GtkWidget	*gedit_unrecoverable_saving_error_message_area_new	 (const gchar         *uri,
									  const GError        *error);

GtkWidget	*gedit_externally_modified_message_area_new		 (const gchar         *uri,
									  gboolean             document_modified);

G_END_DECLS

#endif  /* __GEDIT_IO_ERROR_MESSAGE_AREA_H__  */
