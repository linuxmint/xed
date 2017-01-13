/*
 * xed-tab-label.c
 * This file is part of xed
 *
 * Copyright (C) 2010 - Paolo Borelli
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "xed-tab-label.h"
#include "xed-close-button.h"

#define XED_TAB_LABEL_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), XED_TYPE_TAB_LABEL, XedTabLabelPrivate))

/* Signals */
enum
{
    CLOSE_CLICKED,
    LAST_SIGNAL
};

enum
{
    PROP_0,
    PROP_TAB
};

struct _XedTabLabelPrivate
{
    XedTab *tab;

    GtkWidget *ebox;
    GtkWidget *close_button;
    GtkWidget *spinner;
    GtkWidget *icon;
    GtkWidget *label;

    gboolean close_button_sensitive;
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (XedTabLabel, xed_tab_label, GTK_TYPE_BOX)

static void
xed_tab_label_finalize (GObject *object)
{
    G_OBJECT_CLASS (xed_tab_label_parent_class)->finalize (object);
}

static void
xed_tab_label_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    XedTabLabel *tab_label = XED_TAB_LABEL (object);

    switch (prop_id)
    {
        case PROP_TAB:
            tab_label->priv->tab = XED_TAB (g_value_get_object (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_tab_label_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    XedTabLabel *tab_label = XED_TAB_LABEL (object);

    switch (prop_id)
    {
        case PROP_TAB:
            g_value_set_object (value, tab_label->priv->tab);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
close_button_clicked_cb (GtkWidget   *widget,
                         XedTabLabel *tab_label)
{
    g_signal_emit (tab_label, signals[CLOSE_CLICKED], 0, NULL);
}

static void
sync_tip (XedTab      *tab,
          XedTabLabel *tab_label)
{
    gchar *str;

    str = _xed_tab_get_tooltips (tab);
    g_return_if_fail (str != NULL);

    gtk_widget_set_tooltip_markup (tab_label->priv->ebox, str);
    g_free (str);
}

static void
sync_name (XedTab      *tab,
           GParamSpec  *pspec,
           XedTabLabel *tab_label)
{
    gchar *str;

    g_return_if_fail (tab == tab_label->priv->tab);

    str = _xed_tab_get_name (tab);
    g_return_if_fail (str != NULL);

    gtk_label_set_text (GTK_LABEL (tab_label->priv->label), str);
    g_free (str);

    sync_tip (tab, tab_label);
}

static void
sync_state (XedTab      *tab,
            GParamSpec  *pspec,
            XedTabLabel *tab_label)
{
    XedTabState  state;

    g_return_if_fail (tab == tab_label->priv->tab);

    state = xed_tab_get_state (tab);

    gtk_widget_set_sensitive (tab_label->priv->close_button,
                              tab_label->priv->close_button_sensitive &&
                              (state != XED_TAB_STATE_CLOSING) &&
                              (state != XED_TAB_STATE_SAVING)  &&
                              (state != XED_TAB_STATE_SHOWING_PRINT_PREVIEW) &&
                              (state != XED_TAB_STATE_SAVING_ERROR));

    if ((state == XED_TAB_STATE_LOADING) ||
        (state == XED_TAB_STATE_SAVING) ||
        (state == XED_TAB_STATE_REVERTING))
    {
        gtk_widget_hide (tab_label->priv->icon);

        gtk_widget_show (tab_label->priv->spinner);
        gtk_spinner_start (GTK_SPINNER (tab_label->priv->spinner));
    }
    else
    {
        GdkPixbuf *pixbuf;

        pixbuf = _xed_tab_get_icon (tab);

        if (pixbuf != NULL)
        {
            gtk_image_set_from_pixbuf (GTK_IMAGE (tab_label->priv->icon), pixbuf);
            g_clear_object (&pixbuf);
            gtk_widget_show (tab_label->priv->icon);
        }
        else
        {
            gtk_widget_hide (tab_label->priv->icon);
        }

        gtk_widget_hide (tab_label->priv->spinner);
        gtk_spinner_stop (GTK_SPINNER (tab_label->priv->spinner));
    }

    /* sync tip since encoding is known only after load/save end */
    sync_tip (tab, tab_label);
}

static void
xed_tab_label_constructed (GObject *object)
{
    XedTabLabel *tab_label = XED_TAB_LABEL (object);

    if (!tab_label->priv->tab)
    {
        g_critical ("The tab label was not properly constructed");
        return;
    }

    sync_name (tab_label->priv->tab, NULL, tab_label);
    sync_state (tab_label->priv->tab, NULL, tab_label);

    g_signal_connect_object (tab_label->priv->tab, "notify::name",
                             G_CALLBACK (sync_name), tab_label, 0);

    g_signal_connect_object (tab_label->priv->tab, "notify::state",
                             G_CALLBACK (sync_state), tab_label, 0);
}

static void
xed_tab_label_class_init (XedTabLabelClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = xed_tab_label_finalize;
    object_class->set_property = xed_tab_label_set_property;
    object_class->get_property = xed_tab_label_get_property;
    object_class->constructed = xed_tab_label_constructed;

    signals[CLOSE_CLICKED] =
        g_signal_new ("close-clicked",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedTabLabelClass, close_clicked),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE,
                      0);

    g_object_class_install_property (object_class,
                                     PROP_TAB,
                                     g_param_spec_object ("tab",
                                                          "Tab",
                                                          "The XedTab",
                                                          XED_TYPE_TAB,
                                                          G_PARAM_READWRITE |
                                                          G_PARAM_CONSTRUCT_ONLY));

    g_type_class_add_private (object_class, sizeof(XedTabLabelPrivate));
}

static void
xed_tab_label_init (XedTabLabel *tab_label)
{
    GtkWidget *ebox;
    GtkWidget *hbox;
    GtkWidget *close_button;
    GtkWidget *spinner;
    GtkWidget *icon;
    GtkWidget *label;
    GtkWidget *dummy_label;

    tab_label->priv = XED_TAB_LABEL_GET_PRIVATE (tab_label);

    tab_label->priv->close_button_sensitive = TRUE;

    gtk_orientable_set_orientation (GTK_ORIENTABLE (tab_label), GTK_ORIENTATION_HORIZONTAL);
    ebox = gtk_event_box_new ();
    gtk_event_box_set_visible_window (GTK_EVENT_BOX (ebox), FALSE);
    gtk_box_pack_start (GTK_BOX (tab_label), ebox, TRUE, TRUE, 0);
    tab_label->priv->ebox = ebox;

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_container_add (GTK_CONTAINER (ebox), hbox);

    close_button = xed_close_button_new ();
    gtk_widget_set_tooltip_text (close_button, _("Close document"));
    gtk_box_pack_start (GTK_BOX (tab_label), close_button, FALSE, FALSE, 0);
    tab_label->priv->close_button = close_button;

    g_signal_connect (close_button, "clicked",
                      G_CALLBACK (close_button_clicked_cb), tab_label);

    spinner = gtk_spinner_new ();
    gtk_box_pack_start (GTK_BOX (hbox), spinner, FALSE, FALSE, 0);
    tab_label->priv->spinner = spinner;

    /* setup icon, empty by default */
    icon = gtk_image_new ();
    gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);
    tab_label->priv->icon = icon;

    label = gtk_label_new ("");

    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_margin_left (label, 0);
    gtk_widget_set_margin_right (label, 0);
    gtk_widget_set_margin_top (label, 0);
    gtk_widget_set_margin_bottom (label, 0);

    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    tab_label->priv->label = label;

    dummy_label = gtk_label_new ("");
    gtk_box_pack_start (GTK_BOX (hbox), dummy_label, TRUE, TRUE, 0);

    gtk_widget_show (ebox);
    gtk_widget_show (hbox);
    gtk_widget_show (close_button);
    gtk_widget_show (icon);
    gtk_widget_show (label);
    gtk_widget_show (dummy_label);
}

void
xed_tab_label_set_close_button_sensitive (XedTabLabel *tab_label,
                                          gboolean     sensitive)
{
    XedTabState state;

    g_return_if_fail (XED_IS_TAB_LABEL (tab_label));

    sensitive = (sensitive != FALSE);

    if (sensitive == tab_label->priv->close_button_sensitive)
    {
        return;
    }

    tab_label->priv->close_button_sensitive = sensitive;

    state = xed_tab_get_state (tab_label->priv->tab);

    gtk_widget_set_sensitive (tab_label->priv->close_button,
                              tab_label->priv->close_button_sensitive &&
                              (state != XED_TAB_STATE_CLOSING) &&
                              (state != XED_TAB_STATE_SAVING)  &&
                              (state != XED_TAB_STATE_SHOWING_PRINT_PREVIEW) &&
                              (state != XED_TAB_STATE_PRINTING) &&
                              (state != XED_TAB_STATE_PRINT_PREVIEWING) &&
                              (state != XED_TAB_STATE_SAVING_ERROR));
}

XedTab *
xed_tab_label_get_tab (XedTabLabel *tab_label)
{
    g_return_val_if_fail (XED_IS_TAB_LABEL (tab_label), NULL);

    return tab_label->priv->tab;
}

GtkWidget *
xed_tab_label_new (XedTab *tab)
{
    XedTabLabel *tab_label;

    tab_label = g_object_new (XED_TYPE_TAB_LABEL,
                              "homogeneous", FALSE,
                              "tab", tab,
                              NULL);

    return GTK_WIDGET (tab_label);
}
