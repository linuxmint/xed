#ifndef __GEDIT_FILE_BROWSER_UTILS_H__
#define __GEDIT_FILE_BROWSER_UTILS_H__

#include <gedit/gedit-window.h>
#include <gio/gio.h>

GdkPixbuf *gedit_file_browser_utils_pixbuf_from_theme     (gchar const *name,
                                                           GtkIconSize size);

GdkPixbuf *gedit_file_browser_utils_pixbuf_from_icon	  (GIcon * icon,
                                                           GtkIconSize size);
GdkPixbuf *gedit_file_browser_utils_pixbuf_from_file	  (GFile * file,
                                                           GtkIconSize size);

gchar * gedit_file_browser_utils_file_basename		  (GFile * file);
gchar * gedit_file_browser_utils_uri_basename             (gchar const * uri);

gboolean gedit_file_browser_utils_confirmation_dialog     (GeditWindow * window,
                                                           GtkMessageType type,
                                                           gchar const *message,
		                                           gchar const *secondary, 
		                                           gchar const * button_stock, 
		                                           gchar const * button_label);

#endif /* __GEDIT_FILE_BROWSER_UTILS_H__ */

// ex:ts=8:noet:
