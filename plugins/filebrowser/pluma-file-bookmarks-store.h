/*
 * pluma-file-bookmarks-store.h - Pluma plugin providing easy file access 
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __PLUMA_FILE_BOOKMARKS_STORE_H__
#define __PLUMA_FILE_BOOKMARKS_STORE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS
#define PLUMA_TYPE_FILE_BOOKMARKS_STORE			(pluma_file_bookmarks_store_get_type ())
#define PLUMA_FILE_BOOKMARKS_STORE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_FILE_BOOKMARKS_STORE, PlumaFileBookmarksStore))
#define PLUMA_FILE_BOOKMARKS_STORE_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUMA_TYPE_FILE_BOOKMARKS_STORE, PlumaFileBookmarksStore const))
#define PLUMA_FILE_BOOKMARKS_STORE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), PLUMA_TYPE_FILE_BOOKMARKS_STORE, PlumaFileBookmarksStoreClass))
#define PLUMA_IS_FILE_BOOKMARKS_STORE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUMA_TYPE_FILE_BOOKMARKS_STORE))
#define PLUMA_IS_FILE_BOOKMARKS_STORE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_FILE_BOOKMARKS_STORE))
#define PLUMA_FILE_BOOKMARKS_STORE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), PLUMA_TYPE_FILE_BOOKMARKS_STORE, PlumaFileBookmarksStoreClass))

typedef struct _PlumaFileBookmarksStore        PlumaFileBookmarksStore;
typedef struct _PlumaFileBookmarksStoreClass   PlumaFileBookmarksStoreClass;
typedef struct _PlumaFileBookmarksStorePrivate PlumaFileBookmarksStorePrivate;

enum 
{
	PLUMA_FILE_BOOKMARKS_STORE_COLUMN_ICON = 0,
	PLUMA_FILE_BOOKMARKS_STORE_COLUMN_NAME,
	PLUMA_FILE_BOOKMARKS_STORE_COLUMN_OBJECT,
	PLUMA_FILE_BOOKMARKS_STORE_COLUMN_FLAGS,
	PLUMA_FILE_BOOKMARKS_STORE_N_COLUMNS
};

enum 
{
	PLUMA_FILE_BOOKMARKS_STORE_NONE            	= 0,
	PLUMA_FILE_BOOKMARKS_STORE_IS_SEPARATOR   	= 1 << 0,  /* Separator item */
	PLUMA_FILE_BOOKMARKS_STORE_IS_SPECIAL_DIR 	= 1 << 1,  /* Special user dir */
	PLUMA_FILE_BOOKMARKS_STORE_IS_HOME         	= 1 << 2,  /* The special Home user directory */
	PLUMA_FILE_BOOKMARKS_STORE_IS_DESKTOP      	= 1 << 3,  /* The special Desktop user directory */
	PLUMA_FILE_BOOKMARKS_STORE_IS_DOCUMENTS    	= 1 << 4,  /* The special Documents user directory */
	PLUMA_FILE_BOOKMARKS_STORE_IS_FS        	= 1 << 5,  /* A mount object */
	PLUMA_FILE_BOOKMARKS_STORE_IS_MOUNT        	= 1 << 6,  /* A mount object */
	PLUMA_FILE_BOOKMARKS_STORE_IS_VOLUME        	= 1 << 7,  /* A volume object */
	PLUMA_FILE_BOOKMARKS_STORE_IS_DRIVE        	= 1 << 8,  /* A drive object */
	PLUMA_FILE_BOOKMARKS_STORE_IS_ROOT         	= 1 << 9,  /* The root file system (file:///) */
	PLUMA_FILE_BOOKMARKS_STORE_IS_BOOKMARK     	= 1 << 10,  /* A gtk bookmark */
	PLUMA_FILE_BOOKMARKS_STORE_IS_REMOTE_BOOKMARK	= 1 << 11, /* A remote gtk bookmark */
	PLUMA_FILE_BOOKMARKS_STORE_IS_LOCAL_BOOKMARK	= 1 << 12  /* A local gtk bookmark */
};

struct _PlumaFileBookmarksStore 
{
	GtkTreeStore parent;

	PlumaFileBookmarksStorePrivate *priv;
};

struct _PlumaFileBookmarksStoreClass 
{
	GtkTreeStoreClass parent_class;
};

GType pluma_file_bookmarks_store_get_type               (void) G_GNUC_CONST;
GType pluma_file_bookmarks_store_register_type          (GTypeModule * module);

PlumaFileBookmarksStore *pluma_file_bookmarks_store_new (void);
gchar *pluma_file_bookmarks_store_get_uri               (PlumaFileBookmarksStore * model,
					                 GtkTreeIter * iter);
void pluma_file_bookmarks_store_refresh                 (PlumaFileBookmarksStore * model);

G_END_DECLS
#endif				/* __PLUMA_FILE_BOOKMARKS_STORE_H__ */

// ex:ts=8:noet:
