/*
 * gedit-print-job.h
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
 * $Id$
 */

#ifndef __GEDIT_PRINT_JOB_H__
#define __GEDIT_PRINT_JOB_H__

#include <gtk/gtk.h>
#include <gedit/gedit-view.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GEDIT_TYPE_PRINT_JOB              (gedit_print_job_get_type())
#define GEDIT_PRINT_JOB(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GEDIT_TYPE_PRINT_JOB, GeditPrintJob))
#define GEDIT_PRINT_JOB_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GEDIT_TYPE_PRINT_JOB, GeditPrintJobClass))
#define GEDIT_IS_PRINT_JOB(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GEDIT_TYPE_PRINT_JOB))
#define GEDIT_IS_PRINT_JOB_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_PRINT_JOB))
#define GEDIT_PRINT_JOB_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GEDIT_TYPE_PRINT_JOB, GeditPrintJobClass))


typedef enum
{
	GEDIT_PRINT_JOB_STATUS_INIT,
	GEDIT_PRINT_JOB_STATUS_PAGINATING,
	GEDIT_PRINT_JOB_STATUS_DRAWING,
	GEDIT_PRINT_JOB_STATUS_DONE
} GeditPrintJobStatus;

typedef enum
{
	GEDIT_PRINT_JOB_RESULT_OK,
	GEDIT_PRINT_JOB_RESULT_CANCEL,
	GEDIT_PRINT_JOB_RESULT_ERROR	
} GeditPrintJobResult;

/* Private structure type */
typedef struct _GeditPrintJobPrivate GeditPrintJobPrivate;

/*
 * Main object structure
 */
typedef struct _GeditPrintJob GeditPrintJob;


struct _GeditPrintJob 
{
	GObject parent;
	
	/* <private> */
	GeditPrintJobPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GeditPrintJobClass GeditPrintJobClass;

struct _GeditPrintJobClass 
{
	GObjectClass parent_class;

        /* Signals */
	void (* printing) (GeditPrintJob       *job,
	                   GeditPrintJobStatus  status);

	void (* show_preview) (GeditPrintJob   *job,
	                       GtkWidget       *preview);

        void (*done)      (GeditPrintJob       *job,
		           GeditPrintJobResult  result,
                           const GError        *error);
};

/*
 * Public methods
 */
GType			 gedit_print_job_get_type		(void) G_GNUC_CONST;

GeditPrintJob		*gedit_print_job_new			(GeditView                *view);

void			 gedit_print_job_set_export_filename	(GeditPrintJob            *job,
								 const gchar              *filename);

GtkPrintOperationResult	 gedit_print_job_print			(GeditPrintJob            *job,
								 GtkPrintOperationAction   action,
								 GtkPageSetup             *page_setup,
								 GtkPrintSettings         *settings,
								 GtkWindow                *parent,
								 GError                  **error);

void			 gedit_print_job_cancel			(GeditPrintJob            *job);

const gchar		*gedit_print_job_get_status_string	(GeditPrintJob            *job);

gdouble			 gedit_print_job_get_progress		(GeditPrintJob            *job);

GtkPrintSettings	*gedit_print_job_get_print_settings	(GeditPrintJob            *job);

GtkPageSetup		*gedit_print_job_get_page_setup		(GeditPrintJob            *job);

G_END_DECLS

#endif /* __GEDIT_PRINT_JOB_H__ */
