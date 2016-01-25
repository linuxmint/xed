#ifndef __XEDIT_FILE_BROWSER_UTILS_H__
#define __XEDIT_FILE_BROWSER_UTILS_H__

#include <xedit/xedit-window.h>
#include <gio/gio.h>

GdkPixbuf *xedit_file_browser_utils_pixbuf_from_theme     (gchar const *name,
                                                           GtkIconSize size);

GdkPixbuf *xedit_file_browser_utils_pixbuf_from_icon	  (GIcon * icon,
                                                           GtkIconSize size);
GdkPixbuf *xedit_file_browser_utils_pixbuf_from_file	  (GFile * file,
                                                           GtkIconSize size);

gchar * xedit_file_browser_utils_file_basename		  (GFile * file);
gchar * xedit_file_browser_utils_uri_basename             (gchar const * uri);

gboolean xedit_file_browser_utils_confirmation_dialog     (XeditWindow * window,
                                                           GtkMessageType type,
                                                           gchar const *message,
		                                           gchar const *secondary, 
		                                           gchar const * button_stock, 
		                                           gchar const * button_label);

#endif /* __XEDIT_FILE_BROWSER_UTILS_H__ */

// ex:ts=8:noet:
