/*
 * xed-print.c
 * This file is part of xed
 *
 * Copyright (C) 2000-2001 Chema Celorio, Paolo Maggi
 * Copyright (C) 2002-2008 Paolo Maggi
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
 * Modified by the xed Team, 1998-2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id: xed-print.c 6022 2007-12-09 14:38:57Z pborelli $
 */

#include <config.h>
#include <glib/gi18n.h>
#include <gtksourceview/gtksource.h>

#include "xed-print-job.h"
#include "xed-debug.h"
#include "xed-print-preview.h"
#include "xed-marshal.h"
#include "xed-utils.h"
#include "xed-dirs.h"
#include "xed-settings.h"


#define XED_PRINT_JOB_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
                                          XED_TYPE_PRINT_JOB, \
                                          XedPrintJobPrivate))

struct _XedPrintJobPrivate
{
    GSettings *print_settings;

    XedView *view;
    XedDocument *doc;

    GtkPrintOperation *operation;
    GtkSourcePrintCompositor *compositor;

    GtkPrintSettings *settings;

    GtkWidget *preview;

    XedPrintJobStatus status;

    gchar *status_string;

    gdouble progress;

    gboolean is_preview;

    /* widgets part of the custom print preferences widget.
     * These pointers are valid just when the dialog is displayed */
    GtkWidget *syntax_checkbutton;
    GtkWidget *page_header_checkbutton;
    GtkWidget *line_numbers_checkbutton;
    GtkWidget *line_numbers_hbox;
    GtkWidget *line_numbers_spinbutton;
    GtkWidget *text_wrapping_checkbutton;
    GtkWidget *do_not_split_checkbutton;
    GtkWidget *fonts_table;
    GtkWidget *body_font_label;
    GtkWidget *headers_font_label;
    GtkWidget *numbers_font_label;
    GtkWidget *body_fontbutton;
    GtkWidget *headers_fontbutton;
    GtkWidget *numbers_fontbutton;
    GtkWidget *restore_button;
};

enum
{
    PROP_0,
    PROP_VIEW
};

enum
{
    PRINTING,
    SHOW_PREVIEW,
    DONE,
    LAST_SIGNAL
};

static guint print_job_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (XedPrintJob, xed_print_job, G_TYPE_OBJECT)

static void
set_view (XedPrintJob *job,
          XedView     *view)
{
    job->priv->view = view;
    job->priv->doc = XED_DOCUMENT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)));
}

static void
xed_print_job_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    XedPrintJob *job = XED_PRINT_JOB (object);

    switch (prop_id)
    {
        case PROP_VIEW:
            g_value_set_object (value, job->priv->view);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_print_job_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    XedPrintJob *job = XED_PRINT_JOB (object);

    switch (prop_id)
    {
        case PROP_VIEW:
            set_view (job, g_value_get_object (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_print_job_finalize (GObject *object)
{
    XedPrintJob *job = XED_PRINT_JOB (object);

    g_free (job->priv->status_string);

    if (job->priv->compositor != NULL)
    {
        g_object_unref (job->priv->compositor);
    }

    if (job->priv->operation != NULL)
    {
        g_object_unref (job->priv->operation);
    }

    G_OBJECT_CLASS (xed_print_job_parent_class)->finalize (object);
}

static void
xed_print_job_dispose (GObject *object)
{
    XedPrintJob *job = XED_PRINT_JOB (object);

    g_clear_object (&job->priv->print_settings);

    G_OBJECT_CLASS (xed_print_job_parent_class)->dispose (object);
}

static void
xed_print_job_class_init (XedPrintJobClass *klass)
{
    GObjectClass *object_class;

    object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = xed_print_job_get_property;
    object_class->set_property = xed_print_job_set_property;
    object_class->finalize = xed_print_job_finalize;
    object_class->dispose = xed_print_job_dispose;

    g_object_class_install_property (object_class,
                                     PROP_VIEW,
                                     g_param_spec_object ("view",
                                                          "Xed View",
                                                          "Xed View to print",
                                                          XED_TYPE_VIEW,
                                                          G_PARAM_READWRITE |
                                                          G_PARAM_STATIC_STRINGS |
                                                          G_PARAM_CONSTRUCT_ONLY));

    print_job_signals[PRINTING] =
        g_signal_new ("printing",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedPrintJobClass, printing),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__UINT,
                      G_TYPE_NONE,
                      1,
                      G_TYPE_UINT);

    print_job_signals[SHOW_PREVIEW] =
        g_signal_new ("show-preview",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedPrintJobClass, show_preview),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__OBJECT,
                      G_TYPE_NONE,
                      1,
                      GTK_TYPE_WIDGET);

    print_job_signals[DONE] =
        g_signal_new ("done",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (XedPrintJobClass, done),
                      NULL, NULL,
                      xed_marshal_VOID__UINT_POINTER,
                      G_TYPE_NONE,
                      2,
                      G_TYPE_UINT,
                      G_TYPE_POINTER);

    g_type_class_add_private (object_class, sizeof (XedPrintJobPrivate));
}

static void
line_numbers_checkbutton_toggled (GtkToggleButton *button,
                                  XedPrintJob     *job)
{
    gtk_widget_set_sensitive (job->priv->line_numbers_hbox, gtk_toggle_button_get_active (button));
}

static void
wrap_mode_checkbutton_toggled (GtkToggleButton *button,
                               XedPrintJob     *job)
{
    if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (job->priv->text_wrapping_checkbutton)))
    {
        gtk_widget_set_sensitive (job->priv->do_not_split_checkbutton, FALSE);
        gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON (job->priv->do_not_split_checkbutton), TRUE);
    }
    else
    {
        gtk_widget_set_sensitive (job->priv->do_not_split_checkbutton, TRUE);
        gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON (job->priv->do_not_split_checkbutton), FALSE);
    }
}

static void
restore_button_clicked (GtkButton   *button,
                        XedPrintJob *job)

{
    gchar *body;
    gchar *header;
    gchar *numbers;

    body = g_settings_get_string (job->priv->print_settings, XED_SETTINGS_PRINT_FONT_BODY_PANGO);
    header = g_settings_get_string (job->priv->print_settings, XED_SETTINGS_PRINT_FONT_HEADER_PANGO);
    numbers = g_settings_get_string (job->priv->print_settings, XED_SETTINGS_PRINT_FONT_NUMBERS_PANGO);

    gtk_font_chooser_set_font (GTK_FONT_CHOOSER (job->priv->body_fontbutton), body);
    gtk_font_chooser_set_font (GTK_FONT_CHOOSER (job->priv->headers_fontbutton), header);
    gtk_font_chooser_set_font (GTK_FONT_CHOOSER (job->priv->numbers_fontbutton), numbers);

    g_free (body);
    g_free (header);
    g_free (numbers);
}

static GObject *
create_custom_widget_cb (GtkPrintOperation *operation,
                         XedPrintJob       *job)
{
    GtkBuilder *builder;
    GtkWidget *contents;
    guint line_numbers;
    GtkWrapMode wrap_mode;
    gboolean syntax_hl;
    gboolean print_header;
    gchar *font_body, *font_header, *font_numbers;
    gchar *root_objects[] = {
        "adjustment1",
        "contents",
        NULL
    };

    builder = gtk_builder_new ();
    gtk_builder_add_objects_from_resource (builder, "/org/x/editor/ui/xed-print-preferences.ui", root_objects, NULL);
    contents = GTK_WIDGET (gtk_builder_get_object (builder, "contents"));
    g_object_ref (contents);
    job->priv->syntax_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "syntax_checkbutton"));
    job->priv->line_numbers_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "line_numbers_checkbutton"));
    job->priv->line_numbers_hbox = GTK_WIDGET (gtk_builder_get_object (builder, "line_numbers_hbox"));
    job->priv->line_numbers_spinbutton = GTK_WIDGET (gtk_builder_get_object (builder, "line_numbers_spinbutton"));
    job->priv->page_header_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "page_header_checkbutton"));
    job->priv->text_wrapping_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "text_wrapping_checkbutton"));
    job->priv->do_not_split_checkbutton = GTK_WIDGET (gtk_builder_get_object (builder, "do_not_split_checkbutton"));
    job->priv->fonts_table = GTK_WIDGET (gtk_builder_get_object (builder, "fonts_table"));
    job->priv->body_font_label = GTK_WIDGET (gtk_builder_get_object (builder, "body_font_label"));
    job->priv->body_fontbutton = GTK_WIDGET (gtk_builder_get_object (builder, "body_fontbutton"));
    job->priv->headers_font_label = GTK_WIDGET (gtk_builder_get_object (builder, "headers_font_label"));
    job->priv->headers_fontbutton = GTK_WIDGET (gtk_builder_get_object (builder, "headers_fontbutton"));
    job->priv->numbers_font_label = GTK_WIDGET (gtk_builder_get_object (builder, "numbers_font_label"));
    job->priv->numbers_fontbutton = GTK_WIDGET (gtk_builder_get_object (builder, "numbers_fontbutton"));
    job->priv->restore_button = GTK_WIDGET (gtk_builder_get_object (builder, "restore_button"));
    g_object_unref (builder);

    /* Get all settings values */
    syntax_hl = g_settings_get_boolean (job->priv->print_settings, XED_SETTINGS_PRINT_SYNTAX_HIGHLIGHTING);
    print_header = g_settings_get_boolean (job->priv->print_settings, XED_SETTINGS_PRINT_HEADER);
    line_numbers = g_settings_get_uint (job->priv->print_settings, XED_SETTINGS_PRINT_LINE_NUMBERS);
    font_body = g_settings_get_string (job->priv->print_settings, XED_SETTINGS_PRINT_FONT_BODY_PANGO);
    font_header = g_settings_get_string (job->priv->print_settings, XED_SETTINGS_PRINT_FONT_HEADER_PANGO);
    font_numbers = g_settings_get_string (job->priv->print_settings, XED_SETTINGS_PRINT_FONT_NUMBERS_PANGO);

    /* Print syntax */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (job->priv->syntax_checkbutton), syntax_hl);

    /* Print page headers */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (job->priv->page_header_checkbutton), print_header);

    /* Line numbers */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (job->priv->line_numbers_checkbutton), line_numbers > 0);

    if (line_numbers > 0)
    {
        gtk_spin_button_set_value (GTK_SPIN_BUTTON (job->priv->line_numbers_spinbutton), (guint) line_numbers);
        gtk_widget_set_sensitive (job->priv->line_numbers_hbox, TRUE);
    }
    else
    {
        gtk_spin_button_set_value (GTK_SPIN_BUTTON (job->priv->line_numbers_spinbutton), 1);
        gtk_widget_set_sensitive (job->priv->line_numbers_hbox, FALSE);
    }

    /* Text wrapping */
    wrap_mode = g_settings_get_enum (job->priv->print_settings, XED_SETTINGS_PRINT_WRAP_MODE);

    switch (wrap_mode)
    {
        case GTK_WRAP_WORD:
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (job->priv->text_wrapping_checkbutton), TRUE);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (job->priv->do_not_split_checkbutton), TRUE);
            break;
        case GTK_WRAP_CHAR:
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (job->priv->text_wrapping_checkbutton), TRUE);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (job->priv->do_not_split_checkbutton), FALSE);
            break;
        default:
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (job->priv->text_wrapping_checkbutton), FALSE);
            gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON (job->priv->do_not_split_checkbutton), TRUE);
    }

    gtk_widget_set_sensitive (job->priv->do_not_split_checkbutton, wrap_mode != GTK_WRAP_NONE);

    /* Set initial values */
    gtk_font_chooser_set_font (GTK_FONT_CHOOSER (job->priv->body_fontbutton), font_body);
    g_free (font_body);

    gtk_font_chooser_set_font (GTK_FONT_CHOOSER (job->priv->headers_fontbutton), font_header);
    g_free (font_header);

    gtk_font_chooser_set_font (GTK_FONT_CHOOSER (job->priv->numbers_fontbutton), font_numbers);
    g_free (font_numbers);

    g_signal_connect (job->priv->line_numbers_checkbutton, "toggled",
                      G_CALLBACK (line_numbers_checkbutton_toggled), job);
    g_signal_connect (job->priv->text_wrapping_checkbutton, "toggled",
                      G_CALLBACK (wrap_mode_checkbutton_toggled), job);
    g_signal_connect (job->priv->do_not_split_checkbutton, "toggled",
                      G_CALLBACK (wrap_mode_checkbutton_toggled), job);
    g_signal_connect (job->priv->restore_button, "clicked",
                      G_CALLBACK (restore_button_clicked), job);

    return G_OBJECT (contents);
}

static void
custom_widget_apply_cb (GtkPrintOperation *operation,
                        GtkWidget         *widget,
                        XedPrintJob       *job)
{
    gboolean syntax, page_header;
    const gchar *body, *header, *numbers;
    GtkWrapMode wrap_mode;

    syntax = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (job->priv->syntax_checkbutton));
    page_header = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (job->priv->page_header_checkbutton));
    body = gtk_font_chooser_get_font (GTK_FONT_CHOOSER (job->priv->body_fontbutton));
    header = gtk_font_chooser_get_font (GTK_FONT_CHOOSER (job->priv->headers_fontbutton));
    numbers = gtk_font_chooser_get_font (GTK_FONT_CHOOSER (job->priv->numbers_fontbutton));

    g_settings_set_boolean (job->priv->print_settings, XED_SETTINGS_PRINT_SYNTAX_HIGHLIGHTING, syntax);
    g_settings_set_boolean (job->priv->print_settings, XED_SETTINGS_PRINT_HEADER, page_header);
    g_settings_set_string (job->priv->print_settings, XED_SETTINGS_PRINT_FONT_BODY_PANGO, body);
    g_settings_set_string (job->priv->print_settings, XED_SETTINGS_PRINT_FONT_HEADER_PANGO, header);
    g_settings_set_string (job->priv->print_settings, XED_SETTINGS_PRINT_FONT_NUMBERS_PANGO, numbers);

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (job->priv->line_numbers_checkbutton)))
    {
        g_settings_set_uint (job->priv->print_settings, XED_SETTINGS_PRINT_LINE_NUMBERS,
                             MAX (1, gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (job->priv->line_numbers_spinbutton))));
    }
    else
    {
        g_settings_set_uint (job->priv->print_settings, XED_SETTINGS_PRINT_LINE_NUMBERS, 0);
    }

    if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (job->priv->text_wrapping_checkbutton)))
    {
        wrap_mode = GTK_WRAP_NONE;
    }
    else
    {
        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (job->priv->do_not_split_checkbutton)))
        {
            wrap_mode = GTK_WRAP_WORD;
        }
        else
        {
            wrap_mode = GTK_WRAP_CHAR;
        }
    }

    g_settings_set_enum (job->priv->print_settings, XED_SETTINGS_PRINT_WRAP_MODE, wrap_mode);
}

