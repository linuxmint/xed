/*
 * gedit-file-bookmarks-store.c - Gedit plugin providing easy file access 
 * from the sidepanel
 *
 * Copyright (C) 2006 - Jesse van den Kieboom <jesse@icecrew.nl>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gedit/gedit-utils.h>
#include <gedit/gedit-plugin.h>

#include "gedit-file-bookmarks-store.h"
#include "gedit-file-browser-utils.h"

#define GEDIT_FILE_BOOKMARKS_STORE_GET_PRIVATE(object)( \
		G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_TYPE_FILE_BOOKMARKS_STORE, \
		GeditFileBookmarksStorePrivate))

struct _GeditFileBookmarksStorePrivate 
{
	GVolumeMonitor * volume_monitor;
	GFileMonitor * bookmarks_monitor;
};

static void remove_node               (GtkTreeModel * model, 
                                       GtkTreeIter * iter);

static void on_fs_changed             (GVolumeMonitor 		*monitor,
                                       GObject 			*object,
                                       GeditFileBookmarksStore 	*model);
                                       
static void on_bookmarks_file_changed (GFileMonitor * monitor,
				       GFile * file,
				       GFile * other_file,
				       GFileMonitorEvent event_type,
				       GeditFileBookmarksStore * model);
static gboolean find_with_flags       (GtkTreeModel * model, 
                                       GtkTreeIter * iter,
                                       gpointer obj, 
                                       guint flags,
                                       guint notflags);

GEDIT_PLUGIN_DEFINE_TYPE(GeditFileBookmarksStore, gedit_file_bookmarks_store, GTK_TYPE_TREE_STORE)

static void
gedit_file_bookmarks_store_dispose (GObject * object)
{
	GeditFileBookmarksStore *obj = GEDIT_FILE_BOOKMARKS_STORE (object);

	if (obj->priv->volume_monitor != NULL) {
		g_signal_handlers_disconnect_by_func (obj->priv->volume_monitor,
						      on_fs_changed,
						      obj);

		g_object_unref (obj->priv->volume_monitor);
		obj->priv->volume_monitor = NULL;
	}

	if (obj->priv->bookmarks_monitor != NULL) {
		g_object_unref (obj->priv->bookmarks_monitor);
		obj->priv->bookmarks_monitor = NULL;
	}

	G_OBJECT_CLASS (gedit_file_bookmarks_store_parent_class)->dispose (object);
}

static void
gedit_file_bookmarks_store_finalize (GObject * object)
{
	G_OBJECT_CLASS (gedit_file_bookmarks_store_parent_class)->finalize (object);
}

static void
gedit_file_bookmarks_store_class_init (GeditFileBookmarksStoreClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = gedit_file_bookmarks_store_dispose;
	object_class->finalize = gedit_file_bookmarks_store_finalize;

	g_type_class_add_private (object_class, sizeof (GeditFileBookmarksStorePrivate));
}

static void
gedit_file_bookmarks_store_init (GeditFileBookmarksStore * obj)
{
	obj->priv = GEDIT_FILE_BOOKMARKS_STORE_GET_PRIVATE (obj);
}

/* Private */
static void
add_node (GeditFileBookmarksStore *model, 
	  GdkPixbuf 		  *pixbuf,
	  const gchar 		  *name, 
	  GObject 		  *obj, 
	  guint 		   flags, 
	  GtkTreeIter 		  *iter)
{
	GtkTreeIter newiter;

	gtk_tree_store_append (GTK_TREE_STORE (model), &newiter, NULL);

	gtk_tree_store_set (GTK_TREE_STORE (model), &newiter,
			    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_ICON, pixbuf,
			    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_NAME, name,
			    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_OBJECT, obj,
			    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_FLAGS, flags,
			    -1);

	if (iter != NULL)
		*iter = newiter;
}

