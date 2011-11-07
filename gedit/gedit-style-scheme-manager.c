/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gedit-source-style-manager.c
 *
 * Copyright (C) 2007 - Paolo Borelli and Paolo Maggi
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
 * Modified by the gedit Team, 2007. See the AUTHORS file for a
 * list of people on the gedit Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#include <string.h>
#include <errno.h>

#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include "gedit-style-scheme-manager.h"
#include "gedit-prefs-manager.h"
#include "gedit-dirs.h"

static GtkSourceStyleSchemeManager *style_scheme_manager = NULL;

static gchar *
get_gedit_styles_path (void)
{
	gchar *config_dir;
	gchar *dir = NULL;

	config_dir = gedit_dirs_get_user_config_dir ();

	if (config_dir != NULL)
	{
		dir = g_build_filename (config_dir,
					"styles",
					NULL);
		g_free (config_dir);
	}
	
	return dir;
}

static void
add_gedit_styles_path (GtkSourceStyleSchemeManager *mgr)
{
	gchar *dir;

	dir = get_gedit_styles_path();

	if (dir != NULL)
	{
		gtk_source_style_scheme_manager_append_search_path (mgr, dir);
		g_free (dir);
	}	
}

GtkSourceStyleSchemeManager *
gedit_get_style_scheme_manager (void)
{
	if (style_scheme_manager == NULL)
	{
		style_scheme_manager = gtk_source_style_scheme_manager_new ();
		add_gedit_styles_path (style_scheme_manager);
	}

	return style_scheme_manager;
}

static gint
schemes_compare (gconstpointer a, gconstpointer b)
{
	GtkSourceStyleScheme *scheme_a = (GtkSourceStyleScheme *)a;
	GtkSourceStyleScheme *scheme_b = (GtkSourceStyleScheme *)b;

	const gchar *name_a = gtk_source_style_scheme_get_name (scheme_a);
	const gchar *name_b = gtk_source_style_scheme_get_name (scheme_b);

	return g_utf8_collate (name_a, name_b);
}

GSList *
gedit_style_scheme_manager_list_schemes_sorted (GtkSourceStyleSchemeManager *manager)
{
	const gchar * const * scheme_ids;
	GSList *schemes = NULL;

	g_return_val_if_fail (GTK_IS_SOURCE_STYLE_SCHEME_MANAGER (manager), NULL);

	scheme_ids = gtk_source_style_scheme_manager_get_scheme_ids (manager);
	
	while (*scheme_ids != NULL)
	{
		GtkSourceStyleScheme *scheme;

		scheme = gtk_source_style_scheme_manager_get_scheme (manager, 
								     *scheme_ids);

		schemes = g_slist_prepend (schemes, scheme);

		++scheme_ids;
	}

	if (schemes != NULL)
		schemes = g_slist_sort (schemes, (GCompareFunc)schemes_compare);

	return schemes;
}

gboolean
_gedit_style_scheme_manager_scheme_is_gedit_user_scheme (GtkSourceStyleSchemeManager *manager,
							 const gchar                 *scheme_id)
{
	GtkSourceStyleScheme *scheme;
	const gchar *filename;
	gchar *dir;
	gboolean res = FALSE;

	scheme = gtk_source_style_scheme_manager_get_scheme (manager, scheme_id);
	if (scheme == NULL)
		return FALSE;

	filename = gtk_source_style_scheme_get_filename (scheme);
	if (filename == NULL)
		return FALSE;

	dir = get_gedit_styles_path ();

	res = g_str_has_prefix (filename, dir);

	g_free (dir);

	return res;
}

/**
 * file_copy:
 * @name: a pointer to a %NULL-terminated string, that names
 * the file to be copied, in the GLib file name encoding
 * @dest_name: a pointer to a %NULL-terminated string, that is the
 * name for the destination file, in the GLib file name encoding
 * @error: return location for a #GError, or %NULL
 *
 * Copies file @name to @dest_name.
 *
 * If the call was successful, it returns %TRUE. If the call was not
 * successful, it returns %FALSE and sets @error. The error domain
 * is #G_FILE_ERROR. Possible error
 * codes are those in the #GFileError enumeration.
 *
 * Return value: %TRUE on success, %FALSE otherwise.
 */