static void
create_compositor (XedPrintJob *job)
{
    gchar *print_font_body;
    gchar *print_font_header;
    gchar *print_font_numbers;
    gboolean syntax_hl;
    GtkWrapMode wrap_mode;
    guint print_line_numbers;
    gboolean print_header;

    /* Create and initialize print compositor */
    print_font_body = g_settings_get_string (job->priv->print_settings, XED_SETTINGS_PRINT_FONT_BODY_PANGO);
    print_font_header = g_settings_get_string (job->priv->print_settings, XED_SETTINGS_PRINT_FONT_HEADER_PANGO);
    print_font_numbers = g_settings_get_string (job->priv->print_settings, XED_SETTINGS_PRINT_FONT_NUMBERS_PANGO);
    syntax_hl = g_settings_get_boolean (job->priv->print_settings, XED_SETTINGS_PRINT_SYNTAX_HIGHLIGHTING);
    print_line_numbers = g_settings_get_uint (job->priv->print_settings, XED_SETTINGS_PRINT_LINE_NUMBERS);
    print_header = g_settings_get_boolean (job->priv->print_settings, XED_SETTINGS_PRINT_HEADER);
    wrap_mode = g_settings_get_enum (job->priv->print_settings, XED_SETTINGS_PRINT_WRAP_MODE);

    job->priv->compositor = GTK_SOURCE_PRINT_COMPOSITOR (
                    g_object_new (GTK_SOURCE_TYPE_PRINT_COMPOSITOR,
                             "buffer", GTK_SOURCE_BUFFER (job->priv->doc),
                             "tab-width", gtk_source_view_get_tab_width (GTK_SOURCE_VIEW (job->priv->view)),
                             "highlight-syntax", gtk_source_buffer_get_highlight_syntax (GTK_SOURCE_BUFFER (job->priv->doc)) &&
                                                 syntax_hl,
                             "wrap-mode", wrap_mode,
                             "print-line-numbers", print_line_numbers,
                             "print-header", print_header,
                             "print-footer", FALSE,
                             "body-font-name", print_font_body,
                             "line-numbers-font-name", print_font_numbers,
                             "header-font-name", print_font_header,
                             NULL));

    g_free (print_font_body);
    g_free (print_font_header);
    g_free (print_font_numbers);

    if (print_header)
    {
        gchar *doc_name;
        gchar *name_to_display;
        gchar *left;

        doc_name = xed_document_get_uri_for_display (job->priv->doc);
        name_to_display = xed_utils_str_middle_truncate (doc_name, 60);

        left = g_strdup_printf (_("File: %s"), name_to_display);

        /* Translators: %N is the current page number, %Q is the total
         * number of pages (ex. Page 2 of 10)
         */
        gtk_source_print_compositor_set_header_format (job->priv->compositor,
                                                       TRUE,
                                                       left,
                                                       NULL,
                                                       _("Page %N of %Q"));

        g_free (doc_name);
        g_free (name_to_display);
        g_free (left);
    }
}

