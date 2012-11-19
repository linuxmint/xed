/*
 * pluma-close-button.c
 * This file is part of pluma
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

#include "pluma-close-button.h"

G_DEFINE_TYPE (PlumaCloseButton, pluma_close_button, GTK_TYPE_BUTTON)

static void
pluma_close_button_style_set (GtkWidget *button,
			      GtkStyle *previous_style)
{
	gint h, w;

	gtk_icon_size_lookup_for_settings (gtk_widget_get_settings (button),
					   GTK_ICON_SIZE_MENU, &w, &h);

	gtk_widget_set_size_request (button, w + 2, h + 2);

	GTK_WIDGET_CLASS (pluma_close_button_parent_class)->style_set (button, previous_style);
}

static void
pluma_close_button_class_init (PlumaCloseButtonClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	widget_class->style_set = pluma_close_button_style_set;
}

static void
pluma_close_button_init (PlumaCloseButton *button)
{
	GtkRcStyle *rcstyle;
	GtkWidget *image;

	/* make it as small as possible */
	rcstyle = gtk_rc_style_new ();
	rcstyle->xthickness = rcstyle->ythickness = 0;
	gtk_widget_modify_style (GTK_WIDGET (button), rcstyle);
	g_object_unref (rcstyle);

	image = gtk_image_new_from_stock (GTK_STOCK_CLOSE,
					  GTK_ICON_SIZE_MENU);
	gtk_widget_show (image);

	gtk_container_add (GTK_CONTAINER (button), image);
}

GtkWidget *
pluma_close_button_new ()
{
	PlumaCloseButton *button;

	button = g_object_new (PLUMA_TYPE_CLOSE_BUTTON,
			       "relief", GTK_RELIEF_NONE,
			       "focus-on-click", FALSE,
			       NULL);

	return GTK_WIDGET (button);
}

