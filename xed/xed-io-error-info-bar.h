/*
 * xed-io-error-info-bar.h
 * This file is part of xed
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/*
 * Modified by the xed Team, 2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifndef __XED_IO_ERROR_INFO_BAR_H__
#define __XED_IO_ERROR_INFO_BAR_H__

#include <gtksourceview/gtksource.h>

G_BEGIN_DECLS

GtkWidget *xed_io_loading_error_info_bar_new (GFile                   *location,
                                              const GtkSourceEncoding *encoding,
                                              const GError            *error);

GtkWidget *xed_unrecoverable_reverting_error_info_bar_new (GFile        *location,
                                                           const GError *error);

GtkWidget *xed_conversion_error_while_saving_info_bar_new (GFile                   *location,
                                                           const GtkSourceEncoding *encoding,
                                                           const GError            *error);

const GtkSourceEncoding *xed_conversion_error_info_bar_get_encoding (GtkWidget *info_bar);

GtkWidget *xed_file_already_open_warning_info_bar_new (GFile *location);

GtkWidget *xed_externally_modified_saving_error_info_bar_new (GFile        *location,
                                                              const GError *error);

GtkWidget *xed_no_backup_saving_error_info_bar_new (GFile        *location,
                                                    const GError *error);

GtkWidget *xed_unrecoverable_saving_error_info_bar_new (GFile        *location,
                                                        const GError *error);

GtkWidget *xed_externally_modified_info_bar_new (GFile    *location,
                                                 gboolean  document_modified);

GtkWidget *xed_invalid_character_info_bar_new (GFile *location);

G_END_DECLS

#endif  /* __XED_IO_ERROR_INFO_BAR_H__  */
