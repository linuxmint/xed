/*
 * xed-spell-plugin.c
 *
 * Copyright (C) 2002-2005 Paolo Maggi
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

#include <config.h>

#include "xed-spell-plugin.h"

#include <string.h> /* For strlen */

#include <glib/gi18n-lib.h>
#include <libpeas-gtk/peas-gtk-configurable.h>

#include <xed/xed-app.h>
#include <xed/xed-window.h>
#include <xed/xed-window-activatable.h>
#include <xed/xed-debug.h>
#include <xed/xed-utils.h>
#include <gspell/gspell.h>

#define XED_METADATA_ATTRIBUTE_SPELL_LANGUAGE "metadata::xed-spell-language"
#define XED_METADATA_ATTRIBUTE_SPELL_ENABLED  "metadata::xed-spell-enabled"

#define SPELL_ENABLED_STR "1"

#define MENU_PATH "/MenuBar/ToolsMenu/ToolsOps_1"

/* GSettings keys */
#define SPELL_SCHEMA        "org.x.editor.plugins.spell"
#define AUTOCHECK_TYPE_KEY  "autocheck-type"

static void xed_window_activatable_iface_init (XedWindowActivatableInterface *iface);
static void peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface);

struct _XedSpellPluginPrivate
{
    XedWindow *window;

    GtkActionGroup *action_group;
    guint           ui_id;

    GSettings *settings;
};

enum
{
   PROP_0,
   PROP_WINDOW
};

G_DEFINE_DYNAMIC_TYPE_EXTENDED (XedSpellPlugin,
                                xed_spell_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (XED_TYPE_WINDOW_ACTIVATABLE,
                                                               xed_window_activatable_iface_init)
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_GTK_TYPE_CONFIGURABLE,
                                                               peas_gtk_configurable_iface_init)
                                G_ADD_PRIVATE_DYNAMIC (XedSpellPlugin))

static void check_spell_cb (GtkAction      *action,
                            XedSpellPlugin *plugin);
static void set_language_cb (GtkAction      *action,
                             XedSpellPlugin *plugin);
static void inline_checker_cb (GtkAction      *action,
                               XedSpellPlugin *plugin);

/* UI actions. */
static const GtkActionEntry action_entries[] =
{
    { "CheckSpell",
      "tools-check-spelling-symbolic",
      N_("_Check Spelling..."),
      "<shift>F7",
      N_("Check the current document for incorrect spelling"),
      G_CALLBACK (check_spell_cb)
    },

    { "ConfigSpell",
      NULL,
      N_("Set _Language..."),
      NULL,
      N_("Set the language of the current document"),
      G_CALLBACK (set_language_cb)
    }
};

static const GtkToggleActionEntry toggle_action_entries[] =
{
    { "InlineSpellChecker",
      NULL,
      N_("_Autocheck Spelling"),
      NULL,
      N_("Automatically spell-check the current document"),
      G_CALLBACK (inline_checker_cb),
      FALSE
    }
};

typedef struct _SpellConfigureWidget SpellConfigureWidget;

struct _SpellConfigureWidget
{
    GtkWidget *content;

    GtkWidget *never;
    GtkWidget *always;
    GtkWidget *document;

    GSettings *settings;
};

typedef enum
{
    AUTOCHECK_NEVER = 0,
    AUTOCHECK_DOCUMENT,
    AUTOCHECK_ALWAYS
} XedSpellPluginAutocheckType;

