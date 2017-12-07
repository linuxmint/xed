/*
 * Copyright (C) 2009 Ignacio Casal Quinteiro <icq@gnome.org>
 *               2009 Jesse van den Kieboom <jesse@gnome.org>
 *               2013 Sébastien Wilmet <swilmet@gnome.org>
 *				 2017 Mickael Albertus <mickael.albertus@gmail.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xed-wordcompletion-plugin.h"

#include <glib/gi18n-lib.h>
#include <glib.h>
#include <gio/gio.h>

#include <xed/xed-debug.h>
#include <xed/xed-window.h>
#include <xed/xed-window-activatable.h>
#include <xed/xed-utils.h>
#include <xed/xed-view.h>
#include <xed/xed-view-activatable.h>
#include <libpeas-gtk/peas-gtk-configurable.h>
#include <gtksourceview/gtksource.h>
#include <gtksourceview/completion-providers/words/gtksourcecompletionwords.h>

#define WINDOW_PROVIDER "XedWordCompletionPluginProvider"

#define WORDCOMPLETION_SETTINGS_BASE "org.x.editor.plugins.wordcompletion"
#define SETTINGS_KEY_INTERACTIVE_COMPLETION "interactive-completion"
#define SETTINGS_KEY_MINIMUM_WORD_SIZE "minimum-word-size"

static void xed_window_activatable_iface_init (XedWindowActivatableInterface *iface);
static void xed_view_activatable_iface_init (XedViewActivatableInterface *iface);
static void peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (XedWordCompletionPlugin,
                                xed_wordcompletion_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (XED_TYPE_WINDOW_ACTIVATABLE,
                                                               xed_window_activatable_iface_init)
                                G_IMPLEMENT_INTERFACE_DYNAMIC (XED_TYPE_VIEW_ACTIVATABLE,
                                                               xed_view_activatable_iface_init)
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_GTK_TYPE_CONFIGURABLE,
                                                               peas_gtk_configurable_iface_init))

struct _XedWordCompletionPluginPrivate
{
	GtkWidget *window;
	XedView *view;
	GtkSourceCompletionProvider *provider;
  GSettings *settings;
};

enum
{
	PROP_0,
	PROP_WINDOW,
	PROP_VIEW
};


typedef struct _WordCompletionConfigureWidget WordCompletionConfigureWidget;

struct _WordCompletionConfigureWidget
{
	GtkWidget *dialog;
	GtkWidget *min_word_size;
	GtkWidget *interactive_completion;

	GSettings *settings;
};

static void
xed_wordcompletion_plugin_init (XedWordCompletionPlugin *plugin)
{
	xed_debug_message (DEBUG_PLUGINS, "XedWordCompletionPlugin initializing");

	plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin,
	                                            XED_TYPE_WORDCOMPLETION_PLUGIN,
	                                            XedWordCompletionPluginPrivate);
	                                            
	plugin->priv->settings = g_settings_new (WORDCOMPLETION_SETTINGS_BASE);
}

static void
xed_wordcompletion_plugin_finalize (GObject *object)
{
    XedWordCompletionPlugin *plugin = XED_WORDCOMPLETION_PLUGIN (object);

    xed_debug_message (DEBUG_PLUGINS, "XedWordCompletionPlugin finalizing");

    g_object_unref (G_OBJECT (plugin->priv->settings));

    G_OBJECT_CLASS (xed_wordcompletion_plugin_parent_class)->finalize (object);
}


static void
xed_wordcompletion_plugin_dispose (GObject *object)
{
	XedWordCompletionPlugin *plugin = XED_WORDCOMPLETION_PLUGIN (object);

	if (plugin->priv->window != NULL)
	{
		g_object_unref (plugin->priv->window);
		plugin->priv->window = NULL;
	}

	if (plugin->priv->view != NULL)
	{
		g_object_unref (plugin->priv->view);
		plugin->priv->view = NULL;
	}

	if (plugin->priv->provider != NULL)
	{
		g_object_unref (plugin->priv->provider);
		plugin->priv->provider = NULL;
	}

	G_OBJECT_CLASS (xed_wordcompletion_plugin_parent_class)->dispose (object);
}

static void
xed_wordcompletion_plugin_set_property (GObject      *object,
                                           guint         prop_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
	XedWordCompletionPlugin *plugin = XED_WORDCOMPLETION_PLUGIN (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			plugin->priv->window = g_value_dup_object (value);
			break;
		case PROP_VIEW:
			plugin->priv->view = XED_VIEW (g_value_dup_object (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
xed_wordcompletion_plugin_get_property (GObject    *object,
                                           guint       prop_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
	XedWordCompletionPlugin *plugin = XED_WORDCOMPLETION_PLUGIN (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			g_value_set_object (value, plugin->priv->window);
			break;
		case PROP_VIEW:
			g_value_set_object (value, plugin->priv->view);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
update_activation (GtkSourceCompletionWords *provider,
		   GSettings                *settings)
{
	GtkSourceCompletionActivation activation;

	g_object_get (provider, "activation", &activation, NULL);

	if (g_settings_get_boolean (settings, SETTINGS_KEY_INTERACTIVE_COMPLETION))
	{
		activation |= GTK_SOURCE_COMPLETION_ACTIVATION_INTERACTIVE;
	}
	else
	{
		activation &= ~GTK_SOURCE_COMPLETION_ACTIVATION_INTERACTIVE;
	}

	g_object_set (provider, "activation", activation, NULL);
}

static void
on_interactive_completion_changed_cb (GSettings                *settings,
				      gchar                    *key,
				      GtkSourceCompletionWords *provider)
{
	update_activation (provider, settings);
}

static GtkSourceCompletionWords *
create_provider (void)
{
	GtkSourceCompletionWords *provider;
	GSettings *settings;

	provider = gtk_source_completion_words_new (_("Word completion"), NULL);

	settings = g_settings_new (WORDCOMPLETION_SETTINGS_BASE);

	g_settings_bind (settings, SETTINGS_KEY_MINIMUM_WORD_SIZE,
			 provider, "minimum-word-size",
			 G_SETTINGS_BIND_GET);

	update_activation (provider, settings);

	g_signal_connect_object (settings,
				 "changed::" SETTINGS_KEY_INTERACTIVE_COMPLETION,
				 G_CALLBACK (on_interactive_completion_changed_cb),
				 provider,
				 0);

	g_object_unref (settings);

	return provider;
}

static void
xed_wordcompletion_window_activate (XedWindowActivatable *activatable)
{
	XedWordCompletionPluginPrivate *priv;
	GtkSourceCompletionWords *provider;

	xed_debug (DEBUG_PLUGINS);

	priv = XED_WORDCOMPLETION_PLUGIN (activatable)->priv;

	provider = create_provider ();

	g_object_set_data_full (G_OBJECT (priv->window),
	                        WINDOW_PROVIDER,
	                        provider,
	                        (GDestroyNotify)g_object_unref);
}

static void
xed_wordcompletion_window_deactivate (XedWindowActivatable *activatable)
{
	XedWordCompletionPluginPrivate *priv;

	xed_debug (DEBUG_PLUGINS);

	priv = XED_WORDCOMPLETION_PLUGIN (activatable)->priv;

	g_object_set_data (G_OBJECT (priv->window), WINDOW_PROVIDER, NULL);
}

static void
xed_wordcompletion_view_activate (XedViewActivatable *activatable)
{
	XedWordCompletionPluginPrivate *priv;
	GtkSourceCompletion *completion;
	GtkSourceCompletionProvider *provider;
	GtkTextBuffer *buf;

	xed_debug (DEBUG_PLUGINS);

	priv = XED_WORDCOMPLETION_PLUGIN (activatable)->priv;

	priv->window = gtk_widget_get_toplevel (GTK_WIDGET (priv->view));

	/* We are disposing the window */
	g_object_ref (priv->window);

	completion = gtk_source_view_get_completion (GTK_SOURCE_VIEW (priv->view));
	buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->view));

	provider = g_object_get_data (G_OBJECT (priv->window), WINDOW_PROVIDER);

	if (provider == NULL)
	{
		/* Standalone provider */
		provider = GTK_SOURCE_COMPLETION_PROVIDER (create_provider ());
	}

	priv->provider = g_object_ref (provider);

	gtk_source_completion_add_provider (completion, provider, NULL);
	gtk_source_completion_words_register (GTK_SOURCE_COMPLETION_WORDS (provider),
	                                      buf);
}

static void
xed_wordcompletion_view_deactivate (XedViewActivatable *activatable)
{
	XedWordCompletionPluginPrivate *priv;
	GtkSourceCompletion *completion;
	GtkSourceCompletionProvider *provider;
	GtkTextBuffer *buf;

	xed_debug (DEBUG_PLUGINS);

	priv = XED_WORDCOMPLETION_PLUGIN (activatable)->priv;

	completion = gtk_source_view_get_completion (GTK_SOURCE_VIEW (priv->view));
	buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->view));

	gtk_source_completion_remove_provider (completion,
	                                       priv->provider,
	                                       NULL);

	gtk_source_completion_words_unregister (GTK_SOURCE_COMPLETION_WORDS (priv->provider),
	                                        buf);
}

static void
dialog_response_cb (GtkWidget          *widget,
                    gint                response,
                    gpointer 						data)
{
	gtk_widget_destroy (widget);
}

static void
configure_widget_destroyed (GtkWidget *widget,
                            gpointer   data)
{
    WordCompletionConfigureWidget *conf_widget = (WordCompletionConfigureWidget *) data;

    xed_debug (DEBUG_PLUGINS);

    g_object_unref (conf_widget->settings);
    g_slice_free (WordCompletionConfigureWidget, data);

    xed_debug_message (DEBUG_PLUGINS, "END");
}

