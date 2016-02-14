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

#include "xed-progress-message-area.h"

enum {
	PROP_0,
	PROP_HAS_CANCEL_BUTTON
};


#define XED_PROGRESS_MESSAGE_AREA_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), XED_TYPE_PROGRESS_MESSAGE_AREA, XedProgressMessageAreaPrivate))

struct _XedProgressMessageAreaPrivate
{
	GtkWidget *image;
	GtkWidget *label;
	GtkWidget *progress;
};

G_DEFINE_TYPE(XedProgressMessageArea, xed_progress_message_area, GTK_TYPE_INFO_BAR)

static void
xed_progress_message_area_set_has_cancel_button (XedProgressMessageArea *area,
						   gboolean                  has_button)
{
	if (has_button)
		gtk_info_bar_add_button (GTK_INFO_BAR (area),
					 GTK_STOCK_CANCEL,
					 GTK_RESPONSE_CANCEL);

	g_object_notify (G_OBJECT (area), "has-cancel-button");
}

static void
xed_progress_message_area_set_property (GObject      *object,
					  guint         prop_id,
					  const GValue *value,
					  GParamSpec   *pspec)
{
	XedProgressMessageArea *area;

	area = XED_PROGRESS_MESSAGE_AREA (object);

	switch (prop_id)
	{
	case PROP_HAS_CANCEL_BUTTON:
		xed_progress_message_area_set_has_cancel_button (area,
								   g_value_get_boolean (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
xed_progress_message_area_get_property (GObject      *object,
					  guint         prop_id,
					  GValue       *value,
					  GParamSpec   *pspec)
{
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void 
xed_progress_message_area_class_init (XedProgressMessageAreaClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->set_property = xed_progress_message_area_set_property;
	gobject_class->get_property = xed_progress_message_area_get_property;

	g_object_class_install_property (gobject_class,
					 PROP_HAS_CANCEL_BUTTON,
					 g_param_spec_boolean ("has-cancel-button",
							       "Has Cancel Button",
							       "If the message area has a cancel button",
							       TRUE,
							       G_PARAM_WRITABLE |
							       G_PARAM_CONSTRUCT_ONLY |
							       G_PARAM_STATIC_STRINGS));

	g_type_class_add_private (gobject_class, sizeof(XedProgressMessageAreaPrivate));
}

static void
xed_progress_message_area_init (XedProgressMessageArea *area)
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	
	area->priv = XED_PROGRESS_MESSAGE_AREA_GET_PRIVATE (area);
	
	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
	gtk_widget_show (vbox);

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	area->priv->image = gtk_image_new_from_icon_name (GTK_STOCK_MISSING_IMAGE, 
							  GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_widget_show (area->priv->image);
#if GTK_CHECK_VERSION (3, 0, 0)
	gtk_widget_set_halign (area->priv->image, GTK_ALIGN_CENTER);
	gtk_widget_set_valign (area->priv->image, GTK_ALIGN_CENTER);
#else
	gtk_misc_set_alignment (GTK_MISC (area->priv->image), 0.5, 0.5);
#endif
	gtk_box_pack_start (GTK_BOX (hbox), area->priv->image, FALSE, FALSE, 4);
	
	area->priv->label = gtk_label_new ("");
	gtk_widget_show (area->priv->label);
	gtk_box_pack_start (GTK_BOX (hbox), area->priv->label, TRUE, TRUE, 0);
	gtk_label_set_use_markup (GTK_LABEL (area->priv->label), TRUE);
#if GTK_CHECK_VERSION (3, 16, 0)
	gtk_label_set_xalign (GTK_LABEL (area->priv->label), 0.0);
#else
	gtk_misc_set_alignment (GTK_MISC (area->priv->label), 0.0, 0.5);
#endif
	gtk_label_set_ellipsize (GTK_LABEL (area->priv->label), 
				 PANGO_ELLIPSIZE_END);

	area->priv->progress = gtk_progress_bar_new ();
	gtk_widget_show (area->priv->progress);
	gtk_box_pack_start (GTK_BOX (vbox), area->priv->progress, TRUE, FALSE, 0);
	gtk_widget_set_size_request (area->priv->progress, -1, 15);

	GtkWidget *content;
	
	content = gtk_info_bar_get_content_area (GTK_INFO_BAR (area));
	gtk_container_add (GTK_CONTAINER (content), vbox);
}

GtkWidget *
xed_progress_message_area_new (const gchar *stock_id,
				 const gchar *markup,
				 gboolean     has_cancel)
{
	XedProgressMessageArea *area;

	g_return_val_if_fail (stock_id != NULL, NULL);
	g_return_val_if_fail (markup != NULL, NULL);

	area = XED_PROGRESS_MESSAGE_AREA (g_object_new (XED_TYPE_PROGRESS_MESSAGE_AREA,
							  "has-cancel-button", has_cancel,
							  NULL));

	xed_progress_message_area_set_stock_image (area,
						     stock_id);

	xed_progress_message_area_set_markup (area,
						markup);

	return GTK_WIDGET (area);	
}

void
xed_progress_message_area_set_stock_image (XedProgressMessageArea *area,
					     const gchar              *stock_id)
{
	g_return_if_fail (XED_IS_PROGRESS_MESSAGE_AREA (area));
	g_return_if_fail (stock_id != NULL);
	
	gtk_image_set_from_stock (GTK_IMAGE (area->priv->image),
				  stock_id,
				  GTK_ICON_SIZE_SMALL_TOOLBAR);
}

void
xed_progress_message_area_set_markup (XedProgressMessageArea *area,
					const gchar              *markup)
{
	g_return_if_fail (XED_IS_PROGRESS_MESSAGE_AREA (area));
	g_return_if_fail (markup != NULL);

	gtk_label_set_markup (GTK_LABEL (area->priv->label),
			      markup);
}

void
xed_progress_message_area_set_text (XedProgressMessageArea *area,
				      const gchar              *text)
{
	g_return_if_fail (XED_IS_PROGRESS_MESSAGE_AREA (area));
	g_return_if_fail (text != NULL);

	gtk_label_set_text (GTK_LABEL (area->priv->label),
			    text);
}

void
xed_progress_message_area_set_fraction (XedProgressMessageArea *area,
					  gdouble                   fraction)
{
	g_return_if_fail (XED_IS_PROGRESS_MESSAGE_AREA (area));

	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (area->priv->progress),
				       fraction);
}

void
xed_progress_message_area_pulse (XedProgressMessageArea *area)
{
	g_return_if_fail (XED_IS_PROGRESS_MESSAGE_AREA (area));

	gtk_progress_bar_pulse (GTK_PROGRESS_BAR (area->priv->progress));
}