static gboolean
add_file (GeditFileBookmarksStore *model, 
	  GFile 		  *file,
	  const gchar 		  *name, 
	  guint 		   flags, 
	  GtkTreeIter 		  *iter)
{
	GdkPixbuf *pixbuf = NULL;
	gboolean native;
	gchar *newname;

	native = g_file_is_native (file);

	if (native && !g_file_query_exists (file, NULL)) {
		return FALSE;
	}

	if (flags & GEDIT_FILE_BOOKMARKS_STORE_IS_HOME)
		pixbuf = gedit_file_browser_utils_pixbuf_from_theme ("user-home", GTK_ICON_SIZE_MENU);
	else if (flags & GEDIT_FILE_BOOKMARKS_STORE_IS_DESKTOP)
		pixbuf = gedit_file_browser_utils_pixbuf_from_theme ("user-desktop", GTK_ICON_SIZE_MENU);
	else if (flags & GEDIT_FILE_BOOKMARKS_STORE_IS_ROOT)
		pixbuf = gedit_file_browser_utils_pixbuf_from_theme ("drive-harddisk", GTK_ICON_SIZE_MENU);

	if (pixbuf == NULL) {
		/* getting the icon is a sync get_info call, so we just do it for local files */
		if (native) {
			pixbuf = gedit_file_browser_utils_pixbuf_from_file (file, GTK_ICON_SIZE_MENU);
		} else {
			pixbuf = gedit_file_browser_utils_pixbuf_from_theme ("folder", GTK_ICON_SIZE_MENU);
		}
	}

	if (name == NULL) {
		newname = gedit_file_browser_utils_file_basename (file);
	} else {
		newname = g_strdup (name);
	}

	add_node (model, pixbuf, newname, G_OBJECT (file), flags, iter);

	if (pixbuf)
		g_object_unref (pixbuf);

	g_free (newname);

	return TRUE;
}

static void
check_mount_separator (GeditFileBookmarksStore * model, guint flags,
			gboolean added)
{
	GtkTreeIter iter;
	gboolean found;

	found =
	    find_with_flags (GTK_TREE_MODEL (model), &iter, NULL,
			     flags |
			     GEDIT_FILE_BOOKMARKS_STORE_IS_SEPARATOR, 0);

	if (added && !found) {
		/* Add the separator */
		add_node (model, NULL, NULL, NULL,
			  flags | GEDIT_FILE_BOOKMARKS_STORE_IS_SEPARATOR,
			  NULL);
	} else if (!added && found) {
		remove_node (GTK_TREE_MODEL (model), &iter);
	}
}

static void
init_special_directories (GeditFileBookmarksStore * model)
{
	gchar const *path;
	GFile * file;

	path = g_get_home_dir ();
	if (path != NULL)
	{
		file = g_file_new_for_path (path);
		add_file (model, file, NULL, GEDIT_FILE_BOOKMARKS_STORE_IS_HOME |
			 GEDIT_FILE_BOOKMARKS_STORE_IS_SPECIAL_DIR, NULL);
		g_object_unref (file);
	}

	path = g_get_user_special_dir (G_USER_DIRECTORY_DESKTOP);
	if (path != NULL)
	{
		file = g_file_new_for_path (path);
		add_file (model, file, NULL, GEDIT_FILE_BOOKMARKS_STORE_IS_DESKTOP |
			  GEDIT_FILE_BOOKMARKS_STORE_IS_SPECIAL_DIR, NULL);
		g_object_unref (file);
	}

	path = g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS);
	if (path != NULL)
	{
		file = g_file_new_for_path (path);
		add_file (model, file, NULL, GEDIT_FILE_BOOKMARKS_STORE_IS_DOCUMENTS |
			 GEDIT_FILE_BOOKMARKS_STORE_IS_SPECIAL_DIR, NULL);
		g_object_unref (file);
	}
	
	file = g_file_new_for_uri ("file:///");
	add_file (model, file, _("File System"), GEDIT_FILE_BOOKMARKS_STORE_IS_ROOT, NULL);
	g_object_unref (file);
	
	check_mount_separator (model, GEDIT_FILE_BOOKMARKS_STORE_IS_ROOT, TRUE);
}

static void
get_fs_properties (gpointer    fs,
		   gchar     **name,
		   GdkPixbuf **pixbuf,
		   guint      *flags)
{
	GIcon *icon = NULL;

	*flags = GEDIT_FILE_BOOKMARKS_STORE_IS_FS;
	*name = NULL;
	*pixbuf = NULL;
	
	if (G_IS_DRIVE (fs))
	{
		icon = g_drive_get_icon (G_DRIVE (fs));
		*name = g_drive_get_name (G_DRIVE (fs));
		
		*flags |= GEDIT_FILE_BOOKMARKS_STORE_IS_DRIVE;
	}
	else if (G_IS_VOLUME (fs))
	{
		icon = g_volume_get_icon (G_VOLUME (fs));
		*name = g_volume_get_name (G_VOLUME (fs));
	
		*flags |= GEDIT_FILE_BOOKMARKS_STORE_IS_VOLUME;
	}
	else if (G_IS_MOUNT (fs))
	{
		icon = g_mount_get_icon (G_MOUNT (fs));
		*name = g_mount_get_name (G_MOUNT (fs));
		
		*flags |= GEDIT_FILE_BOOKMARKS_STORE_IS_MOUNT;
	}
	
	if (icon)
	{
		*pixbuf = gedit_file_browser_utils_pixbuf_from_icon (icon, GTK_ICON_SIZE_MENU);
		g_object_unref (icon);
	}
}
		   

