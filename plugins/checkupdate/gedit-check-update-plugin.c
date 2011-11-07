/*
 * Copyright (C) 2009 - Ignacio Casal Quinteiro <icq@gnome.org>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gedit-check-update-plugin.h"

#include <glib/gi18n-lib.h>
#include <gedit/gedit-debug.h>
#include <gedit/gedit-utils.h>
#include <libsoup/soup.h>
#include <gtk/gtk.h>
#include <stdlib.h>

#include <mateconf/mateconf-client.h>

#if !GTK_CHECK_VERSION(2, 17, 1)
#include <gedit/gedit-message-area.h>
#endif

#define MATECONF_KEY_BASE "/apps/gedit-2/plugins/checkupdate"
#define MATECONF_KEY_IGNORE_VERSION   MATECONF_KEY_BASE "/ignore_version"

#define WINDOW_DATA_KEY "GeditCheckUpdatePluginWindowData"

#define VERSION_PLACE "<a href=\"[0-9]\\.[0-9]+/\">"

#ifdef G_OS_WIN32
#define GEDIT_URL "http://ftp.acc.umu.se/pub/mate/binaries/win32/gedit/"
#define FILE_REGEX "gedit\\-setup\\-[0-9]+\\.[0-9]+\\.[0-9]+(\\-[0-9]+)?\\.exe"
#else
#define GEDIT_URL "http://ftp.acc.umu.se/pub/mate/binaries/mac/gedit/"
#define FILE_REGEX "gedit\\-[0-9]+\\.[0-9]+\\.[0-9]+(\\-[0-9]+)?\\.dmg"
#endif

#ifdef OS_OSX
#include "gedit/osx/gedit-osx.h"
#endif

#define GEDIT_CHECK_UPDATE_PLUGIN_GET_PRIVATE(object) \
				(G_TYPE_INSTANCE_GET_PRIVATE ((object),	\
				GEDIT_TYPE_CHECK_UPDATE_PLUGIN,		\
				GeditCheckUpdatePluginPrivate))

GEDIT_PLUGIN_REGISTER_TYPE (GeditCheckUpdatePlugin, gedit_check_update_plugin)

struct _GeditCheckUpdatePluginPrivate
{
	SoupSession *session;

	MateConfClient *mateconf_client;
};

typedef struct
{
	GeditCheckUpdatePlugin *plugin;

	gchar *url;
	gchar *version;
} WindowData;

static void
free_window_data (gpointer data)
{
	WindowData *window_data;

	if (data == NULL)
		return;

	window_data = (WindowData *)data;

	g_free (window_data->url);
	g_free (window_data->version);
	g_slice_free (WindowData, data);
}

static void
gedit_check_update_plugin_init (GeditCheckUpdatePlugin *plugin)
{
	plugin->priv = GEDIT_CHECK_UPDATE_PLUGIN_GET_PRIVATE (plugin);

	gedit_debug_message (DEBUG_PLUGINS,
			     "GeditCheckUpdatePlugin initializing");

	plugin->priv->session = soup_session_async_new ();

	plugin->priv->mateconf_client = mateconf_client_get_default ();

	mateconf_client_add_dir (plugin->priv->mateconf_client,
			      MATECONF_KEY_BASE,
			      MATECONF_CLIENT_PRELOAD_ONELEVEL,
			      NULL);
}

static void
gedit_check_update_plugin_dispose (GObject *object)
{
	GeditCheckUpdatePlugin *plugin = GEDIT_CHECK_UPDATE_PLUGIN (object);

	if (plugin->priv->session != NULL)
	{
		g_object_unref (plugin->priv->session);
		plugin->priv->session = NULL;
	}

	if (plugin->priv->mateconf_client != NULL)
	{
		mateconf_client_suggest_sync (plugin->priv->mateconf_client, NULL);

		g_object_unref (G_OBJECT (plugin->priv->mateconf_client));
		
		plugin->priv->mateconf_client = NULL;
	}

	gedit_debug_message (DEBUG_PLUGINS,
			     "GeditCheckUpdatePlugin disposing");
	
	G_OBJECT_CLASS (gedit_check_update_plugin_parent_class)->dispose (object);
}

static void
gedit_check_update_plugin_finalize (GObject *object)
{
	gedit_debug_message (DEBUG_PLUGINS,
			     "GeditCheckUpdatePlugin finalizing");

	G_OBJECT_CLASS (gedit_check_update_plugin_parent_class)->finalize (object);
}

static void
set_contents (GtkWidget *infobar,
	      GtkWidget *contents)
{
#if !GTK_CHECK_VERSION (2, 17, 1)
	gedit_message_area_set_contents (GEDIT_MESSAGE_AREA (infobar),
					 contents);
#else
	GtkWidget *content_area;
	
	content_area = gtk_info_bar_get_content_area (GTK_INFO_BAR (infobar));
	gtk_container_add (GTK_CONTAINER (content_area), contents);
#endif
}

static void
set_message_area_text_and_icon (GtkWidget        *message_area,
				const gchar      *icon_stock_id,
				const gchar      *primary_text,
				const gchar      *secondary_text)
{
	GtkWidget *hbox_content;
	GtkWidget *image;
	GtkWidget *vbox;
	gchar *primary_markup;
	gchar *secondary_markup;
	GtkWidget *primary_label;
	GtkWidget *secondary_label;

	hbox_content = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox_content);

	image = gtk_image_new_from_stock (icon_stock_id, GTK_ICON_SIZE_DIALOG);
	gtk_widget_show (image);
	gtk_box_pack_start (GTK_BOX (hbox_content), image, FALSE, FALSE, 0);
	gtk_misc_set_alignment (GTK_MISC (image), 0.5, 0);

	vbox = gtk_vbox_new (FALSE, 6);
	gtk_widget_show (vbox);
	gtk_box_pack_start (GTK_BOX (hbox_content), vbox, TRUE, TRUE, 0);

	primary_markup = g_strdup_printf ("<b>%s</b>", primary_text);
	primary_label = gtk_label_new (primary_markup);
	g_free (primary_markup);
	gtk_widget_show (primary_label);
	gtk_box_pack_start (GTK_BOX (vbox), primary_label, TRUE, TRUE, 0);
	gtk_label_set_use_markup (GTK_LABEL (primary_label), TRUE);
	gtk_label_set_line_wrap (GTK_LABEL (primary_label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (primary_label), 0, 0.5);
	GTK_WIDGET_SET_FLAGS (primary_label, GTK_CAN_FOCUS);
	gtk_label_set_selectable (GTK_LABEL (primary_label), TRUE);

	if (secondary_text != NULL)
	{
		secondary_markup = g_strdup_printf ("<small>%s</small>",
						    secondary_text);
		secondary_label = gtk_label_new (secondary_markup);
		g_free (secondary_markup);
		gtk_widget_show (secondary_label);
		gtk_box_pack_start (GTK_BOX (vbox), secondary_label, TRUE, TRUE, 0);
		GTK_WIDGET_SET_FLAGS (secondary_label, GTK_CAN_FOCUS);
		gtk_label_set_use_markup (GTK_LABEL (secondary_label), TRUE);
		gtk_label_set_line_wrap (GTK_LABEL (secondary_label), TRUE);
		gtk_label_set_selectable (GTK_LABEL (secondary_label), TRUE);
		gtk_misc_set_alignment (GTK_MISC (secondary_label), 0, 0.5);
	}
	
	set_contents (message_area, hbox_content);
}

static void
on_response_cb (GtkWidget   *infobar,
		gint         response_id,
		GeditWindow *window)
{
	if (response_id == GTK_RESPONSE_YES)
	{
		GError *error = NULL;
		WindowData *data;
		
		data = g_object_get_data (G_OBJECT (window),
					  WINDOW_DATA_KEY);

#ifdef OS_OSX
		gedit_osx_show_url (data->url);
#else
		gtk_show_uri (gtk_widget_get_screen (GTK_WIDGET (window)),
			      data->url,
			      GDK_CURRENT_TIME,
			      &error);
#endif
		if (error != NULL)
		{
			GtkWidget *dialog;

			dialog = gtk_message_dialog_new (GTK_WINDOW (window),
							 GTK_DIALOG_DESTROY_WITH_PARENT,
							 GTK_MESSAGE_ERROR,
							 GTK_BUTTONS_CLOSE, 
							 _("There was an error displaying the URI."));

			gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
								  "%s", error->message);

			g_signal_connect (G_OBJECT (dialog),
					  "response",
					  G_CALLBACK (gtk_widget_destroy),
					  NULL);

			gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

			gtk_widget_show (dialog);

			g_error_free (error);
		}
	}
	else if (response_id == GTK_RESPONSE_NO)
	{
		WindowData *data;
		
		data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);

		mateconf_client_set_string (data->plugin->priv->mateconf_client,
					 MATECONF_KEY_IGNORE_VERSION,
					 data->version,
					 NULL);
	}

	g_object_set_data (G_OBJECT (window),
			   WINDOW_DATA_KEY,
			   NULL);

	gtk_widget_destroy (infobar);
}

static GtkWidget *
create_infobar (GeditWindow *window,
                const gchar *version)
{
	GtkWidget *infobar;
	gchar *message;

#if !GTK_CHECK_VERSION (2, 17, 1)
	infobar = gedit_message_area_new ();
	
	gedit_message_area_add_stock_button_with_text (GEDIT_MESSAGE_AREA (infobar),
						       _("_Download"),
						       GTK_STOCK_SAVE,
						       GTK_RESPONSE_YES);
	gedit_message_area_add_stock_button_with_text (GEDIT_MESSAGE_AREA (infobar),
						       _("_Ignore Version"),
						       GTK_STOCK_DISCARD,
						       GTK_RESPONSE_NO);
	gedit_message_area_add_button (GEDIT_MESSAGE_AREA (infobar),
				       GTK_STOCK_CANCEL,
				       GTK_RESPONSE_CANCEL);
#else
	GtkWidget *button;

	infobar = gtk_info_bar_new ();
	
	button = gedit_gtk_button_new_with_stock_icon (_("_Download"),
						       GTK_STOCK_SAVE);
	gtk_widget_show (button);

	gtk_info_bar_add_action_widget (GTK_INFO_BAR (infobar),
					button,
					GTK_RESPONSE_YES);

	button = gedit_gtk_button_new_with_stock_icon (_("_Ignore Version"),
						       GTK_STOCK_DISCARD);
	gtk_widget_show (button);

	gtk_info_bar_add_action_widget (GTK_INFO_BAR (infobar),
					button,
					GTK_RESPONSE_NO);

	gtk_info_bar_add_button (GTK_INFO_BAR (infobar),
				 GTK_STOCK_CANCEL,
				 GTK_RESPONSE_CANCEL);
	
	gtk_info_bar_set_message_type (GTK_INFO_BAR (infobar),
				       GTK_MESSAGE_INFO);
#endif

	message = g_strdup_printf ("%s (%s)", _("There is a new version of gedit"), version);
	set_message_area_text_and_icon (infobar,
					"gtk-dialog-info",
					message,
					_("You can download the new version of gedit"
					  " by clicking on the download button or"
					  " ignore that version and wait for a new one"));

	g_free (message);

	g_signal_connect (infobar, "response",
			  G_CALLBACK (on_response_cb),
			  window);
	
	return infobar;
}

static void
pack_infobar (GtkWidget *window,
	      GtkWidget *infobar)
{
	GtkWidget *vbox;
	
	vbox = gtk_bin_get_child (GTK_BIN (window));
	
	gtk_box_pack_start (GTK_BOX (vbox), infobar, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (vbox), infobar, 2);
}

static gchar *
get_file (const gchar *text,
	  const gchar *regex_place)
{
	GRegex *regex;
	GMatchInfo *match_info;
	gchar *word = NULL;
	
	regex = g_regex_new (regex_place, 0, 0, NULL);
	g_regex_match (regex, text, 0, &match_info);
	while (g_match_info_matches (match_info))
	{
		g_free (word);
		
		word = g_match_info_fetch (match_info, 0);
		
		g_match_info_next (match_info, NULL);
	}
	g_match_info_free (match_info);
	g_regex_unref (regex);
	
	return word;
}

static void
get_numbers (const gchar *version,
	     gint *major,
	     gint *minor,
	     gint *micro)
{
	gchar **split;
	gint num = 2;
	
	if (micro != NULL)
		num = 3;
	
	split = g_strsplit (version, ".", num);
	*major = atoi (split[0]);
	*minor = atoi (split[1]);
	if (micro != NULL)
		*micro = atoi (split[2]);
	
	g_strfreev (split);
}

static gboolean
newer_version (const gchar *v1,
	       const gchar *v2,
	       gboolean with_micro)
{
	gboolean newer = FALSE;
	gint major1, minor1, micro1;
	gint major2, minor2, micro2;
	
	if (v1 == NULL || v2 == NULL)
		return FALSE;

	if (with_micro)
	{
		get_numbers (v1, &major1, &minor1, &micro1);
		get_numbers (v2, &major2, &minor2, &micro2);
	}
	else
	{
		get_numbers (v1, &major1, &minor1, NULL);
		get_numbers (v2, &major2, &minor2, NULL);
	}

	if (major1 > major2)
	{
		newer = TRUE;
	}
	else if (minor1 > minor2 && major1 == major2)
	{
		newer = TRUE;
	}
	else if (with_micro && micro1 > micro2 && minor1 == minor2)
	{
		newer = TRUE;
	}
	
	return newer;
}

static gchar *
parse_file_version (const gchar *file)
{
	gchar *p, *aux;
	
	p = (gchar *)file;
	
	while (*p != '\0' && !g_ascii_isdigit (*p))
	{
		p++;
	}
	
	if (*p == '\0')
		return NULL;
	
	aux = g_strrstr (p, "-");
	if (aux == NULL)
		aux = g_strrstr (p, ".");
	
	return g_strndup (p, aux - p);
}

static gchar *
get_ignore_version (GeditCheckUpdatePlugin *plugin)
{
	return mateconf_client_get_string (plugin->priv->mateconf_client,
					MATECONF_KEY_IGNORE_VERSION,
					NULL);
}

static void
parse_page_file (SoupSession *session,
		 SoupMessage *msg,
		 GeditWindow *window)
{
	if (msg->status_code == SOUP_STATUS_OK)
	{
		gchar *file;
		gchar *file_version;
		gchar *ignore_version;
		WindowData *data;

		data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);

		file = get_file (msg->response_body->data, FILE_REGEX);
		file_version = parse_file_version (file);
		ignore_version = get_ignore_version (data->plugin);

		if (newer_version (file_version, VERSION, TRUE) &&
		    (ignore_version == NULL || *ignore_version == '\0' ||
		     newer_version (file_version, ignore_version, TRUE)))
		{
			GtkWidget *infobar;
			WindowData *data;
			gchar *file_url;

			data = g_object_get_data (G_OBJECT (window),
						  WINDOW_DATA_KEY);
			
			file_url = g_strconcat (data->url, file, NULL);

			g_free (data->url);
			data->url = file_url;
			data->version = g_strdup (file_version);
		
			infobar = create_infobar (window, file_version);
			pack_infobar (GTK_WIDGET (window), infobar);
			gtk_widget_show (infobar);
		}

		g_free (ignore_version);
		g_free (file_version);
		g_free (file);
	}
	else
	{
		g_object_set_data (G_OBJECT (window),
				   WINDOW_DATA_KEY,
				   NULL);
	}
}

static gboolean
is_unstable (const gchar *version)
{
	gchar **split;
	gint minor;
	gboolean unstable = TRUE;;
	
	split = g_strsplit (version, ".", 2);
	minor = atoi (split[1]);
	g_strfreev (split);
	
	if ((minor % 2) == 0)
		unstable = FALSE;
	
	return unstable;
}

static gchar *
get_file_page_version (const gchar *text,
		       const gchar *regex_place)
{
	GRegex *regex;
	GMatchInfo *match_info;
	GString *string = NULL;
	gchar *unstable = NULL;
	gchar *stable = NULL;
	
	regex = g_regex_new (regex_place, 0, 0, NULL);
	g_regex_match (regex, text, 0, &match_info);
	while (g_match_info_matches (match_info))
	{
		gint end;
		gint i;
		
		g_match_info_fetch_pos (match_info, 0, NULL, &end);
		
		string = g_string_new ("");
		
		i = end;
		while (text[i] != '/')
		{
			string = g_string_append_c (string, text[i]);
			i++;
		}
		
		if (is_unstable (string->str))
		{
			g_free (unstable);
			unstable = g_string_free (string, FALSE);
		}
		else
		{
			g_free (stable);
			stable = g_string_free (string, FALSE);
		}
		
		g_match_info_next (match_info, NULL);
	}
	g_match_info_free (match_info);
	g_regex_unref (regex);
	
	if ((GEDIT_MINOR_VERSION % 2) == 0)
	{
		g_free (unstable);
		
		return stable;
	}
	else
	{
		/* We need to check that stable isn't newer than unstable */
		if (newer_version (stable, unstable, FALSE))
		{
			g_free (unstable);
			
			return stable;
		}
		else
		{
			g_free (stable);
		
			return unstable;
		}
	}
}