static void
begin_print_cb (GtkPrintOperation *operation,
                GtkPrintContext   *context,
                XedPrintJob       *job)
{
    create_compositor (job);

    job->priv->status = XED_PRINT_JOB_STATUS_PAGINATING;

    job->priv->progress = 0.0;

    g_signal_emit (job, print_job_signals[PRINTING], 0, job->priv->status);
}

static void
preview_ready (GtkPrintOperationPreview *gtk_preview,
               GtkPrintContext          *context,
               XedPrintJob              *job)
{
    job->priv->is_preview = TRUE;

    g_signal_emit (job, print_job_signals[SHOW_PREVIEW], 0, job->priv->preview);
}

static void
preview_destroyed (GtkWidget                *preview,
                   GtkPrintOperationPreview *gtk_preview)
{
    gtk_print_operation_preview_end_preview (gtk_preview);
}

static gboolean
preview_cb (GtkPrintOperation        *op,
            GtkPrintOperationPreview *gtk_preview,
            GtkPrintContext          *context,
            GtkWindow                *parent,
            XedPrintJob              *job)
{
    job->priv->preview = xed_print_preview_new (op, gtk_preview, context);

    g_signal_connect_after (gtk_preview, "ready",
                            G_CALLBACK (preview_ready), job);

    /* FIXME: should this go in the preview widget itself? */
    g_signal_connect (job->priv->preview, "destroy",
                      G_CALLBACK (preview_destroyed), gtk_preview);

    return TRUE;
}

static gboolean
paginate_cb (GtkPrintOperation *operation,
             GtkPrintContext   *context,
             XedPrintJob       *job)
{
    gboolean res;

    job->priv->status = XED_PRINT_JOB_STATUS_PAGINATING;

    res = gtk_source_print_compositor_paginate (job->priv->compositor, context);

    if (res)
    {
        gint n_pages;

        n_pages = gtk_source_print_compositor_get_n_pages (job->priv->compositor);
        gtk_print_operation_set_n_pages (job->priv->operation, n_pages);
    }

    job->priv->progress = gtk_source_print_compositor_get_pagination_progress (job->priv->compositor);

    /* When previewing, the progress is just for pagination, when printing
     * it's split between pagination and rendering */
    if (!job->priv->is_preview)
    {
        job->priv->progress /= 2.0;
    }

    g_signal_emit (job, print_job_signals[PRINTING], 0, job->priv->status);

    return res;
}

static void
draw_page_cb (GtkPrintOperation *operation,
              GtkPrintContext   *context,
              gint               page_nr,
              XedPrintJob       *job)
{
    gint n_pages;

    /* In preview, pages are drawn on the fly, so rendering is
     * not part of the progress */
    if (!job->priv->is_preview)
    {
        g_free (job->priv->status_string);

        n_pages = gtk_source_print_compositor_get_n_pages (job->priv->compositor);

        job->priv->status = XED_PRINT_JOB_STATUS_DRAWING;

        job->priv->status_string = g_strdup_printf ("Rendering page %d of %d...", page_nr + 1, n_pages);

        job->priv->progress = page_nr / (2.0 * n_pages) + 0.5;

        g_signal_emit (job, print_job_signals[PRINTING], 0, job->priv->status);
    }

    gtk_source_print_compositor_draw_page (job->priv->compositor, context, page_nr);
}

static void
end_print_cb (GtkPrintOperation *operation,
              GtkPrintContext   *context,
              XedPrintJob       *job)
{
    g_object_unref (job->priv->compositor);
    job->priv->compositor = NULL;
}

static void
done_cb (GtkPrintOperation       *operation,
         GtkPrintOperationResult  result,
         XedPrintJob             *job)
{
    GError *error = NULL;
    XedPrintJobResult print_result;

    switch (result)
    {
        case GTK_PRINT_OPERATION_RESULT_CANCEL:
            print_result = XED_PRINT_JOB_RESULT_CANCEL;
            break;

        case GTK_PRINT_OPERATION_RESULT_APPLY:
            print_result = XED_PRINT_JOB_RESULT_OK;
            break;

        case GTK_PRINT_OPERATION_RESULT_ERROR:
            print_result = XED_PRINT_JOB_RESULT_ERROR;
            gtk_print_operation_get_error (operation, &error);
            break;

        default:
            g_return_if_reached ();
    }

    /* Avoid job is destroyed in the handler of the "done" message */
    g_object_ref (job);

    g_signal_emit (job, print_job_signals[DONE], 0, print_result, error);

    g_object_unref (operation);
    job->priv->operation = NULL;

    g_object_unref (job);
}