static WordCompletionConfigureWidget *
get_configure_widget (XedWordCompletionPlugin *plugin)
{
	XedWordCompletionPluginPrivate *priv;
	WordCompletionConfigureWidget *widget = NULL;
	gchar *data_dir;
	gchar *ui_file;
	GtkWidget *error_widget;
	gboolean ret;

	xed_debug (DEBUG_PLUGINS);

	priv = plugin->priv;
  
	widget = g_slice_new (WordCompletionConfigureWidget);
	widget->settings = g_object_ref (plugin->priv->settings);
  
	data_dir = peas_extension_base_get_data_dir (PEAS_EXTENSION_BASE (plugin));
	ui_file = g_build_filename (data_dir, "xed-wordcompletion-configure.ui", NULL);
	ret = xed_utils_get_ui_objects (ui_file,
                                  NULL,
                                  &error_widget,
                                  "configure_dialog", &widget->dialog,
                                  "spin_button_min_word_size", &widget->min_word_size,
                                  "check_button_interactive_completion", &widget->interactive_completion,
	                                NULL);
	  
	g_free (data_dir);
	g_free (ui_file);

	if (!ret)
  {
      return NULL;
  }
    
  gtk_window_set_modal (GTK_WINDOW (widget->dialog), TRUE);
  
	g_settings_bind (widget->settings, SETTINGS_KEY_INTERACTIVE_COMPLETION,
			 widget->interactive_completion, "active",
			 G_SETTINGS_BIND_DEFAULT | G_SETTINGS_BIND_GET_NO_CHANGES);

	g_settings_bind (widget->settings, SETTINGS_KEY_MINIMUM_WORD_SIZE,
			 widget->min_word_size, "value",
			 G_SETTINGS_BIND_DEFAULT | G_SETTINGS_BIND_GET_NO_CHANGES);

 	g_signal_connect (widget->dialog, "destroy",
                      G_CALLBACK (configure_widget_destroyed), widget);

	gtk_widget_show (GTK_WIDGET (widget->dialog));
	g_signal_connect (widget->dialog, "response",
                              G_CALLBACK (dialog_response_cb), widget);

	return widget;
}

static GtkWidget *
xed_wordcompletion_create_configure_widget (PeasGtkConfigurable *configurable)
{

	WordCompletionConfigureWidget *widget;

    widget = get_configure_widget (XED_WORDCOMPLETION_PLUGIN (configurable));

    return widget->dialog;
}

static void
xed_wordcompletion_plugin_class_init (XedWordCompletionPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = xed_wordcompletion_plugin_finalize;
	object_class->dispose = xed_wordcompletion_plugin_dispose;
	object_class->set_property = xed_wordcompletion_plugin_set_property;
	object_class->get_property = xed_wordcompletion_plugin_get_property;

	g_object_class_override_property (object_class, PROP_WINDOW, "window");
	g_object_class_override_property (object_class, PROP_VIEW, "view");

	g_type_class_add_private (klass, sizeof (XedWordCompletionPluginPrivate));
}

static void
xed_wordcompletion_plugin_class_finalize (XedWordCompletionPluginClass *klass)
{
}

static void
xed_window_activatable_iface_init (XedWindowActivatableInterface *iface)
{
	iface->activate = xed_wordcompletion_window_activate;
	iface->deactivate = xed_wordcompletion_window_deactivate;
}

static void
xed_view_activatable_iface_init (XedViewActivatableInterface *iface)
{
	iface->activate = xed_wordcompletion_view_activate;
	iface->deactivate = xed_wordcompletion_view_deactivate;
}

static void
peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface)
{
	iface->create_configure_widget = xed_wordcompletion_create_configure_widget;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	xed_wordcompletion_plugin_register_type (G_TYPE_MODULE (module));

	peas_object_module_register_extension_type (module,
	                                            XED_TYPE_WINDOW_ACTIVATABLE,
	                                            XED_TYPE_WORDCOMPLETION_PLUGIN);

	peas_object_module_register_extension_type (module,
	                                            XED_TYPE_VIEW_ACTIVATABLE,
	                                            XED_TYPE_WORDCOMPLETION_PLUGIN);

	peas_object_module_register_extension_type (module,
	                                            PEAS_GTK_TYPE_CONFIGURABLE,
	                                            XED_TYPE_WORDCOMPLETION_PLUGIN);
}