static void
xed_spell_plugin_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
    XedSpellPlugin *plugin = XED_SPELL_PLUGIN (object);

    switch (prop_id)
    {
        case PROP_WINDOW:
            plugin->priv->window = XED_WINDOW (g_value_dup_object (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_spell_plugin_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
    XedSpellPlugin *plugin = XED_SPELL_PLUGIN (object);

    switch (prop_id)
    {
        case PROP_WINDOW:
            g_value_set_object (value, plugin->priv->window);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_spell_plugin_dispose (GObject *object)
{
    XedSpellPlugin *plugin = XED_SPELL_PLUGIN (object);

    xed_debug_message (DEBUG_PLUGINS, "XedSpellPlugin disposing");

    g_clear_object (&plugin->priv->settings);
    g_clear_object (&plugin->priv->window);
    g_clear_object (&plugin->priv->action_group);
    g_clear_object (&plugin->priv->settings);

    G_OBJECT_CLASS (xed_spell_plugin_parent_class)->dispose (object);
}

static void
xed_spell_plugin_class_init (XedSpellPluginClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = xed_spell_plugin_dispose;
    object_class->set_property = xed_spell_plugin_set_property;
    object_class->get_property = xed_spell_plugin_get_property;

    g_object_class_override_property (object_class, PROP_WINDOW, "window");
}

static void
xed_spell_plugin_class_finalize (XedSpellPluginClass *klass)
{
}

static void
xed_spell_plugin_init (XedSpellPlugin *plugin)
{
    xed_debug_message (DEBUG_PLUGINS, "XedSpellPlugin initializing");

    plugin->priv = xed_spell_plugin_get_instance_private (plugin);

    plugin->priv->settings = g_settings_new (SPELL_SCHEMA);
}

static GspellChecker *
get_spell_checker (XedDocument *doc)
{
    GspellTextBuffer *gspell_buffer;

    gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (GTK_TEXT_BUFFER (doc));
    return gspell_text_buffer_get_spell_checker (gspell_buffer);
}

static const GspellLanguage *
get_language_from_metadata (XedDocument *doc)
{
    const GspellLanguage *lang = NULL;
    gchar *language_code = NULL;

    language_code = xed_document_get_metadata (doc, XED_METADATA_ATTRIBUTE_SPELL_LANGUAGE);

    if (language_code != NULL)
    {
        lang = gspell_language_lookup (language_code);
        g_free (language_code);
    }

    return lang;
}

static void
check_spell_cb (GtkAction      *action,
                XedSpellPlugin *plugin)
{
    XedSpellPluginPrivate *priv;
    XedView *view;
    GspellNavigator *navigator;
    GtkWidget *dialog;

    xed_debug (DEBUG_PLUGINS);

    priv = plugin->priv;

    view = xed_window_get_active_view (priv->window);
    g_return_if_fail (view != NULL);

    navigator = gspell_navigator_text_view_new (GTK_TEXT_VIEW (view));
    dialog = gspell_checker_dialog_new (GTK_WINDOW (priv->window), navigator);

    gtk_widget_show (dialog);
}

static XedSpellPluginAutocheckType
get_autocheck_type (XedSpellPlugin *plugin)
{
    XedSpellPluginAutocheckType autocheck_type;

    autocheck_type = g_settings_get_enum (plugin->priv->settings, AUTOCHECK_TYPE_KEY);

    return autocheck_type;
}

static void
set_autocheck_type (GSettings                  *settings,
                    XedSpellPluginAutocheckType autocheck_type)
{
    if (!g_settings_is_writable (settings, AUTOCHECK_TYPE_KEY))
    {
        return;
    }

    g_settings_set_enum (settings, AUTOCHECK_TYPE_KEY, autocheck_type);
}

static void
language_dialog_response_cb (GtkDialog *dialog,
                             gint       response_id,
                             gpointer   user_data)
{
    if (response_id == GTK_RESPONSE_HELP)
    {
        xed_app_show_help (XED_APP (g_application_get_default ()),
                           GTK_WINDOW (dialog),
                           NULL,
                           "xed-spell-checker-plugin");
        return;
    }

    gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
configure_widget_button_toggled (GtkToggleButton      *button,
                                 SpellConfigureWidget *conf_widget)
{
    xed_debug (DEBUG_PLUGINS);

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (conf_widget->always)))
    {
        set_autocheck_type (conf_widget->settings, AUTOCHECK_ALWAYS);
    }
    else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (conf_widget->document)))
    {
        set_autocheck_type (conf_widget->settings, AUTOCHECK_DOCUMENT);
    }
    else
    {
        set_autocheck_type (conf_widget->settings, AUTOCHECK_NEVER);
    }
}

static void
configure_widget_destroyed (GtkWidget *widget,
                            gpointer   data)
{
    SpellConfigureWidget *conf_widget = (SpellConfigureWidget *)data;

    xed_debug (DEBUG_PLUGINS);

    g_object_unref (conf_widget->settings);
    g_slice_free (SpellConfigureWidget, data);

    xed_debug_message (DEBUG_PLUGINS, "END");
}

static SpellConfigureWidget *
get_configure_widget (XedSpellPlugin *plugin)
{
    SpellConfigureWidget *widget;
    gchar *data_dir;
    gchar *ui_file;
    XedSpellPluginAutocheckType autocheck_type;
    GtkWidget *error_widget;
    gboolean ret;
    gchar *root_objects[] = {
        "spell_dialog_content",
        NULL
    };

    xed_debug (DEBUG_PLUGINS);

    widget = g_slice_new (SpellConfigureWidget);
    widget->settings = g_object_ref (plugin->priv->settings);

    data_dir = peas_extension_base_get_data_dir (PEAS_EXTENSION_BASE (plugin));
    ui_file = g_build_filename (data_dir, "xed-spell-setup-dialog.ui", NULL);
    ret = xed_utils_get_ui_objects (ui_file,
                                    root_objects,
                                    &error_widget,
                                    "spell_dialog_content", &widget->content,
                                    "autocheck_never", &widget->never,
                                    "autocheck_document", &widget->document,
                                    "autocheck_always", &widget->always,
                                    NULL);

    g_free (data_dir);
    g_free (ui_file);

    if (!ret)
    {
        return NULL;
    }

    autocheck_type = get_autocheck_type (plugin);

    if (autocheck_type == AUTOCHECK_ALWAYS)
    {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget->always), TRUE);
    }
    else if (autocheck_type == AUTOCHECK_DOCUMENT)
    {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget->document), TRUE);
    }
    else
    {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget->never), TRUE);
    }

    g_signal_connect (widget->always, "toggled",
                      G_CALLBACK (configure_widget_button_toggled), widget);
    g_signal_connect (widget->document, "toggled",
                      G_CALLBACK (configure_widget_button_toggled), widget);
    g_signal_connect (widget->never, "toggled",
                      G_CALLBACK (configure_widget_button_toggled), widget);
    g_signal_connect (widget->content, "destroy",
                      G_CALLBACK (configure_widget_destroyed), widget);

    return widget;
}