static void
add_fs (GeditFileBookmarksStore *model,
	gpointer 		 fs,
	guint 			 flags,
	GtkTreeIter		*iter)
{
	gchar *name;
	GdkPixbuf *pixbuf;
	guint fsflags;
	
	get_fs_properties (fs, &name, &pixbuf, &fsflags);
	add_node (model, pixbuf, name, fs, flags | fsflags, iter);
	
	if (pixbuf)
		g_object_unref (pixbuf);

	g_free (name);
	check_mount_separator (model, GEDIT_FILE_BOOKMARKS_STORE_IS_FS, TRUE);
}

static void
process_volume_cb (GVolume 		   *volume,
		   GeditFileBookmarksStore *model)
{
	GMount *mount;
	guint flags = GEDIT_FILE_BOOKMARKS_STORE_NONE;
	mount = g_volume_get_mount (volume);
	
	/* CHECK: should we use the LOCAL/REMOTE thing still? */
	if (mount)
	{
		/* Show mounted volume */
		add_fs (model, mount, flags, NULL);
		g_object_unref (mount);
	}
	else if (g_volume_can_mount (volume))
	{
		/* We also show the unmounted volume here so users can
		   mount it if they want to access it */
		add_fs (model, volume, flags, NULL);
	}
}

static void
process_drive_novolumes (GeditFileBookmarksStore *model,
			 GDrive			 *drive)
{
	if (g_drive_is_media_removable (drive) &&
	   !g_drive_is_media_check_automatic (drive) &&
	    g_drive_can_poll_for_media (drive))
	{
		/* This can be the case for floppy drives or other 
		   drives where media detection fails. We show the
		   drive and poll for media when the user activates
		   it */
		add_fs (model, drive, GEDIT_FILE_BOOKMARKS_STORE_NONE, NULL);
	}
}

static void
process_drive_cb (GDrive   	          *drive,
	          GeditFileBookmarksStore *model)
{
	GList *volumes;

	volumes = g_drive_get_volumes (drive);

	if (volumes)
	{
		/* Add all volumes for the drive */
		g_list_foreach (volumes, (GFunc)process_volume_cb, model);
		g_list_free (volumes);
	}
	else
	{
		process_drive_novolumes (model, drive);
	}	
}

static void
init_drives (GeditFileBookmarksStore *model)
{
	GList *drives;
		
	drives = g_volume_monitor_get_connected_drives (model->priv->volume_monitor);

	g_list_foreach (drives, (GFunc)process_drive_cb, model);
	g_list_foreach (drives, (GFunc)g_object_unref, NULL);
	g_list_free (drives);
}

static void
process_volume_nodrive_cb (GVolume 		   *volume,
			   GeditFileBookmarksStore *model)
{
	GDrive *drive;
	
	drive = g_volume_get_drive (volume);
	
	if (drive)
	{
		g_object_unref (drive);
		return;
	}
	
	process_volume_cb (volume, model);
}

static void
init_volumes (GeditFileBookmarksStore *model)
{
	GList *volumes;
		
	volumes = g_volume_monitor_get_volumes (model->priv->volume_monitor);
	
	g_list_foreach (volumes, (GFunc)process_volume_nodrive_cb, model);
	g_list_foreach (volumes, (GFunc)g_object_unref, NULL);
	g_list_free (volumes);
}

static void
process_mount_novolume_cb (GMount 		   *mount,
			   GeditFileBookmarksStore *model)
{
	GVolume *volume;
	
	volume = g_mount_get_volume (mount);
	
	if (volume)
	{
		g_object_unref (volume);
	}
	else if (!g_mount_is_shadowed (mount))
	{
		/* Add the mount */
		add_fs (model, mount, GEDIT_FILE_BOOKMARKS_STORE_NONE, NULL);
	}
}