static gboolean
file_copy (const gchar  *name,
	   const gchar  *dest_name,
	   GError      **error)
{
	gchar *contents;
	gsize length;
	gchar *dest_dir;

	/* FIXME - Paolo (Aug. 13, 2007):
	 * Since the style scheme files are relatively small, we can implement
	 * file copy getting all the content of the source file in a buffer and
	 * then write the content to the destination file. In this way we
	 * can use the g_file_get_contents and g_file_set_contents and avoid to
	 * write custom code to copy the file (with sane error management).
	 * If needed we can improve this code later. */

	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (dest_name != NULL, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	/* Note: we allow to copy a file to itself since this is not a problem
	 * in our use case */

	/* Ensure the destination directory exists */
	dest_dir = g_path_get_dirname (dest_name);

	errno = 0;
	if (g_mkdir_with_parents (dest_dir, 0755) != 0)
	{
		gint save_errno = errno;
		gchar *display_filename = g_filename_display_name (dest_dir);

		g_set_error (error,
			     G_FILE_ERROR,
			     g_file_error_from_errno (save_errno),
			     _("Directory '%s' could not be created: g_mkdir_with_parents() failed: %s"),
			     display_filename,
			     g_strerror (save_errno));

		g_free (dest_dir);
		g_free (display_filename);

		return FALSE;
	}

	g_free (dest_dir);

	if (!g_file_get_contents (name, &contents, &length, error))
		return FALSE;

	if (!g_file_set_contents (dest_name, contents, length, error))
		return FALSE;

	g_free (contents);

	return TRUE;
}

/**
 * _gedit_style_scheme_manager_install_scheme:
 * @manager: a #GtkSourceStyleSchemeManager
 * @fname: the file name of the style scheme to be installed
 *
 * Install a new user scheme.
 * This function copies @fname in #GEDIT_STYLES_DIR and ask the style manager to
 * recompute the list of available style schemes. It then checks if a style
 * scheme with the right file name exists.
 *
 * If the call was succesful, it returns the id of the installed scheme
 * otherwise %NULL.
 *
 * Return value: the id of the installed scheme, %NULL otherwise.
 */
const gchar *
_gedit_style_scheme_manager_install_scheme (GtkSourceStyleSchemeManager *manager,
					    const gchar                 *fname)
{
	gchar *new_file_name = NULL;
	gchar *dirname;
	gchar *styles_dir;
	GError *error = NULL;
	gboolean copied = FALSE;

	const gchar* const *ids;

	g_return_val_if_fail (GTK_IS_SOURCE_STYLE_SCHEME_MANAGER (manager), NULL);
	g_return_val_if_fail (fname != NULL, NULL);

	dirname = g_path_get_dirname (fname);
	styles_dir = get_gedit_styles_path();

	if (strcmp (dirname, styles_dir) != 0)
	{
		gchar *basename;

		basename = g_path_get_basename (fname);
		new_file_name = g_build_filename (styles_dir, basename, NULL);
		g_free (basename);

		/* Copy the style scheme file into GEDIT_STYLES_DIR */
		if (!file_copy (fname, new_file_name, &error))
		{
			g_free (new_file_name);

			g_message ("Cannot install style scheme:\n%s",
				   error->message);

			return NULL;
		}

		copied = TRUE;
	}
	else
	{
		new_file_name = g_strdup (fname);
	}

	g_free (dirname);
	g_free (styles_dir);

	/* Reload the available style schemes */
	gtk_source_style_scheme_manager_force_rescan (manager);

	/* Check the new style scheme has been actually installed */
	ids = gtk_source_style_scheme_manager_get_scheme_ids (manager);

	while (*ids != NULL)
	{
		GtkSourceStyleScheme *scheme;
		const gchar *filename;

		scheme = gtk_source_style_scheme_manager_get_scheme (
				gedit_get_style_scheme_manager (), *ids);

		filename = gtk_source_style_scheme_get_filename (scheme);

		if (filename && (strcmp (filename, new_file_name) == 0))
		{
			/* The style scheme has been correctly installed */
			g_free (new_file_name);

			return gtk_source_style_scheme_get_id (scheme);
		}
		++ids;
	}

	/* The style scheme has not been correctly installed */
	if (copied)
		g_unlink (new_file_name);

	g_free (new_file_name);

	return NULL;
}

/**
 * _gedit_style_scheme_manager_uninstall_scheme:
 * @manager: a #GtkSourceStyleSchemeManager
 * @id: the id of the style scheme to be uninstalled
 *
 * Uninstall a user scheme.
 *
 * If the call was succesful, it returns %TRUE
 * otherwise %FALSE.
 *
 * Return value: %TRUE on success, %FALSE otherwise.
 */
gboolean
_gedit_style_scheme_manager_uninstall_scheme (GtkSourceStyleSchemeManager *manager,
					      const gchar                 *id)
{
	GtkSourceStyleScheme *scheme;
	const gchar *filename;

	g_return_val_if_fail (GTK_IS_SOURCE_STYLE_SCHEME_MANAGER (manager), FALSE);
	g_return_val_if_fail (id != NULL, FALSE);

	scheme = gtk_source_style_scheme_manager_get_scheme (manager, id);
	if (scheme == NULL)
		return FALSE;

	filename = gtk_source_style_scheme_get_filename (scheme);
	if (filename == NULL)
		return FALSE;

	if (g_unlink (filename) == -1)
		return FALSE;
		
	/* Reload the available style schemes */
	gtk_source_style_scheme_manager_force_rescan (manager);
	
	return TRUE;	
}
