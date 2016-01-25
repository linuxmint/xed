/*
 * xedit-file-bookmarks-store.h - Xedit plugin providing easy file access 
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

#ifndef __XEDIT_FILE_BOOKMARKS_STORE_H__
#define __XEDIT_FILE_BOOKMARKS_STORE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS
#define XEDIT_TYPE_FILE_BOOKMARKS_STORE			(xedit_file_bookmarks_store_get_type ())
#define XEDIT_FILE_BOOKMARKS_STORE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_FILE_BOOKMARKS_STORE, XeditFileBookmarksStore))
#define XEDIT_FILE_BOOKMARKS_STORE_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), XEDIT_TYPE_FILE_BOOKMARKS_STORE, XeditFileBookmarksStore const))
#define XEDIT_FILE_BOOKMARKS_STORE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), XEDIT_TYPE_FILE_BOOKMARKS_STORE, XeditFileBookmarksStoreClass))
#define XEDIT_IS_FILE_BOOKMARKS_STORE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XEDIT_TYPE_FILE_BOOKMARKS_STORE))
#define XEDIT_IS_FILE_BOOKMARKS_STORE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_FILE_BOOKMARKS_STORE))
#define XEDIT_FILE_BOOKMARKS_STORE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), XEDIT_TYPE_FILE_BOOKMARKS_STORE, XeditFileBookmarksStoreClass))

typedef struct _XeditFileBookmarksStore        XeditFileBookmarksStore;
typedef struct _XeditFileBookmarksStoreClass   XeditFileBookmarksStoreClass;
typedef struct _XeditFileBookmarksStorePrivate XeditFileBookmarksStorePrivate;

enum 
{
	XEDIT_FILE_BOOKMARKS_STORE_COLUMN_ICON = 0,
	XEDIT_FILE_BOOKMARKS_STORE_COLUMN_NAME,
	XEDIT_FILE_BOOKMARKS_STORE_COLUMN_OBJECT,
	XEDIT_FILE_BOOKMARKS_STORE_COLUMN_FLAGS,
	XEDIT_FILE_BOOKMARKS_STORE_N_COLUMNS
};

enum 
{
	XEDIT_FILE_BOOKMARKS_STORE_NONE            	= 0,
	XEDIT_FILE_BOOKMARKS_STORE_IS_SEPARATOR   	= 1 << 0,  /* Separator item */
	XEDIT_FILE_BOOKMARKS_STORE_IS_SPECIAL_DIR 	= 1 << 1,  /* Special user dir */
	XEDIT_FILE_BOOKMARKS_STORE_IS_HOME         	= 1 << 2,  /* The special Home user directory */
	XEDIT_FILE_BOOKMARKS_STORE_IS_DESKTOP      	= 1 << 3,  /* The special Desktop user directory */
	XEDIT_FILE_BOOKMARKS_STORE_IS_DOCUMENTS    	= 1 << 4,  /* The special Documents user directory */
	XEDIT_FILE_BOOKMARKS_STORE_IS_FS        	= 1 << 5,  /* A mount object */
	XEDIT_FILE_BOOKMARKS_STORE_IS_MOUNT        	= 1 << 6,  /* A mount object */
	XEDIT_FILE_BOOKMARKS_STORE_IS_VOLUME        	= 1 << 7,  /* A volume object */
	XEDIT_FILE_BOOKMARKS_STORE_IS_DRIVE        	= 1 << 8,  /* A drive object */
	XEDIT_FILE_BOOKMARKS_STORE_IS_ROOT         	= 1 << 9,  /* The root file system (file:///) */
	XEDIT_FILE_BOOKMARKS_STORE_IS_BOOKMARK     	= 1 << 10,  /* A gtk bookmark */
	XEDIT_FILE_BOOKMARKS_STORE_IS_REMOTE_BOOKMARK	= 1 << 11, /* A remote gtk bookmark */
	XEDIT_FILE_BOOKMARKS_STORE_IS_LOCAL_BOOKMARK	= 1 << 12  /* A local gtk bookmark */
};

struct _XeditFileBookmarksStore 
{
	GtkTreeStore parent;

	XeditFileBookmarksStorePrivate *priv;
};

struct _XeditFileBookmarksStoreClass 
{
	GtkTreeStoreClass parent_class;
};

GType xedit_file_bookmarks_store_get_type               (void) G_GNUC_CONST;
GType xedit_file_bookmarks_store_register_type          (GTypeModule * module);

XeditFileBookmarksStore *xedit_file_bookmarks_store_new (void);
gchar *xedit_file_bookmarks_store_get_uri               (XeditFileBookmarksStore * model,
					                 GtkTreeIter * iter);
void xedit_file_bookmarks_store_refresh                 (XeditFileBookmarksStore * model);

G_END_DECLS
#endif				/* __XEDIT_FILE_BOOKMARKS_STORE_H__ */

// ex:ts=8:noet:
