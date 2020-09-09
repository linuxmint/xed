/*
 * xed-highlight-mode-dialog.c
 * This file is part of xed
 *
 * Copyright (C) 2013 - Ignacio Casal Quinteiro
 *
 * xed is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xed is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xed. If not, see <http://www.gnu.org/licenses/>.
 */

#include "xed-highlight-mode-dialog.h"

#include <gtk/gtk.h>

struct _XedHighlightModeDialog
{
    GtkDialog parent_instance;

    XedHighlightModeSelector *selector;
    gulong on_language_selected_id;
};

G_DEFINE_TYPE (XedHighlightModeDialog, xed_highlight_mode_dialog, GTK_TYPE_DIALOG)

static void
xed_highlight_mode_dialog_response (GtkDialog *dialog,
                                    gint       response_id)
{
    XedHighlightModeDialog *dlg = XED_HIGHLIGHT_MODE_DIALOG (dialog);

    if (response_id == GTK_RESPONSE_OK)
    {
        g_signal_handler_block (dlg->selector, dlg->on_language_selected_id);
        xed_highlight_mode_selector_activate_selected_language (dlg->selector);
        g_signal_handler_unblock (dlg->selector, dlg->on_language_selected_id);
    }

    gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
on_language_selected (XedHighlightModeSelector *sel,
                      GtkSourceLanguage        *language,
                      XedHighlightModeDialog   *dlg)
{
    g_signal_handler_block (dlg->selector, dlg->on_language_selected_id);
    xed_highlight_mode_selector_activate_selected_language (dlg->selector);
    g_signal_handler_unblock (dlg->selector, dlg->on_language_selected_id);

    gtk_widget_destroy (GTK_WIDGET (dlg));
}

static void
on_dialog_cancelled (XedHighlightModeSelector *sel,
                     XedHighlightModeDialog   *dlg)
{
    gtk_widget_destroy (GTK_WIDGET (dlg));
}

static void
xed_highlight_mode_dialog_class_init (XedHighlightModeDialogClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GtkDialogClass *dialog_class = GTK_DIALOG_CLASS (klass);

    dialog_class->response = xed_highlight_mode_dialog_response;

    /* Bind class to template */
    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/org/x/editor/ui/xed-highlight-mode-dialog.ui");
    gtk_widget_class_bind_template_child (widget_class, XedHighlightModeDialog, selector);
}

static void
xed_highlight_mode_dialog_init (XedHighlightModeDialog *dlg)
{
    gtk_widget_init_template (GTK_WIDGET (dlg));
    gtk_dialog_set_default_response (GTK_DIALOG (dlg), GTK_RESPONSE_OK);

    dlg->on_language_selected_id = g_signal_connect (dlg->selector, "language-selected",
                                                     G_CALLBACK (on_language_selected), dlg);

    g_signal_connect (dlg->selector, "cancelled",
                      G_CALLBACK (on_dialog_cancelled), dlg);
}

GtkWidget *
xed_highlight_mode_dialog_new (GtkWindow *parent)
{
    return GTK_WIDGET (g_object_new (XED_TYPE_HIGHLIGHT_MODE_DIALOG,
                                     "transient-for", parent,
                                     NULL));
}

XedHighlightModeSelector *
xed_highlight_mode_dialog_get_selector (XedHighlightModeDialog *dlg)
{
    g_return_val_if_fail (XED_IS_HIGHLIGHT_MODE_DIALOG (dlg), NULL);

    return dlg->selector;
}
