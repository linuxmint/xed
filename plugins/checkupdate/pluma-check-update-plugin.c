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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "pluma-check-update-plugin.h"

#include <glib/gi18n-lib.h>
#include <pluma/pluma-debug.h>
#include <pluma/pluma-utils.h>
#include <libsoup/soup.h>
#include <gtk/gtk.h>
#include <stdlib.h>

#include <gio/gio.h>

#if !GTK_CHECK_VERSION(2, 17, 1)
#include <pluma/pluma-message-area.h>
#endif

#define SETTINGS_SCHEMA           "org.mate.pluma.plugins.checkupdate"
#define SETTINGS_IGNORE_VERSION   "ignore-version"

#define WINDOW_DATA_KEY "PlumaCheckUpdatePluginWindowData"

#define VERSION_PLACE "<a href=\"[0-9]\\.[0-9]+/\">"

#ifdef G_OS_WIN32
#define PLUMA_URL "http://pub.mate-desktop.org/sources/pluma/"
#define FILE_REGEX "pluma\\-setup\\-[0-9]+\\.[0-9]+\\.[0-9]+(\\-[0-9]+)?\\.exe"
#else
#define PLUMA_URL "http://pub.mate-desktop.org/sources/pluma/"
#define FILE_REGEX "pluma\\-[0-9]+\\.[0-9]+\\.[0-9]+(\\-[0-9]+)?\\.dmg"
#endif

#ifdef OS_OSX
#include "pluma/osx/pluma-osx.h"
#endif

#define PLUMA_CHECK_UPDATE_PLUGIN_GET_PRIVATE(object) \
				(G_TYPE_INSTANCE_GET_PRIVATE ((object),	\
				PLUMA_TYPE_CHECK_UPDATE_PLUGIN,		\
				PlumaCheckUpdatePluginPrivate))

PLUMA_PLUGIN_REGISTER_TYPE (PlumaCheckUpdatePlugin, pluma_check_update_plugin)

struct _PlumaCheckUpdatePluginPrivate
{
	SoupSession *session;

	GSettings *settings;
};

typedef struct
{
	PlumaCheckUpdatePlugin *plugin;

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
pluma_check_update_plugin_init (PlumaCheckUpdatePlugin *plugin)
{
	plugin->priv = PLUMA_CHECK_UPDATE_PLUGIN_GET_PRIVATE (plugin);

	pluma_debug_message (DEBUG_PLUGINS,
			     "PlumaCheckUpdatePlugin initializing");

	plugin->priv->session = soup_session_async_new ();

	plugin->priv->settings = g_settings_new (SETTINGS_SCHEMA);
}

static void
pluma_check_update_plugin_dispose (GObject *object)
{
	PlumaCheckUpdatePlugin *plugin = PLUMA_CHECK_UPDATE_PLUGIN (object);

	if (plugin->priv->session != NULL)
	{
		g_object_unref (plugin->priv->session);
		plugin->priv->session = NULL;
	}

	if (plugin->priv->settings != NULL)
	{
		g_object_unref (G_OBJECT (plugin->priv->settings));

		plugin->priv->settings = NULL;
	}

	pluma_debug_message (DEBUG_PLUGINS,
			     "PlumaCheckUpdatePlugin disposing");

	G_OBJECT_CLASS (pluma_check_update_plugin_parent_class)->dispose (object);
}

static void
pluma_check_update_plugin_finalize (GObject *object)
{
	pluma_debug_message (DEBUG_PLUGINS,
			     "PlumaCheckUpdatePlugin finalizing");

	G_OBJECT_CLASS (pluma_check_update_plugin_parent_class)->finalize (object);
}

static void
set_contents (GtkWidget *infobar,
	      GtkWidget *contents)
{
#if !GTK_CHECK_VERSION (2, 17, 1)
	pluma_message_area_set_contents (PLUMA_MESSAGE_AREA (infobar),
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
		PlumaWindow *window)
{
	if (response_id == GTK_RESPONSE_YES)
	{
		GError *error = NULL;
		WindowData *data;

		data = g_object_get_data (G_OBJECT (window),
					  WINDOW_DATA_KEY);

#ifdef OS_OSX
		pluma_osx_show_url (data->url);
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

		g_settings_set_string (data->plugin->priv->settings,
					 SETTINGS_IGNORE_VERSION,
					 data->version);
	}

	g_object_set_data (G_OBJECT (window),
			   WINDOW_DATA_KEY,
			   NULL);

	gtk_widget_destroy (infobar);
}

static GtkWidget *
create_infobar (PlumaWindow *window,
                const gchar *version)
{
	GtkWidget *infobar;
	gchar *message;

#if !GTK_CHECK_VERSION (2, 17, 1)
	infobar = pluma_message_area_new ();

	pluma_message_area_add_stock_button_with_text (PLUMA_MESSAGE_AREA (infobar),
						       _("_Download"),
						       GTK_STOCK_SAVE,
						       GTK_RESPONSE_YES);
	pluma_message_area_add_stock_button_with_text (PLUMA_MESSAGE_AREA (infobar),
						       _("_Ignore Version"),
						       GTK_STOCK_DISCARD,
						       GTK_RESPONSE_NO);
	pluma_message_area_add_button (PLUMA_MESSAGE_AREA (infobar),
				       GTK_STOCK_CANCEL,
				       GTK_RESPONSE_CANCEL);
#else
	GtkWidget *button;

	infobar = gtk_info_bar_new ();

	button = pluma_gtk_button_new_with_stock_icon (_("_Download"),
						       GTK_STOCK_SAVE);
	gtk_widget_show (button);

	gtk_info_bar_add_action_widget (GTK_INFO_BAR (infobar),
					button,
					GTK_RESPONSE_YES);

	button = pluma_gtk_button_new_with_stock_icon (_("_Ignore Version"),
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

	message = g_strdup_printf ("%s (%s)", _("There is a new version of pluma"), version);
	set_message_area_text_and_icon (infobar,
					"gtk-dialog-info",
					message,
					_("You can download the new version of pluma"
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
get_ignore_version (PlumaCheckUpdatePlugin *plugin)
{
	return g_settings_get_string (plugin->priv->settings,
					SETTINGS_IGNORE_VERSION);
}

static void
parse_page_file (SoupSession *session,
		 SoupMessage *msg,
		 PlumaWindow *window)
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

	if ((PLUMA_MINOR_VERSION % 2) == 0)
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
		    PlumaWindow *window)
{
	if (msg->status_code == SOUP_STATUS_OK)
	{
		gchar *version;
		SoupMessage *msg2;
		WindowData *data;

		data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);

		version = get_file_page_version (msg->response_body->data,
						 VERSION_PLACE);

		data->url = g_strconcat (PLUMA_URL, version, "/", NULL);
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
impl_activate (PlumaPlugin *plugin,
	       PlumaWindow *window)
{
	SoupMessage *msg;
	WindowData *data;

	pluma_debug (DEBUG_PLUGINS);

	data = g_slice_new (WindowData);
	data->plugin = PLUMA_CHECK_UPDATE_PLUGIN (plugin);
	data->url = NULL;
	data->version = NULL;

	g_object_set_data_full (G_OBJECT (window),
				WINDOW_DATA_KEY,
				data,
				free_window_data);

	msg = soup_message_new ("GET", PLUMA_URL);

	soup_session_queue_message (PLUMA_CHECK_UPDATE_PLUGIN (plugin)->priv->session, msg,
				    (SoupSessionCallback)parse_page_version,
				    window);
}

static void
impl_deactivate (PlumaPlugin *plugin,
		 PlumaWindow *window)
{

	pluma_debug (DEBUG_PLUGINS);

	soup_session_abort (PLUMA_CHECK_UPDATE_PLUGIN (plugin)->priv->session);

	g_object_set_data (G_OBJECT (window),
			   WINDOW_DATA_KEY,
			   NULL);
}

static void
pluma_check_update_plugin_class_init (PlumaCheckUpdatePluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	PlumaPluginClass *plugin_class = PLUMA_PLUGIN_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (PlumaCheckUpdatePluginPrivate));

	object_class->finalize = pluma_check_update_plugin_finalize;
	object_class->dispose = pluma_check_update_plugin_dispose;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
}
