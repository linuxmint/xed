/*
 * xedit-file-browser-error.h - Xedit plugin providing easy file access 
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

#ifndef __XEDIT_FILE_BROWSER_ERROR_H__
#define __XEDIT_FILE_BROWSER_ERROR_H__

G_BEGIN_DECLS

typedef enum {
	XEDIT_FILE_BROWSER_ERROR_NONE,
	XEDIT_FILE_BROWSER_ERROR_RENAME,
	XEDIT_FILE_BROWSER_ERROR_DELETE,
	XEDIT_FILE_BROWSER_ERROR_NEW_FILE,
	XEDIT_FILE_BROWSER_ERROR_NEW_DIRECTORY,
	XEDIT_FILE_BROWSER_ERROR_OPEN_DIRECTORY,
	XEDIT_FILE_BROWSER_ERROR_SET_ROOT,
	XEDIT_FILE_BROWSER_ERROR_LOAD_DIRECTORY,
	XEDIT_FILE_BROWSER_ERROR_NUM
} XeditFileBrowserError;

G_END_DECLS

#endif /* __XEDIT_FILE_BROWSER_ERROR_H__ */
