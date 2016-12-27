/*
 * xed-panel.c
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

#include "xed-panel.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include "xed-close-button.h"
#include "xed-window.h"
#include "xed-debug.h"

#define PANEL_ITEM_KEY "XedPanelItemKey"

#define XED_PANEL_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), XED_TYPE_PANEL, XedPanelPrivate))

struct _XedPanelPrivate
{
    GtkOrientation orientation;

    GtkWidget *main_box;
    GtkWidget *notebook;
};

typedef struct _XedPanelItem XedPanelItem;

struct _XedPanelItem
{
    gchar     *name;
    GtkWidget *icon;
};

/* Properties */
enum
{
    PROP_0,
    PROP_ORIENTATION
};

/* Signals */
enum
{
    ITEM_ADDED,
    ITEM_REMOVED,
    CLOSE,
    FOCUS_DOCUMENT,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GObject *xed_panel_constructor (GType                  type,
                                       guint                  n_construct_properties,
                                       GObjectConstructParam *construct_properties);


G_DEFINE_TYPE (XedPanel, xed_panel, GTK_TYPE_BIN)

static void
xed_panel_finalize (GObject *obj)
{
    if (G_OBJECT_CLASS (xed_panel_parent_class)->finalize)
    {
        (*G_OBJECT_CLASS (xed_panel_parent_class)->finalize) (obj);
    }
}

static void
xed_panel_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
    XedPanel *panel = XED_PANEL (object);

    switch (prop_id)
    {
        case PROP_ORIENTATION:
            g_value_set_enum(value, panel->priv->orientation);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_panel_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
    XedPanel *panel = XED_PANEL (object);

    switch (prop_id)
    {
        case PROP_ORIENTATION:
            panel->priv->orientation = g_value_get_enum (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_panel_close (XedPanel *panel)
{
    gtk_widget_hide (GTK_WIDGET (panel));
}

static void
xed_panel_focus_document (XedPanel *panel)
{
    GtkWidget *toplevel = gtk_widget_get_toplevel (GTK_WIDGET (panel));
    if (gtk_widget_is_toplevel (toplevel) && XED_IS_WINDOW (toplevel))
    {
        XedView *view;

        view = xed_window_get_active_view (XED_WINDOW (toplevel));
        if (view != NULL)
        {
            gtk_widget_grab_focus (GTK_WIDGET (view));
        }
    }
}

static void
xed_panel_get_size (GtkWidget      *widget,
                    GtkOrientation  orientation,
                    gint           *minimum,
                    gint           *natural)
{
    GtkBin *bin = GTK_BIN (widget);
    GtkWidget *child;

    if (minimum)
    {
        *minimum = 0;
    }

    if (natural)
    {
        *natural = 0;
    }

    child = gtk_bin_get_child (bin);
    if (child && gtk_widget_get_visible (child))
    {
        if (orientation == GTK_ORIENTATION_HORIZONTAL)
        {
            gtk_widget_get_preferred_width (child, minimum, natural);
        }
        else
        {
            gtk_widget_get_preferred_height (child, minimum, natural);
        }
    }
}

static void
xed_panel_get_preferred_width (GtkWidget *widget,
                               gint      *minimum,
                               gint      *natural)
{
    xed_panel_get_size (widget, GTK_ORIENTATION_HORIZONTAL, minimum, natural);
}

static void
xed_panel_get_preferred_height (GtkWidget *widget,
                                gint      *minimum,
                                gint      *natural)
{
   xed_panel_get_size (widget, GTK_ORIENTATION_VERTICAL, minimum, natural);
}

static void
xed_panel_size_allocate (GtkWidget     *widget,
                         GtkAllocation *allocation)
{
    GtkBin *bin = GTK_BIN (widget);
    GtkWidget *child;

    GTK_WIDGET_CLASS (xed_panel_parent_class)->size_allocate (widget, allocation);

    child = gtk_bin_get_child (bin);
    if (child && gtk_widget_get_visible (child))
    {
       gtk_widget_size_allocate (child, allocation);
    }
}

static void
xed_panel_grab_focus (GtkWidget *w)
{
    gint n;
    GtkWidget *tab;
    XedPanel *panel = XED_PANEL (w);

    n = gtk_notebook_get_current_page (GTK_NOTEBOOK (panel->priv->notebook));
    if (n == -1)
    {
        return;
    }

    tab = gtk_notebook_get_nth_page (GTK_NOTEBOOK (panel->priv->notebook), n);
    g_return_if_fail (tab != NULL);

    gtk_widget_grab_focus (tab);
}

static void
xed_panel_class_init (XedPanelClass *klass)
{
    GtkBindingSet *binding_set;
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    g_type_class_add_private (klass, sizeof (XedPanelPrivate));

    object_class->constructor = xed_panel_constructor;
    object_class->finalize = xed_panel_finalize;
    object_class->get_property = xed_panel_get_property;
    object_class->set_property = xed_panel_set_property;

    widget_class->get_preferred_width = xed_panel_get_preferred_width;
    widget_class->get_preferred_height = xed_panel_get_preferred_height;
    widget_class->size_allocate = xed_panel_size_allocate;
    widget_class->grab_focus = xed_panel_grab_focus;

    klass->close = xed_panel_close;
    klass->focus_document = xed_panel_focus_document;

    g_object_class_install_property (object_class,
                                     PROP_ORIENTATION,
                                     g_param_spec_enum ("orientation",
                                                        "Panel Orientation",
                                                        "The panel's orientation",
                                                        GTK_TYPE_ORIENTATION,
                                                        GTK_ORIENTATION_VERTICAL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_STRINGS));

    signals[ITEM_ADDED] =
        g_signal_new ("item_added",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (XedPanelClass, item_added),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__OBJECT,
                      G_TYPE_NONE,
                      1,
                      GTK_TYPE_WIDGET);
    signals[ITEM_REMOVED] =
        g_signal_new ("item_removed",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (XedPanelClass, item_removed),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__OBJECT,
                      G_TYPE_NONE,
                      1,
                      GTK_TYPE_WIDGET);

    /* Keybinding signals */
    signals[CLOSE] =
        g_signal_new ("close",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (XedPanelClass, close),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);
    signals[FOCUS_DOCUMENT] =
        g_signal_new ("focus_document",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (XedPanelClass, focus_document),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);
    binding_set = gtk_binding_set_by_class (klass);

    gtk_binding_entry_add_signal (binding_set,
                                  GDK_KEY_Escape,
                                  0,
                                  "close",
                                  0);
    gtk_binding_entry_add_signal (binding_set,
                                  GDK_KEY_Return,
                                  GDK_CONTROL_MASK,
                                  "focus_document",
                                  0);
}

/* This is ugly, since it supports only known
 * storage types of GtkImage, otherwise fall back
 * to the empty icon.
 * See http://bugzilla.gnome.org/show_bug.cgi?id=317520.
 */
static void
set_gtk_image_from_gtk_image (GtkImage *image,
                              GtkImage *source)
{
    switch (gtk_image_get_storage_type (source))
    {
        case GTK_IMAGE_EMPTY:
            gtk_image_clear (image);
            break;
        case GTK_IMAGE_PIXBUF:
            {
                GdkPixbuf *pb;

                pb = gtk_image_get_pixbuf (source);
                gtk_image_set_from_pixbuf (image, pb);
            }
            break;
        case GTK_IMAGE_STOCK:
            {
                gchar *s_id;
                GtkIconSize s;

                gtk_image_get_stock (source, &s_id, &s);
                gtk_image_set_from_stock (image, s_id, s);
            }
            break;
        case GTK_IMAGE_ICON_SET:
            {
                GtkIconSet *is;
                GtkIconSize s;

                gtk_image_get_icon_set (source, &is, &s);
                gtk_image_set_from_icon_set (image, is, s);
            }
            break;
        case GTK_IMAGE_ANIMATION:
            {
                GdkPixbufAnimation *a;

                a = gtk_image_get_animation (source);
                gtk_image_set_from_animation (image, a);
            }
            break;
        case GTK_IMAGE_ICON_NAME:
            {
                const gchar *n;
                GtkIconSize s;

                gtk_image_get_icon_name (source, &n, &s);
                gtk_image_set_from_icon_name (image, n, s);
            }
            break;
        default:
            gtk_image_set_from_stock (image, GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
    }
}

static void
xed_panel_init (XedPanel *panel)
{
    panel->priv = XED_PANEL_GET_PRIVATE (panel);

    panel->priv->main_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_show (panel->priv->main_box);
    gtk_container_add (GTK_CONTAINER (panel), panel->priv->main_box);
}

static void
build_notebook_for_panel (XedPanel *panel)
{
    /* Create the panel notebook */
    panel->priv->notebook = gtk_notebook_new ();

    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (panel->priv->notebook), GTK_POS_BOTTOM);
    gtk_notebook_set_scrollable (GTK_NOTEBOOK (panel->priv->notebook), TRUE);
    gtk_notebook_popup_enable (GTK_NOTEBOOK (panel->priv->notebook));

    gtk_widget_show (GTK_WIDGET (panel->priv->notebook));
}

static void
build_horizontal_panel (XedPanel *panel)
{
    gtk_box_pack_start (GTK_BOX (panel->priv->main_box), panel->priv->notebook, TRUE, TRUE, 0);
}

static void
build_vertical_panel (XedPanel *panel)
{
    gtk_box_pack_start (GTK_BOX (panel->priv->main_box), panel->priv->notebook, TRUE, TRUE, 0);
}

static GObject *
xed_panel_constructor (GType                  type,
                       guint                  n_construct_properties,
                       GObjectConstructParam *construct_properties)
{

    /* Invoke parent constructor. */
    XedPanelClass *klass = XED_PANEL_CLASS (g_type_class_peek (XED_TYPE_PANEL));
    GObjectClass *parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
    GObject *obj = parent_class->constructor (type, n_construct_properties, construct_properties);

    /* Build the panel, now that we know the orientation (_init has been called previously) */
    XedPanel *panel = XED_PANEL (obj);

    build_notebook_for_panel (panel);
    if (panel->priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
        build_horizontal_panel (panel);
    }
    else
    {
        build_vertical_panel (panel);
    }

    return obj;
}

/**
 * xed_panel_new:
 * @orientation: a #GtkOrientation
 *
 * Creates a new #XedPanel with the given @orientation. You shouldn't create
 * a new panel use xed_window_get_side_panel() or xed_window_get_bottom_panel()
 * instead.
 *
 * Returns: a new #XedPanel object.
 */
GtkWidget *
xed_panel_new (GtkOrientation orientation)
{
    return GTK_WIDGET (g_object_new (XED_TYPE_PANEL, "orientation", orientation, NULL));
}

static GtkWidget *
build_tab_label (XedPanel    *panel,
                 GtkWidget   *item,
                 const gchar *name,
                 GtkWidget   *icon)
{
    GtkWidget *hbox;
    GtkWidget *label_hbox;
    GtkWidget *label_ebox;
    GtkWidget *label;

    /* set hbox spacing and label padding (see below) so that there's an
     * equal amount of space around the label */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);

    label_ebox = gtk_event_box_new ();
    gtk_event_box_set_visible_window (GTK_EVENT_BOX (label_ebox), FALSE);
    gtk_box_pack_start (GTK_BOX (hbox), label_ebox, TRUE, TRUE, 0);

    label_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_container_add (GTK_CONTAINER (label_ebox), label_hbox);

    /* setup icon */
    gtk_box_pack_start (GTK_BOX (label_hbox), icon, FALSE, FALSE, 0);

    /* setup label */
    label = gtk_label_new (name);

    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_widget_set_margin_left (label, 0);
    gtk_widget_set_margin_right (label, 0);
    gtk_widget_set_margin_top (label, 0);
    gtk_widget_set_margin_bottom (label, 0);

    gtk_box_pack_start (GTK_BOX (label_hbox), label, TRUE, TRUE, 0);

    gtk_widget_set_tooltip_text (label_ebox, name);

    gtk_widget_show_all (hbox);

    if (panel->priv->orientation == GTK_ORIENTATION_VERTICAL)
    {
        gtk_widget_hide(label);
    }

    g_object_set_data (G_OBJECT (item), "label", label);
    g_object_set_data (G_OBJECT (item), "hbox", hbox);

    return hbox;
}

/**
 * xed_panel_add_item:
 * @panel: a #XedPanel
 * @item: the #GtkWidget to add to the @panel
 * @name: the name to be shown in the @panel
 * @image: the image to be shown in the @panel
 *
 * Adds a new item to the @panel.
 */
void
xed_panel_add_item (XedPanel    *panel,
                    GtkWidget   *item,
                    const gchar *name,
                    GtkWidget   *image)
{
    XedPanelItem *data;
    GtkWidget *tab_label;
    GtkWidget *menu_label;
    gint w, h;

    g_return_if_fail (XED_IS_PANEL (panel));
    g_return_if_fail (GTK_IS_WIDGET (item));
    g_return_if_fail (name != NULL);
    g_return_if_fail (image == NULL || GTK_IS_IMAGE (image));

    data = g_new (XedPanelItem, 1);

    data->name = g_strdup (name);

    if (image == NULL)
    {
        /* default to empty */
        data->icon = gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
    }
    else
    {
        data->icon = image;
    }

    gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &w, &h);
    gtk_widget_set_size_request (data->icon, w, h);

    g_object_set_data (G_OBJECT (item), PANEL_ITEM_KEY, data);

    tab_label = build_tab_label (panel, item, data->name, data->icon);

    menu_label = gtk_label_new (name);

    gtk_misc_set_alignment (GTK_MISC (menu_label), 0.0, 0.5);

    if (!gtk_widget_get_visible (item))
    {
        gtk_widget_show (item);
    }

    gtk_notebook_append_page_menu (GTK_NOTEBOOK (panel->priv->notebook), item, tab_label, menu_label);

    g_signal_emit (G_OBJECT (panel), signals[ITEM_ADDED], 0, item);
}

/**
 * xed_panel_add_item_with_stock_icon:
 * @panel: a #XedPanel
 * @item: the #GtkWidget to add to the @panel
 * @name: the name to be shown in the @panel
 * @stock_id: a stock id
 *
 * Same as xed_panel_add_item() but using an image from stock.
 */
void
xed_panel_add_item_with_stock_icon (XedPanel    *panel,
                                    GtkWidget   *item,
                                    const gchar *name,
                                    const gchar *stock_id)
{
    GtkWidget *icon = NULL;

    if (stock_id != NULL)
    {
        icon = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_MENU);
    }

    xed_panel_add_item (panel, item, name, icon);
}

/**
 * xed_panel_remove_item:
 * @panel: a #XedPanel
 * @item: the item to be removed from the panel
 *
 * Removes the widget @item from the panel if it is in the @panel and returns
 * %TRUE if there was not any problem.
 *
 * Returns: %TRUE if it was well removed.
 */
gboolean
xed_panel_remove_item (XedPanel  *panel,
                       GtkWidget *item)
{
    XedPanelItem *data;
    gint page_num;

    g_return_val_if_fail (XED_IS_PANEL (panel), FALSE);
    g_return_val_if_fail (GTK_IS_WIDGET (item), FALSE);

    page_num = gtk_notebook_page_num (GTK_NOTEBOOK (panel->priv->notebook), item);

    if (page_num == -1)
    {
        return FALSE;
    }

    data = (XedPanelItem *)g_object_get_data (G_OBJECT (item), PANEL_ITEM_KEY);
    g_return_val_if_fail (data != NULL, FALSE);

    g_free (data->name);
    g_free (data);

    g_object_set_data (G_OBJECT (item), PANEL_ITEM_KEY, NULL);

    /* ref the item to keep it alive during signal emission */
    g_object_ref (G_OBJECT (item));

    gtk_notebook_remove_page (GTK_NOTEBOOK (panel->priv->notebook), page_num);

    g_signal_emit (G_OBJECT (panel), signals[ITEM_REMOVED], 0, item);

    g_object_unref (G_OBJECT (item));

    return TRUE;
}

/**
 * xed_panel_activate_item:
 * @panel: a #XedPanel
 * @item: the item to be activated
 *
 * Switches to the page that contains @item.
 *
 * Returns: %TRUE if it was activated
 */
gboolean
xed_panel_activate_item (XedPanel  *panel,
                         GtkWidget *item)
{
    gint page_num;

    g_return_val_if_fail (XED_IS_PANEL (panel), FALSE);
    g_return_val_if_fail (GTK_IS_WIDGET (item), FALSE);

    page_num = gtk_notebook_page_num (GTK_NOTEBOOK (panel->priv->notebook), item);

    if (page_num == -1)
    {
        return FALSE;
    }

    gtk_notebook_set_current_page (GTK_NOTEBOOK (panel->priv->notebook), page_num);

    return TRUE;
}

/**
 * xed_panel_item_is_active:
 * @panel: a #XedPanel
 * @item: a #GtkWidget
 *
 * Returns whether @item is the active widget in @panel
 *
 * Returns: %TRUE if @item is the active widget
 */
gboolean
xed_panel_item_is_active (XedPanel  *panel,
                          GtkWidget *item)
{
    gint cur_page;
    gint page_num;

    g_return_val_if_fail (XED_IS_PANEL (panel), FALSE);
    g_return_val_if_fail (GTK_IS_WIDGET (item), FALSE);

    page_num = gtk_notebook_page_num (GTK_NOTEBOOK (panel->priv->notebook), item);

    if (page_num == -1)
    {
        return FALSE;
    }

    cur_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (panel->priv->notebook));