static void
parse_page_version (SoupSession *session,
		    SoupMessage *msg,
		    GeditWindow *window)
{
	if (msg->status_code == SOUP_STATUS_OK)
	{
		gchar *version;
		SoupMessage *msg2;
		WindowData *data;

		data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
		
		version = get_file_page_version (msg->response_body->data,
						 VERSION_PLACE);
		
		data->url = g_strconcat (GEDIT_URL, version, "/", NULL);
		g_free (version);
		msg2 = soup_message_new ("GET", data->url);
	
		soup_session_queue_message (session, msg2,
					    (SoupSessionCallback)parse_page_file,
					    window);
	}
	else
	{
		g_object_set_data (G_OBJECT (window),
				   WINDOW_DATA_KEY,
				   NULL);
	}
}

static void
impl_activate (GeditPlugin *plugin,
	       GeditWindow *window)
{
	SoupMessage *msg;
	WindowData *data;
	
	gedit_debug (DEBUG_PLUGINS);

	data = g_slice_new (WindowData);
	data->plugin = GEDIT_CHECK_UPDATE_PLUGIN (plugin);
	data->url = NULL;
	data->version = NULL;

	g_object_set_data_full (G_OBJECT (window),
				WINDOW_DATA_KEY,
				data,
				free_window_data);

	msg = soup_message_new ("GET", GEDIT_URL);
	
	soup_session_queue_message (GEDIT_CHECK_UPDATE_PLUGIN (plugin)->priv->session, msg,
				    (SoupSessionCallback)parse_page_version,
				    window);
}

static void
impl_deactivate (GeditPlugin *plugin,
		 GeditWindow *window)
{

	gedit_debug (DEBUG_PLUGINS);
	
	soup_session_abort (GEDIT_CHECK_UPDATE_PLUGIN (plugin)->priv->session);
	
	g_object_set_data (G_OBJECT (window),
			   WINDOW_DATA_KEY,
			   NULL);
}

static void
gedit_check_update_plugin_class_init (GeditCheckUpdatePluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GeditPluginClass *plugin_class = GEDIT_PLUGIN_CLASS (klass);
	
	g_type_class_add_private (object_class, sizeof (GeditCheckUpdatePluginPrivate));

	object_class->finalize = gedit_check_update_plugin_finalize;
	object_class->dispose = gedit_check_update_plugin_dispose;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
}
