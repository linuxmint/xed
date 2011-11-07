/*
 * gedit-print.c
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */
 
/*
 * Modified by the gedit Team, 1998-2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id: gedit-print.c 6022 2007-12-09 14:38:57Z pborelli $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <gtksourceview/gtksourceprintcompositor.h>

#include "gedit-print-job.h"
#include "gedit-debug.h"
#include "gedit-prefs-manager.h"
#include "gedit-print-preview.h"
#include "gedit-marshal.h"
#include "gedit-utils.h"
#include "gedit-dirs.h"


#define GEDIT_PRINT_JOB_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
					    GEDIT_TYPE_PRINT_JOB, \
					    GeditPrintJobPrivate))

struct _GeditPrintJobPrivate
{
	GeditView                *view;
	GeditDocument            *doc;

	GtkPrintOperation        *operation;
	GtkSourcePrintCompositor *compositor;

	GtkPrintSettings         *settings;

	GtkWidget                *preview;

	GeditPrintJobStatus       status;
	
	gchar                    *status_string;

	gdouble			  progress;

	gboolean                  is_preview;

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

G_DEFINE_TYPE (GeditPrintJob, gedit_print_job, G_TYPE_OBJECT)

static void
set_view (GeditPrintJob *job, GeditView *view)
{
	job->priv->view = view;
	job->priv->doc = GEDIT_DOCUMENT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)));
}

static void 
gedit_print_job_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
	GeditPrintJob *job = GEDIT_PRINT_JOB (object);

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
gedit_print_job_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
	GeditPrintJob *job = GEDIT_PRINT_JOB (object);

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
gedit_print_job_finalize (GObject *object)
{
	GeditPrintJob *job = GEDIT_PRINT_JOB (object);

	g_free (job->priv->status_string);
	
	if (job->priv->compositor != NULL)
		g_object_unref (job->priv->compositor);

	if (job->priv->operation != NULL)
		g_object_unref (job->priv->operation);

	G_OBJECT_CLASS (gedit_print_job_parent_class)->finalize (object);
}

static void 
gedit_print_job_class_init (GeditPrintJobClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gedit_print_job_get_property;
	object_class->set_property = gedit_print_job_set_property;
	object_class->finalize = gedit_print_job_finalize;

	g_object_class_install_property (object_class,
					 PROP_VIEW,
					 g_param_spec_object ("view",
							      "Gedit View",
							      "Gedit View to print",
							      GEDIT_TYPE_VIEW,
							      G_PARAM_READWRITE |
							      G_PARAM_STATIC_STRINGS |
							      G_PARAM_CONSTRUCT_ONLY));

	print_job_signals[PRINTING] =
		g_signal_new ("printing",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GeditPrintJobClass, printing),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__UINT,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_UINT);

	print_job_signals[SHOW_PREVIEW] =
		g_signal_new ("show-preview",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GeditPrintJobClass, show_preview),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE,
			      1,
			      GTK_TYPE_WIDGET);

	print_job_signals[DONE] =
		g_signal_new ("done",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GeditPrintJobClass, done),
			      NULL, NULL,
			      gedit_marshal_VOID__UINT_POINTER,
			      G_TYPE_NONE,
			      2,
			      G_TYPE_UINT,
			      G_TYPE_POINTER);
			      			      
	g_type_class_add_private (object_class, sizeof (GeditPrintJobPrivate));
}

static void
line_numbers_checkbutton_toggled (GtkToggleButton *button,
				  GeditPrintJob   *job)
{
	if (gtk_toggle_button_get_active (button))
	{
		gtk_widget_set_sensitive (job->priv->line_numbers_hbox, 
					  gedit_prefs_manager_print_line_numbers_can_set ());
	}
	else
	{
		gtk_widget_set_sensitive (job->priv->line_numbers_hbox, FALSE);
	}
}

static void
wrap_mode_checkbutton_toggled (GtkToggleButton *button,
			       GeditPrintJob   *job)
{
	if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (job->priv->text_wrapping_checkbutton)))
	{
		gtk_widget_set_sensitive (job->priv->do_not_split_checkbutton, 
					  FALSE);
		gtk_toggle_button_set_inconsistent (
			GTK_TOGGLE_BUTTON (job->priv->do_not_split_checkbutton),
					   TRUE);
	}
	else
	{
		gtk_widget_set_sensitive (job->priv->do_not_split_checkbutton,
					  TRUE);
		gtk_toggle_button_set_inconsistent (
			GTK_TOGGLE_BUTTON (job->priv->do_not_split_checkbutton),
					   FALSE);
	}
}

static void
restore_button_clicked (GtkButton     *button,
			GeditPrintJob *job)

{
	if (gedit_prefs_manager_print_font_body_can_set ())
	{
		const gchar *font;

		font = gedit_prefs_manager_get_default_print_font_body ();

		gtk_font_button_set_font_name (
				GTK_FONT_BUTTON (job->priv->body_fontbutton),
				font);
	}
	
	if (gedit_prefs_manager_print_font_header_can_set ())
	{
		const gchar *font;

		font = gedit_prefs_manager_get_default_print_font_header ();

		gtk_font_button_set_font_name (
				GTK_FONT_BUTTON (job->priv->headers_fontbutton),
				font);
	}

	if (gedit_prefs_manager_print_font_numbers_can_set ())
	{
		const gchar *font;

		font = gedit_prefs_manager_get_default_print_font_numbers ();

		gtk_font_button_set_font_name (
				GTK_FONT_BUTTON (job->priv->numbers_fontbutton),
				font);
	}
}

static GObject *
create_custom_widget_cb (GtkPrintOperation *operation, 
			 GeditPrintJob     *job)
{
	gboolean ret;
	GtkWidget *widget;
	GtkWidget *error_widget;
	gchar *font;
	gint line_numbers;
	gboolean can_set;
	GtkWrapMode wrap_mode;
	gchar *file;
	gchar *root_objects[] = {
		"adjustment1",
		"contents",
		NULL
	};

	file = gedit_dirs_get_ui_file ("gedit-print-preferences.ui");
	ret = gedit_utils_get_ui_objects (file,
					  root_objects,
					  &error_widget,
					  "contents", &widget,
					  "syntax_checkbutton", &job->priv->syntax_checkbutton,
					  "line_numbers_checkbutton", &job->priv->line_numbers_checkbutton,
					  "line_numbers_hbox", &job->priv->line_numbers_hbox,
					  "line_numbers_spinbutton", &job->priv->line_numbers_spinbutton,
					  "page_header_checkbutton", &job->priv->page_header_checkbutton,
					  "text_wrapping_checkbutton", &job->priv->text_wrapping_checkbutton,
					  "do_not_split_checkbutton", &job->priv->do_not_split_checkbutton,
					  "fonts_table", &job->priv->fonts_table,
					  "body_font_label", &job->priv->body_font_label,
					  "body_fontbutton", &job->priv->body_fontbutton,
					  "headers_font_label", &job->priv->headers_font_label,
					  "headers_fontbutton", &job->priv->headers_fontbutton,
					  "numbers_font_label", &job->priv->numbers_font_label,
					  "numbers_fontbutton", &job->priv->numbers_fontbutton,
					  "restore_button", &job->priv->restore_button,
					  NULL);
	g_free (file);

	if (!ret)
	{
		return G_OBJECT (error_widget);
	}

	/* Print syntax */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (job->priv->syntax_checkbutton),
				      gedit_prefs_manager_get_print_syntax_hl ());
	gtk_widget_set_sensitive (job->priv->syntax_checkbutton,
				  gedit_prefs_manager_print_syntax_hl_can_set ());

	/* Print page headers */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (job->priv->page_header_checkbutton),
				      gedit_prefs_manager_get_print_header ());
	gtk_widget_set_sensitive (job->priv->page_header_checkbutton,
				  gedit_prefs_manager_print_header_can_set ());

	/* Line numbers */
	line_numbers =  gedit_prefs_manager_get_print_line_numbers ();
	can_set = gedit_prefs_manager_print_line_numbers_can_set ();

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (job->priv->line_numbers_checkbutton),
				      line_numbers > 0);
	gtk_widget_set_sensitive (job->priv->line_numbers_checkbutton, can_set);

	if (line_numbers > 0)
	{
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (job->priv->line_numbers_spinbutton),
					   (guint) line_numbers);
		gtk_widget_set_sensitive (job->priv->line_numbers_hbox, can_set);	
	}
	else
	{
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (job->priv->line_numbers_spinbutton),
					   1);
		gtk_widget_set_sensitive (job->priv->line_numbers_hbox, FALSE);
	}

	/* Text wrapping */
	wrap_mode = gedit_prefs_manager_get_print_wrap_mode ();

	switch (wrap_mode)
	{
		case GTK_WRAP_WORD:
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (job->priv->text_wrapping_checkbutton), TRUE);
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (job->priv->do_not_split_checkbutton), TRUE);
			break;
		case GTK_WRAP_CHAR:
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (job->priv->text_wrapping_checkbutton), TRUE);
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (job->priv->do_not_split_checkbutton), FALSE);
			break;
		default:
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (job->priv->text_wrapping_checkbutton), FALSE);
			gtk_toggle_button_set_inconsistent (
				GTK_TOGGLE_BUTTON (job->priv->do_not_split_checkbutton), TRUE);
	}

	can_set = gedit_prefs_manager_print_wrap_mode_can_set ();

	gtk_widget_set_sensitive (job->priv->text_wrapping_checkbutton, can_set);
	gtk_widget_set_sensitive (job->priv->do_not_split_checkbutton, 
				  can_set && (wrap_mode != GTK_WRAP_NONE));

	/* Set initial values */
	font = gedit_prefs_manager_get_print_font_body ();
	gtk_font_button_set_font_name (GTK_FONT_BUTTON (job->priv->body_fontbutton),
				       font);
	g_free (font);

	font = gedit_prefs_manager_get_print_font_header ();
	gtk_font_button_set_font_name (GTK_FONT_BUTTON (job->priv->headers_fontbutton),
				       font);
	g_free (font);

	font = gedit_prefs_manager_get_print_font_numbers ();
	gtk_font_button_set_font_name (GTK_FONT_BUTTON (job->priv->numbers_fontbutton),
				       font);
	g_free (font);

	can_set = gedit_prefs_manager_print_font_body_can_set ();
	gtk_widget_set_sensitive (job->priv->body_fontbutton, can_set);
	gtk_widget_set_sensitive (job->priv->body_font_label, can_set);

	can_set = gedit_prefs_manager_print_font_header_can_set ();
	gtk_widget_set_sensitive (job->priv->headers_fontbutton, can_set);
	gtk_widget_set_sensitive (job->priv->headers_font_label, can_set);

	can_set = gedit_prefs_manager_print_font_numbers_can_set ();
	gtk_widget_set_sensitive (job->priv->numbers_fontbutton, can_set);
	gtk_widget_set_sensitive (job->priv->numbers_font_label, can_set);

	g_signal_connect (job->priv->line_numbers_checkbutton,
			  "toggled",
			  G_CALLBACK (line_numbers_checkbutton_toggled),
			  job);
	g_signal_connect (job->priv->text_wrapping_checkbutton,
			  "toggled",
			  G_CALLBACK (wrap_mode_checkbutton_toggled),
			  job);
	g_signal_connect (job->priv->do_not_split_checkbutton,
			  "toggled",
			  G_CALLBACK (wrap_mode_checkbutton_toggled),
			  job);
	g_signal_connect (job->priv->restore_button,
			  "clicked",
			  G_CALLBACK (restore_button_clicked),
			  job);

	return G_OBJECT (widget);
}

static void
custom_widget_apply_cb (GtkPrintOperation *operation,
			GtkWidget         *widget,
			GeditPrintJob     *job)
{
	gedit_prefs_manager_set_print_syntax_hl (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (job->priv->syntax_checkbutton)));

	gedit_prefs_manager_set_print_header (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (job->priv->page_header_checkbutton)));

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (job->priv->line_numbers_checkbutton)))
	{
		gedit_prefs_manager_set_print_line_numbers (
			MAX (1, gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (job->priv->line_numbers_spinbutton))));
	}
	else
	{
		gedit_prefs_manager_set_print_line_numbers (0);
	}

	if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (job->priv->text_wrapping_checkbutton)))
	{
		gedit_prefs_manager_set_print_wrap_mode (GTK_WRAP_NONE);
	}
	else
	{
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (job->priv->do_not_split_checkbutton)))
		{
			gedit_prefs_manager_set_print_wrap_mode (GTK_WRAP_WORD);
		}
		else
		{
			gedit_prefs_manager_set_print_wrap_mode (GTK_WRAP_CHAR);
		}	
	}

	gedit_prefs_manager_set_print_font_body (gtk_font_button_get_font_name (GTK_FONT_BUTTON (job->priv->body_fontbutton)));
	gedit_prefs_manager_set_print_font_header (gtk_font_button_get_font_name (GTK_FONT_BUTTON (job->priv->headers_fontbutton)));
	gedit_prefs_manager_set_print_font_numbers (gtk_font_button_get_font_name (GTK_FONT_BUTTON (job->priv->numbers_fontbutton)));
}

static void
create_compositor (GeditPrintJob *job)
{
	gchar *print_font_body;
	gchar *print_font_header;
	gchar *print_font_numbers;
	
	/* Create and initialize print compositor */
	print_font_body = gedit_prefs_manager_get_print_font_body ();
	print_font_header = gedit_prefs_manager_get_print_font_header ();
	print_font_numbers = gedit_prefs_manager_get_print_font_numbers ();
	
	job->priv->compositor = GTK_SOURCE_PRINT_COMPOSITOR (
					g_object_new (GTK_TYPE_SOURCE_PRINT_COMPOSITOR,
						     "buffer", GTK_SOURCE_BUFFER (job->priv->doc),
						     "tab-width", gtk_source_view_get_tab_width (GTK_SOURCE_VIEW (job->priv->view)),
						     "highlight-syntax", gtk_source_buffer_get_highlight_syntax (GTK_SOURCE_BUFFER (job->priv->doc)) &&
					   				 gedit_prefs_manager_get_print_syntax_hl (),
						     "wrap-mode", gedit_prefs_manager_get_print_wrap_mode (),
						     "print-line-numbers", gedit_prefs_manager_get_print_line_numbers (),
						     "print-header", gedit_prefs_manager_get_print_header (),
						     "print-footer", FALSE,
						     "body-font-name", print_font_body,
						     "line-numbers-font-name", print_font_numbers,
						     "header-font-name", print_font_header,
						     NULL));

	g_free (print_font_body);
	g_free (print_font_header);
	g_free (print_font_numbers);
	
	if (gedit_prefs_manager_get_print_header ())
	{
		gchar *doc_name;
		gchar *name_to_display;
		gchar *left;

		doc_name = gedit_document_get_uri_for_display (job->priv->doc);
		name_to_display = gedit_utils_str_middle_truncate (doc_name, 60);

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
	        GeditPrintJob     *job)
{
	create_compositor (job);

	job->priv->status = GEDIT_PRINT_JOB_STATUS_PAGINATING;

	job->priv->progress = 0.0;
	
	g_signal_emit (job, print_job_signals[PRINTING], 0, job->priv->status);
}

static void
preview_ready (GtkPrintOperationPreview *gtk_preview,
	       GtkPrintContext          *context,
	       GeditPrintJob            *job)
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
	    GeditPrintJob            *job)
{
	job->priv->preview = gedit_print_preview_new (op, gtk_preview, context);

	g_signal_connect_after (gtk_preview,
			        "ready",
				G_CALLBACK (preview_ready),
				job);

	/* FIXME: should this go in the preview widget itself? */
	g_signal_connect (job->priv->preview,
			  "destroy",
			  G_CALLBACK (preview_destroyed),
			  gtk_preview);

	return TRUE;
}

static gboolean
paginate_cb (GtkPrintOperation *operation, 
	     GtkPrintContext   *context,
	     GeditPrintJob     *job)
{
	gboolean res;	
	
	job->priv->status = GEDIT_PRINT_JOB_STATUS_PAGINATING;
	
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
		job->priv->progress /= 2.0;

	g_signal_emit (job, print_job_signals[PRINTING], 0, job->priv->status);

	return res;
}

static void
draw_page_cb (GtkPrintOperation *operation,
	      GtkPrintContext   *context,
	      gint               page_nr,
	      GeditPrintJob     *job)
{
	gint n_pages;

	/* In preview, pages are drawn on the fly, so rendering is
	 * not part of the progress */
	if (!job->priv->is_preview)
	{
		g_free (job->priv->status_string);
	
		n_pages = gtk_source_print_compositor_get_n_pages (job->priv->compositor);
	
		job->priv->status = GEDIT_PRINT_JOB_STATUS_DRAWING;
	
		job->priv->status_string = g_strdup_printf ("Rendering page %d of %d...", 
							    page_nr + 1,
							    n_pages);
	
		job->priv->progress = page_nr / (2.0 * n_pages) + 0.5;

		g_signal_emit (job, print_job_signals[PRINTING], 0, job->priv->status);
	}	

	gtk_source_print_compositor_draw_page (job->priv->compositor, context, page_nr);
}

static void
end_print_cb (GtkPrintOperation *operation, 
	      GtkPrintContext   *context,
	      GeditPrintJob     *job)
{
	g_object_unref (job->priv->compositor);
	job->priv->compositor = NULL;
}

