/*
 * xedit-tab-label.c
 * This file is part of xedit
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
#include "xedit-tab-label.h"
#include "xedit-close-button.h"

#define XEDIT_TAB_LABEL_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), XEDIT_TYPE_TAB_LABEL, XeditTabLabelPrivate))

#if GTK_CHECK_VERSION (3, 0, 0)
#define gtk_hbox_new(X,Y) gtk_box_new(GTK_ORIENTATION_HORIZONTAL,Y)
#endif

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

struct _XeditTabLabelPrivate
{
	XeditTab *tab;

	GtkWidget *ebox;
	GtkWidget *close_button;
	GtkWidget *spinner;
	GtkWidget *icon;
	GtkWidget *label;

	gboolean close_button_sensitive;
};

static guint signals[LAST_SIGNAL] = { 0 };

#if GTK_CHECK_VERSION (3, 0, 0)
G_DEFINE_TYPE (XeditTabLabel, xedit_tab_label, GTK_TYPE_BOX)
#else
G_DEFINE_TYPE (XeditTabLabel, xedit_tab_label, GTK_TYPE_HBOX)
#endif

static void
xedit_tab_label_finalize (GObject *object)
{
	G_OBJECT_CLASS (xedit_tab_label_parent_class)->finalize (object);
}

static void
xedit_tab_label_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
	XeditTabLabel *tab_label = XEDIT_TAB_LABEL (object);

	switch (prop_id)
	{
		case PROP_TAB:
			tab_label->priv->tab = XEDIT_TAB (g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
xedit_tab_label_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
	XeditTabLabel *tab_label = XEDIT_TAB_LABEL (object);

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
close_button_clicked_cb (GtkWidget     *widget, 
			 XeditTabLabel *tab_label)
{
	g_signal_emit (tab_label, signals[CLOSE_CLICKED], 0, NULL);
}

static void
sync_tip (XeditTab *tab, XeditTabLabel *tab_label)
{
	gchar *str;

	str = _xedit_tab_get_tooltips (tab);
	g_return_if_fail (str != NULL);

	gtk_widget_set_tooltip_markup (tab_label->priv->ebox, str);
	g_free (str);
}

static void
sync_name (XeditTab *tab, GParamSpec *pspec, XeditTabLabel *tab_label)
{
	gchar *str;

	g_return_if_fail (tab == tab_label->priv->tab);

	str = _xedit_tab_get_name (tab);
	g_return_if_fail (str != NULL);

	gtk_label_set_text (GTK_LABEL (tab_label->priv->label), str);
	g_free (str);

	sync_tip (tab, tab_label);
}

static void
sync_state (XeditTab *tab, GParamSpec *pspec, XeditTabLabel *tab_label)
{
	XeditTabState  state;

	g_return_if_fail (tab == tab_label->priv->tab);

	state = xedit_tab_get_state (tab);

	gtk_widget_set_sensitive (tab_label->priv->close_button,
				  tab_label->priv->close_button_sensitive &&
				  (state != XEDIT_TAB_STATE_CLOSING) &&
				  (state != XEDIT_TAB_STATE_SAVING)  &&
				  (state != XEDIT_TAB_STATE_SHOWING_PRINT_PREVIEW) &&
				  (state != XEDIT_TAB_STATE_SAVING_ERROR));

	if ((state == XEDIT_TAB_STATE_LOADING)   ||
	    (state == XEDIT_TAB_STATE_SAVING)    ||
	    (state == XEDIT_TAB_STATE_REVERTING))
	{
		gtk_widget_hide (tab_label->priv->icon);

		gtk_widget_show (tab_label->priv->spinner);
		gtk_spinner_start (GTK_SPINNER (tab_label->priv->spinner));
	}
	else
	{
		GdkPixbuf *pixbuf;

		pixbuf = _xedit_tab_get_icon (tab);
		gtk_image_set_from_pixbuf (GTK_IMAGE (tab_label->priv->icon), pixbuf);

		if (pixbuf != NULL)
			g_object_unref (pixbuf);

		gtk_widget_show (tab_label->priv->icon);

		gtk_widget_hide (tab_label->priv->spinner);
		gtk_spinner_stop (GTK_SPINNER (tab_label->priv->spinner));
	}

	/* sync tip since encoding is known only after load/save end */
	sync_tip (tab, tab_label);
}

static void
xedit_tab_label_constructed (GObject *object)
{
	XeditTabLabel *tab_label = XEDIT_TAB_LABEL (object);

	if (!tab_label->priv->tab)
	{
		g_critical ("The tab label was not properly constructed");
		return;
	}

	sync_name (tab_label->priv->tab, NULL, tab_label);
	sync_state (tab_label->priv->tab, NULL, tab_label);

	g_signal_connect_object (tab_label->priv->tab,
				 "notify::name",
				 G_CALLBACK (sync_name),
				 tab_label,
				 0);

	g_signal_connect_object (tab_label->priv->tab,
				 "notify::state",
				 G_CALLBACK (sync_state),
				 tab_label,
				 0);
}

static void
xedit_tab_label_class_init (XeditTabLabelClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	object_class->finalize = xedit_tab_label_finalize;
	object_class->set_property = xedit_tab_label_set_property;
	object_class->get_property = xedit_tab_label_get_property;
	object_class->constructed = xedit_tab_label_constructed;

	signals[CLOSE_CLICKED] =
		g_signal_new ("close-clicked",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (XeditTabLabelClass, close_clicked),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

	g_object_class_install_property (object_class,
					 PROP_TAB,
					 g_param_spec_object ("tab",
							      "Tab",
							      "The XeditTab",
							      XEDIT_TYPE_TAB,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private (object_class, sizeof(XeditTabLabelPrivate));
}

static void
xedit_tab_label_init (XeditTabLabel *tab_label)
{
	GtkWidget *ebox;
	GtkWidget *hbox;
	GtkWidget *close_button;
	GtkWidget *spinner;
	GtkWidget *icon;
	GtkWidget *label;
	GtkWidget *dummy_label;

	tab_label->priv = XEDIT_TAB_LABEL_GET_PRIVATE (tab_label);

	tab_label->priv->close_button_sensitive = TRUE;

#if GTK_CHECK_VERSION (3, 0, 0)
	gtk_orientable_set_orientation (GTK_ORIENTABLE (tab_label),
	                                GTK_ORIENTATION_HORIZONTAL);
#endif
	ebox = gtk_event_box_new ();
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (ebox), FALSE);
	gtk_box_pack_start (GTK_BOX (tab_label), ebox, TRUE, TRUE, 0);
	tab_label->priv->ebox = ebox;

	hbox = gtk_hbox_new (FALSE, 4);
	gtk_container_add (GTK_CONTAINER (ebox), hbox);

	close_button = xedit_close_button_new ();
	gtk_widget_set_tooltip_text (close_button, _("Close document"));
	gtk_box_pack_start (GTK_BOX (tab_label), close_button, FALSE, FALSE, 0);
	tab_label->priv->close_button = close_button;

	g_signal_connect (close_button,
			  "clicked",
			  G_CALLBACK (close_button_clicked_cb),
			  tab_label);

	spinner = gtk_spinner_new ();
	gtk_box_pack_start (GTK_BOX (hbox), spinner, FALSE, FALSE, 0);
	tab_label->priv->spinner = spinner;

	/* setup icon, empty by default */
	icon = gtk_image_new ();
	gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);
	tab_label->priv->icon = icon;

	label = gtk_label_new ("");
#if GTK_CHECK_VERSION (3, 16, 0)
	gtk_label_set_xalign (GTK_LABEL (label), 0.0);
#else
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
#if GTK_CHECK_VERSION (3, 0, 0)
	gtk_widget_set_margin_left (label, 0);
	gtk_widget_set_margin_right (label, 0);
	gtk_widget_set_margin_top (label, 0);
	gtk_widget_set_margin_bottom (label, 0);
#else
	gtk_misc_set_padding (GTK_MISC (label), 0, 0);
#endif
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
xedit_tab_label_set_close_button_sensitive (XeditTabLabel *tab_label,
					    gboolean       sensitive)
{
	XeditTabState state;

	g_return_if_fail (XEDIT_IS_TAB_LABEL (tab_label));

	sensitive = (sensitive != FALSE);

	if (sensitive == tab_label->priv->close_button_sensitive)
		return;

	tab_label->priv->close_button_sensitive = sensitive;

	state = xedit_tab_get_state (tab_label->priv->tab);

	gtk_widget_set_sensitive (tab_label->priv->close_button, 
				  tab_label->priv->close_button_sensitive &&
				  (state != XEDIT_TAB_STATE_CLOSING) &&
				  (state != XEDIT_TAB_STATE_SAVING)  &&
				  (state != XEDIT_TAB_STATE_SHOWING_PRINT_PREVIEW) &&
				  (state != XEDIT_TAB_STATE_PRINTING) &&
				  (state != XEDIT_TAB_STATE_PRINT_PREVIEWING) &&
				  (state != XEDIT_TAB_STATE_SAVING_ERROR));
}

XeditTab *
xedit_tab_label_get_tab (XeditTabLabel *tab_label)
{
	g_return_val_if_fail (XEDIT_IS_TAB_LABEL (tab_label), NULL);

	return tab_label->priv->tab;
}

GtkWidget *
xedit_tab_label_new (XeditTab *tab)
{
	XeditTabLabel *tab_label;

	tab_label = g_object_new (XEDIT_TYPE_TAB_LABEL,
				  "homogeneous", FALSE,
				  "tab", tab,
				  NULL);

	return GTK_WIDGET (tab_label);
}
