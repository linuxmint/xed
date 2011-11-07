/*
 * gedit-utils.c
 * This file is part of gedit
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2002 Chema Celorio, Paolo Maggi 
 * Copyright (C) 2003-2005 Paolo Maggi 
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
 * Modified by the gedit Team, 1998-2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>

#include "gedit-utils.h"

#include "gedit-document.h"
#include "gedit-prefs-manager.h"
#include "gedit-debug.h"

/* For the workspace/viewport stuff */
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#endif

#define STDIN_DELAY_MICROSECONDS 100000

/* Returns true if uri is a file: uri and is not a chained uri */
gboolean
gedit_utils_uri_has_file_scheme (const gchar *uri)
{
	GFile *gfile;
	gboolean res;

	gfile = g_file_new_for_uri (uri);
	res = g_file_has_uri_scheme (gfile, "file");
	
	g_object_unref (gfile);
	return res;
}

/* FIXME: we should check for chained URIs */
gboolean
gedit_utils_uri_has_writable_scheme (const gchar *uri)
{
	GFile *gfile;
	gchar *scheme;
	GSList *writable_schemes;
	gboolean res;

	gfile = g_file_new_for_uri (uri);
	scheme = g_file_get_uri_scheme (gfile);

	g_return_val_if_fail (scheme != NULL, FALSE);

	g_object_unref (gfile);

	writable_schemes = gedit_prefs_manager_get_writable_vfs_schemes ();

	/* CHECK: should we use g_ascii_strcasecmp? - Paolo (Nov 6, 2005) */
	res = (g_slist_find_custom (writable_schemes,
				    scheme,
				    (GCompareFunc)strcmp) != NULL);

	g_slist_foreach (writable_schemes, (GFunc)g_free, NULL);
	g_slist_free (writable_schemes);

	g_free (scheme);

	return res;
}

static void
widget_get_origin (GtkWidget *widget, gint *x, gint *y)

{
	GdkWindow *window;

	window = gtk_widget_get_window (widget);
	gdk_window_get_origin (window, x, y);
}

void
gedit_utils_menu_position_under_widget (GtkMenu  *menu,
					gint     *x,
					gint     *y,
					gboolean *push_in,
					gpointer  user_data)
{
	GtkWidget *widget;
	GtkRequisition requisition;

	widget = GTK_WIDGET (user_data);
	widget_get_origin (widget, x, y);

	gtk_widget_size_request (GTK_WIDGET (menu), &requisition);

	if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
	{
		*x += widget->allocation.x + widget->allocation.width - requisition.width;
	}
	else
	{
		*x += widget->allocation.x;
	}

	*y += widget->allocation.y + widget->allocation.height;

	*push_in = TRUE;
}

void
gedit_utils_menu_position_under_tree_view (GtkMenu  *menu,
					   gint     *x,
					   gint     *y,
					   gboolean *push_in,
					   gpointer  user_data)
{
	GtkTreeView *tree = GTK_TREE_VIEW (user_data);
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	
	model = gtk_tree_view_get_model (tree);
	g_return_if_fail (model != NULL);

	selection = gtk_tree_view_get_selection (tree);
	g_return_if_fail (selection != NULL);

	if (gtk_tree_selection_get_selected (selection, NULL, &iter))
	{
		GtkTreePath *path;
		GdkRectangle rect;

		widget_get_origin (GTK_WIDGET (tree), x, y);
			
		path = gtk_tree_model_get_path (model, &iter);
		gtk_tree_view_get_cell_area (tree, path,
					     gtk_tree_view_get_column (tree, 0), /* FIXME 0 for RTL ? */
					     &rect);
		gtk_tree_path_free (path);
		
		*x += rect.x;
		*y += rect.y + rect.height;
		
		if (gtk_widget_get_direction (GTK_WIDGET (tree)) == GTK_TEXT_DIR_RTL)
		{
			GtkRequisition requisition;
			gtk_widget_size_request (GTK_WIDGET (menu), &requisition);
			*x += rect.width - requisition.width;
		}
	}
	else
	{
		/* no selection -> regular "under widget" positioning */
		gedit_utils_menu_position_under_widget (menu,
							x, y, push_in,
							tree);
	}
}

/* FIXME: remove this with gtk 2.12, it has gdk_color_to_string */
gchar * 
gedit_gdk_color_to_string (GdkColor color)
{
	return g_strdup_printf ("#%04x%04x%04x",
				color.red, 
				color.green,
				color.blue);
}

GtkWidget *
gedit_gtk_button_new_with_stock_icon (const gchar *label,
				      const gchar *stock_id)
{
	GtkWidget *button;

	button = gtk_button_new_with_mnemonic (label);
	gtk_button_set_image (GTK_BUTTON (button),
			      gtk_image_new_from_stock (stock_id,
							GTK_ICON_SIZE_BUTTON));

        return button;
}

GtkWidget *
gedit_dialog_add_button (GtkDialog   *dialog,
			 const gchar *text,
			 const gchar *stock_id,
			 gint         response_id)
{
	GtkWidget *button;

	g_return_val_if_fail (GTK_IS_DIALOG (dialog), NULL);
	g_return_val_if_fail (text != NULL, NULL);
	g_return_val_if_fail (stock_id != NULL, NULL);

	button = gedit_gtk_button_new_with_stock_icon (text, stock_id);
	g_return_val_if_fail (button != NULL, NULL);

	GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

	gtk_widget_show (button);

	gtk_dialog_add_action_widget (dialog, button, response_id);	

	return button;
}

/*
 * n: len of the string in bytes
 */
gboolean 
g_utf8_caselessnmatch (const char *s1, const char *s2, gssize n1, gssize n2)
{
	gchar *casefold;
	gchar *normalized_s1;
      	gchar *normalized_s2;
	gint len_s1;
	gint len_s2;
	gboolean ret = FALSE;

	g_return_val_if_fail (s1 != NULL, FALSE);
	g_return_val_if_fail (s2 != NULL, FALSE);
	g_return_val_if_fail (n1 > 0, FALSE);
	g_return_val_if_fail (n2 > 0, FALSE);

	casefold = g_utf8_casefold (s1, n1);
	normalized_s1 = g_utf8_normalize (casefold, -1, G_NORMALIZE_NFD);
	g_free (casefold);

	casefold = g_utf8_casefold (s2, n2);
	normalized_s2 = g_utf8_normalize (casefold, -1, G_NORMALIZE_NFD);
	g_free (casefold);

	len_s1 = strlen (normalized_s1);
	len_s2 = strlen (normalized_s2);

	if (len_s1 < len_s2)
		goto finally_2;

	ret = (strncmp (normalized_s1, normalized_s2, len_s2) == 0);
	
finally_2:
	g_free (normalized_s1);
	g_free (normalized_s2);	

	return ret;
}

/**
 * gedit_utils_set_atk_name_description
 * @widget : The Gtk widget for which name/description to be set
 * @name : Atk name string
 * @description : Atk description string
 * Description : This function sets up name and description
 * for a specified gtk widget.
 */
void
gedit_utils_set_atk_name_description (GtkWidget *widget, 
				      const gchar *name,
				      const gchar *description)
{
	AtkObject *aobj;

	aobj = gtk_widget_get_accessible (widget);

	if (!(GTK_IS_ACCESSIBLE (aobj)))
		return;

	if(name)
		atk_object_set_name (aobj, name);

	if(description)
		atk_object_set_description (aobj, description);
}

/**
 * gedit_set_atk__relation
 * @obj1,@obj2 : specified widgets.
 * @rel_type : the type of relation to set up.
 * Description : This function establishes atk relation
 * between 2 specified widgets.
 */
void
gedit_utils_set_atk_relation (GtkWidget *obj1, 
			      GtkWidget *obj2, 
			      AtkRelationType rel_type )
{
	AtkObject *atk_obj1, *atk_obj2;
	AtkRelationSet *relation_set;
	AtkObject *targets[1];
	AtkRelation *relation;

	atk_obj1 = gtk_widget_get_accessible (obj1);
	atk_obj2 = gtk_widget_get_accessible (obj2);

	if (!(GTK_IS_ACCESSIBLE (atk_obj1)) || !(GTK_IS_ACCESSIBLE (atk_obj2)))
		return;

	relation_set = atk_object_ref_relation_set (atk_obj1);
	targets[0] = atk_obj2;

	relation = atk_relation_new (targets, 1, rel_type);
	atk_relation_set_add (relation_set, relation);

	g_object_unref (G_OBJECT (relation));
}

gboolean
gedit_utils_uri_exists (const gchar* text_uri)
{
	GFile *gfile;
	gboolean res;
		
	g_return_val_if_fail (text_uri != NULL, FALSE);
	
	gedit_debug_message (DEBUG_UTILS, "text_uri: %s", text_uri);

	gfile = g_file_new_for_uri (text_uri);
	res = g_file_query_exists (gfile, NULL);

	g_object_unref (gfile);

	gedit_debug_message (DEBUG_UTILS, res ? "TRUE" : "FALSE");

	return res;
}

gchar *
gedit_utils_escape_search_text (const gchar* text)
{
	GString *str;
	gint length;
	const gchar *p;
 	const gchar *end;

	if (text == NULL)
		return NULL;

	gedit_debug_message (DEBUG_SEARCH, "Text: %s", text);

    	length = strlen (text);

	/* no escape when typing.
	 * The short circuit works only for ascii, but we only
	 * care about not escaping a single '\' */
	if (length == 1)
		return g_strdup (text);

	str = g_string_new ("");

	p = text;
  	end = text + length;

  	while (p != end)
    	{
      		const gchar *next;
      		next = g_utf8_next_char (p);

		switch (*p)
        	{
       			case '\n':
          			g_string_append (str, "\\n");
          			break;
			case '\r':
          			g_string_append (str, "\\r");
          			break;
			case '\t':
          			g_string_append (str, "\\t");
          			break;
			case '\\':
          			g_string_append (str, "\\\\");
          			break;
        		default:
          			g_string_append_len (str, p, next - p);
          			break;
        	}

      		p = next;
    	}

	return g_string_free (str, FALSE);
}

gchar *
gedit_utils_unescape_search_text (const gchar *text)
{
	GString *str;
	gint length;
	gboolean drop_prev = FALSE;
	const gchar *cur;
	const gchar *end;
	const gchar *prev;
	
	if (text == NULL)
		return NULL;

	length = strlen (text);

	str = g_string_new ("");

	cur = text;
	end = text + length;
	prev = NULL;
	
	while (cur != end) 
	{
		const gchar *next;
		next = g_utf8_next_char (cur);

		if (prev && (*prev == '\\')) 
		{
			switch (*cur) 
			{
				case 'n':
					str = g_string_append (str, "\n");
				break;
				case 'r':
					str = g_string_append (str, "\r");
				break;
				case 't':
					str = g_string_append (str, "\t");
				break;
				case '\\':
					str = g_string_append (str, "\\");
					drop_prev = TRUE;
				break;
				default:
					str = g_string_append (str, "\\");
					str = g_string_append_len (str, cur, next - cur);
				break;
			}
		} 
		else if (*cur != '\\') 
		{
			str = g_string_append_len (str, cur, next - cur);
		} 
		else if ((next == end) && (*cur == '\\')) 
		{
			str = g_string_append (str, "\\");
		}
		
		if (!drop_prev)
		{
			prev = cur;
		}
		else 
		{
			prev = NULL;
			drop_prev = FALSE;
		}

		cur = next;
	}

	return g_string_free (str, FALSE);
}

void 
gedit_warning (GtkWindow *parent, const gchar *format, ...)
{
	va_list         args;
	gchar          *str;
	GtkWidget      *dialog;
	GtkWindowGroup *wg = NULL;
	
	g_return_if_fail (format != NULL);

	if (parent != NULL)
		wg = gtk_window_get_group (parent);
		
	va_start (args, format);
	str = g_strdup_vprintf (format, args);
	va_end (args);

	dialog = gtk_message_dialog_new_with_markup (
			parent,
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		   	GTK_MESSAGE_ERROR,
		   	GTK_BUTTONS_OK,
			"%s", str);

	g_free (str);

	if (wg != NULL)
		gtk_window_group_add_window (wg, GTK_WINDOW (dialog));
		
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

	g_signal_connect (G_OBJECT (dialog),
			  "response",
			  G_CALLBACK (gtk_widget_destroy),
			  NULL);
			  
	gtk_widget_show (dialog);
}

/*
 * Doubles underscore to avoid spurious menu accels.
 */
gchar * 
gedit_utils_escape_underscores (const gchar* text,
				gssize       length)
{
	GString *str;
	const gchar *p;
	const gchar *end;

	g_return_val_if_fail (text != NULL, NULL);

	if (length < 0)
		length = strlen (text);

	str = g_string_sized_new (length);

	p = text;
	end = text + length;

	while (p != end)
	{
		const gchar *next;
		next = g_utf8_next_char (p);

		switch (*p)
		{
			case '_':
				g_string_append (str, "__");
				break;
			default:
				g_string_append_len (str, p, next - p);
				break;
		}

		p = next;
	}

	return g_string_free (str, FALSE);
}

/* the following functions are taken from eel */

static gchar *
gedit_utils_str_truncate (const gchar *string,
			  guint        truncate_length,
			  gboolean middle)
{
	GString     *truncated;
	guint        length;
	guint        n_chars;
	guint        num_left_chars;
	guint        right_offset;
	guint        delimiter_length;
	const gchar *delimiter = "\342\200\246";

	g_return_val_if_fail (string != NULL, NULL);

	length = strlen (string);

	g_return_val_if_fail (g_utf8_validate (string, length, NULL), NULL);

	/* It doesnt make sense to truncate strings to less than
	 * the size of the delimiter plus 2 characters (one on each
	 * side)
	 */
	delimiter_length = g_utf8_strlen (delimiter, -1);
	if (truncate_length < (delimiter_length + 2)) {
		return g_strdup (string);
	}

	n_chars = g_utf8_strlen (string, length);

	/* Make sure the string is not already small enough. */
	if (n_chars <= truncate_length) {
		return g_strdup (string);
	}

	/* Find the 'middle' where the truncation will occur. */
	if (middle)
	{
		num_left_chars = (truncate_length - delimiter_length) / 2;
		right_offset = n_chars - truncate_length + num_left_chars + delimiter_length;

		truncated = g_string_new_len (string,
					      g_utf8_offset_to_pointer (string, num_left_chars) - string);
		g_string_append (truncated, delimiter);
		g_string_append (truncated, g_utf8_offset_to_pointer (string, right_offset));
	}
	else
	{
		num_left_chars = truncate_length - delimiter_length;
		truncated = g_string_new_len (string,
					      g_utf8_offset_to_pointer (string, num_left_chars) - string);
		g_string_append (truncated, delimiter);
	}
	
	return g_string_free (truncated, FALSE);
}

gchar *
gedit_utils_str_middle_truncate (const gchar *string,
				 guint        truncate_length)
{
	return gedit_utils_str_truncate (string, truncate_length, TRUE);
}

gchar *
gedit_utils_str_end_truncate (const gchar *string,
			      guint        truncate_length)
{
	return gedit_utils_str_truncate (string, truncate_length, FALSE);
}

gchar *
gedit_utils_make_valid_utf8 (const char *name)
{
	GString *string;
	const char *remainder, *invalid;
	int remaining_bytes, valid_bytes;

	g_return_val_if_fail (name != NULL, NULL);

	string = NULL;
	remainder = name;
	remaining_bytes = strlen (name);

	while (remaining_bytes != 0) {
		if (g_utf8_validate (remainder, remaining_bytes, &invalid)) {
			break;
		}
		valid_bytes = invalid - remainder;

		if (string == NULL) {
			string = g_string_sized_new (remaining_bytes);
		}
		g_string_append_len (string, remainder, valid_bytes);
		/* append U+FFFD REPLACEMENT CHARACTER */
		g_string_append (string, "\357\277\275");

		remaining_bytes -= valid_bytes + 1;
		remainder = invalid + 1;
	}

	if (string == NULL) {
		return g_strdup (name);
	}

	g_string_append (string, remainder);
	
	g_assert (g_utf8_validate (string->str, -1, NULL));

	return g_string_free (string, FALSE);
}

/* Note that this function replace home dir with ~ */
gchar *
gedit_utils_uri_get_dirname (const gchar *uri)
{
	gchar *res;
	gchar *str;

	g_return_val_if_fail (uri != NULL, NULL);

	/* CHECK: does it work with uri chaining? - Paolo */
	str = g_path_get_dirname (uri);
	g_return_val_if_fail (str != NULL, g_strdup ("."));

	if ((strlen (str) == 1) && (*str == '.'))
	{
		g_free (str);
		
		return NULL;
	}

	res = gedit_utils_replace_home_dir_with_tilde (str);

	g_free (str);
	
	return res;
}

/**
 * gedit_utils_location_get_dirname_for_display
 * @file: the location
 *
 * Returns a string suitable to be displayed in the UI indicating
 * the name of the directory where the file is located.
 * For remote files it may also contain the hostname etc.
 * For local files it tries to replace the home dir with ~.
 *
 * Returns: a string to display the dirname
 */
gchar *
gedit_utils_location_get_dirname_for_display (GFile *location)
{
	gchar *uri;
	gchar *res;
	GMount *mount;

	g_return_val_if_fail (location != NULL, NULL);

	/* we use the parse name, that is either the local path
	 * or an uri but which is utf8 safe */
	uri = g_file_get_parse_name (location);

	/* FIXME: this is sync... is it a problem? */
	mount = g_file_find_enclosing_mount (location, NULL, NULL);
	if (mount != NULL)
	{
		gchar *mount_name;
		gchar *path;

		mount_name = g_mount_get_name (mount);
		g_object_unref (mount);

		/* obtain the "path" patrt of the uri */
		if (gedit_utils_decode_uri (uri,
					    NULL, NULL,
					    NULL, NULL,
					    &path))
		{
			gchar *dirname;
			
			dirname = gedit_utils_uri_get_dirname (path);
			res = g_strdup_printf ("%s %s", mount_name, dirname);

			g_free (path);
			g_free (dirname);
			g_free (mount_name);
		}
		else
		{
			res = mount_name;
		}
	}
	else
	{
		/* fallback for local files or uris without mounts */
		res = gedit_utils_uri_get_dirname (uri);
	}

	g_free (uri);

	return res;
}

gchar *
gedit_utils_replace_home_dir_with_tilde (const gchar *uri)
{
	gchar *tmp;
	gchar *home;

	g_return_val_if_fail (uri != NULL, NULL);

	/* Note that g_get_home_dir returns a const string */
	tmp = (gchar *)g_get_home_dir ();

	if (tmp == NULL)
		return g_strdup (uri);

	home = g_filename_to_utf8 (tmp, -1, NULL, NULL, NULL);
	if (home == NULL)
		return g_strdup (uri);

	if (strcmp (uri, home) == 0)
	{
		g_free (home);
		
		return g_strdup ("~");
	}

	tmp = home;
	home = g_strdup_printf ("%s/", tmp);
	g_free (tmp);

	if (g_str_has_prefix (uri, home))
	{
		gchar *res;

		res = g_strdup_printf ("~/%s", uri + strlen (home));

		g_free (home);
		
		return res;		
	}

	g_free (home);

	return g_strdup (uri);
}

/* the following two functions are courtesy of galeon */

/**
 * gedit_utils_get_current_workspace: Get the current workspace
 *
 * Get the currently visible workspace for the #GdkScreen.
 *
 * If the X11 window property isn't found, 0 (the first workspace)
 * is returned.
 */
guint
gedit_utils_get_current_workspace (GdkScreen *screen)
{
#ifdef GDK_WINDOWING_X11
	GdkWindow *root_win;
	GdkDisplay *display;
	Atom type;
	gint format;
	gulong nitems;
	gulong bytes_after;
	guint *current_desktop;
	gint err, result;
	guint ret = 0;

	g_return_val_if_fail (GDK_IS_SCREEN (screen), 0);

	root_win = gdk_screen_get_root_window (screen);
	display = gdk_screen_get_display (screen);

	gdk_error_trap_push ();
	result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (display), GDK_WINDOW_XID (root_win),
				     gdk_x11_get_xatom_by_name_for_display (display, "_NET_CURRENT_DESKTOP"),
				     0, G_MAXLONG, False, XA_CARDINAL, &type, &format, &nitems,
				     &bytes_after, (gpointer) &current_desktop);
	err = gdk_error_trap_pop ();

	if (err != Success || result != Success)
		return ret;

	if (type == XA_CARDINAL && format == 32 && nitems > 0)
		ret = current_desktop[0];

	XFree (current_desktop);
	return ret;
#else
	/* FIXME: on mac etc proably there are native APIs
	 * to get the current workspace etc */
	return 0;
#endif
}

/**
 * gedit_utils_get_window_workspace: Get the workspace the window is on
 *
 * This function gets the workspace that the #GtkWindow is visible on,
 * it returns GEDIT_ALL_WORKSPACES if the window is sticky, or if
 * the window manager doesn support this function
 */
guint
gedit_utils_get_window_workspace (GtkWindow *gtkwindow)
{
#ifdef GDK_WINDOWING_X11
	GdkWindow *window;
	GdkDisplay *display;
	Atom type;
	gint format;
	gulong nitems;
	gulong bytes_after;
	guint *workspace;
	gint err, result;
	guint ret = GEDIT_ALL_WORKSPACES;

	g_return_val_if_fail (GTK_IS_WINDOW (gtkwindow), 0);
	g_return_val_if_fail (GTK_WIDGET_REALIZED (GTK_WIDGET (gtkwindow)), 0);

	window = gtk_widget_get_window (GTK_WIDGET (gtkwindow));
	display = gdk_drawable_get_display (window);

	gdk_error_trap_push ();
	result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (display), GDK_WINDOW_XID (window),
				     gdk_x11_get_xatom_by_name_for_display (display, "_NET_WM_DESKTOP"),
				     0, G_MAXLONG, False, XA_CARDINAL, &type, &format, &nitems,
				     &bytes_after, (gpointer) &workspace);
	err = gdk_error_trap_pop ();

	if (err != Success || result != Success)
		return ret;

	if (type == XA_CARDINAL && format == 32 && nitems > 0)
		ret = workspace[0];

	XFree (workspace);
	return ret;
#else
	/* FIXME: on mac etc proably there are native APIs
	 * to get the current workspace etc */
	return 0;
#endif
}

/**
 * gedit_utils_get_current_viewport: Get the current viewport origin
 *
 * Get the currently visible viewport origin for the #GdkScreen.
 *
 * If the X11 window property isn't found, (0, 0) is returned.
 */
void
gedit_utils_get_current_viewport (GdkScreen    *screen,
				  gint         *x,
				  gint         *y)
{
#ifdef GDK_WINDOWING_X11
	GdkWindow *root_win;
	GdkDisplay *display;
	Atom type;
	gint format;
	gulong nitems;
	gulong bytes_after;
	gulong *coordinates;
	gint err, result;

	g_return_if_fail (GDK_IS_SCREEN (screen));
	g_return_if_fail (x != NULL && y != NULL);

	/* Default values for the viewport origin */
	*x = 0;
	*y = 0;

	root_win = gdk_screen_get_root_window (screen);
	display = gdk_screen_get_display (screen);

	gdk_error_trap_push ();
	result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (display), GDK_WINDOW_XID (root_win),
				     gdk_x11_get_xatom_by_name_for_display (display, "_NET_DESKTOP_VIEWPORT"),
				     0, G_MAXLONG, False, XA_CARDINAL, &type, &format, &nitems,
				     &bytes_after, (void*) &coordinates);
	err = gdk_error_trap_pop ();

	if (err != Success || result != Success)
		return;

	if (type != XA_CARDINAL || format != 32 || nitems < 2)
	{
		XFree (coordinates);
		return;
	}

	*x = coordinates[0];
	*y = coordinates[1];
	XFree (coordinates);
#else
	/* FIXME: on mac etc proably there are native APIs
	 * to get the current workspace etc */
	*x = 0;
	*y = 0;
#endif
}

void
gedit_utils_activate_url (GtkAboutDialog *about,
			  const gchar    *url,
			  gpointer        data)
{
	gtk_show_uri (gtk_widget_get_screen (GTK_WIDGET (about)), url, GDK_CURRENT_TIME, NULL);
}

static gboolean
is_valid_scheme_character (gchar c)
{
	return g_ascii_isalnum (c) || c == '+' || c == '-' || c == '.';
}

static gboolean
has_valid_scheme (const gchar *uri)
{
	const gchar *p;

	p = uri;

	if (!is_valid_scheme_character (*p)) {
		return FALSE;
	}

	do {
		p++;
	} while (is_valid_scheme_character (*p));

	return *p == ':';
}

gboolean
gedit_utils_is_valid_uri (const gchar *uri)
{
	const guchar *p;

	if (uri == NULL)
		return FALSE;

	if (!has_valid_scheme (uri))
		return FALSE;

	/* We expect to have a fully valid set of characters */
	for (p = (const guchar *)uri; *p; p++) {
		if (*p == '%')
		{
			++p;
			if (!g_ascii_isxdigit (*p))
				return FALSE;

			++p;		
			if (!g_ascii_isxdigit (*p))
				return FALSE;
		}
		else
		{
			if (*p <= 32 || *p >= 128)
				return FALSE;
		}
	}

	return TRUE;
}

static GtkWidget *
handle_builder_error (const gchar *message, ...)
{
	GtkWidget *label;
	gchar *msg;
	gchar *msg_plain;
	va_list args;

	va_start (args, message);
	msg_plain = g_strdup_vprintf (message, args);
	va_end (args);

	label = gtk_label_new (NULL);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);

	msg = g_strconcat ("<span size=\"large\" weight=\"bold\">",
			   msg_plain, "</span>\n\n",
			   _("Please check your installation."),
			   NULL);

	gtk_label_set_markup (GTK_LABEL (label), msg);

	g_free (msg_plain);
	g_free (msg);

	gtk_misc_set_padding (GTK_MISC (label), 5, 5);

	return label;
}

/**
 * gedit_utils_get_ui_objects:
 * @filename: the path to the gtk builder file
 * @root_objects: a NULL terminated list of root objects to load or NULL to
 *                load all objects
 * @error_widget: a pointer were a #GtkLabel
 * @object_name: the name of the first object
 * @...: a pointer were the first object is returned, followed by more
 *       name / object pairs and terminated by NULL.
 *
 * This function gets the requested objects from a GtkBuilder ui file. In case
 * of error it returns FALSE and sets error_widget to a GtkLabel containing
 * the error message to display.
 *
 * Returns FALSE if an error occurs, TRUE on success.
 */
gboolean
gedit_utils_get_ui_objects (const gchar  *filename,
			    gchar       **root_objects,
			    GtkWidget   **error_widget,
			    const gchar  *object_name,
			    ...)
{

	GtkBuilder *builder;
	va_list args;
	const gchar *name;
	GError *error = NULL;
	gchar *filename_markup;
	gboolean ret = TRUE;

	g_return_val_if_fail (filename != NULL, FALSE);
	g_return_val_if_fail (error_widget != NULL, FALSE);
	g_return_val_if_fail (object_name != NULL, FALSE);

	filename_markup = g_markup_printf_escaped ("<i>%s</i>", filename);
	*error_widget = NULL;

	builder = gtk_builder_new ();
	
	if (root_objects != NULL)
	{
		gtk_builder_add_objects_from_file (builder, 
						   filename, 
						   root_objects, 
						   &error);
	}
	else
	{
		gtk_builder_add_from_file (builder,
					   filename,
					   &error);
	}

	if (error != NULL)
	{
		*error_widget = handle_builder_error (_("Unable to open UI file %s. Error: %s"),
						      filename_markup,
						      error->message);
		g_error_free (error);
		g_free (filename_markup);
		g_object_unref (builder);

		return FALSE;
	}

	va_start (args, object_name);
	for (name = object_name; name; name = va_arg (args, const gchar *) )
	{
		GObject **gobj;

		gobj = va_arg (args, GObject **);
		*gobj = gtk_builder_get_object (builder, name);

		if (!*gobj)
		{
			*error_widget = handle_builder_error (_("Unable to find the object '%s' inside file %s."), 
							      name, 
							      filename_markup),
			ret = FALSE;
			break;
		}

		/* we return a new ref for the root objects,
		 * the others are already reffed by their parent root object */
		if (root_objects != NULL)
		{
			gint i;

			for (i = 0; root_objects[i] != NULL; ++i)
			{
				if ((strcmp (name, root_objects[i]) == 0))
				{
					g_object_ref (*gobj);
				}
			}
		}
	}
	va_end (args);

	g_free (filename_markup);
	g_object_unref (builder);

	return ret;
}