static void
set_language_cb (GtkAction      *action,
                 XedSpellPlugin *plugin)
{
    XedSpellPluginPrivate *priv;
    XedDocument *doc;
    GspellChecker *checker;
    const GspellLanguage *lang;
    GtkWidget *dialog;
    GtkWindowGroup *window_group;

    xed_debug (DEBUG_PLUGINS);

    priv = plugin->priv;

    doc = xed_window_get_active_document (priv->window);
    g_return_if_fail (doc != NULL);

    checker = get_spell_checker (doc);
    g_return_if_fail (checker != NULL);

    lang = gspell_checker_get_language (checker);

    dialog = gspell_language_chooser_dialog_new (GTK_WINDOW (priv->window),
                                                 lang,
                                                 GTK_DIALOG_MODAL |
                                                 GTK_DIALOG_DESTROY_WITH_PARENT);

    g_object_bind_property (dialog, "language",
                            checker, "language",
                            G_BINDING_DEFAULT);

    window_group = xed_window_get_group (priv->window);

    gtk_window_group_add_window (window_group, GTK_WINDOW (dialog));

    gtk_dialog_add_button (GTK_DIALOG (dialog), _("_Help"), GTK_RESPONSE_HELP);

    g_signal_connect (dialog, "response",
                      G_CALLBACK (language_dialog_response_cb), NULL);

    gtk_widget_show (dialog);
}

static void
inline_checker_cb (GtkAction      *action,
                   XedSpellPlugin *plugin)
{
    XedSpellPluginPrivate *priv;
    XedView *view;
    gboolean active;

    xed_debug (DEBUG_PLUGINS);

    priv = plugin->priv;
    active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

    xed_debug_message (DEBUG_PLUGINS, active ? "Inline Checker activated" : "Inline Checker deactivated");

    view = xed_window_get_active_view (priv->window);
    if (view != NULL)
    {
        XedDocument *doc;
        GspellTextView *gspell_view;

        doc = XED_DOCUMENT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)));

        if (get_autocheck_type (plugin) == AUTOCHECK_DOCUMENT)
        {
            xed_document_set_metadata (doc,
                                       XED_METADATA_ATTRIBUTE_SPELL_ENABLED,
                                       active ? SPELL_ENABLED_STR : NULL,
                                       NULL);
        }

        gspell_view = gspell_text_view_get_from_gtk_text_view (GTK_TEXT_VIEW (view));
        gspell_text_view_set_inline_spell_checking (gspell_view, active);
    }
}

static void
update_ui (XedSpellPlugin *plugin)
{
    XedSpellPluginPrivate *priv;
    XedView *view;
    GtkAction *action;

    xed_debug (DEBUG_PLUGINS);

    priv = plugin->priv;

    view = xed_window_get_active_view (priv->window);

    if (view != NULL)
    {
        XedTab *tab;
        GtkTextBuffer *buffer;

        tab = xed_window_get_active_tab (priv->window);
        g_return_if_fail (xed_tab_get_view (tab) == view);

        /* If the document is loading we can't get the metadata so we
           endup with an useless speller */
        if (xed_tab_get_state (tab) == XED_TAB_STATE_NORMAL)
        {
            GspellTextView *gspell_view;
            gboolean inline_checking_enabled;

            gspell_view = gspell_text_view_get_from_gtk_text_view (GTK_TEXT_VIEW (view));
            inline_checking_enabled = gspell_text_view_get_inline_spell_checking (gspell_view);

            action = gtk_action_group_get_action (priv->action_group, "InlineSpellChecker");

            g_signal_handlers_block_by_func (action, inline_checker_cb, plugin);
            gspell_text_view_set_inline_spell_checking (gspell_view, inline_checking_enabled);
            gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), inline_checking_enabled);
            g_signal_handlers_unblock_by_func (action, inline_checker_cb, plugin);
        }
    }

    gtk_action_group_set_sensitive (priv->action_group,
                                    (view != NULL) &&
                                    gtk_text_view_get_editable (GTK_TEXT_VIEW (view)));
}

static void
setup_inline_checker_from_metadata (XedSpellPlugin *plugin,
                                    XedView        *view)
{
    XedSpellPluginPrivate *priv;
    XedDocument *doc;
    gboolean enabled = FALSE;
    gchar *enabled_str = NULL;
    GspellTextView *gspell_view;
    XedView *active_view;
    XedSpellPluginAutocheckType autocheck_type;

    priv = plugin->priv;

    autocheck_type = get_autocheck_type (plugin);

    doc = XED_DOCUMENT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)));

    switch (autocheck_type)
    {
        case AUTOCHECK_ALWAYS:
        {
            enabled = TRUE;
            break;
        }
        case AUTOCHECK_DOCUMENT:
        {
            enabled_str = xed_document_get_metadata (doc, XED_METADATA_ATTRIBUTE_SPELL_ENABLED);
            break;
        }
        case AUTOCHECK_NEVER:
        default:
            enabled = FALSE;
            break;
    }

    if (enabled_str != NULL)
    {
        enabled = g_str_equal (enabled_str, SPELL_ENABLED_STR);
        g_free (enabled_str);
    }

    gspell_view = gspell_text_view_get_from_gtk_text_view (GTK_TEXT_VIEW (view));
    gspell_text_view_set_inline_spell_checking (gspell_view, enabled);

    /* In case that the view is the active one we mark the spell action */
    active_view = xed_window_get_active_view (plugin->priv->window);

    if (active_view == view && priv->action_group != NULL)
    {
        GtkAction *action;

        action = gtk_action_group_get_action (priv->action_group, "InlineSpellChecker");

        g_signal_handlers_block_by_func (action, inline_checker_cb, plugin);
        gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), enabled);
        g_signal_handlers_unblock_by_func (action, inline_checker_cb, plugin);
    }
}

static void
language_notify_cb (GspellChecker *checker,
                    GParamSpec    *pspec,
                    XedDocument   *doc)
{
    const GspellLanguage *lang;
    const gchar *language_code;

    g_return_if_fail (XED_IS_DOCUMENT (doc));

    lang = gspell_checker_get_language (checker);
    g_return_if_fail (lang != NULL);

    language_code = gspell_language_get_code (lang);
    g_return_if_fail (language_code != NULL);

    xed_document_set_metadata (doc, XED_METADATA_ATTRIBUTE_SPELL_LANGUAGE, language_code, NULL);
}

static void
on_document_loaded (XedDocument    *doc,
                    XedSpellPlugin *plugin)
{

    GspellChecker *checker;
    XedTab *tab;
    XedView *view;

    checker = get_spell_checker (doc);

    if (checker != NULL)
    {
        const GspellLanguage *lang;

        lang = get_language_from_metadata (doc);

        if (lang != NULL)
        {
            g_signal_handlers_block_by_func (checker, language_notify_cb, doc);
            gspell_checker_set_language (checker, lang);
            g_signal_handlers_unblock_by_func (checker, language_notify_cb, doc);
        }
    }

    tab = xed_tab_get_from_document (doc);
    view = xed_tab_get_view (tab);
    setup_inline_checker_from_metadata (plugin, view);
}