static void
init_mounts (GeditFileBookmarksStore *model)
{
	GList *mounts;
		
	mounts = g_volume_monitor_get_mounts (model->priv->volume_monitor);
	
	g_list_foreach (mounts, (GFunc)process_mount_novolume_cb, model);
	g_list_foreach (mounts, (GFunc)g_object_unref, NULL);
	g_list_free (mounts);
}

static void
init_fs (GeditFileBookmarksStore * model)
{
	if (model->priv->volume_monitor == NULL) {
		const gchar **ptr;
		const gchar *signals[] = {
			"drive-connected", "drive-changed", "drive-disconnected",
			"volume-added", "volume-removed", "volume-changed",
			"mount-added", "mount-removed", "mount-changed",
			NULL
		};
		
		model->priv->volume_monitor = g_volume_monitor_get ();

		/* Connect signals */		
		for (ptr = signals; *ptr; ptr++)
		{
			g_signal_connect (model->priv->volume_monitor,
					  *ptr,
					  G_CALLBACK (on_fs_changed), model);
		}
	}

	/* First go through all the connected drives */
	init_drives (model);
	
	/* Then add all volumes, not associated with a drive */
	init_volumes (model);
	
	/* Then finally add all mounts that have no volume */
	init_mounts (model);
}

static gboolean
add_bookmark (GeditFileBookmarksStore * model, 
	      gchar const * name,
	      gchar const * uri)
{
	GFile * file;
	gboolean ret;
	guint flags = GEDIT_FILE_BOOKMARKS_STORE_IS_BOOKMARK;
	GtkTreeIter iter;
	
	file = g_file_new_for_uri (uri);
	
	if (g_file_is_native (file)) {
		flags |= GEDIT_FILE_BOOKMARKS_STORE_IS_LOCAL_BOOKMARK;
	} else {
		flags |= GEDIT_FILE_BOOKMARKS_STORE_IS_REMOTE_BOOKMARK;
	}

	ret = add_file (model, file, name, flags, &iter);

	g_object_unref (file);
	
	return ret;
}

static void
init_bookmarks (GeditFileBookmarksStore * model)
{
	gchar *bookmarks;
	GError *error = NULL;
	gchar *contents;
	gchar **lines;
	gchar **line;
	gboolean added = FALSE;

	/* Read the bookmarks file */
	bookmarks = g_build_filename (g_get_home_dir (),
				      ".gtk-bookmarks",
				      NULL);

	if (g_file_get_contents (bookmarks, &contents, NULL, &error)) {
		lines = g_strsplit (contents, "\n", 0);

		for (line = lines; *line; ++line) {
			if (**line) {
				gchar *pos;
				gchar *name;

				/* CHECK: is this really utf8? */
				pos = g_utf8_strchr (*line, -1, ' ');

				if (pos != NULL) {
					*pos = '\0';
					name = pos + 1;
				} else {
					name = NULL;
				}

				/* the bookmarks file should contain valid
				 * URIs, but paranoia is good */
				if (gedit_utils_is_valid_uri (*line)) {
					added |= add_bookmark (model, name, *line);
				}
			}
		}

		g_strfreev (lines);
		g_free (contents);

		/* Add a watch */
		if (model->priv->bookmarks_monitor == NULL) {
			GFile * file;

			file = g_file_new_for_path (bookmarks);
			model->priv->bookmarks_monitor = g_file_monitor_file (file, G_FILE_MONITOR_NONE, NULL, NULL);
			g_object_unref (file);

			g_signal_connect (model->priv->bookmarks_monitor, 
					  "changed", 
					  (GCallback)on_bookmarks_file_changed, 
					  model);
		}
	} else {
		/* The bookmarks file doesn't exist (which is perfectly fine) */
		g_error_free (error);
	}

	if (added) {
		/* Bookmarks separator */
		add_node (model, NULL, NULL, NULL,
			  GEDIT_FILE_BOOKMARKS_STORE_IS_BOOKMARK |
			  GEDIT_FILE_BOOKMARKS_STORE_IS_SEPARATOR, NULL);
	}

	g_free (bookmarks);
}

static gint flags_order[] = {
	GEDIT_FILE_BOOKMARKS_STORE_IS_HOME,
	GEDIT_FILE_BOOKMARKS_STORE_IS_DESKTOP,
	GEDIT_FILE_BOOKMARKS_STORE_IS_SPECIAL_DIR,
	GEDIT_FILE_BOOKMARKS_STORE_IS_ROOT,
	GEDIT_FILE_BOOKMARKS_STORE_IS_FS,
	GEDIT_FILE_BOOKMARKS_STORE_IS_BOOKMARK,
	-1
};

static gint
utf8_casecmp (gchar const *s1, const gchar * s2)
{
	gchar *n1;
	gchar *n2;
	gint result;

	n1 = g_utf8_casefold (s1, -1);
	n2 = g_utf8_casefold (s2, -1);

	result = g_utf8_collate (n1, n2);

	g_free (n1);
	g_free (n2);

	return result;
}

static gint
bookmarks_compare_names (GtkTreeModel * model, GtkTreeIter * a,
			 GtkTreeIter * b)
{
	gchar *n1;
	gchar *n2;
	gint result;
	guint f1;
	guint f2;

	gtk_tree_model_get (model, a,
			    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_NAME, &n1,
			    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_FLAGS, &f1,
			    -1);
	gtk_tree_model_get (model, b,
			    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_NAME, &n2,
			    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_FLAGS, &f2,
			    -1);

	/* do not sort actual bookmarks to keep same order as in caja */
	if ((f1 & GEDIT_FILE_BOOKMARKS_STORE_IS_BOOKMARK) &&
	    (f2 & GEDIT_FILE_BOOKMARKS_STORE_IS_BOOKMARK))
		result = 0;
	else if (n1 == NULL && n2 == NULL)
		result = 0;
	else if (n1 == NULL)
		result = -1;
	else if (n2 == NULL)
		result = 1;
	else
		result = utf8_casecmp (n1, n2);

	g_free (n1);
	g_free (n2);

	return result;
}

static gint
bookmarks_compare_flags (GtkTreeModel * model, GtkTreeIter * a,
			 GtkTreeIter * b)
{
	guint f1;
	guint f2;
	gint *flags;
	guint sep;

	sep = GEDIT_FILE_BOOKMARKS_STORE_IS_SEPARATOR;

	gtk_tree_model_get (model, a,
			    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_FLAGS, &f1,
			    -1);
	gtk_tree_model_get (model, b,
			    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_FLAGS, &f2,
			    -1);

	for (flags = flags_order; *flags != -1; ++flags) {
		if ((f1 & *flags) != (f2 & *flags)) {
			if (f1 & *flags) {
				return -1;
			} else {
				return 1;
			}
		} else if ((f1 & *flags) && (f1 & sep) != (f2 & sep)) {
			if (f1 & sep)
				return -1;
			else
				return 1;
		}
	}

	return 0;
}

static gint
bookmarks_compare_func (GtkTreeModel * model, GtkTreeIter * a,
			GtkTreeIter * b, gpointer user_data)
{
	gint result;

	result = bookmarks_compare_flags (model, a, b);

	if (result == 0)
		result = bookmarks_compare_names (model, a, b);

	return result;
}

static gboolean
find_with_flags (GtkTreeModel * model, GtkTreeIter * iter, gpointer obj,
		 guint flags, guint notflags)
{
	GtkTreeIter child;
	guint childflags = 0;
 	GObject * childobj;
 	gboolean fequal;
	
	if (!gtk_tree_model_get_iter_first (model, &child))
		return FALSE;

	do {
		gtk_tree_model_get (model, &child,
				    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_OBJECT,
				    &childobj,
				    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_FLAGS,
				    &childflags, -1);
		
		fequal = (obj == childobj);
		
		if (childobj)
			g_object_unref (childobj);
		 
		if ((obj == NULL || fequal) &&
		    (childflags & flags) == flags
		    && !(childflags & notflags)) {
			*iter = child;
			return TRUE;
		}
	} while (gtk_tree_model_iter_next (model, &child));

	return FALSE;
}

static void
remove_node (GtkTreeModel * model, GtkTreeIter * iter)
{
	guint flags;

	gtk_tree_model_get (model, iter,
			    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_FLAGS, &flags,
			    -1);

	if (!(flags & GEDIT_FILE_BOOKMARKS_STORE_IS_SEPARATOR)) {
		if (flags & GEDIT_FILE_BOOKMARKS_STORE_IS_FS) {
			check_mount_separator (GEDIT_FILE_BOOKMARKS_STORE (model),
			     		       flags & GEDIT_FILE_BOOKMARKS_STORE_IS_FS,
					       FALSE);
		}
	}

	gtk_tree_store_remove (GTK_TREE_STORE (model), iter);
}

static void
remove_bookmarks (GeditFileBookmarksStore * model)
{
	GtkTreeIter iter;

	while (find_with_flags (GTK_TREE_MODEL (model), &iter, NULL,
				GEDIT_FILE_BOOKMARKS_STORE_IS_BOOKMARK,
				0)) {
		remove_node (GTK_TREE_MODEL (model), &iter);
	}
}

static void
initialize_fill (GeditFileBookmarksStore * model)
{
	init_special_directories (model);
	init_fs (model);
	init_bookmarks (model);
}

/* Public */
GeditFileBookmarksStore *
gedit_file_bookmarks_store_new (void)
{
	GeditFileBookmarksStore *model;
	GType column_types[] = {
		GDK_TYPE_PIXBUF,
		G_TYPE_STRING,
		G_TYPE_OBJECT,
		G_TYPE_UINT
	};

	model = g_object_new (GEDIT_TYPE_FILE_BOOKMARKS_STORE, NULL);
	gtk_tree_store_set_column_types (GTK_TREE_STORE (model),
					 GEDIT_FILE_BOOKMARKS_STORE_N_COLUMNS,
					 column_types);

	gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE (model),
						 bookmarks_compare_func,
						 NULL, NULL);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model),
					      GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
					      GTK_SORT_ASCENDING);

	initialize_fill (model);

	return model;
}

gchar *
gedit_file_bookmarks_store_get_uri (GeditFileBookmarksStore * model,
				    GtkTreeIter * iter)
{
	GObject * obj;
	GFile * file = NULL;
	guint flags;
	gchar * ret = NULL;
	gboolean isfs;
	
	g_return_val_if_fail (GEDIT_IS_FILE_BOOKMARKS_STORE (model), NULL);
	g_return_val_if_fail (iter != NULL, NULL);

	gtk_tree_model_get (GTK_TREE_MODEL (model), iter,
			    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_FLAGS,
			    &flags,
			    GEDIT_FILE_BOOKMARKS_STORE_COLUMN_OBJECT, 
			    &obj,
			    -1);

	if (obj == NULL)
		return NULL;

	isfs = (flags & GEDIT_FILE_BOOKMARKS_STORE_IS_FS);
	
	if (isfs && (flags & GEDIT_FILE_BOOKMARKS_STORE_IS_MOUNT))
	{
		file = g_mount_get_root (G_MOUNT (obj));
	}
	else if (!isfs)
	{
		file = g_object_ref (obj);
	}
	
	g_object_unref (obj);
	
	if (file)
	{
		ret = g_file_get_uri (file);
		g_object_unref (file);
	}
	
	return ret;
}

void
gedit_file_bookmarks_store_refresh (GeditFileBookmarksStore * model)
{
	gtk_tree_store_clear (GTK_TREE_STORE (model));
	initialize_fill (model);
}

static void
on_fs_changed (GVolumeMonitor 	      *monitor,
	       GObject 		      *object,
	       GeditFileBookmarksStore *model)
{
	GtkTreeModel *tree_model = GTK_TREE_MODEL (model);
	guint flags = GEDIT_FILE_BOOKMARKS_STORE_IS_FS;
	guint noflags = GEDIT_FILE_BOOKMARKS_STORE_IS_SEPARATOR;
	GtkTreeIter iter;

	/* clear all fs items */
	while (find_with_flags (tree_model, &iter, NULL, flags, noflags))
		remove_node (tree_model, &iter);
	
	/* then reinitialize */
	init_fs (model);
}

static void
on_bookmarks_file_changed (GFileMonitor * monitor,
			   GFile * file,
			   GFile * other_file,
			   GFileMonitorEvent event_type,
			   GeditFileBookmarksStore * model)
{
	switch (event_type) {
	case G_FILE_MONITOR_EVENT_CHANGED:
	case G_FILE_MONITOR_EVENT_CREATED:
		/* Re-initialize bookmarks */
		remove_bookmarks (model);
		init_bookmarks (model);
		break;
	case G_FILE_MONITOR_EVENT_DELETED: // FIXME: shouldn't we also monitor the directory?
		/* Remove bookmarks */
		remove_bookmarks (model);
		g_object_unref (monitor);
		model->priv->bookmarks_monitor = NULL;
		break;
	default:
		break;
	}
}

// ex:ts=8:noet:
