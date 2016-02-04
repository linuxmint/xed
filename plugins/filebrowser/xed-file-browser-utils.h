#ifndef __XED_FILE_BROWSER_UTILS_H__
#define __XED_FILE_BROWSER_UTILS_H__

#include <xed/xed-window.h>
#include <gio/gio.h>

GdkPixbuf *xed_file_browser_utils_pixbuf_from_theme     (gchar const *name,
                                                           GtkIconSize size);

GdkPixbuf *xed_file_browser_utils_pixbuf_from_icon	  (GIcon * icon,
                                                           GtkIconSize size);
GdkPixbuf *xed_file_browser_utils_pixbuf_from_file	  (GFile * file,
                                                           GtkIconSize size);

gchar * xed_file_browser_utils_file_basename		  (GFile * file);
gchar * xed_file_browser_utils_uri_basename             (gchar const * uri);

gboolean xed_file_browser_utils_confirmation_dialog     (XedWindow * window,
                                                           GtkMessageType type,
                                                           gchar const *message,
		                                           gchar const *secondary, 
		                                           gchar const * button_stock, 
		                                           gchar const * button_label);

#endif /* __XED_FILE_BROWSER_UTILS_H__ */

// ex:ts=8:noet:
