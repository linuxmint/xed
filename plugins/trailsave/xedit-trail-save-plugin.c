/*
 * xedit-trail-save-plugin.c
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
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xedit-trail-save-plugin.h"

XEDIT_PLUGIN_REGISTER_TYPE(XeditTrailSavePlugin, xedit_trail_save_plugin)

static void
strip_trailing_spaces (GtkTextBuffer *text_buffer)
{
	gint line_count, line_num;
	GtkTextIter line_start, line_end;
	gchar *slice;
	gchar byte;
	gint byte_index;
	gint strip_start_index, strip_end_index;
	gboolean should_strip;
	GtkTextIter strip_start, strip_end;

	g_assert (text_buffer != NULL);

	line_count = gtk_text_buffer_get_line_count (text_buffer);

	for (line_num = 0; line_num < line_count; ++line_num)
	{
		/* Get line text */
		gtk_text_buffer_get_iter_at_line (text_buffer, &line_start, line_num);

		if (line_num == line_count - 1)
		{
			gtk_text_buffer_get_end_iter (text_buffer, &line_end);
		}
		else
		{
			gtk_text_buffer_get_iter_at_line (text_buffer, &line_end, line_num + 1);
		}

		slice = gtk_text_buffer_get_slice (text_buffer, &line_start, &line_end, TRUE);

		if (slice == NULL)
		{
			continue;
		}

		/* Find indices of bytes that should be stripped */
		should_strip = FALSE;

		for (byte_index = 0; slice [byte_index] != 0; ++byte_index)
		{
			byte = slice [byte_index];

			if ((byte == ' ') || (byte == '\t'))
			{
				if (!should_strip)
				{
					strip_start_index = byte_index;
					should_strip = TRUE;
				}

				strip_end_index = byte_index + 1;
			}
			else if ((byte == '\r') || (byte == '\n'))
			{
				break;
			}
			else
			{
				should_strip = FALSE;
			}
		}

		g_free (slice);

		/* Strip trailing spaces */
		if (should_strip)
		{
			gtk_text_buffer_get_iter_at_line_index (text_buffer, &strip_start, line_num, strip_start_index);
			gtk_text_buffer_get_iter_at_line_index (text_buffer, &strip_end, line_num, strip_end_index);
			gtk_text_buffer_delete (text_buffer, &strip_start, &strip_end);
		}
	}
}

static void
on_save (XeditDocument         *document,
	 const gchar           *uri,
	 XeditEncoding         *encoding,
	 XeditDocumentSaveFlags save_flags,
	 XeditPlugin           *plugin)
{
	GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (document);

	strip_trailing_spaces (text_buffer);
}

static void
on_tab_added (XeditWindow *window,
	      XeditTab    *tab,
	      XeditPlugin *plugin)
{
	XeditDocument *document;

	document = xedit_tab_get_document (tab);
	g_signal_connect (document, "save", G_CALLBACK (on_save), plugin);
}

static void
on_tab_removed (XeditWindow *window,
		XeditTab    *tab,
		XeditPlugin *plugin)
{
	XeditDocument *document;

	document = xedit_tab_get_document (tab);
	g_signal_handlers_disconnect_by_data (document, plugin);
}

static void
impl_activate (XeditPlugin *plugin,
	       XeditWindow *window)
{
	GList *documents;
	GList *documents_iter;
	XeditDocument *document;

	xedit_debug (DEBUG_PLUGINS);

	g_signal_connect (window, "tab_added", G_CALLBACK (on_tab_added), plugin);
	g_signal_connect (window, "tab_removed", G_CALLBACK (on_tab_removed), plugin);

	documents = xedit_window_get_documents (window);

	for (documents_iter = documents;
	     documents_iter && documents_iter->data;
	     documents_iter = documents_iter->next)
	{
		document = (XeditDocument *) documents_iter->data;
		g_signal_connect (document, "save", G_CALLBACK (on_save), plugin);
	}

	g_list_free (documents);
}

static void
impl_deactivate (XeditPlugin *plugin,
		 XeditWindow *window)
{
	GList *documents;
	GList *documents_iter;
	XeditDocument *document;

	xedit_debug (DEBUG_PLUGINS);

	g_signal_handlers_disconnect_by_data (window, plugin);

	documents = xedit_window_get_documents (window);

	for (documents_iter = documents;
	     documents_iter && documents_iter->data;
	     documents_iter = documents_iter->next)
	{
		document = (XeditDocument *) documents_iter->data;
		g_signal_handlers_disconnect_by_data (document, plugin);
	}

	g_list_free (documents);
}

static void
xedit_trail_save_plugin_init (XeditTrailSavePlugin *plugin)
{
	xedit_debug_message (DEBUG_PLUGINS, "XeditTrailSavePlugin initializing");
}

static void
xedit_trail_save_plugin_finalize (GObject *object)
{
	xedit_debug_message (DEBUG_PLUGINS, "XeditTrailSavePlugin finalizing");

	G_OBJECT_CLASS (xedit_trail_save_plugin_parent_class)->finalize (object);
}

static void
xedit_trail_save_plugin_class_init (XeditTrailSavePluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	XeditPluginClass *plugin_class = XEDIT_PLUGIN_CLASS (klass);

	object_class->finalize = xedit_trail_save_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
}
