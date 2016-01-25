/*
 * xedit-print-job.h
 * This file is part of xedit
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
 * Modified by the xedit Team, 1998-2005. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifndef __XEDIT_PRINT_JOB_H__
#define __XEDIT_PRINT_JOB_H__

#include <gtk/gtk.h>
#include <xedit/xedit-view.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XEDIT_TYPE_PRINT_JOB              (xedit_print_job_get_type())
#define XEDIT_PRINT_JOB(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), XEDIT_TYPE_PRINT_JOB, XeditPrintJob))
#define XEDIT_PRINT_JOB_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), XEDIT_TYPE_PRINT_JOB, XeditPrintJobClass))
#define XEDIT_IS_PRINT_JOB(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), XEDIT_TYPE_PRINT_JOB))
#define XEDIT_IS_PRINT_JOB_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XEDIT_TYPE_PRINT_JOB))
#define XEDIT_PRINT_JOB_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), XEDIT_TYPE_PRINT_JOB, XeditPrintJobClass))


typedef enum
{
	XEDIT_PRINT_JOB_STATUS_INIT,
	XEDIT_PRINT_JOB_STATUS_PAGINATING,
	XEDIT_PRINT_JOB_STATUS_DRAWING,
	XEDIT_PRINT_JOB_STATUS_DONE
} XeditPrintJobStatus;

typedef enum
{
	XEDIT_PRINT_JOB_RESULT_OK,
	XEDIT_PRINT_JOB_RESULT_CANCEL,
	XEDIT_PRINT_JOB_RESULT_ERROR	
} XeditPrintJobResult;

/* Private structure type */
typedef struct _XeditPrintJobPrivate XeditPrintJobPrivate;

/*
 * Main object structure
 */
typedef struct _XeditPrintJob XeditPrintJob;


struct _XeditPrintJob 
{
	GObject parent;
	
	/* <private> */
	XeditPrintJobPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XeditPrintJobClass XeditPrintJobClass;

struct _XeditPrintJobClass 
{
	GObjectClass parent_class;

        /* Signals */
	void (* printing) (XeditPrintJob       *job,
	                   XeditPrintJobStatus  status);

	void (* show_preview) (XeditPrintJob   *job,
	                       GtkWidget       *preview);

        void (*done)      (XeditPrintJob       *job,
		           XeditPrintJobResult  result,
                           const GError        *error);
};

/*
 * Public methods
 */
GType			 xedit_print_job_get_type		(void) G_GNUC_CONST;

XeditPrintJob		*xedit_print_job_new			(XeditView                *view);

void			 xedit_print_job_set_export_filename	(XeditPrintJob            *job,
								 const gchar              *filename);

GtkPrintOperationResult	 xedit_print_job_print			(XeditPrintJob            *job,
								 GtkPrintOperationAction   action,
								 GtkPageSetup             *page_setup,
								 GtkPrintSettings         *settings,
								 GtkWindow                *parent,
								 GError                  **error);

void			 xedit_print_job_cancel			(XeditPrintJob            *job);

const gchar		*xedit_print_job_get_status_string	(XeditPrintJob            *job);

gdouble			 xedit_print_job_get_progress		(XeditPrintJob            *job);

GtkPrintSettings	*xedit_print_job_get_print_settings	(XeditPrintJob            *job);

GtkPageSetup		*xedit_print_job_get_page_setup		(XeditPrintJob            *job);

G_END_DECLS

#endif /* __XEDIT_PRINT_JOB_H__ */