    return (page_num == cur_page);
}

/**
 * xed_panel_get_orientation:
 * @panel: a #XedPanel
 *
 * Gets the orientation of the @panel.
 *
 * Returns: the #GtkOrientation of #XedPanel
 */
GtkOrientation
xed_panel_get_orientation (XedPanel *panel)
{
    g_return_val_if_fail (XED_IS_PANEL (panel), GTK_ORIENTATION_VERTICAL);

    return panel->priv->orientation;
}

/**
 * xed_panel_get_n_items:
 * @panel: a #XedPanel
 *
 * Gets the number of items in a @panel.
 *
 * Returns: the number of items contained in #XedPanel
 */
gint
xed_panel_get_n_items (XedPanel *panel)
{
    g_return_val_if_fail (XED_IS_PANEL (panel), -1);

    return gtk_notebook_get_n_pages (GTK_NOTEBOOK (panel->priv->notebook));
}

gint
_xed_panel_get_active_item_id (XedPanel *panel)
{
    gint cur_page;
    GtkWidget *item;
    XedPanelItem *data;

    g_return_val_if_fail (XED_IS_PANEL (panel), 0);

    cur_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (panel->priv->notebook));
    if (cur_page == -1)
    {
        return 0;
    }

    item = gtk_notebook_get_nth_page (GTK_NOTEBOOK (panel->priv->notebook), cur_page);

    /* FIXME: for now we use as the hash of the name as id.
     * However the name is not guaranteed to be unique and
     * it is a translated string, so it's subotimal, but should
     * be good enough for now since we don't want to add an
     * ad hoc id argument.
     */

    data = (XedPanelItem *)g_object_get_data (G_OBJECT (item), PANEL_ITEM_KEY);
    g_return_val_if_fail (data != NULL, 0);

    return g_str_hash (data->name);
}

void
_xed_panel_set_active_item_by_id (XedPanel *panel,
                                  gint      id)
{
    gint n, i;

    g_return_if_fail (XED_IS_PANEL (panel));

    if (id == 0)
    {
        return;
    }

    n = gtk_notebook_get_n_pages (GTK_NOTEBOOK (panel->priv->notebook));

    for (i = 0; i < n; i++)
    {
        GtkWidget *item;
        XedPanelItem *data;

        item = gtk_notebook_get_nth_page (GTK_NOTEBOOK (panel->priv->notebook), i);

        data = (XedPanelItem *)g_object_get_data (G_OBJECT (item), PANEL_ITEM_KEY);
        g_return_if_fail (data != NULL);

        if (g_str_hash (data->name) == id)
        {
            gtk_notebook_set_current_page (GTK_NOTEBOOK (panel->priv->notebook), i);

            return;
        }
    }
}
