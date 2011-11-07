/*
 * gedit-tab-label.c
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "gedit-tab-label.h"
#include "gedit-close-button.h"

#ifdef BUILD_SPINNER
#include "gedit-spinner.h"
#endif

#define GEDIT_TAB_LABEL_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_TYPE_TAB_LABEL, GeditTabLabelPrivate))

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

struct _GeditTabLabelPrivate
{
	GeditTab *tab;

	GtkWidget *ebox;
	GtkWidget *close_button;
	GtkWidget *spinner;
	GtkWidget *icon;
	GtkWidget *label;

	gboolean close_button_sensitive;
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (GeditTabLabel, gedit_tab_label, GTK_TYPE_HBOX)

static void
gedit_tab_label_finalize (GObject *object)
{
	G_OBJECT_CLASS (gedit_tab_label_parent_class)->finalize (object);
}

static void
gedit_tab_label_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
	GeditTabLabel *tab_label = GEDIT_TAB_LABEL (object);

	switch (prop_id)
	{
		case PROP_TAB:
			tab_label->priv->tab = GEDIT_TAB (g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gedit_tab_label_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
	GeditTabLabel *tab_label = GEDIT_TAB_LABEL (object);

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
			 GeditTabLabel *tab_label)
{
	g_signal_emit (tab_label, signals[CLOSE_CLICKED], 0, NULL);
}

static void
sync_tip (GeditTab *tab, GeditTabLabel *tab_label)
{
	gchar *str;

	str = _gedit_tab_get_tooltips (tab);
	g_return_if_fail (str != NULL);

	gtk_widget_set_tooltip_markup (tab_label->priv->ebox, str);
	g_free (str);
}

static void
sync_name (GeditTab *tab, GParamSpec *pspec, GeditTabLabel *tab_label)
{
	gchar *str;

	g_return_if_fail (tab == tab_label->priv->tab);

	str = _gedit_tab_get_name (tab);
	g_return_if_fail (str != NULL);

	gtk_label_set_text (GTK_LABEL (tab_label->priv->label), str);
	g_free (str);

	sync_tip (tab, tab_label);
}

static void
sync_state (GeditTab *tab, GParamSpec *pspec, GeditTabLabel *tab_label)
{
	GeditTabState  state;

	g_return_if_fail (tab == tab_label->priv->tab);

	state = gedit_tab_get_state (tab);

	gtk_widget_set_sensitive (tab_label->priv->close_button,
				  tab_label->priv->close_button_sensitive &&
				  (state != GEDIT_TAB_STATE_CLOSING) &&
				  (state != GEDIT_TAB_STATE_SAVING)  &&
				  (state != GEDIT_TAB_STATE_SHOWING_PRINT_PREVIEW) &&
				  (state != GEDIT_TAB_STATE_SAVING_ERROR));

	if ((state == GEDIT_TAB_STATE_LOADING)   ||
	    (state == GEDIT_TAB_STATE_SAVING)    ||
	    (state == GEDIT_TAB_STATE_REVERTING))
	{
		gtk_widget_hide (tab_label->priv->icon);

		gtk_widget_show (tab_label->priv->spinner);
#ifdef BUILD_SPINNER
		gedit_spinner_start (GEDIT_SPINNER (tab_label->priv->spinner));
#else
		gtk_spinner_start (GTK_SPINNER (tab_label->priv->spinner));
#endif
	}
	else
	{
		GdkPixbuf *pixbuf;

		pixbuf = _gedit_tab_get_icon (tab);
		gtk_image_set_from_pixbuf (GTK_IMAGE (tab_label->priv->icon), pixbuf);

		if (pixbuf != NULL)
			g_object_unref (pixbuf);

		gtk_widget_show (tab_label->priv->icon);

		gtk_widget_hide (tab_label->priv->spinner);
#ifdef BUILD_SPINNER
		gedit_spinner_stop (GEDIT_SPINNER (tab_label->priv->spinner));
#else
		gtk_spinner_stop (GTK_SPINNER (tab_label->priv->spinner));
#endif
	}

	/* sync tip since encoding is known only after load/save end */
	sync_tip (tab, tab_label);
}

static void
gedit_tab_label_constructed (GObject *object)
{
	GeditTabLabel *tab_label = GEDIT_TAB_LABEL (object);

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
gedit_tab_label_class_init (GeditTabLabelClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	object_class->finalize = gedit_tab_label_finalize;
	object_class->set_property = gedit_tab_label_set_property;
	object_class->get_property = gedit_tab_label_get_property;
	object_class->constructed = gedit_tab_label_constructed;

	signals[CLOSE_CLICKED] =
		g_signal_new ("close-clicked",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GeditTabLabelClass, close_clicked),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

	g_object_class_install_property (object_class,
					 PROP_TAB,
					 g_param_spec_object ("tab",
							      "Tab",
							      "The GeditTab",
							      GEDIT_TYPE_TAB,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private (object_class, sizeof(GeditTabLabelPrivate));
}

static void
gedit_tab_label_init (GeditTabLabel *tab_label)
{
	GtkWidget *ebox;
	GtkWidget *hbox;
	GtkWidget *close_button;
	GtkWidget *spinner;
	GtkWidget *icon;
	GtkWidget *label;
	GtkWidget *dummy_label;

	tab_label->priv = GEDIT_TAB_LABEL_GET_PRIVATE (tab_label);

	tab_label->priv->close_button_sensitive = TRUE;

	ebox = gtk_event_box_new ();
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (ebox), FALSE);
	gtk_box_pack_start (GTK_BOX (tab_label), ebox, TRUE, TRUE, 0);
	tab_label->priv->ebox = ebox;

	hbox = gtk_hbox_new (FALSE, 4);
	gtk_container_add (GTK_CONTAINER (ebox), hbox);

	close_button = gedit_close_button_new ();
	gtk_widget_set_tooltip_text (close_button, _("Close document"));
	gtk_box_pack_start (GTK_BOX (tab_label), close_button, FALSE, FALSE, 0);
	tab_label->priv->close_button = close_button;

	g_signal_connect (close_button,
			  "clicked",
			  G_CALLBACK (close_button_clicked_cb),
			  tab_label);

#ifdef BUILD_SPINNER
	spinner = gedit_spinner_new ();
	gedit_spinner_set_size (GEDIT_SPINNER (spinner), GTK_ICON_SIZE_MENU);
#else
	spinner = gtk_spinner_new ();
#endif
	gtk_box_pack_start (GTK_BOX (hbox), spinner, FALSE, FALSE, 0);
	tab_label->priv->spinner = spinner;

	/* setup icon, empty by default */
	icon = gtk_image_new ();
	gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);
	tab_label->priv->icon = icon;

	label = gtk_label_new ("");
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), 0, 0);
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
gedit_tab_label_set_close_button_sensitive (GeditTabLabel *tab_label,
					    gboolean       sensitive)
{
	GeditTabState state;

	g_return_if_fail (GEDIT_IS_TAB_LABEL (tab_label));

	sensitive = (sensitive != FALSE);

	if (sensitive == tab_label->priv->close_button_sensitive)
		return;

	tab_label->priv->close_button_sensitive = sensitive;

	state = gedit_tab_get_state (tab_label->priv->tab);

	gtk_widget_set_sensitive (tab_label->priv->close_button, 
				  tab_label->priv->close_button_sensitive &&
				  (state != GEDIT_TAB_STATE_CLOSING) &&
				  (state != GEDIT_TAB_STATE_SAVING)  &&
				  (state != GEDIT_TAB_STATE_SHOWING_PRINT_PREVIEW) &&
				  (state != GEDIT_TAB_STATE_PRINTING) &&
				  (state != GEDIT_TAB_STATE_PRINT_PREVIEWING) &&
				  (state != GEDIT_TAB_STATE_SAVING_ERROR));
}

GeditTab *
gedit_tab_label_get_tab (GeditTabLabel *tab_label)
{
	g_return_val_if_fail (GEDIT_IS_TAB_LABEL (tab_label), NULL);

	return tab_label->priv->tab;
}

GtkWidget *
gedit_tab_label_new (GeditTab *tab)
{
	GeditTabLabel *tab_label;

	tab_label = g_object_new (GEDIT_TYPE_TAB_LABEL,
				  "homogeneous", FALSE,
				  "tab", tab,
				  NULL);

	return GTK_WIDGET (tab_label);
}
