/*
 * xed-print-job.h
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
 * $Id$
 */

#ifndef __XED_PRINT_JOB_H__
#define __XED_PRINT_JOB_H__

#include <gtk/gtk.h>
#include <xed/xed-view.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XED_TYPE_PRINT_JOB              (xed_print_job_get_type())
#define XED_PRINT_JOB(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XED_TYPE_PRINT_JOB, XedPrintJob))
#define XED_PRINT_JOB_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XED_TYPE_PRINT_JOB, XedPrintJobClass))
#define XED_IS_PRINT_JOB(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XED_TYPE_PRINT_JOB))
#define XED_IS_PRINT_JOB_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XED_TYPE_PRINT_JOB))
#define XED_PRINT_JOB_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XED_TYPE_PRINT_JOB, XedPrintJobClass))


typedef enum
{
	XED_PRINT_JOB_STATUS_INIT,
	XED_PRINT_JOB_STATUS_PAGINATING,
	XED_PRINT_JOB_STATUS_DRAWING,
	XED_PRINT_JOB_STATUS_DONE
} XedPrintJobStatus;

typedef enum
{
	XED_PRINT_JOB_RESULT_OK,
	XED_PRINT_JOB_RESULT_CANCEL,
	XED_PRINT_JOB_RESULT_ERROR	
} XedPrintJobResult;

/* Private structure type */
typedef struct _XedPrintJobPrivate XedPrintJobPrivate;

/*
 * Main object structure
 */
typedef struct _XedPrintJob XedPrintJob;


struct _XedPrintJob 
{
	GObject parent;
	
	/* <private> */
	XedPrintJobPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XedPrintJobClass XedPrintJobClass;

struct _XedPrintJobClass 
{
	GObjectClass parent_class;

        /* Signals */
	void (* printing) (XedPrintJob       *job,
	                   XedPrintJobStatus  status);

	void (* show_preview) (XedPrintJob   *job,
	                       GtkWidget       *preview);

        void (*done)      (XedPrintJob       *job,
		           XedPrintJobResult  result,
                           const GError        *error);
};

/*
 * Public methods
 */
GType			 xed_print_job_get_type		(void) G_GNUC_CONST;

XedPrintJob		*xed_print_job_new			(XedView                *view);

void			 xed_print_job_set_export_filename	(XedPrintJob            *job,
								 const gchar              *filename);

GtkPrintOperationResult	 xed_print_job_print			(XedPrintJob            *job,
								 GtkPrintOperationAction   action,
								 GtkPageSetup             *page_setup,
								 GtkPrintSettings         *settings,
								 GtkWindow                *parent,
								 GError                  **error);

void			 xed_print_job_cancel			(XedPrintJob            *job);

const gchar		*xed_print_job_get_status_string	(XedPrintJob            *job);

gdouble			 xed_print_job_get_progress		(XedPrintJob            *job);

GtkPrintSettings	*xed_print_job_get_print_settings	(XedPrintJob            *job);

GtkPageSetup		*xed_print_job_get_page_setup		(XedPrintJob            *job);

G_END_DECLS

#endif /* __XED_PRINT_JOB_H__ */
