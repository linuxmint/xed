/*
 * xed-progress-message-area.c
 * This file is part of xed
 *
 * Copyright (C) 2005 - Paolo Maggi
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

 /* TODO: add properties */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "xed-progress-info-bar.h"

enum
{
    PROP_0,
    PROP_HAS_CANCEL_BUTTON
};


#define XED_PROGRESS_INFO_BAR_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), XED_TYPE_PROGRESS_INFO_BAR, XedProgressInfoBarPrivate))

struct _XedProgressInfoBarPrivate
{
    GtkWidget *image;
    GtkWidget *label;
    GtkWidget *progress;
};

G_DEFINE_TYPE(XedProgressInfoBar, xed_progress_info_bar, GTK_TYPE_INFO_BAR)

static void
xed_progress_info_bar_set_has_cancel_button (XedProgressInfoBar *bar,
                                             gboolean            has_button)
{
    if (has_button)
    {
        gtk_info_bar_add_button (GTK_INFO_BAR (bar), _("Cancel"), GTK_RESPONSE_CANCEL);
    }

    g_object_notify (G_OBJECT (bar), "has-cancel-button");
}

static void
xed_progress_info_bar_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
    XedProgressInfoBar *bar;

    bar = XED_PROGRESS_INFO_BAR (object);

    switch (prop_id)
    {
        case PROP_HAS_CANCEL_BUTTON:
            xed_progress_info_bar_set_has_cancel_button (bar, g_value_get_boolean (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_progress_info_bar_get_property (GObject    *object,
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
xed_progress_info_bar_class_init (XedProgressInfoBarClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = xed_progress_info_bar_set_property;
    gobject_class->get_property = xed_progress_info_bar_get_property;

    g_object_class_install_property (gobject_class,
                                     PROP_HAS_CANCEL_BUTTON,
                                     g_param_spec_boolean ("has-cancel-button",
                                                           "Has Cancel Button",
                                                           "If the message area has a cancel button",
                                                           TRUE,
                                                           G_PARAM_WRITABLE |
                                                           G_PARAM_CONSTRUCT_ONLY |
                                                           G_PARAM_STATIC_STRINGS));

    g_type_class_add_private (gobject_class, sizeof (XedProgressInfoBarPrivate));
}

static void
xed_progress_info_bar_init (XedProgressInfoBar *bar)
{
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *content;

    bar->priv = XED_PROGRESS_INFO_BAR_GET_PRIVATE (bar);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_show (vbox);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_show (hbox);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

    bar->priv->image = gtk_image_new_from_icon_name ("image-missing", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_show (bar->priv->image);
    gtk_widget_set_halign (bar->priv->image, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (bar->priv->image, GTK_ALIGN_CENTER);
    gtk_box_pack_start (GTK_BOX (hbox), bar->priv->image, FALSE, FALSE, 4);

    bar->priv->label = gtk_label_new ("");
    gtk_widget_show (bar->priv->label);
    gtk_box_pack_start (GTK_BOX (hbox), bar->priv->label, TRUE, TRUE, 0);
    gtk_label_set_use_markup (GTK_LABEL (bar->priv->label), TRUE);
    gtk_widget_set_halign (bar->priv->label, GTK_ALIGN_START);
    gtk_label_set_ellipsize (GTK_LABEL (bar->priv->label), PANGO_ELLIPSIZE_END);

    bar->priv->progress = gtk_progress_bar_new ();
    gtk_widget_show (bar->priv->progress);
    gtk_box_pack_start (GTK_BOX (vbox), bar->priv->progress, TRUE, FALSE, 0);
    gtk_widget_set_size_request (bar->priv->progress, -1, 15);

    content = gtk_info_bar_get_content_area (GTK_INFO_BAR (bar));
    gtk_container_add (GTK_CONTAINER (content), vbox);
}

GtkWidget *
xed_progress_info_bar_new (const gchar *icon_name,
                           const gchar *markup,
                           gboolean     has_cancel)
{
    XedProgressInfoBar *bar;

    g_return_val_if_fail (icon_name != NULL, NULL);
    g_return_val_if_fail (markup != NULL, NULL);

    bar = XED_PROGRESS_INFO_BAR (g_object_new (XED_TYPE_PROGRESS_INFO_BAR,
                                 "has-cancel-button", has_cancel,
                                 NULL));

    xed_progress_info_bar_set_image (bar, icon_name);
    xed_progress_info_bar_set_markup (bar, markup);

    return GTK_WIDGET (bar);
}

void
xed_progress_info_bar_set_image (XedProgressInfoBar *bar,
                                 const gchar        *icon_name)
{
    g_return_if_fail (XED_IS_PROGRESS_INFO_BAR (bar));
    g_return_if_fail (icon_name != NULL);

    gtk_image_set_from_icon_name (GTK_IMAGE (bar->priv->image), icon_name, GTK_ICON_SIZE_SMALL_TOOLBAR);
}

void
xed_progress_info_bar_set_markup (XedProgressInfoBar *bar,
                                  const gchar        *markup)
{
    g_return_if_fail (XED_IS_PROGRESS_INFO_BAR (bar));
    g_return_if_fail (markup != NULL);

    gtk_label_set_markup (GTK_LABEL (bar->priv->label), markup);
}

void
xed_progress_info_bar_set_text (XedProgressInfoBar *bar,
                                const gchar        *text)
{
    g_return_if_fail (XED_IS_PROGRESS_INFO_BAR (bar));
    g_return_if_fail (text != NULL);

    gtk_label_set_text (GTK_LABEL (bar->priv->label), text);
}

void
xed_progress_info_bar_set_fraction (XedProgressInfoBar *bar,
                                    gdouble             fraction)
{
    g_return_if_fail (XED_IS_PROGRESS_INFO_BAR (bar));

    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (bar->priv->progress), fraction);
}

void
xed_progress_info_bar_pulse (XedProgressInfoBar *bar)
{
    g_return_if_fail (XED_IS_PROGRESS_INFO_BAR (bar));

    gtk_progress_bar_pulse (GTK_PROGRESS_BAR (bar->priv->progress));
}
