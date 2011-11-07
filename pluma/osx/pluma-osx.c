#include "pluma-osx.h"
#include <gdk/gdkquartz.h>
#include <Carbon/Carbon.h>

#import "pluma-osx-delegate.h"

void
pluma_osx_set_window_title (PlumaWindow   *window, 
			    gchar const   *title,
			    PlumaDocument *document)
{
	NSWindow *native;

	g_return_if_fail (PLUMA_IS_WINDOW (window));

	if (GTK_WIDGET (window)->window == NULL)
	{
		return;
	}

	native = gdk_quartz_window_get_nswindow (GTK_WIDGET (window)->window);

	if (document)
	{
		bool ismodified;

		if (pluma_document_is_untitled (document))
		{
			[native setRepresentedURL:nil];
		}
		else
		{
			const gchar *uri = pluma_document_get_uri (document);
			NSURL *nsurl = [NSURL URLWithString:[NSString stringWithUTF8String:uri]];
			
			[native setRepresentedURL:nsurl];
		}

		ismodified = !pluma_document_is_untouched (document); 
		[native setDocumentEdited:ismodified];
	}
	else
	{
		[native setRepresentedURL:nil];
		[native setDocumentEdited:false];
	}

	gtk_window_set_title (GTK_WINDOW (window), title);
}

gboolean
pluma_osx_show_url (const gchar *url)
{
 	return [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
}

gboolean
pluma_osx_show_help (const gchar *link_id)
{
	gchar *link;
	gboolean ret;

	if (link_id)
	{
		link = g_strdup_printf ("http://library.gnome.org/users/pluma/stable/%s",
					link_id);
	}
	else
	{
		link = g_strdup ("http://library.gnome.org/users/pluma/stable/");
	}

	ret = pluma_osx_show_url (link);
	g_free (link);

	return ret;
}

static void
destroy_delegate (PlumaOSXDelegate *delegate)
{
	[delegate dealloc];
}

void
pluma_osx_init(PlumaApp *app)
{
	PlumaOSXDelegate *delegate = [[PlumaOSXDelegate alloc] init];
	
	g_object_set_data_full (G_OBJECT (app),
	                        "PlumaOSXDelegate",
	                        delegate,
							(GDestroyNotify)destroy_delegate);
}