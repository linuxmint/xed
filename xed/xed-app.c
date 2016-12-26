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

#include <glib/gi18n.h>

#include "xed-app.h"
#include "xed-prefs-manager-app.h"
#include "xed-commands.h"
#include "xed-notebook.h"
#include "xed-debug.h"
#include "xed-utils.h"
#include "xed-enum-types.h"
#include "xed-dirs.h"

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
    GList            *windows;
    XedWindow        *active_window;
    GtkPageSetup     *page_setup;
    GtkPrintSettings *print_settings;
};

G_DEFINE_TYPE(XedApp, xed_app, G_TYPE_OBJECT)

static void
xed_app_finalize (GObject *object)
{
    XedApp *app = XED_APP (object);

    g_list_free (app->priv->windows);

    if (app->priv->page_setup)
    {
        g_object_unref (app->priv->page_setup);
    }

    if (app->priv->print_settings)
    {
        g_object_unref (app->priv->print_settings);
    }

    G_OBJECT_CLASS (xed_app_parent_class)->finalize (object);
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
xed_app_class_init (XedAppClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = xed_app_finalize;
    object_class->get_property = xed_app_get_property;

    g_type_class_add_private (object_class, sizeof(XedAppPrivate));
}

static gboolean
ensure_user_config_dir (void)
{
    gchar *config_dir;
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

    g_free (config_dir);

    return ret;
}

static void
load_accels (void)
{
    gchar *filename;

    filename = xed_dirs_get_user_accels_file ();
    if (filename != NULL)
    {
        xed_debug_message (DEBUG_APP, "Loading keybindings from %s\n", filename);
        gtk_accel_map_load (filename);
        g_free (filename);
    }
}

static void
save_accels (void)
{
    gchar *filename;

    filename = xed_dirs_get_user_accels_file ();
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
    gchar *config_dir;
    gchar *setup = NULL;

    config_dir = xed_dirs_get_user_config_dir ();

    if (config_dir != NULL)
    {
        setup = g_build_filename (config_dir, XED_PAGE_SETUP_FILE, NULL);
        g_free (config_dir);
    }

    return setup;
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
    gchar *config_dir;
    gchar *settings = NULL;

    config_dir = xed_dirs_get_user_config_dir ();

    if (config_dir != NULL)
    {
        settings = g_build_filename (config_dir, XED_PRINT_SETTINGS_FILE, NULL);
        g_free (config_dir);
    }

    return settings;
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
xed_app_init (XedApp *app)
{
    app->priv = XED_APP_GET_PRIVATE (app);

    load_accels ();
}

static void
app_weak_notify (gpointer data,
                 GObject *where_the_app_was)
{
    gtk_main_quit ();
}

/**
 * xed_app_get_default:
 *
 * Returns the #XedApp object. This object is a singleton and
 * represents the running xed instance.
 *
 * Return value: (transfer none): the #XedApp pointer
 */
XedApp *
xed_app_get_default (void)
{
    static XedApp *app = NULL;

    if (app != NULL)
        return app;

    app = XED_APP (g_object_new (XED_TYPE_APP, NULL));

    g_object_add_weak_pointer (G_OBJECT (app), (gpointer) &app);
    g_object_weak_ref (G_OBJECT (app), app_weak_notify, NULL);

    return app;
}

static void
set_active_window (XedApp    *app,
                   XedWindow *window)
{
    app->priv->active_window = window;
}

static gboolean
window_focus_in_event (XedWindow     *window,
                       GdkEventFocus *event,
                       XedApp        *app)
{
    /* updates active_view and active_child when a new toplevel receives focus */
    g_return_val_if_fail (XED_IS_WINDOW (window), FALSE);

    set_active_window (app, window);

    return FALSE;
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

static void
window_destroy (XedWindow *window,
                XedApp    *app)
{
    app->priv->windows = g_list_remove (app->priv->windows,
                        window);

    if (window == app->priv->active_window)
    {
        set_active_window (app, app->priv->windows != NULL ? app->priv->windows->data : NULL);
    }

/* CHECK: I don't think we have to disconnect this function, since windows
   is being destroyed */
/*
    g_signal_handlers_disconnect_by_func (window,
                          G_CALLBACK (window_focus_in_event),
                          app);
    g_signal_handlers_disconnect_by_func (window,
                          G_CALLBACK (window_destroy),
                          app);
*/
    if (app->priv->windows == NULL)
    {
        /* Last window is gone... save some settings and exit */
        ensure_user_config_dir ();

        save_accels ();
        save_page_setup (app);
        save_print_settings (app);

        g_object_unref (app);
    }
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

    xed_debug (DEBUG_APP);

    /*
     * We need to be careful here, there is a race condition:
     * when another xed is launched it checks active_window,
     * so we must do our best to ensure that active_window
     * is never NULL when at least a window exists.
     */
    if (app->priv->windows == NULL)
    {
        window = g_object_new (XED_TYPE_WINDOW, NULL);
        set_active_window (app, window);
    }
    else
    {
        window = g_object_new (XED_TYPE_WINDOW, NULL);
    }

    app->priv->windows = g_list_prepend (app->priv->windows, window);

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

        state = xed_prefs_manager_get_window_state ();

        if ((state & GDK_WINDOW_STATE_MAXIMIZED) != 0)
        {
            xed_prefs_manager_get_default_window_size (&w, &h);
            gtk_window_set_default_size (GTK_WINDOW (window), w, h);
            gtk_window_maximize (GTK_WINDOW (window));
        }
        else
        {
            xed_prefs_manager_get_window_size (&w, &h);
            gtk_window_set_default_size (GTK_WINDOW (window), w, h);
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

    g_signal_connect (window, "focus_in_event", G_CALLBACK (window_focus_in_event), app);
    g_signal_connect (window, "delete_event", G_CALLBACK (window_delete_event), app);
    g_signal_connect (window, "destroy", G_CALLBACK (window_destroy), app);

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
 * xed_app_get_windows:
 * @app: the #XedApp
 *
 * Returns all the windows currently present in #XedApp.
 *
 * Return value: (element-type Xed.Window) (transfer none): the list of #XedWindows objects.
 * The list should not be freed
 */
const GList *
xed_app_get_windows (XedApp *app)
{
    g_return_val_if_fail (XED_IS_APP (app), NULL);

    return app->priv->windows;
}

/**
 * xed_app_get_active_window:
 * @app: the #XedApp
 *
 * Retrives the #XedWindow currently active.
 *
 * Return value: (transfer none): the active #XedWindow
 */
XedWindow *
xed_app_get_active_window (XedApp *app)
{
    g_return_val_if_fail (XED_IS_APP (app), NULL);

    /* make sure our active window is always realized:
     * this is needed on startup if we launch two xed fast
     * enough that the second instance comes up before the
     * first one shows its window.
     */
    if (!gtk_widget_get_realized (GTK_WIDGET (app->priv->active_window)))
    {
        gtk_widget_realize (GTK_WIDGET (app->priv->active_window));
    }

    return app->priv->active_window;
}

static gboolean
is_in_viewport (XedWindow *window,
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

    s = gtk_window_get_screen (GTK_WINDOW (window));
    display = gdk_screen_get_display (s);
    name = gdk_display_get_name (display);
    n = gdk_screen_get_number (s);

    if (strcmp (cur_name, name) != 0 || cur_n != n)
    {
        return FALSE;
    }

    /* Check for workspace match */
    ws = xed_utils_get_window_workspace (GTK_WINDOW (window));
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
           y  >= viewport_y &&
           y + height <= viewport_y + sc_height;
}

/**
 * _xed_app_get_window_in_viewport
 * @app: the #XedApp
 * @screen: the #GdkScreen
 * @workspace: the workspace number
 * @viewport_x: the viewport horizontal origin
 * @viewport_y: the viewport vertical origin
 *
 * Since a workspace can be larger than the screen, it is divided into several
 * equal parts called viewports. This function retrives the #XedWindow in
 * the given viewport of the given workspace.
 *
 * Return value: the #XedWindow in the given viewport of the given workspace.
 */
XedWindow *
_xed_app_get_window_in_viewport (XedApp    *app,
                                 GdkScreen *screen,
                                 gint       workspace,
                                 gint       viewport_x,
                                 gint       viewport_y)
{
    XedWindow *window;

    GList *l;

    g_return_val_if_fail (XED_IS_APP (app), NULL);

    /* first try if the active window */
    window = app->priv->active_window;

    g_return_val_if_fail (XED_IS_WINDOW (window), NULL);

    if (is_in_viewport (window, screen, workspace, viewport_x, viewport_y))
    {
        return window;
    }

    /* otherwise try to see if there is a window on this workspace */
    for (l = app->priv->windows; l != NULL; l = l->next)
    {
        window = l->data;

        if (is_in_viewport (window, screen, workspace, viewport_x, viewport_y))
        {
            return window;
        }
    }

    /* no window on this workspace... create a new one */
    return xed_app_create_window (app, screen);
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
    GList *windows;

    g_return_val_if_fail (XED_IS_APP (app), NULL);

    windows = app->priv->windows;

    while (windows != NULL)
    {
        res = g_list_concat (res, xed_window_get_documents (XED_WINDOW (windows->data)));
        windows = g_list_next (windows);
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
    GList *windows;

    g_return_val_if_fail (XED_IS_APP (app), NULL);

    windows = app->priv->windows;

    while (windows != NULL)
    {
        res = g_list_concat (res, xed_window_get_views (XED_WINDOW (windows->data)));
        windows = g_list_next (windows);
    }

    return res;
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