static void
done_cb (GtkPrintOperation       *operation,
	 GtkPrintOperationResult  result,
	 GeditPrintJob           *job)
{
	GError *error = NULL;
	GeditPrintJobResult print_result;

	switch (result) 
	{
		case GTK_PRINT_OPERATION_RESULT_CANCEL:
			print_result = GEDIT_PRINT_JOB_RESULT_CANCEL;
			break;

		case GTK_PRINT_OPERATION_RESULT_APPLY:
			print_result = GEDIT_PRINT_JOB_RESULT_OK;
			break;

		case GTK_PRINT_OPERATION_RESULT_ERROR:
			print_result = GEDIT_PRINT_JOB_RESULT_ERROR;
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

/* Note that gedit_print_job_print can can only be called once on a given GeditPrintJob */
GtkPrintOperationResult	 
gedit_print_job_print (GeditPrintJob            *job,
		       GtkPrintOperationAction   action,
		       GtkPageSetup             *page_setup,
		       GtkPrintSettings         *settings,
		       GtkWindow                *parent,
		       GError                  **error)
{
	GeditPrintJobPrivate *priv;
	gchar *job_name;

	g_return_val_if_fail (job->priv->compositor == NULL, GTK_PRINT_OPERATION_RESULT_ERROR);

	priv = job->priv;

	/* Check if we are previewing */
	priv->is_preview = (action == GTK_PRINT_OPERATION_ACTION_PREVIEW);

	/* Create print operation */
	job->priv->operation = gtk_print_operation_new ();

	if (settings)
		gtk_print_operation_set_print_settings (priv->operation,
							settings);

	if (page_setup != NULL)
		gtk_print_operation_set_default_page_setup (priv->operation,
							    page_setup);

	job_name = gedit_document_get_short_name_for_display (priv->doc);
	gtk_print_operation_set_job_name (priv->operation, job_name);
	g_free (job_name);

#if GTK_CHECK_VERSION (2, 17, 4)
	gtk_print_operation_set_embed_page_setup (priv->operation, TRUE);
#endif

	gtk_print_operation_set_custom_tab_label (priv->operation,
						  _("Text Editor"));

	gtk_print_operation_set_allow_async (priv->operation, TRUE);

	g_signal_connect (priv->operation,
			  "create-custom-widget", 
			  G_CALLBACK (create_custom_widget_cb),
			  job);
	g_signal_connect (priv->operation,
			  "custom-widget-apply", 
			  G_CALLBACK (custom_widget_apply_cb), 
			  job);
  	g_signal_connect (priv->operation,
			  "begin-print", 
			  G_CALLBACK (begin_print_cb),
			  job);
	g_signal_connect (priv->operation,
			  "preview",
			  G_CALLBACK (preview_cb),
			  job);
  	g_signal_connect (priv->operation,
			  "paginate", 
			  G_CALLBACK (paginate_cb),
			  job);
	g_signal_connect (priv->operation,
			  "draw-page", 
			  G_CALLBACK (draw_page_cb),
			  job);
	g_signal_connect (priv->operation,
			  "end-print", 
			  G_CALLBACK (end_print_cb),
			  job);
	g_signal_connect (priv->operation,
			  "done", 
			  G_CALLBACK (done_cb),
			  job);			  

	return gtk_print_operation_run (priv->operation, 
					action,
					parent,
					error);
}

static void
gedit_print_job_init (GeditPrintJob *job)
{
	job->priv = GEDIT_PRINT_JOB_GET_PRIVATE (job);
	
	job->priv->status = GEDIT_PRINT_JOB_STATUS_INIT;
	
	job->priv->status_string = g_strdup (_("Preparing..."));
}

GeditPrintJob *
gedit_print_job_new (GeditView *view)
{
	GeditPrintJob *job;
	
	g_return_val_if_fail (GEDIT_IS_VIEW (view), NULL);
	
	job = GEDIT_PRINT_JOB (g_object_new (GEDIT_TYPE_PRINT_JOB,
					     "view", view,
					      NULL));

	return job;
}

void
gedit_print_job_cancel (GeditPrintJob *job)
{
	g_return_if_fail (GEDIT_IS_PRINT_JOB (job));

	gtk_print_operation_cancel (job->priv->operation);
}

const gchar *
gedit_print_job_get_status_string (GeditPrintJob *job)
{
	g_return_val_if_fail (GEDIT_IS_PRINT_JOB (job), NULL);
	g_return_val_if_fail (job->priv->status_string != NULL, NULL);
	
	return job->priv->status_string;
}

gdouble
gedit_print_job_get_progress (GeditPrintJob *job)
{
	g_return_val_if_fail (GEDIT_IS_PRINT_JOB (job), 0.0);

	return job->priv->progress;
}

GtkPrintSettings *
gedit_print_job_get_print_settings (GeditPrintJob *job)
{
	g_return_val_if_fail (GEDIT_IS_PRINT_JOB (job), NULL);

	return gtk_print_operation_get_print_settings (job->priv->operation);
}

GtkPageSetup *
gedit_print_job_get_page_setup (GeditPrintJob *job)
{
	g_return_val_if_fail (GEDIT_IS_PRINT_JOB (job), NULL);

	return gtk_print_operation_get_default_page_setup (job->priv->operation);
}
