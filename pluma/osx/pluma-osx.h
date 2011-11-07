#ifndef __PLUMA_OSX_H__
#define __PLUMA_OSX_H__

#include <gtk/gtk.h>
#include <pluma/pluma-window.h>
#include <pluma/pluma-app.h>

void	pluma_osx_init (PlumaApp *app);

void 	pluma_osx_set_window_title 	(PlumaWindow   *window, 
					 gchar const   *title,
					 PlumaDocument *document);

gboolean pluma_osx_show_url 		(const gchar *url);
gboolean pluma_osx_show_help		(const gchar *link_id);

#endif /* __PLUMA_OSX_H__ */