static void
on_document_saved (XedDocument    *doc,
                   XedSpellPlugin *plugin)
{
    XedTab *tab;
    XedView *view;
    GspellChecker *checker;
    const gchar *language_code = NULL;
    GspellTextView *gspell_view;
    gboolean inline_checking_enabled;

    /* Make sure to save the metadata here too */
    checker = get_spell_checker (doc);

    if (checker != NULL)
    {
        const GspellLanguage *lang;

        lang = gspell_checker_get_language (checker);
        if (lang != NULL)
        {
            language_code = gspell_language_get_code (lang);
        }
    }

    tab = xed_tab_get_from_document (doc);
    view = xed_tab_get_view (tab);

    gspell_view = gspell_text_view_get_from_gtk_text_view (GTK_TEXT_VIEW (view));
    inline_checking_enabled = gspell_text_view_get_inline_spell_checking (gspell_view);

    if (get_autocheck_type (plugin) == AUTOCHECK_DOCUMENT)
    {

        xed_document_set_metadata (doc,
                                   XED_METADATA_ATTRIBUTE_SPELL_ENABLED,
                                   inline_checking_enabled ? SPELL_ENABLED_STR : NULL,
                                   XED_METADATA_ATTRIBUTE_SPELL_LANGUAGE,
                                   language_code,
                                   NULL);
    }
    else
    {
        xed_document_set_metadata (doc, XED_METADATA_ATTRIBUTE_SPELL_LANGUAGE, language_code, NULL);
    }
}

static void
activate_spell_checking_in_view (XedSpellPlugin *plugin,
                                 XedView        *view)
{
    XedDocument *doc;

    doc = XED_DOCUMENT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)));

    /* It is possible that a GspellChecker has already been set, for example
     * if a XedTab has moved to another window.
     */
    if (get_spell_checker (doc) == NULL)
    {
        const GspellLanguage *lang;
        GspellChecker *checker;
        GspellTextBuffer *gspell_buffer;

        lang = get_language_from_metadata (doc);
        checker = gspell_checker_new (lang);

        g_signal_connect_object (checker, "notify::language",
                                 G_CALLBACK (language_notify_cb), doc, 0);

        gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (GTK_TEXT_BUFFER (doc));
        gspell_text_buffer_set_spell_checker (gspell_buffer, checker);
        g_object_unref (checker);

        setup_inline_checker_from_metadata (plugin, view);
    }

    g_signal_connect_object (doc, "loaded",
                             G_CALLBACK (on_document_loaded), plugin, 0);
    g_signal_connect_object (doc, "saved",
                             G_CALLBACK (on_document_saved), plugin, 0);
}

static void
disconnect_view (XedSpellPlugin *plugin,
                 XedView        *view)
{
    GtkTextBuffer *buffer;

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

    /* It should still be the same buffer as the one where the signal
     * handlers were connected. If not, we assume that the old buffer is
     * finalized. And it is anyway safe to call
     * g_signal_handlers_disconnect_by_func() if no signal handlers are
     * found.
     */
    g_signal_handlers_disconnect_by_func (buffer, on_document_loaded, plugin);
    g_signal_handlers_disconnect_by_func (buffer, on_document_saved, plugin);
}

static void
deactivate_spell_checking_in_view (XedSpellPlugin *plugin,
                                   XedView        *view)
{
    GtkTextBuffer *gtk_buffer;
    GspellTextBuffer *gspell_buffer;
    GspellTextView *gspell_view;

    disconnect_view (plugin, view);

    gtk_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (gtk_buffer);
    gspell_text_buffer_set_spell_checker (gspell_buffer, NULL);

    gspell_view = gspell_text_view_get_from_gtk_text_view (GTK_TEXT_VIEW (view));
    gspell_text_view_set_inline_spell_checking (gspell_view, FALSE);
}

static void
tab_added_cb (XedWindow      *window,
              XedTab         *tab,
              XedSpellPlugin *plugin)
{
    activate_spell_checking_in_view (plugin, xed_tab_get_view (tab));
}

static void
tab_removed_cb (XedWindow      *window,
                XedTab         *tab,
                XedSpellPlugin *plugin)
{
    /* Don't deactivate completely the spell checking in @tab, since the tab
     * can be moved to another window and we don't want to loose the spell
     * checking settings (they are not saved in metadata for unsaved
     * documents).
     */
    disconnect_view (plugin, xed_tab_get_view (tab));
}