gchar *
gedit_utils_make_canonical_uri_from_shell_arg (const gchar *str)
{	
	GFile *gfile;
	gchar *uri;

	g_return_val_if_fail (str != NULL, NULL);
	g_return_val_if_fail (*str != '\0', NULL);
	
	/* Note for the future: 
	 * FIXME: is still still relevant?
	 *
	 * <federico> paolo: and flame whoever tells 
	 * you that file:///mate/test_files/hëllò 
	 * doesn't work --- that's not a valid URI
	 *
	 * <paolo> federico: well, another solution that 
	 * does not requires patch to _from_shell_args 
	 * is to check that the string returned by it 
	 * contains only ASCII chars
	 * <federico> paolo: hmmmm, isn't there 
	 * mate_vfs_is_uri_valid() or something?
	 * <paolo>: I will use gedit_utils_is_valid_uri ()
	 *
	 */

	gfile = g_file_new_for_commandline_arg (str);
	uri = g_file_get_uri (gfile);
	g_object_unref (gfile);

	if (gedit_utils_is_valid_uri (uri))
		return uri;
	
	g_free (uri);
	return NULL;
}

/**
 * gedit_utils_file_has_parent:
 * @gfile: the GFile to check the parent for
 *
 * Return TRUE if the specified gfile has a parent (is not the root), FALSE
 * otherwise
 */
gboolean
gedit_utils_file_has_parent (GFile *gfile)
{
	GFile *parent;
	gboolean ret;
	
	parent = g_file_get_parent (gfile);
	ret = parent != NULL;
	
	if (parent)
		g_object_unref (parent);
	
	return ret;
}

/**
 * gedit_utils_basename_for_display:
 * @uri: uri for which the basename should be displayed
 *
 * Return the basename of a file suitable for display to users.
 */
gchar *
gedit_utils_basename_for_display (gchar const *uri)
{
	gchar *name;
	GFile *gfile;
	gchar *hn;

	g_return_val_if_fail (uri != NULL, NULL);

	gfile = g_file_new_for_uri (uri);

	/* First, try to query the display name, but only on local files */
	if (g_file_has_uri_scheme (gfile, "file"))
	{
		GFileInfo *info;
		info = g_file_query_info (gfile,
					  G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME, 
					  G_FILE_QUERY_INFO_NONE, 
					  NULL, 
					  NULL);

		if (info)
		{
			/* Simply get the display name to use as the basename */
			name = g_strdup (g_file_info_get_display_name (info));
			g_object_unref (info);
		}
		else
		{
			/* This is a local file, and therefore we will use
			 * g_filename_display_basename on the local path */
			gchar *local_path;
		
			local_path = g_file_get_path (gfile);
			name = g_filename_display_basename (local_path);
			g_free (local_path);
		}
	}
	else if (gedit_utils_file_has_parent (gfile) || !gedit_utils_decode_uri (uri, NULL, NULL, &hn, NULL, NULL))
	{
		/* For remote files with a parent (so not just http://foo.com)
		   or remote file for which the decoding of the host name fails,
		   use the _parse_name and take basename of that */
		gchar *parse_name;
		gchar *base;

		parse_name = g_file_get_parse_name (gfile);
		base = g_filename_display_basename (parse_name);
		name = g_uri_unescape_string (base, NULL);
		
		g_free (base);		
		g_free (parse_name);
	}
	else
	{
		/* display '/ on <host>' using the decoded host */
		gchar *hn_utf8;

		if  (hn != NULL)
			hn_utf8 = gedit_utils_make_valid_utf8 (hn);
		else
			/* we should never get here */
			hn_utf8 = g_strdup ("?");

		/* Translators: '/ on <remote-share>' */
		name = g_strdup_printf (_("/ on %s"), hn_utf8);

		g_free (hn_utf8);
		g_free (hn);
	}

	g_object_unref (gfile);

	return name;
}

/**
 * gedit_utils_uri_for_display:
 * @uri: uri to be displayed.
 *
 * Filter, modify, unescape and change @uri to make it appropriate
 * for display to users.
 * 
 * This function is a convenient wrapper for g_file_get_parse_name
 *
 * Return value: a string which represents @uri and can be displayed.
 */
gchar *
gedit_utils_uri_for_display (const gchar *uri)
{
	GFile *gfile;
	gchar *parse_name;

	gfile = g_file_new_for_uri (uri);
	parse_name = g_file_get_parse_name (gfile);
	g_object_unref (gfile);

	return parse_name;
}

