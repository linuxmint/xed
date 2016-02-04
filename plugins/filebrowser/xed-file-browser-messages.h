/*
 * xed-file-browser-messages.h - Xed plugin providing easy file access 
 * from the sidepanel
 *
 * Copyright (C) 2008 - Jesse van den Kieboom <jesse@icecrew.nl>
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

#ifndef __XED_FILE_BROWSER_MESSAGES_H__
#define __XED_FILE_BROWSER_MESSAGES_H__

#include <xed/xed-window.h>
#include <xed/xed-message-bus.h>
#include "xed-file-browser-widget.h"

void xed_file_browser_messages_register   (XedWindow *window,
					     XedFileBrowserWidget *widget);
void xed_file_browser_messages_unregister (XedWindow *window);

#endif /* __XED_FILE_BROWSER_MESSAGES_H__ */

// ex:ts=8:noet:
