/*
 * xed-debug.c
 * This file is part of xed
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi
 * Copyright (C) 2002 - 2005 Paolo Maggi
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
 * Modified by the xed Team, 1998-2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#include <config.h>
#include <stdio.h>

#include "xed-debug.h"

#define ENABLE_PROFILING

#ifdef ENABLE_PROFILING
static GTimer *timer = NULL;
static gdouble last = 0.0;
#endif

static XedDebugSection debug = XED_NO_DEBUG;

void
xed_debug_init (void)
{
	if (g_getenv ("XED_DEBUG") != NULL)
	{
		/* enable all debugging */
		debug = ~XED_NO_DEBUG;
		goto out;
	}

	if (g_getenv ("XED_DEBUG_VIEW") != NULL)
		debug = debug | XED_DEBUG_VIEW;
	if (g_getenv ("XED_DEBUG_SEARCH") != NULL)
		debug = debug | XED_DEBUG_SEARCH;
	if (g_getenv ("XED_DEBUG_PREFS") != NULL)
		debug = debug | XED_DEBUG_PREFS;
	if (g_getenv ("XED_DEBUG_PRINT") != NULL)
		debug = debug | XED_DEBUG_PRINT;
	if (g_getenv ("XED_DEBUG_PLUGINS") != NULL)
		debug = debug | XED_DEBUG_PLUGINS;
	if (g_getenv ("XED_DEBUG_TAB") != NULL)
		debug = debug | XED_DEBUG_TAB;
	if (g_getenv ("XED_DEBUG_DOCUMENT") != NULL)
		debug = debug | XED_DEBUG_DOCUMENT;
	if (g_getenv ("XED_DEBUG_COMMANDS") != NULL)
		debug = debug | XED_DEBUG_COMMANDS;
	if (g_getenv ("XED_DEBUG_APP") != NULL)
		debug = debug | XED_DEBUG_APP;
	if (g_getenv ("XED_DEBUG_SESSION") != NULL)
		debug = debug | XED_DEBUG_SESSION;
	if (g_getenv ("XED_DEBUG_UTILS") != NULL)
		debug = debug | XED_DEBUG_UTILS;
	if (g_getenv ("XED_DEBUG_METADATA") != NULL)
		debug = debug | XED_DEBUG_METADATA;
	if (g_getenv ("XED_DEBUG_WINDOW") != NULL)
		debug = debug | XED_DEBUG_WINDOW;
	if (g_getenv ("XED_DEBUG_LOADER") != NULL)
		debug = debug | XED_DEBUG_LOADER;
	if (g_getenv ("XED_DEBUG_SAVER") != NULL)
		debug = debug | XED_DEBUG_SAVER;

out:

#ifdef ENABLE_PROFILING
	if (debug != XED_NO_DEBUG)
		timer = g_timer_new ();
#endif
	return;
}

void
xed_debug_message (XedDebugSection  section,
		     const gchar       *file,
		     gint               line,
		     const gchar       *function,
		     const gchar       *format, ...)
{
	if (G_UNLIKELY (debug & section))
	{
#ifdef ENABLE_PROFILING
		gdouble seconds;
		g_return_if_fail (timer != NULL);
#endif

		va_list args;
		gchar *msg;

		g_return_if_fail (format != NULL);

		va_start (args, format);
		msg = g_strdup_vprintf (format, args);
		va_end (args);

#ifdef ENABLE_PROFILING
		seconds = g_timer_elapsed (timer, NULL);
		g_print ("[%f (%f)] %s:%d (%s) %s\n",
			 seconds, seconds - last,  file, line, function, msg);
		last = seconds;
#else
		g_print ("%s:%d (%s) %s\n", file, line, function, msg);
#endif

		fflush (stdout);

		g_free (msg);
	}
}

void xed_debug (XedDebugSection  section,
		  const gchar       *file,
		  gint               line,
		  const gchar       *function)
{
	if (G_UNLIKELY (debug & section))
	{
#ifdef ENABLE_PROFILING
		gdouble seconds;

		g_return_if_fail (timer != NULL);

		seconds = g_timer_elapsed (timer, NULL);
		g_print ("[%f (%f)] %s:%d (%s)\n",
			 seconds, seconds - last, file, line, function);
		last = seconds;
#else
		g_print ("%s:%d (%s)\n", file, line, function);
#endif
		fflush (stdout);
	}
}