/**
 * gedit_utils_drop_get_uris:
 * @selection_data: the #GtkSelectionData from drag_data_received
 * @info: the info from drag_data_received
 *
 * Create a list of valid uri's from a uri-list drop.
 * 
 * Return value: a string array which will hold the uris or NULL if there 
 *		 were no valid uris. g_strfreev should be used when the 
 *		 string array is no longer used
 */
gchar **
gedit_utils_drop_get_uris (GtkSelectionData *selection_data)
{
	gchar **uris;
	gint i;
	gint p = 0;
	gchar **uri_list;

	uris = g_uri_list_extract_uris ((gchar *) gtk_selection_data_get_data (selection_data));
	uri_list = g_new0(gchar *, g_strv_length (uris) + 1);

	for (i = 0; uris[i] != NULL; i++)
	{
		gchar *uri;
		
		uri = gedit_utils_make_canonical_uri_from_shell_arg (uris[i]);
		
		/* Silently ignore malformed URI/filename */
		if (uri != NULL)
			uri_list[p++] = uri;
	}

	if (*uri_list == NULL)
	{
		g_free(uri_list);
		return NULL;
	}

	return uri_list;
}

static void
null_ptr (gchar **ptr)
{
	if (ptr)
		*ptr = NULL;
}

/**
 * gedit_utils_decode_uri:
 * @uri: the uri to decode
 * @scheme: return value pointer for the uri's scheme (e.g. http, sftp, ...)
 * @user: return value pointer for the uri user info
 * @port: return value pointer for the uri port
 * @host: return value pointer for the uri host
 * @path: return value pointer for the uri path
 *
 * Parse and break an uri apart in its individual components like the uri
 * scheme, user info, port, host and path. The return value pointer can be
 * NULL to ignore certain parts of the uri. If the function returns TRUE, then
 * all return value pointers should be freed using g_free
 * 
 * Return value: TRUE if the uri could be properly decoded, FALSE otherwise.
 */
gboolean
gedit_utils_decode_uri (const gchar *uri,
			gchar **scheme,
			gchar **user,
			gchar **host,
			gchar **port,
			gchar **path
)
{
	/* Largely copied from glib/gio/gdummyfile.c:_g_decode_uri. This 
	 * functionality should be in glib/gio, but for now we implement it
	 * ourselves (see bug #546182) */

	const char *p, *in, *hier_part_start, *hier_part_end;
	char *out;
	char c;

	/* From RFC 3986 Decodes:
	 * URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
	 */ 

	p = uri;
	
	null_ptr (scheme);
	null_ptr (user);
	null_ptr (port);
	null_ptr (host);
	null_ptr (path);

	/* Decode scheme:
	 * scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
	 */

	if (!g_ascii_isalpha (*p))
		return FALSE;

	while (1)
	{
		c = *p++;

		if (c == ':')
			break;

		if (!(g_ascii_isalnum(c) ||
		      c == '+' ||
		      c == '-' ||
		      c == '.'))
			return FALSE;
	}
	
	if (scheme)
	{
		*scheme = g_malloc (p - uri);
		out = *scheme;
	
		for (in = uri; in < p - 1; in++)
			*out++ = g_ascii_tolower (*in);
			
		*out = '\0';
	}
	
	hier_part_start = p;
	hier_part_end = p + strlen (p);
	
	if (hier_part_start[0] == '/' && hier_part_start[1] == '/')
	{
		const char *authority_start, *authority_end;
		const char *userinfo_start, *userinfo_end;
		const char *host_start, *host_end;
		const char *port_start;
		
		authority_start = hier_part_start + 2;
		/* authority is always followed by / or nothing */
		authority_end = memchr (authority_start, '/', hier_part_end - authority_start);
		
		if (authority_end == NULL)
			authority_end = hier_part_end;

		/* 3.2:
		 * authority = [ userinfo "@" ] host [ ":" port ]
		 */

		userinfo_end = memchr (authority_start, '@', authority_end - authority_start);
		
		if (userinfo_end)
		{
			userinfo_start = authority_start;
			
			if (user)
				*user = g_uri_unescape_segment (userinfo_start, userinfo_end, NULL);
			
			if (user && *user == NULL)
			{
				if (scheme)
					g_free (*scheme);

				return FALSE;
			}
	
			host_start = userinfo_end + 1;
		}
		else
			host_start = authority_start;

		port_start = memchr (host_start, ':', authority_end - host_start);

		if (port_start)
		{
			host_end = port_start++;

			if (port)
				*port = g_strndup (port_start, authority_end - port_start);
		}
		else
			host_end = authority_end;

		if (host)
			*host = g_strndup (host_start, host_end - host_start);

		hier_part_start = authority_end;
	}

	if (path)
		*path = g_uri_unescape_segment (hier_part_start, hier_part_end, "/");
	
	return TRUE;
}
