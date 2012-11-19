/*
 * pluma-print-job.h
 * This file is part of pluma
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
 * Modified by the pluma Team, 1998-2005. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __PLUMA_PRINT_JOB_H__
#define __PLUMA_PRINT_JOB_H__

#include <gtk/gtk.h>
#include <pluma/pluma-view.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PLUMA_TYPE_PRINT_JOB              (pluma_print_job_get_type())
#define PLUMA_PRINT_JOB(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), PLUMA_TYPE_PRINT_JOB, PlumaPrintJob))
#define PLUMA_PRINT_JOB_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), PLUMA_TYPE_PRINT_JOB, PlumaPrintJobClass))
#define PLUMA_IS_PRINT_JOB(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), PLUMA_TYPE_PRINT_JOB))
#define PLUMA_IS_PRINT_JOB_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUMA_TYPE_PRINT_JOB))
#define PLUMA_PRINT_JOB_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), PLUMA_TYPE_PRINT_JOB, PlumaPrintJobClass))


typedef enum
{
	PLUMA_PRINT_JOB_STATUS_INIT,
	PLUMA_PRINT_JOB_STATUS_PAGINATING,
	PLUMA_PRINT_JOB_STATUS_DRAWING,
	PLUMA_PRINT_JOB_STATUS_DONE
} PlumaPrintJobStatus;

typedef enum
{
	PLUMA_PRINT_JOB_RESULT_OK,
	PLUMA_PRINT_JOB_RESULT_CANCEL,
	PLUMA_PRINT_JOB_RESULT_ERROR	
} PlumaPrintJobResult;

/* Private structure type */
typedef struct _PlumaPrintJobPrivate PlumaPrintJobPrivate;

/*
 * Main object structure
 */
typedef struct _PlumaPrintJob PlumaPrintJob;


struct _PlumaPrintJob 
{
	GObject parent;
	
	/* <private> */
	PlumaPrintJobPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _PlumaPrintJobClass PlumaPrintJobClass;

struct _PlumaPrintJobClass 
{
	GObjectClass parent_class;

        /* Signals */
	void (* printing) (PlumaPrintJob       *job,
	                   PlumaPrintJobStatus  status);

	void (* show_preview) (PlumaPrintJob   *job,
	                       GtkWidget       *preview);

        void (*done)      (PlumaPrintJob       *job,
		           PlumaPrintJobResult  result,
                           const GError        *error);
};

/*
 * Public methods
 */
GType			 pluma_print_job_get_type		(void) G_GNUC_CONST;

PlumaPrintJob		*pluma_print_job_new			(PlumaView                *view);

void			 pluma_print_job_set_export_filename	(PlumaPrintJob            *job,
								 const gchar              *filename);

GtkPrintOperationResult	 pluma_print_job_print			(PlumaPrintJob            *job,
								 GtkPrintOperationAction   action,
								 GtkPageSetup             *page_setup,
								 GtkPrintSettings         *settings,
								 GtkWindow                *parent,
								 GError                  **error);

void			 pluma_print_job_cancel			(PlumaPrintJob            *job);

const gchar		*pluma_print_job_get_status_string	(PlumaPrintJob            *job);

gdouble			 pluma_print_job_get_progress		(PlumaPrintJob            *job);

GtkPrintSettings	*pluma_print_job_get_print_settings	(PlumaPrintJob            *job);

GtkPageSetup		*pluma_print_job_get_page_setup		(PlumaPrintJob            *job);

G_END_DECLS

#endif /* __PLUMA_PRINT_JOB_H__ */
