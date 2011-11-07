/*
 * gedit-file-bookmarks-store.h - Gedit plugin providing easy file access 
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

#ifndef __GEDIT_FILE_BOOKMARKS_STORE_H__
#define __GEDIT_FILE_BOOKMARKS_STORE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS
#define GEDIT_TYPE_FILE_BOOKMARKS_STORE			(gedit_file_bookmarks_store_get_type ())
#define GEDIT_FILE_BOOKMARKS_STORE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_FILE_BOOKMARKS_STORE, GeditFileBookmarksStore))
#define GEDIT_FILE_BOOKMARKS_STORE_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_FILE_BOOKMARKS_STORE, GeditFileBookmarksStore const))
#define GEDIT_FILE_BOOKMARKS_STORE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_FILE_BOOKMARKS_STORE, GeditFileBookmarksStoreClass))
#define GEDIT_IS_FILE_BOOKMARKS_STORE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_FILE_BOOKMARKS_STORE))
#define GEDIT_IS_FILE_BOOKMARKS_STORE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_FILE_BOOKMARKS_STORE))
#define GEDIT_FILE_BOOKMARKS_STORE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_TYPE_FILE_BOOKMARKS_STORE, GeditFileBookmarksStoreClass))

typedef struct _GeditFileBookmarksStore        GeditFileBookmarksStore;
typedef struct _GeditFileBookmarksStoreClass   GeditFileBookmarksStoreClass;
typedef struct _GeditFileBookmarksStorePrivate GeditFileBookmarksStorePrivate;

enum 
{
	GEDIT_FILE_BOOKMARKS_STORE_COLUMN_ICON = 0,
	GEDIT_FILE_BOOKMARKS_STORE_COLUMN_NAME,
	GEDIT_FILE_BOOKMARKS_STORE_COLUMN_OBJECT,
	GEDIT_FILE_BOOKMARKS_STORE_COLUMN_FLAGS,
	GEDIT_FILE_BOOKMARKS_STORE_N_COLUMNS
};

enum 
{
	GEDIT_FILE_BOOKMARKS_STORE_NONE            	= 0,
	GEDIT_FILE_BOOKMARKS_STORE_IS_SEPARATOR   	= 1 << 0,  /* Separator item */
	GEDIT_FILE_BOOKMARKS_STORE_IS_SPECIAL_DIR 	= 1 << 1,  /* Special user dir */
	GEDIT_FILE_BOOKMARKS_STORE_IS_HOME         	= 1 << 2,  /* The special Home user directory */
	GEDIT_FILE_BOOKMARKS_STORE_IS_DESKTOP      	= 1 << 3,  /* The special Desktop user directory */
	GEDIT_FILE_BOOKMARKS_STORE_IS_DOCUMENTS    	= 1 << 4,  /* The special Documents user directory */
	GEDIT_FILE_BOOKMARKS_STORE_IS_FS        	= 1 << 5,  /* A mount object */
	GEDIT_FILE_BOOKMARKS_STORE_IS_MOUNT        	= 1 << 6,  /* A mount object */
	GEDIT_FILE_BOOKMARKS_STORE_IS_VOLUME        	= 1 << 7,  /* A volume object */
	GEDIT_FILE_BOOKMARKS_STORE_IS_DRIVE        	= 1 << 8,  /* A drive object */
	GEDIT_FILE_BOOKMARKS_STORE_IS_ROOT         	= 1 << 9,  /* The root file system (file:///) */
	GEDIT_FILE_BOOKMARKS_STORE_IS_BOOKMARK     	= 1 << 10,  /* A gtk bookmark */
	GEDIT_FILE_BOOKMARKS_STORE_IS_REMOTE_BOOKMARK	= 1 << 11, /* A remote gtk bookmark */
	GEDIT_FILE_BOOKMARKS_STORE_IS_LOCAL_BOOKMARK	= 1 << 12  /* A local gtk bookmark */
};

struct _GeditFileBookmarksStore 
{
	GtkTreeStore parent;

	GeditFileBookmarksStorePrivate *priv;
};

struct _GeditFileBookmarksStoreClass 
{
	GtkTreeStoreClass parent_class;
};

GType gedit_file_bookmarks_store_get_type               (void) G_GNUC_CONST;
GType gedit_file_bookmarks_store_register_type          (GTypeModule * module);

GeditFileBookmarksStore *gedit_file_bookmarks_store_new (void);
gchar *gedit_file_bookmarks_store_get_uri               (GeditFileBookmarksStore * model,
					                 GtkTreeIter * iter);
void gedit_file_bookmarks_store_refresh                 (GeditFileBookmarksStore * model);

G_END_DECLS
#endif				/* __GEDIT_FILE_BOOKMARKS_STORE_H__ */

// ex:ts=8:noet:
