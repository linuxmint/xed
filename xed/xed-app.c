/*
 * xed-app.c
 * This file is part of xed
 *
 * Copyright (C) 2005-2006 - Paolo Maggi
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/*
 * Modified by the xed Team, 2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <glib/gi18n.h>
#include <libpeas/peas-extension-set.h>
#include <gtksourceview/gtksource.h>

#ifdef ENABLE_INTROSPECTION
#include <girepository.h>
#endif

#include "xed-app.h"
#include "xed-commands.h"
#include "xed-notebook.h"
#include "xed-debug.h"
#include "xed-utils.h"
#include "xed-enum-types.h"
#include "xed-dirs.h"
#include "xed-app-activatable.h"
#include "xed-plugins-engine.h"
#include "xed-settings.h"

#ifndef ENABLE_GVFS_METADATA
#include "xed-metadata-manager.h"
#define METADATA_FILE "xed-metadata.xml"
#endif

#define XED_PAGE_SETUP_FILE     "xed-page-setup"
#define XED_PRINT_SETTINGS_FILE "xed-print-settings"

#define XED_APP_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), XED_TYPE_APP, XedAppPrivate))

/* Properties */
enum
{
    PROP_0,
};

struct _XedAppPrivate
{
    XedPluginsEngine *engine;

    GtkPageSetup *page_setup;
    GtkPrintSettings *print_settings;

    GObject *settings;
    GSettings *window_settings;

    PeasExtensionSet *extensions;

    /* command line parsing */
    gboolean new_window;
    gboolean new_document;
    gchar *geometry;
    const GtkSourceEncoding *encoding;
    GInputStream *stdin_stream;
    GSList *file_list;
    gint line_position;
    GApplicationCommandLine *command_line;
};

G_DEFINE_TYPE (XedApp, xed_app, GTK_TYPE_APPLICATION)

static const GOptionEntry options[] =
{
    /* Version */
    {
        "version", 'V', 0, G_OPTION_ARG_NONE, NULL,
        N_("Show the application's version"), NULL
    },

    /* List available encodings */
    {
        "list-encodings", '\0', 0, G_OPTION_ARG_NONE, NULL,
        N_("Display list of possible values for the encoding option"),
        NULL
    },

    /* Encoding */
    {
        "encoding", '\0', 0, G_OPTION_ARG_STRING, NULL,
        N_("Set the character encoding to be used to open the files listed on the command line"),
        N_("ENCODING")
    },

    /* Open a new window */
    {
        "new-window", '\0', 0, G_OPTION_ARG_NONE, NULL,
        N_("Create a new top-level window in an existing instance of xed"),
        NULL
    },

    /* Create a new empty document */
    {
        "new-document", '\0', 0, G_OPTION_ARG_NONE, NULL,
        N_("Create a new document in an existing instance of xed"),
        NULL
    },

    /* Window geometry */
    {
        "geometry", 'g', 0, G_OPTION_ARG_STRING, NULL,
        N_("Set the size and position of the window (WIDTHxHEIGHT+X+Y)"),
        N_("GEOMETRY")
    },

    /* Wait for closing documents */
    {
        "wait", 'w', 0, G_OPTION_ARG_NONE, NULL,
        N_("Open files and block process until files are closed"),
        NULL
    },

    /* New instance */
    {
        "standalone", 's', 0, G_OPTION_ARG_NONE, NULL,
        N_("Run xed in standalone mode"),
        NULL
    },

    /* collects file arguments */
    {
        G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_FILENAME_ARRAY, NULL, NULL,
        N_("[FILE...] [+LINE]")
    },

    {NULL}
};

static void
xed_app_dispose (GObject *object)
{
    XedApp *app = XED_APP (object);

    g_clear_object (&app->priv->window_settings);
    g_clear_object (&app->priv->settings);
    g_clear_object (&app->priv->page_setup);
    g_clear_object (&app->priv->print_settings);
    g_clear_object (&app->priv->extensions);
    g_clear_object (&app->priv->engine);

    G_OBJECT_CLASS (xed_app_parent_class)->dispose (object);
}

static void
xed_app_get_property (GObject    *object,
                      guint       prop_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
    switch (prop_id)
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
extension_added (PeasExtensionSet *extensions,
                 PeasPluginInfo   *info,
                 PeasExtension    *exten,
                 XedApp           *app)
{
    peas_extension_call (exten, "activate");
}

static void
extension_removed (PeasExtensionSet *extensions,
                   PeasPluginInfo   *info,
                   PeasExtension    *exten,
                   XedApp           *app)
{
    peas_extension_call (exten, "deactivate");
}

static void
xed_app_startup (GApplication *application)
{
    XedApp *app = XED_APP (application);
    GtkSourceStyleSchemeManager *manager;
    const gchar *dir;
    gchar *icon_dir;
#ifndef ENABLE_GVFS_METADATA
    const gchar *cache_dir;
    gchar *metadata_filename;
#endif

    G_APPLICATION_CLASS (xed_app_parent_class)->startup (application);

    /* Setup debugging */
    xed_debug_init ();
    xed_debug_message (DEBUG_APP, "Startup");
    xed_debug_message (DEBUG_APP, "Set icon");

    dir = xed_dirs_get_xed_data_dir ();
    icon_dir = g_build_filename (dir, "icons", NULL);

    gtk_icon_theme_append_search_path (gtk_icon_theme_get_default (), icon_dir);
    g_free (icon_dir);

#ifndef ENABLE_GVFS_METADATA
    /* Setup metadata-manager */
    cache_dir = xed_dirs_get_user_cache_dir ();

    metadata_filename = g_build_filename (cache_dir, METADATA_FILE, NULL);

    xed_metadata_manager_init (metadata_filename);

    g_free (metadata_filename);
#endif

   /* Load settings */
   app->priv->settings = xed_settings_new ();
   app->priv->window_settings = g_settings_new ("org.x.editor.state.window");

   /*
    * We use the default gtksourceview style scheme manager so that plugins
    * can obtain it easily without a xed specific api, but we need to
    * add our search path at startup before the manager is actually used.
    */
   manager = gtk_source_style_scheme_manager_get_default ();
   gtk_source_style_scheme_manager_append_search_path (manager, xed_dirs_get_user_styles_dir ());

   app->priv->engine = xed_plugins_engine_get_default ();
   app->priv->extensions = peas_extension_set_new (PEAS_ENGINE (app->priv->engine),
                                                   XED_TYPE_APP_ACTIVATABLE,
                                                   "app", app,
                                                   NULL);

   g_signal_connect (app->priv->extensions, "extension-added",
                     G_CALLBACK (extension_added), app);

   g_signal_connect (app->priv->extensions, "extension-removed",
                     G_CALLBACK (extension_removed), app);

   peas_extension_set_foreach (app->priv->extensions,
                               (PeasExtensionSetForeachFunc) extension_added,
                               app);
}

static gboolean
is_in_viewport (GtkWindow *window,
                GdkScreen *screen,
                gint       workspace,
                gint       viewport_x,
                gint       viewport_y)
{
    GdkScreen *s;
    GdkDisplay *display;
    GdkWindow *gdkwindow;
    const gchar *cur_name;
    const gchar *name;
    gint cur_n;
    gint n;
    gint ws;
    gint sc_width, sc_height;
    gint x, y, width, height;
    gint vp_x, vp_y;

    /* Check for screen and display match */
    display = gdk_screen_get_display (screen);
    cur_name = gdk_display_get_name (display);
    cur_n = gdk_screen_get_number (screen);

    s = gtk_window_get_screen (window);
    display = gdk_screen_get_display (s);
    name = gdk_display_get_name (display);
    n = gdk_screen_get_number (s);

    if (strcmp (cur_name, name) != 0 || cur_n != n)
    {
       return FALSE;
    }

    /* Check for workspace match */
    ws = xed_utils_get_window_workspace (window);
    if (ws != workspace && ws != XED_ALL_WORKSPACES)
    {
       return FALSE;
    }

    /* Check for viewport match */
    gdkwindow = gtk_widget_get_window (GTK_WIDGET (window));
    gdk_window_get_position (gdkwindow, &x, &y);
    width = gdk_window_get_width (gdkwindow);
    height = gdk_window_get_height (gdkwindow);
    xed_utils_get_current_viewport (screen, &vp_x, &vp_y);
    x += vp_x;
    y += vp_y;

    sc_width = gdk_screen_get_width (screen);
    sc_height = gdk_screen_get_height (screen);

    return x + width * .25 >= viewport_x &&
           x + width * .75 <= viewport_x + sc_width &&
           y >= viewport_y &&
           y + height <= viewport_y + sc_height;
}

static XedWindow *
get_active_window (GtkApplication *app)
{
    GdkScreen *screen;
    guint workspace;
    gint viewport_x, viewport_y;
    GList *windows, *l;

    screen = gdk_screen_get_default ();

    workspace = xed_utils_get_current_workspace (screen);
    xed_utils_get_current_viewport (screen, &viewport_x, &viewport_y);

    /* Gtk documentation says the window list is always in MRU order */
    windows = gtk_application_get_windows (app);
    for (l = windows; l != NULL; l = l->next)
    {
       GtkWindow *window = l->data;

        if (is_in_viewport (window, screen, workspace, viewport_x, viewport_y))
        {
            return XED_WINDOW (window);
        }
    }

   return NULL;
}

static void
set_command_line_wait (XedApp *app,
                       XedTab *tab)
{
    g_object_set_data_full (G_OBJECT (tab),
                            "XedTabCommandLineWait",
                            g_object_ref (app->priv->command_line),
                            (GDestroyNotify)g_object_unref);
}

static void
open_files (GApplication            *application,
            gboolean                 new_window,
            gboolean                 new_document,
            gchar                   *geometry,
            gint                     line_position,
            const GtkSourceEncoding *encoding,
            GInputStream            *stdin_stream,
            GSList                  *file_list,
            GApplicationCommandLine *command_line)
{
    XedWindow *window = NULL;
    XedTab *tab;
    gboolean doc_created = FALSE;

    if (!new_window)
    {
        window = get_active_window (GTK_APPLICATION (application));
    }

    if (window == NULL)
    {
        xed_debug_message (DEBUG_APP, "Create main window");
        window = xed_app_create_window (XED_APP (application), NULL);

        xed_debug_message (DEBUG_APP, "Show window");
        gtk_widget_show (GTK_WIDGET (window));
    }

    if (geometry)
    {
        gtk_window_parse_geometry (GTK_WINDOW (window), geometry);
    }

    if (stdin_stream)
    {
        xed_debug_message (DEBUG_APP, "Load stdin");

        tab = xed_window_create_tab_from_stream (window,
                                                 stdin_stream,
                                                 encoding,
                                                 line_position,
                                                 TRUE);
        doc_created = tab != NULL;

        if (doc_created && command_line)
        {
            set_command_line_wait (XED_APP (application), tab);
        }
        g_input_stream_close (stdin_stream, NULL, NULL);
    }

    if (file_list != NULL)
    {
        GSList *loaded;

        xed_debug_message (DEBUG_APP, "Load files");
        loaded = _xed_cmd_load_files_from_prompt (window, file_list, encoding, line_position);

        doc_created = doc_created || loaded != NULL;

        if (command_line)
        {
            g_slist_foreach (loaded, (GFunc)set_command_line_wait, NULL);
        }
        g_slist_free (loaded);
    }

    if (!doc_created || new_document)
    {
        xed_debug_message (DEBUG_APP, "Create tab");
        tab = xed_window_create_tab (window, TRUE);

        if (command_line)
        {
            set_command_line_wait (XED_APP (application), tab);
        }
    }

    gtk_window_present (GTK_WINDOW (window));
}

static void
xed_app_activate (GApplication *application)
{
    XedAppPrivate *priv = XED_APP (application)->priv;

    open_files (application,
                priv->new_window,
                priv->new_document,
                priv->geometry,
                priv->line_position,
                priv->encoding,
                priv->stdin_stream,
                priv->file_list,
                priv->command_line);
}

static void
clear_options (XedApp *app)
{
    XedAppPrivate *priv = app->priv;

    g_free (priv->geometry);
    g_clear_object (&priv->stdin_stream);
    g_slist_free_full (priv->file_list, g_object_unref);

    priv->new_window = FALSE;
    priv->new_document = FALSE;
    priv->geometry = NULL;
    priv->encoding = NULL;
    priv->file_list = NULL;
    priv->line_position = 0;
    priv->column_position = 0;
    priv->command_line = NULL;
}

static void
get_line_position (const gchar *arg,
                   gint        *line)
{
    *line = atoi (arg);
}

static gint
xed_app_command_line (GApplication            *application,
                      GApplicationCommandLine *cl)
{
    XedAppPrivate *priv;
    GVariantDict *options;
    const gchar *encoding_charset;
    const gchar **remaining_args;

    priv = XED_APP (application)->priv;

    options = g_application_command_line_get_options_dict (cl);

    g_variant_dict_lookup (options, "new-window", "b", &priv->new_window);
    g_variant_dict_lookup (options, "new-document", "b", &priv->new_document);
    g_variant_dict_lookup (options, "geometry", "s", &priv->geometry);

    if (g_variant_dict_contains (options, "wait"))
    {
        priv->command_line = cl;
    }

    if (g_variant_dict_lookup (options, "encoding", "&s", &encoding_charset))
    {
        priv->encoding = gtk_source_encoding_get_from_charset (encoding_charset);

        if (priv->encoding == NULL)
        {
            g_application_command_line_printerr (cl, _("%s: invalid encoding."), encoding_charset);
        }
    }

    /* Parse filenames */
    if (g_variant_dict_lookup (options, G_OPTION_REMAINING, "^a&ay", &remaining_args))
    {
        gint i;

        for (i = 0; remaining_args[i]; i++)
        {
            if (*remaining_args[i] == '+')
            {
                if (*(remaining_args[i] + 1) == '\0')
                {
                    /* goto the last line of the document */
                    priv->line_position = G_MAXINT;
                }
                else
                {
                    get_line_position (remaining_args[i] + 1, &priv->line_position);
                }
            }

            else if (*remaining_args[i] == '-' && *(remaining_args[i] + 1) == '\0')
            {
                priv->stdin_stream = g_application_command_line_get_stdin (cl);
            }
            else
            {
                GFile *file;

                file = g_application_command_line_create_file_for_arg (cl, remaining_args[i]);
                priv->file_list = g_slist_prepend (priv->file_list, file);
            }
        }

        priv->file_list = g_slist_reverse (priv->file_list);
        g_free (remaining_args);
    }

    g_application_activate (application);
    clear_options (XED_APP (application));

    return 0;
}

static void
print_all_encodings (void)
{
    GSList *all_encodings;
    GSList *l;

    all_encodings = gtk_source_encoding_get_all ();

    for (l = all_encodings; l != NULL; l = l->next)
    {
        const GtkSourceEncoding *encoding = l->data;
        g_print ("%s\n", gtk_source_encoding_get_charset (encoding));
    }

    g_slist_free (all_encodings);
}

static gint
xed_app_handle_local_options (GApplication *application,
                              GVariantDict *options)
{
    if (g_variant_dict_contains (options, "version"))
    {
        g_print ("%s - Version %s\n", g_get_application_name (), VERSION);
        return 0;
    }

    if (g_variant_dict_contains (options, "list-encodings"))
    {
        print_all_encodings ();
        return 0;
    }

    if (g_variant_dict_contains (options, "standalone"))
    {
        GApplicationFlags old_flags;

        old_flags = g_application_get_flags (application);
        g_application_set_flags (application, old_flags | G_APPLICATION_NON_UNIQUE);
    }

    if (g_variant_dict_contains (options, "wait"))
    {
        GApplicationFlags old_flags;

        old_flags = g_application_get_flags (application);
        g_application_set_flags (application, old_flags | G_APPLICATION_IS_LAUNCHER);
    }

    return -1;
}

/* Note: when launched from command line we do not reach this method
 * since we manually handle the command line parameters in order to
 * parse +LINE, stdin, etc.
 * However this method is called when open() is called via dbus, for
 * instance when double clicking on a file in nautilus
 */
static void
xed_app_open (GApplication  *application,
              GFile        **files,
              gint           n_files,
              const gchar   *hint)
{
    gint i;
    GSList *file_list = NULL;

    for (i = 0; i < n_files; i++)
    {
        file_list = g_slist_prepend (file_list, files[i]);
    }

    file_list = g_slist_reverse (file_list);

    open_files (application,
                FALSE,
                FALSE,
                NULL,
                0,
                NULL,
                NULL,
                file_list,
                NULL);

    g_slist_free (file_list);
}

static gboolean
ensure_user_config_dir (void)
{
    const gchar *config_dir;
    gboolean ret = TRUE;
    gint res;

    config_dir = xed_dirs_get_user_config_dir ();
    if (config_dir == NULL)
    {
        g_warning ("Could not get config directory\n");
        return FALSE;
    }

    res = g_mkdir_with_parents (config_dir, 0755);
    if (res < 0)
    {
        g_warning ("Could not create config directory\n");
        ret = FALSE;
    }

    return ret;
}

static void
save_accels (void)
{
    gchar *filename;

    filename = g_build_filename (xed_dirs_get_user_config_dir (), "accels", NULL);
    if (filename != NULL)
    {
        xed_debug_message (DEBUG_APP, "Saving keybindings in %s\n", filename);
        gtk_accel_map_save (filename);
        g_free (filename);
    }
}

static gchar *
get_page_setup_file (void)
{
    const gchar *config_dir;
    gchar *setup = NULL;

    config_dir = xed_dirs_get_user_config_dir ();

    if (config_dir != NULL)
    {
        setup = g_build_filename (config_dir, XED_PAGE_SETUP_FILE, NULL);
    }

    return setup;
}

static void
save_page_setup (XedApp *app)
{
    gchar *filename;
    GError *error = NULL;

    if (app->priv->page_setup == NULL)
    {
        return;
    }

    filename = get_page_setup_file ();

    gtk_page_setup_to_file (app->priv->page_setup, filename, &error);
    if (error)
    {
        g_warning ("%s", error->message);
        g_error_free (error);
    }

    g_free (filename);
}

static gchar *
get_print_settings_file (void)
{
    const gchar *config_dir;
    gchar *settings = NULL;

    config_dir = xed_dirs_get_user_config_dir ();

    if (config_dir != NULL)
    {
        settings = g_build_filename (config_dir, XED_PRINT_SETTINGS_FILE, NULL);
    }

    return settings;
}

static void
save_print_settings (XedApp *app)
{
    gchar *filename;
    GError *error = NULL;

    if (app->priv->print_settings == NULL)
    {
        return;
    }

    filename = get_print_settings_file ();

    gtk_print_settings_to_file (app->priv->print_settings, filename, &error);
    if (error)
    {
        g_warning ("%s", error->message);
        g_error_free (error);
    }

    g_free (filename);
}

static void
xed_app_shutdown (GApplication *app)
{
    xed_debug_message (DEBUG_APP, "Quitting\n");

    /* Last window is gone... save some settings and exit */
    ensure_user_config_dir ();

    save_accels ();
    save_page_setup (XED_APP (app));
    save_print_settings (XED_APP (app));

    /* GTK+ can still hold references to some xed objects, for example
     * XedDocument for the clipboard. So the metadata-manager should be
     * shutdown after.
     */
    G_APPLICATION_CLASS (xed_app_parent_class)->shutdown (app);

#ifndef ENABLE_GVFS_METADATA
    xed_metadata_manager_shutdown ();
#endif

    xed_dirs_shutdown ();
}

static void
xed_app_class_init (XedAppClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GApplicationClass *app_class =  G_APPLICATION_CLASS (klass);

    object_class->dispose = xed_app_dispose;
    object_class->get_property = xed_app_get_property;

    app_class->startup = xed_app_startup;
    app_class->activate = xed_app_activate;
    app_class->command_line = xed_app_command_line;
    app_class->handle_local_options = xed_app_handle_local_options;
    app_class->open = xed_app_open;
    app_class->shutdown = xed_app_shutdown;

    g_type_class_add_private (object_class, sizeof (XedAppPrivate));
}

static void
load_accels (void)
{
    gchar *filename;

    filename = g_build_filename (xed_dirs_get_user_config_dir (), "accels", NULL);
    if (filename != NULL)
    {
        xed_debug_message (DEBUG_APP, "Loading keybindings from %s\n", filename);
        gtk_accel_map_load (filename);
        g_free (filename);
    }
}

static void
load_page_setup (XedApp *app)
{
    gchar *filename;
    GError *error = NULL;

    g_return_if_fail (app->priv->page_setup == NULL);

    filename = get_page_setup_file ();

    app->priv->page_setup = gtk_page_setup_new_from_file (filename, &error);
    if (error)
    {
        /* Ignore file not found error */
        if (error->domain != G_FILE_ERROR || error->code != G_FILE_ERROR_NOENT)
        {
            g_warning ("%s", error->message);
        }

        g_error_free (error);
    }

    g_free (filename);

    /* fall back to default settings */
    if (app->priv->page_setup == NULL)
    {
        app->priv->page_setup = gtk_page_setup_new ();
    }
}

static void
load_print_settings (XedApp *app)
{
    gchar *filename;
    GError *error = NULL;

    g_return_if_fail (app->priv->print_settings == NULL);

    filename = get_print_settings_file ();

    app->priv->print_settings = gtk_print_settings_new_from_file (filename, &error);
    if (error)
    {
        /* Ignore file not found error */
        if (error->domain != G_FILE_ERROR || error->code != G_FILE_ERROR_NOENT)
        {
            g_warning ("%s", error->message);
        }

        g_error_free (error);
    }

    g_free (filename);

    /* fall back to default settings */
    if (app->priv->print_settings == NULL)
    {
        app->priv->print_settings = gtk_print_settings_new ();
    }
}

static void
xed_app_init (XedApp *app)
{
    app->priv = XED_APP_GET_PRIVATE (app);

    g_set_application_name ("xed");
    gtk_window_set_default_icon_name ("accessories-text-editor");

    g_application_add_main_option_entries (G_APPLICATION (app), options);

#ifdef ENABLE_INTROSPECTION
    g_application_add_option_group (G_APPLICATION (app), g_irepository_get_option_group ());
#endif

    load_accels ();
}

static gboolean
window_delete_event (XedWindow *window,
                     GdkEvent  *event,
                     XedApp    *app)
{
    XedWindowState ws;

    ws = xed_window_get_state (window);

    if (ws &
        (XED_WINDOW_STATE_SAVING |
         XED_WINDOW_STATE_PRINTING |
         XED_WINDOW_STATE_SAVING_SESSION))
    {
            return TRUE;
    }

    _xed_cmd_file_quit (NULL, window);

    /* Do not destroy the window */
    return TRUE;
}

/* Generates a unique string for a window role */
static gchar *
gen_role (void)
{
    GTimeVal result;
    static gint serial;

    g_get_current_time (&result);

    return g_strdup_printf ("xed-window-%ld-%ld-%d-%s",
                            result.tv_sec,
                            result.tv_usec,
                            serial++,
                            g_get_host_name ());
}

static XedWindow *
xed_app_create_window_real (XedApp      *app,
                            gboolean     set_geometry,
                            const gchar *role)
{
    XedWindow *window;

    window = g_object_new (XED_TYPE_WINDOW, "application", app, NULL);

    xed_debug_message (DEBUG_APP, "Window created");

    if (role != NULL)
    {
        gtk_window_set_role (GTK_WINDOW (window), role);
    }
    else
    {
        gchar *newrole;

        newrole = gen_role ();
        gtk_window_set_role (GTK_WINDOW (window), newrole);
        g_free (newrole);
    }

    if (set_geometry)
    {
        GdkWindowState state;
        gint w, h;

        state = g_settings_get_int (app->priv->window_settings, XED_SETTINGS_WINDOW_STATE);
        g_settings_get (app->priv->window_settings, XED_SETTINGS_WINDOW_SIZE, "(ii)", &w, &h);
        gtk_window_set_default_size (GTK_WINDOW (window), w, h);

        if ((state & GDK_WINDOW_STATE_MAXIMIZED) != 0)
        {
            gtk_window_maximize (GTK_WINDOW (window));
        }
        else
        {
            gtk_window_unmaximize (GTK_WINDOW (window));
        }

        if ((state & GDK_WINDOW_STATE_STICKY ) != 0)
        {
            gtk_window_stick (GTK_WINDOW (window));
        }
        else
        {
            gtk_window_unstick (GTK_WINDOW (window));
        }
    }

    g_signal_connect (window, "delete_event", G_CALLBACK (window_delete_event), app);

    return window;
}

/**
 * xed_app_create_window:
 * @app: the #XedApp
 * @screen: (allow-none):
 *
 * Create a new #XedWindow part of @app.
 *
 * Return value: (transfer none): the new #XedWindow
 */
XedWindow *
xed_app_create_window (XedApp    *app,
                       GdkScreen *screen)
{
    XedWindow *window;

    window = xed_app_create_window_real (app, TRUE, NULL);

    if (screen != NULL)
    {
        gtk_window_set_screen (GTK_WINDOW (window), screen);
    }

    return window;
}

/*
 * Same as _create_window, but doesn't set the geometry.
 * The session manager takes care of it. Used in mate-session.
 */
XedWindow *
_xed_app_restore_window (XedApp *app,
                         const   gchar *role)
{
    XedWindow *window;

    window = xed_app_create_window_real (app, FALSE, role);

    return window;
}

/**
 * xed_app_get_main_windows:
 * @app: the #GeditApp
 *
 * Returns all #XedWindows currently open in #XedApp.
 * This differs from gtk_application_get_windows() since it does not
 * include the preferences dialog and other auxiliary windows.
 *
 * Return value: (element-type Xed.Window) (transfer container):
 * a newly allocated list of #XedWindow objects
 */
GList *
xed_app_get_main_windows (XedApp *app)
{
    GList *res = NULL;
    GList *windows, *l;

    g_return_val_if_fail (XED_IS_APP (app), NULL);

    windows = gtk_application_get_windows (GTK_APPLICATION (app));
    for (l = windows; l != NULL; l = g_list_next (l))
    {
        if (XED_IS_WINDOW (l->data))
        {
            res = g_list_prepend (res, l->data);
        }
    }

    return g_list_reverse (res);
}

/**
 * xed_app_get_documents:
 * @app: the #XedApp
 *
 * Returns all the documents currently open in #XedApp.
 *
 * Return value: (element-type Xed.Document) (transfer container):
 * a newly allocated list of #XedDocument objects
 */
GList *
xed_app_get_documents   (XedApp *app)
{
    GList *res = NULL;
    GList *windows, *l;

    g_return_val_if_fail (XED_IS_APP (app), NULL);

    windows = gtk_application_get_windows (GTK_APPLICATION (app));
    for (l = windows; l != NULL; l = g_list_next (l))
    {
        res = g_list_concat (res, xed_window_get_documents (XED_WINDOW (l->data)));
    }

    return res;
}

/**
 * xed_app_get_views:
 * @app: the #XedApp
 *
 * Returns all the views currently present in #XedApp.
 *
 * Return value: (element-type Xed.View) (transfer container):
 * a newly allocated list of #XedView objects
 */
GList *
xed_app_get_views (XedApp *app)
{
    GList *res = NULL;
    GList *windows, *l;

    g_return_val_if_fail (XED_IS_APP (app), NULL);

    windows = gtk_application_get_windows (GTK_APPLICATION (app));
    for (l = windows; l != NULL; l = g_list_next (l))
    {
        res = g_list_concat (res, xed_window_get_views (XED_WINDOW (l->data)));
    }

    return res;
}

gboolean
xed_app_show_help (XedApp      *app,
                   GtkWindow   *parent,
                   const gchar *name,
                   const gchar *link_id)
{
    g_return_val_if_fail (XED_IS_APP (app), FALSE);
    g_return_val_if_fail (parent == NULL || GTK_IS_WINDOW (parent), FALSE);

    GError *error = NULL;
    gboolean ret;
    gchar *link;

    if (name == NULL)
    {
        name = "xed";
    }
    else if (strcmp (name, "xed.xml") == 0)
    {
        g_warning ("%s: Using \"xed.xml\" for the help name is deprecated, use \"xed\" or simply NULL instead", G_STRFUNC);
        name = "xed";
    }

    if (link_id)
    {
        link = g_strdup_printf ("help:%s/%s", name, link_id);
    }
    else
    {
        link = g_strdup_printf ("help:%s", name);
    }

    ret = gtk_show_uri (gtk_widget_get_screen (GTK_WIDGET (parent)), link, GDK_CURRENT_TIME, &error);

    g_free (link);

    if (error != NULL)
    {
        GtkWidget *dialog;

        dialog = gtk_message_dialog_new (parent,
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_MESSAGE_ERROR,
                                         GTK_BUTTONS_CLOSE,
                                         _("There was an error displaying the help."));

        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s", error->message);

        g_signal_connect (G_OBJECT (dialog), "response",
                          G_CALLBACK (gtk_widget_destroy), NULL);

        gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

        gtk_widget_show (dialog);

        g_error_free (error);
    }

    return ret;
}

void
xed_app_set_window_title (XedApp      *app,
                          XedWindow   *window,
                          const gchar *title)
{
   gtk_window_set_title (GTK_WINDOW (window), title);
}

/* Returns a copy */
GtkPageSetup *
_xed_app_get_default_page_setup (XedApp *app)
{
    g_return_val_if_fail (XED_IS_APP (app), NULL);

    if (app->priv->page_setup == NULL)
    {
        load_page_setup (app);
    }

    return gtk_page_setup_copy (app->priv->page_setup);
}

void
_xed_app_set_default_page_setup (XedApp       *app,
                                 GtkPageSetup *page_setup)
{
    g_return_if_fail (XED_IS_APP (app));
    g_return_if_fail (GTK_IS_PAGE_SETUP (page_setup));

    if (app->priv->page_setup != NULL)
    {
        g_object_unref (app->priv->page_setup);
    }

    app->priv->page_setup = g_object_ref (page_setup);
}

/* Returns a copy */
GtkPrintSettings *
_xed_app_get_default_print_settings (XedApp *app)
{
    g_return_val_if_fail (XED_IS_APP (app), NULL);

    if (app->priv->print_settings == NULL)
    {
        load_print_settings (app);
    }

    return gtk_print_settings_copy (app->priv->print_settings);
}

void
_xed_app_set_default_print_settings (XedApp           *app,
                                     GtkPrintSettings *settings)
{
    g_return_if_fail (XED_IS_APP (app));
    g_return_if_fail (GTK_IS_PRINT_SETTINGS (settings));

    if (app->priv->print_settings != NULL)
    {
        g_object_unref (app->priv->print_settings);
    }

    app->priv->print_settings = g_object_ref (settings);
}

GObject *
_xed_app_get_settings (XedApp *app)
{
    g_return_val_if_fail (XED_IS_APP (app), NULL);

    return app->priv->settings;
}