static void
xed_spell_plugin_activate (XedWindowActivatable *activatable)
{
    XedSpellPlugin *plugin;
    XedSpellPluginPrivate *priv;
    GtkUIManager *manager;
    GList *views;
    GList *l;

    xed_debug (DEBUG_PLUGINS);

    plugin = XED_SPELL_PLUGIN (activatable);
    priv = plugin->priv;

    manager = xed_window_get_ui_manager (priv->window);

    priv->action_group = gtk_action_group_new ("XedSpellPluginActions");
    gtk_action_group_set_translation_domain (priv->action_group, GETTEXT_PACKAGE);
    gtk_action_group_add_actions (priv->action_group,
                                  action_entries,
                                  G_N_ELEMENTS (action_entries),
                                  activatable);
    gtk_action_group_add_toggle_actions (priv->action_group,
                                         toggle_action_entries,
                                         G_N_ELEMENTS (toggle_action_entries),
                                         activatable);

    gtk_ui_manager_insert_action_group (manager, priv->action_group, -1);

    priv->ui_id = gtk_ui_manager_new_merge_id (manager);

    gtk_ui_manager_add_ui (manager,
                           priv->ui_id,
                           MENU_PATH,
                           "CheckSpell",
                           "CheckSpell",
                           GTK_UI_MANAGER_MENUITEM,
                           FALSE);

    gtk_ui_manager_add_ui (manager,
                           priv->ui_id,
                           MENU_PATH,
                           "InlineSpellChecker",
                           "InlineSpellChecker",
                           GTK_UI_MANAGER_MENUITEM,
                           FALSE);

    gtk_ui_manager_add_ui (manager,
                           priv->ui_id,
                           MENU_PATH,
                           "ConfigSpell",
                           "ConfigSpell",
                           GTK_UI_MANAGER_MENUITEM,
                           FALSE);

    update_ui (XED_SPELL_PLUGIN (activatable));

    views = xed_window_get_views (priv->window);
    for (l = views; l != NULL; l = l->next)
    {
        activate_spell_checking_in_view (plugin, XED_VIEW (l->data));
    }

    g_signal_connect (priv->window, "tab-added",
                      G_CALLBACK (tab_added_cb), activatable);
    g_signal_connect (priv->window, "tab-removed",
                      G_CALLBACK (tab_removed_cb), activatable);
}

static void
xed_spell_plugin_deactivate (XedWindowActivatable *activatable)
{
    XedSpellPlugin *plugin;
    XedSpellPluginPrivate *priv;
    GtkUIManager *manager;
    GList *views;
    GList *l;

    xed_debug (DEBUG_PLUGINS);

    plugin = XED_SPELL_PLUGIN (activatable);
    priv = plugin->priv;
    manager = xed_window_get_ui_manager (priv->window);

    gtk_ui_manager_remove_ui (manager, priv->ui_id);
    gtk_ui_manager_remove_action_group (manager, priv->action_group);

    g_signal_handlers_disconnect_by_func (priv->window, tab_added_cb, activatable);
    g_signal_handlers_disconnect_by_func (priv->window, tab_removed_cb, activatable);

    views = xed_window_get_views (priv->window);
    for (l = views; l != NULL; l = l->next)
    {
        deactivate_spell_checking_in_view (plugin, XED_VIEW (l->data));
    }
}

static void
xed_spell_plugin_update_state (XedWindowActivatable *activatable)
{
    xed_debug (DEBUG_PLUGINS);

    update_ui (XED_SPELL_PLUGIN (activatable));
}

static GtkWidget *
xed_spell_plugin_create_configure_widget (PeasGtkConfigurable *configurable)
{
    SpellConfigureWidget *widget;

    widget = get_configure_widget (XED_SPELL_PLUGIN (configurable));

    return widget->content;
}

static void
xed_window_activatable_iface_init (XedWindowActivatableInterface *iface)
{
    iface->activate = xed_spell_plugin_activate;
    iface->deactivate = xed_spell_plugin_deactivate;
    iface->update_state = xed_spell_plugin_update_state;
}

static void
peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface)
{
    iface->create_configure_widget = xed_spell_plugin_create_configure_widget;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
    xed_spell_plugin_register_type (G_TYPE_MODULE (module));

    peas_object_module_register_extension_type (module,
                                                XED_TYPE_WINDOW_ACTIVATABLE,
                                                XED_TYPE_SPELL_PLUGIN);

    peas_object_module_register_extension_type (module,
                                                PEAS_GTK_TYPE_CONFIGURABLE,
                                                XED_TYPE_SPELL_PLUGIN);
}
