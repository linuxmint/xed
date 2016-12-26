/*
 * xed-status-combo-box.c
 * This file is part of xed
 *
 * Copyright (C) 2008 - Jesse van den Kieboom
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

#include "xed-status-combo-box.h"

#define COMBO_BOX_TEXT_DATA "XedStatusComboBoxTextData"

#define XED_STATUS_COMBO_BOX_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), XED_TYPE_STATUS_COMBO_BOX, XedStatusComboBoxPrivate))

static void  menu_deactivate (GtkMenu *menu, XedStatusComboBox *combo);

struct _XedStatusComboBoxPrivate
{
    GtkWidget *frame;
    GtkWidget *button;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *item;
    GtkWidget *arrow;

    GtkWidget *menu;
    GtkWidget *current_item;
};

struct _XedStatusComboBoxClassPrivate
{
    GtkCssProvider *css;
};

/* Signals */
enum
{
    CHANGED,
    NUM_SIGNALS
};

/* Properties */
enum
{
    PROP_0,

    PROP_LABEL
};

static guint signals[NUM_SIGNALS] = { 0 };

G_DEFINE_TYPE_WITH_CODE (XedStatusComboBox, xed_status_combo_box, GTK_TYPE_EVENT_BOX,
                         g_type_add_class_private (g_define_type_id, sizeof (XedStatusComboBoxClassPrivate)))

static void
xed_status_combo_box_finalize (GObject *object)
{
    G_OBJECT_CLASS (xed_status_combo_box_parent_class)->finalize (object);
}

static void
xed_status_combo_box_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
    XedStatusComboBox *obj = XED_STATUS_COMBO_BOX (object);

    switch (prop_id)
    {
        case PROP_LABEL:
            g_value_set_string (value, xed_status_combo_box_get_label (obj));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_status_combo_box_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
    XedStatusComboBox *obj = XED_STATUS_COMBO_BOX (object);

    switch (prop_id)
    {
        case PROP_LABEL:
            xed_status_combo_box_set_label (obj, g_value_get_string (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_status_combo_box_destroy (GtkWidget *widget)
{
    XedStatusComboBox *combo = XED_STATUS_COMBO_BOX (widget);

    if (combo->priv->menu)
    {
        g_signal_handlers_disconnect_by_func (combo->priv->menu,
                                              menu_deactivate,
                                              combo);
        gtk_menu_detach (GTK_MENU (combo->priv->menu));
    }

    GTK_WIDGET_CLASS (xed_status_combo_box_parent_class)->destroy (widget);
}

static void
xed_status_combo_box_changed (XedStatusComboBox *combo,
                                GtkMenuItem         *item)
{
    const gchar *text;

    text = g_object_get_data (G_OBJECT (item), COMBO_BOX_TEXT_DATA);

    if (text != NULL)
    {
        gtk_label_set_markup (GTK_LABEL (combo->priv->item), text);
        combo->priv->current_item = GTK_WIDGET (item);
    }
}

static void
xed_status_combo_box_class_init (XedStatusComboBoxClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    static const gchar style[] =
        "* {\n"
          "-GtkButton-default-border : 0;\n"
          "-GtkButton-default-outside-border : 0;\n"
          "-GtkButton-inner-border: 0;\n"
          "-GtkWidget-focus-line-width : 0;\n"
          "-GtkWidget-focus-padding : 0;\n"
          "padding: 0;\n"
        "}";

    object_class->finalize = xed_status_combo_box_finalize;
    object_class->get_property = xed_status_combo_box_get_property;
    object_class->set_property = xed_status_combo_box_set_property;

    widget_class->destroy = xed_status_combo_box_destroy;

    klass->changed = xed_status_combo_box_changed;

    signals[CHANGED] =
        g_signal_new ("changed",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedStatusComboBoxClass,
                                       changed), NULL, NULL,
                      g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1,
                      GTK_TYPE_MENU_ITEM);

    g_object_class_install_property (object_class, PROP_LABEL,
                                     g_param_spec_string ("label",
                                                          "LABEL",
                                                          "The label",
                                                          NULL,
                                                          G_PARAM_READWRITE));

    g_type_class_add_private (object_class, sizeof(XedStatusComboBoxPrivate));

    klass->priv = G_TYPE_CLASS_GET_PRIVATE (klass, XED_TYPE_STATUS_COMBO_BOX, XedStatusComboBoxClassPrivate);

    klass->priv->css = gtk_css_provider_new ();
    gtk_css_provider_load_from_data (klass->priv->css, style, -1, NULL);
}

static void
menu_deactivate (GtkMenu             *menu,
                 XedStatusComboBox *combo)
{
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (combo->priv->button), FALSE);
}

static void
menu_position_func (GtkMenu             *menu,
                    gint                *x,
                    gint                *y,
                    gboolean            *push_in,
                    XedStatusComboBox *combo)
{
    GtkRequisition request;
    GtkAllocation allocation;

    *push_in = FALSE;

    gtk_widget_get_preferred_size (gtk_widget_get_toplevel (GTK_WIDGET (menu)), &request, NULL);

    /* get the origin... */
    gdk_window_get_origin (gtk_widget_get_window (GTK_WIDGET (combo)), x, y);
    gtk_widget_get_allocation (GTK_WIDGET (combo), &allocation);

    /* make the menu as wide as the widget */
    if (request.width < allocation.width)
    {
        gtk_widget_set_size_request (GTK_WIDGET (menu), allocation.width, -1);
    }

    /* position it above the widget */
    *y -= request.height;
}

static void
show_menu (XedStatusComboBox *combo,
           guint                button,
           guint32              time)
{
    GtkRequisition request;
    gint max_height;
    GtkAllocation allocation;

    gtk_widget_get_preferred_size (combo->priv->menu,
                                   &request, NULL);

    /* do something relative to our own height here, maybe we can do better */
    gtk_widget_get_allocation (GTK_WIDGET (combo), &allocation);
    max_height = allocation.height * 20;

    if (request.height > max_height)
    {
        gtk_widget_set_size_request (combo->priv->menu, -1, max_height);
        gtk_widget_set_size_request (gtk_widget_get_toplevel (combo->priv->menu), -1, max_height);
    }

    gtk_menu_popup (GTK_MENU (combo->priv->menu),
                    NULL,
                    NULL,
                    (GtkMenuPositionFunc)menu_position_func,
                    combo,
                    button,
                    time);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (combo->priv->button), TRUE);

    if (combo->priv->current_item)
    {
        gtk_menu_shell_select_item (GTK_MENU_SHELL (combo->priv->menu),
                        combo->priv->current_item);
    }
}

static void
menu_detached (GtkWidget *widget,
               GtkMenu   *menu)
{
    XedStatusComboBox *combo = XED_STATUS_COMBO_BOX (widget);

    g_return_if_fail (GTK_MENU (combo->priv->menu) == menu);

    combo->priv->menu = NULL;
}

static gboolean
button_press_event (GtkWidget           *widget,
                    GdkEventButton      *event,
                    XedStatusComboBox *combo)
{
    if (event->type == GDK_BUTTON_PRESS && event->button == 1)
    {
        show_menu (combo, event->button, event->time);
        return TRUE;
    }

    return FALSE;
}

static void
set_shadow_type (XedStatusComboBox *combo)
{
    GtkStyleContext *context;
    GtkShadowType shadow_type;
    GtkWidget *statusbar;

    /* This is a hack needed to use the shadow type of a statusbar */
    statusbar = gtk_statusbar_new ();
    context = gtk_widget_get_style_context (statusbar);

    gtk_style_context_get_style (context, "shadow-type", &shadow_type, NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (combo->priv->frame), shadow_type);

    gtk_widget_destroy (statusbar);
}

static void
xed_status_combo_box_init (XedStatusComboBox *self)
{
    GtkStyleContext *context;

    self->priv = XED_STATUS_COMBO_BOX_GET_PRIVATE (self);

    gtk_event_box_set_visible_window (GTK_EVENT_BOX (self), TRUE);

    self->priv->frame = gtk_frame_new (NULL);
    gtk_widget_show (self->priv->frame);

    self->priv->button = gtk_toggle_button_new ();
    gtk_button_set_relief (GTK_BUTTON (self->priv->button), GTK_RELIEF_NONE);
    gtk_widget_show (self->priv->button);

    set_shadow_type (self);

    self->priv->hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3);
    gtk_widget_show (self->priv->hbox);

    gtk_container_add (GTK_CONTAINER (self), self->priv->frame);
    gtk_container_add (GTK_CONTAINER (self->priv->frame), self->priv->button);
    gtk_container_add (GTK_CONTAINER (self->priv->button), self->priv->hbox);

    self->priv->label = gtk_label_new ("");
    gtk_widget_show (self->priv->label);

    gtk_label_set_single_line_mode (GTK_LABEL (self->priv->label), TRUE);
    gtk_widget_set_halign (GTK_WIDGET (self->priv->label), GTK_ALIGN_START);

    gtk_box_pack_start (GTK_BOX (self->priv->hbox), self->priv->label, FALSE, TRUE, 0);

    self->priv->item = gtk_label_new ("");
    gtk_widget_show (self->priv->item);

    gtk_label_set_single_line_mode (GTK_LABEL (self->priv->item), TRUE);
    gtk_widget_set_halign (self->priv->item, GTK_ALIGN_START);

    gtk_box_pack_start (GTK_BOX (self->priv->hbox), self->priv->item, TRUE, TRUE, 0);

    self->priv->arrow = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_NONE);
    gtk_widget_show (self->priv->arrow);

    gtk_widget_set_halign (self->priv->arrow, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (self->priv->arrow, GTK_ALIGN_CENTER);

    gtk_box_pack_start (GTK_BOX (self->priv->hbox), self->priv->arrow, FALSE, TRUE, 0);

    self->priv->menu = gtk_menu_new ();
    gtk_menu_attach_to_widget (GTK_MENU (self->priv->menu),
                               GTK_WIDGET (self),
                               menu_detached);

    g_signal_connect (self->priv->button,
                      "button-press-event",
                      G_CALLBACK (button_press_event),
                      self);
    g_signal_connect (self->priv->menu,
                      "deactivate",
                      G_CALLBACK (menu_deactivate),
                      self);

    context = gtk_widget_get_style_context (GTK_WIDGET (self->priv->button));
    gtk_style_context_add_provider (context,
                                    GTK_STYLE_PROVIDER (XED_STATUS_COMBO_BOX_GET_CLASS (self)->priv->css),
                                    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    context = gtk_widget_get_style_context (GTK_WIDGET (self->priv->frame));
    gtk_style_context_add_provider (context,
                                    GTK_STYLE_PROVIDER (XED_STATUS_COMBO_BOX_GET_CLASS (self)->priv->css),
                                    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

/* public functions */

/**
 * xed_status_combo_box_new:
 * @label: (allow-none):
 */
GtkWidget *
xed_status_combo_box_new (const gchar *label)
{
    return g_object_new (XED_TYPE_STATUS_COMBO_BOX, "label", label, NULL);
}

/**
 * xed_status_combo_box_set_label:
 * @combo:
 * @label: (allow-none):
 */
void
xed_status_combo_box_set_label (XedStatusComboBox *combo,
                                  const gchar         *label)
{
    gchar *text;

    g_return_if_fail (XED_IS_STATUS_COMBO_BOX (combo));

    text = g_strconcat ("  ", label, ": ", NULL);
    gtk_label_set_markup (GTK_LABEL (combo->priv->label), text);
    g_free (text);
}

const gchar *
xed_status_combo_box_get_label (XedStatusComboBox *combo)
{
    g_return_val_if_fail (XED_IS_STATUS_COMBO_BOX (combo), NULL);

    return gtk_label_get_label (GTK_LABEL (combo->priv->label));
}

static void
item_activated (GtkMenuItem         *item,
                XedStatusComboBox *combo)
{
    xed_status_combo_box_set_item (combo, item);
}

/**
 * xed_status_combo_box_add_item:
 * @combo:
 * @item:
 * @text: (allow-none):
 */
void
xed_status_combo_box_add_item (XedStatusComboBox *combo,
                                 GtkMenuItem         *item,
                                 const gchar         *text)
{
    g_return_if_fail (XED_IS_STATUS_COMBO_BOX (combo));
    g_return_if_fail (GTK_IS_MENU_ITEM (item));

    gtk_menu_shell_append (GTK_MENU_SHELL (combo->priv->menu), GTK_WIDGET (item));

    xed_status_combo_box_set_item_text (combo, item, text);
    g_signal_connect (item, "activate", G_CALLBACK (item_activated), combo);
}

void
xed_status_combo_box_remove_item (XedStatusComboBox *combo,
                                    GtkMenuItem         *item)
{
    g_return_if_fail (XED_IS_STATUS_COMBO_BOX (combo));
    g_return_if_fail (GTK_IS_MENU_ITEM (item));

    gtk_container_remove (GTK_CONTAINER (combo->priv->menu),
                          GTK_WIDGET (item));
}


/**
 * xed_status_combo_box_get_items:
 * @combo:
 *
 * Returns: (element-type Gtk.Widget) (transfer container):
 */
GList *
xed_status_combo_box_get_items (XedStatusComboBox *combo)
{
    g_return_val_if_fail (XED_IS_STATUS_COMBO_BOX (combo), NULL);

    return gtk_container_get_children (GTK_CONTAINER (combo->priv->menu));
}

const gchar *
xed_status_combo_box_get_item_text (XedStatusComboBox *combo,
                                      GtkMenuItem     *item)
{
    const gchar *ret = NULL;

    g_return_val_if_fail (XED_IS_STATUS_COMBO_BOX (combo), NULL);
    g_return_val_if_fail (GTK_IS_MENU_ITEM (item), NULL);

    ret = g_object_get_data (G_OBJECT (item), COMBO_BOX_TEXT_DATA);

    return ret;
}

/**
 * xed_status_combo_box_set_item_text:
 * @combo:
 * @item:
 * @text: (allow-none):
 */
void
xed_status_combo_box_set_item_text (XedStatusComboBox *combo,
                                      GtkMenuItem     *item,
                                      const gchar         *text)
{
    g_return_if_fail (XED_IS_STATUS_COMBO_BOX (combo));
    g_return_if_fail (GTK_IS_MENU_ITEM (item));

    g_object_set_data_full (G_OBJECT (item),
                            COMBO_BOX_TEXT_DATA,
                            g_strdup (text),
                            (GDestroyNotify)g_free);
}

void
xed_status_combo_box_set_item (XedStatusComboBox *combo,
                                 GtkMenuItem         *item)
{
    g_return_if_fail (XED_IS_STATUS_COMBO_BOX (combo));
    g_return_if_fail (GTK_IS_MENU_ITEM (item));

    g_signal_emit (combo, signals[CHANGED], 0, item, NULL);
}

GtkLabel *
xed_status_combo_box_get_item_label (XedStatusComboBox *combo)
{
    g_return_val_if_fail (XED_IS_STATUS_COMBO_BOX (combo), NULL);

    return GTK_LABEL (combo->priv->item);
}