/* Note that xed_print_job_print can can only be called once on a given XedPrintJob */
GtkPrintOperationResult
xed_print_job_print (XedPrintJob              *job,
                     GtkPrintOperationAction   action,
                     GtkPageSetup             *page_setup,
                     GtkPrintSettings         *settings,
                     GtkWindow                *parent,
                     GError                  **error)
{
    XedPrintJobPrivate *priv;
    gchar *job_name;

    g_return_val_if_fail (job->priv->compositor == NULL, GTK_PRINT_OPERATION_RESULT_ERROR);

    priv = job->priv;

    /* Check if we are previewing */
    priv->is_preview = (action == GTK_PRINT_OPERATION_ACTION_PREVIEW);

    /* Create print operation */
    job->priv->operation = gtk_print_operation_new ();

    if (settings)
    {
        gtk_print_operation_set_print_settings (priv->operation, settings);
    }

    if (page_setup != NULL)
    {
        gtk_print_operation_set_default_page_setup (priv->operation, page_setup);
    }

    job_name = xed_document_get_short_name_for_display (priv->doc);
    gtk_print_operation_set_job_name (priv->operation, job_name);
    g_free (job_name);

    gtk_print_operation_set_embed_page_setup (priv->operation, TRUE);

    gtk_print_operation_set_custom_tab_label (priv->operation, _("Text Editor"));

    gtk_print_operation_set_allow_async (priv->operation, TRUE);

    g_signal_connect (priv->operation, "create-custom-widget",
                      G_CALLBACK (create_custom_widget_cb), job);
    g_signal_connect (priv->operation, "custom-widget-apply",
                      G_CALLBACK (custom_widget_apply_cb), job);
    g_signal_connect (priv->operation, "begin-print",
                      G_CALLBACK (begin_print_cb), job);
    g_signal_connect (priv->operation, "preview",
                      G_CALLBACK (preview_cb), job);
    g_signal_connect (priv->operation, "paginate",
                      G_CALLBACK (paginate_cb), job);
    g_signal_connect (priv->operation, "draw-page",
                      G_CALLBACK (draw_page_cb), job);
    g_signal_connect (priv->operation, "end-print",
                      G_CALLBACK (end_print_cb), job);
    g_signal_connect (priv->operation, "done",
                      G_CALLBACK (done_cb), job);

    return gtk_print_operation_run (priv->operation, action, parent, error);
}

static void
xed_print_job_init (XedPrintJob *job)
{
    job->priv = XED_PRINT_JOB_GET_PRIVATE (job);

    job->priv->print_settings = g_settings_new ("org.x.editor.preferences.print");

    job->priv->status = XED_PRINT_JOB_STATUS_INIT;

    job->priv->status_string = g_strdup (_("Preparing..."));
}

XedPrintJob *
xed_print_job_new (XedView *view)
{
    XedPrintJob *job;

    g_return_val_if_fail (XED_IS_VIEW (view), NULL);

    job = XED_PRINT_JOB (g_object_new (XED_TYPE_PRINT_JOB, "view", view, NULL));

    return job;
}

void
xed_print_job_cancel (XedPrintJob *job)
{
    g_return_if_fail (XED_IS_PRINT_JOB (job));

    gtk_print_operation_cancel (job->priv->operation);
}

const gchar *
xed_print_job_get_status_string (XedPrintJob *job)
{
    g_return_val_if_fail (XED_IS_PRINT_JOB (job), NULL);
    g_return_val_if_fail (job->priv->status_string != NULL, NULL);

    return job->priv->status_string;
}

gdouble
xed_print_job_get_progress (XedPrintJob *job)
{
    g_return_val_if_fail (XED_IS_PRINT_JOB (job), 0.0);

    return job->priv->progress;
}

GtkPrintSettings *
xed_print_job_get_print_settings (XedPrintJob *job)
{
    g_return_val_if_fail (XED_IS_PRINT_JOB (job), NULL);

    return gtk_print_operation_get_print_settings (job->priv->operation);
}

GtkPageSetup *
xed_print_job_get_page_setup (XedPrintJob *job)
{
    g_return_val_if_fail (XED_IS_PRINT_JOB (job), NULL);

    return gtk_print_operation_get_default_page_setup (job->priv->operation);
}
