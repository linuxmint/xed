/*
 * pluma-notebook.c
 * This file is part of pluma
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
 * Modified by the pluma Team, 2005. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 */

/* This file is a modified version of the epiphany file ephy-notebook.c
 * Here the relevant copyright:
 *
 *  Copyright (C) 2002 Christophe Fergeau
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003, 2004 Christian Persch
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "pluma-notebook.h"
#include "pluma-tab.h"
#include "pluma-tab-label.h"
#include "pluma-marshal.h"
#include "pluma-window.h"

#ifdef BUILD_SPINNER
#include "pluma-spinner.h"
#endif

#define AFTER_ALL_TABS -1
#define NOT_IN_APP_WINDOWS -2

#define PLUMA_NOTEBOOK_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), PLUMA_TYPE_NOTEBOOK, PlumaNotebookPrivate))

struct _PlumaNotebookPrivate
{
	GList         *focused_pages;
	gulong         motion_notify_handler_id;
	gint           x_start;
	gint           y_start;
	gint           drag_in_progress : 1;
	gint	       always_show_tabs : 1;
	gint           close_buttons_sensitive : 1;
	gint           tab_drag_and_drop_enabled : 1;
	guint          destroy_has_run : 1;
};

G_DEFINE_TYPE(PlumaNotebook, pluma_notebook, GTK_TYPE_NOTEBOOK)

static void pluma_notebook_finalize (GObject *object);

static gboolean pluma_notebook_change_current_page (GtkNotebook *notebook,
						    gint         offset);

static void move_current_tab_to_another_notebook  (PlumaNotebook  *src,
						   PlumaNotebook  *dest,
						   GdkEventMotion *event,
						   gint            dest_position);

/* Local variables */
static GdkCursor *cursor = NULL;

/* Signals */
enum
{
	TAB_ADDED,
	TAB_REMOVED,
	TABS_REORDERED,
	TAB_DETACHED,
	TAB_CLOSE_REQUEST,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static void
#if GTK_CHECK_VERSION (3, 0, 0)
pluma_notebook_dispose (GObject *object)
#else
pluma_notebook_destroy (GtkObject *object)
#endif
{
	PlumaNotebook *notebook = PLUMA_NOTEBOOK (object);

	if (!notebook->priv->destroy_has_run)
	{
		GList *children, *l;

		children = gtk_container_get_children (GTK_CONTAINER (notebook));

		for (l = children; l != NULL; l = g_list_next (l))
		{
			pluma_notebook_remove_tab (notebook,
						   PLUMA_TAB (l->data));
		}

		g_list_free (children);
		notebook->priv->destroy_has_run = TRUE;
	}

#if GTK_CHECK_VERSION (3, 0, 0)
	G_OBJECT_CLASS (pluma_notebook_parent_class)->dispose (object);
#else
	GTK_OBJECT_CLASS (pluma_notebook_parent_class)->destroy (object);
#endif
}

static void
pluma_notebook_class_init (PlumaNotebookClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
#if !GTK_CHECK_VERSION (3, 0, 0)
	GtkObjectClass *gtkobject_class = GTK_OBJECT_CLASS (klass);
#endif
	GtkNotebookClass *notebook_class = GTK_NOTEBOOK_CLASS (klass);

	object_class->finalize = pluma_notebook_finalize;
#if GTK_CHECK_VERSION (3, 0, 0)
	object_class->dispose = pluma_notebook_dispose;
#else
	gtkobject_class->destroy = pluma_notebook_destroy;
#endif

	notebook_class->change_current_page = pluma_notebook_change_current_page;

	signals[TAB_ADDED] =
		g_signal_new ("tab_added",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (PlumaNotebookClass, tab_added),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE,
			      1,
			      PLUMA_TYPE_TAB);
	signals[TAB_REMOVED] =
		g_signal_new ("tab_removed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (PlumaNotebookClass, tab_removed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE,
			      1,
			      PLUMA_TYPE_TAB);
	signals[TAB_DETACHED] =
		g_signal_new ("tab_detached",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (PlumaNotebookClass, tab_detached),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE,
			      1,
			      PLUMA_TYPE_TAB);
	signals[TABS_REORDERED] =
		g_signal_new ("tabs_reordered",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (PlumaNotebookClass, tabs_reordered),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
	signals[TAB_CLOSE_REQUEST] =
		g_signal_new ("tab-close-request",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (PlumaNotebookClass, tab_close_request),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE,
			      1,
			      PLUMA_TYPE_TAB);

	g_type_class_add_private (object_class, sizeof(PlumaNotebookPrivate));
}

static PlumaNotebook *
find_notebook_at_pointer (gint abs_x, gint abs_y)
{
	GdkWindow *win_at_pointer;
	GdkWindow *toplevel_win;
	gpointer toplevel = NULL;
	gint x, y;

	/* FIXME multi-head */
	win_at_pointer = gdk_window_at_pointer (&x, &y);
	if (win_at_pointer == NULL)
	{
		/* We are outside all windows of the same application */
		return NULL;
	}

	toplevel_win = gdk_window_get_toplevel (win_at_pointer);

	/* get the GtkWidget which owns the toplevel GdkWindow */
	gdk_window_get_user_data (toplevel_win, &toplevel);

	/* toplevel should be an PlumaWindow */
	if ((toplevel != NULL) && 
	    PLUMA_IS_WINDOW (toplevel))
	{
		return PLUMA_NOTEBOOK (_pluma_window_get_notebook
						(PLUMA_WINDOW (toplevel)));
	}

	/* We are outside all windows containing a notebook */
	return NULL;
}

static gboolean
is_in_notebook_window (PlumaNotebook *notebook,
		       gint           abs_x, 
		       gint           abs_y)
{
	PlumaNotebook *nb_at_pointer;

	g_return_val_if_fail (notebook != NULL, FALSE);

	nb_at_pointer = find_notebook_at_pointer (abs_x, abs_y);

	return (nb_at_pointer == notebook);
}

static gint
find_tab_num_at_pos (PlumaNotebook *notebook, 
		     gint           abs_x, 
		     gint           abs_y)
{
	GtkPositionType tab_pos;
	int page_num = 0;
	GtkNotebook *nb = GTK_NOTEBOOK (notebook);
	GtkWidget *page;

	tab_pos = gtk_notebook_get_tab_pos (GTK_NOTEBOOK (notebook));

#if !GTK_CHECK_VERSION (3, 0, 0)
	if (GTK_NOTEBOOK (notebook)->first_tab == NULL)
	{
		return AFTER_ALL_TABS;
	}
#endif

	/* For some reason unfullscreen + quick click can
	   cause a wrong click event to be reported to the tab */
	if (!is_in_notebook_window (notebook, abs_x, abs_y))
	{
		return NOT_IN_APP_WINDOWS;
	}

	while ((page = gtk_notebook_get_nth_page (nb, page_num)) != NULL)
	{
		GtkAllocation allocation;
		GtkWidget *tab;
		gint max_x, max_y;
		gint x_root, y_root;

		tab = gtk_notebook_get_tab_label (nb, page);
		g_return_val_if_fail (tab != NULL, AFTER_ALL_TABS);

#if GTK_CHECK_VERSION (3, 0, 0)
		if (!gtk_widget_get_mapped (tab))
#else
		if (!GTK_WIDGET_MAPPED (GTK_WIDGET (tab)))
#endif
		{
			++page_num;
			continue;
		}

#if GTK_CHECK_VERSION (3, 0, 0)
		gdk_window_get_origin (GDK_WINDOW (gtk_widget_get_window (tab)),
#else
		gdk_window_get_origin (GDK_WINDOW (tab->window),
#endif
				       &x_root, &y_root);

#if GTK_CHECK_VERSION (3, 0, 0)
		gtk_widget_get_allocation(tab, &allocation);
#else
		allocation = tab->allocation;
#endif

		max_x = x_root + allocation.x + allocation.width;
		max_y = y_root + allocation.y + allocation.height;

		if (((tab_pos == GTK_POS_TOP) || 
		     (tab_pos == GTK_POS_BOTTOM)) &&
		    (abs_x <= max_x))
		{
			return page_num;
		}
		else if (((tab_pos == GTK_POS_LEFT) || 
		          (tab_pos == GTK_POS_RIGHT)) && 
		         (abs_y <= max_y))
		{
			return page_num;
		}

		++page_num;
	}
	
	return AFTER_ALL_TABS;
}

static gint 
find_notebook_and_tab_at_pos (gint            abs_x, 
			      gint            abs_y,
			      PlumaNotebook **notebook,
			      gint           *page_num)
{
	*notebook = find_notebook_at_pointer (abs_x, abs_y);
	if (*notebook == NULL)
	{
		return NOT_IN_APP_WINDOWS;
	}
	
	*page_num = find_tab_num_at_pos (*notebook, abs_x, abs_y);

	if (*page_num < 0)
	{
		return *page_num;
	}
	else
	{
		return 0;
	}
}

/**
 * pluma_notebook_move_tab:
 * @src: a #PlumaNotebook
 * @dest: a #PlumaNotebook
 * @tab: a #PlumaTab
 * @dest_position: the position for @tab
 *
 * Moves @tab from @src to @dest.
 * If dest_position is greater than or equal to the number of tabs 
 * of the destination nootebook or negative, tab will be moved to the 
 * end of the tabs.
 */
void
pluma_notebook_move_tab (PlumaNotebook *src,
			 PlumaNotebook *dest,
			 PlumaTab      *tab,
			 gint           dest_position)
{
	g_return_if_fail (PLUMA_IS_NOTEBOOK (src));	
	g_return_if_fail (PLUMA_IS_NOTEBOOK (dest));
	g_return_if_fail (src != dest);
	g_return_if_fail (PLUMA_IS_TAB (tab));

	/* make sure the tab isn't destroyed while we move it */
	g_object_ref (tab);
	pluma_notebook_remove_tab (src, tab);
	pluma_notebook_add_tab (dest, tab, dest_position, TRUE);
	g_object_unref (tab);
}

/**
 * pluma_notebook_reorder_tab:
 * @src: a #PlumaNotebook
 * @tab: a #PlumaTab
 * @dest_position: the position for @tab
 *
 * Reorders the page containing @tab, so that it appears in @dest_position position.
 * If dest_position is greater than or equal to the number of tabs 
 * of the destination notebook or negative, tab will be moved to the 
 * end of the tabs.
 */
void
pluma_notebook_reorder_tab (PlumaNotebook *src,
			    PlumaTab      *tab,
			    gint           dest_position)
{
	gint old_position;
	
	g_return_if_fail (PLUMA_IS_NOTEBOOK (src));	
	g_return_if_fail (PLUMA_IS_TAB (tab));

	old_position = gtk_notebook_page_num (GTK_NOTEBOOK (src), 
				    	      GTK_WIDGET (tab));
				    	      
	if (old_position == dest_position)
		return;

	gtk_notebook_reorder_child (GTK_NOTEBOOK (src), 
				    GTK_WIDGET (tab),
				    dest_position);
		
	if (!src->priv->drag_in_progress)
	{
		g_signal_emit (G_OBJECT (src), 
			       signals[TABS_REORDERED], 
			       0);
	}
}

static void
drag_start (PlumaNotebook *notebook,
	    guint32        time)
{
	notebook->priv->drag_in_progress = TRUE;

	/* get a new cursor, if necessary */
	/* FIXME multi-head */
	if (cursor == NULL)
		cursor = gdk_cursor_new (GDK_FLEUR);

	/* grab the pointer */
	gtk_grab_add (GTK_WIDGET (notebook));

	/* FIXME multi-head */
	if (!gdk_pointer_is_grabbed ())
	{
		gdk_pointer_grab (gtk_widget_get_window (GTK_WIDGET (notebook)),
				  FALSE,
				  GDK_BUTTON1_MOTION_MASK | GDK_BUTTON_RELEASE_MASK,
				  NULL, 
				  cursor, 
				  time);
	}
}

static void
drag_stop (PlumaNotebook *notebook)
{
	if (notebook->priv->drag_in_progress)
	{
		g_signal_emit (G_OBJECT (notebook), 
			       signals[TABS_REORDERED], 
			       0);
	}

	notebook->priv->drag_in_progress = FALSE;
	if (notebook->priv->motion_notify_handler_id != 0)
	{
		g_signal_handler_disconnect (G_OBJECT (notebook),
					     notebook->priv->motion_notify_handler_id);
		notebook->priv->motion_notify_handler_id = 0;
	}
}

/* This function is only called during dnd, we don't need to emit TABS_REORDERED
 * here, instead we do it on drag_stop
 */
static void
move_current_tab (PlumaNotebook *notebook,
	          gint           dest_position)
{
	gint cur_page_num;

	cur_page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));

	if (dest_position != cur_page_num)
	{
		GtkWidget *cur_tab;
		
		cur_tab = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook),
						     cur_page_num);
						     
		pluma_notebook_reorder_tab (PLUMA_NOTEBOOK (notebook),
					    PLUMA_TAB (cur_tab),
					    dest_position);
	}
}

static gboolean
motion_notify_cb (PlumaNotebook  *notebook,
		  GdkEventMotion *event,
		  gpointer        data)
{
	PlumaNotebook *dest;
	gint page_num;
	gint result;

	if (notebook->priv->drag_in_progress == FALSE)
	{
		if (notebook->priv->tab_drag_and_drop_enabled == FALSE)
			return FALSE;
			
		if (gtk_drag_check_threshold (GTK_WIDGET (notebook),
					      notebook->priv->x_start,
					      notebook->priv->y_start,
					      event->x_root, 
					      event->y_root))
		{
			drag_start (notebook, event->time);
			return TRUE;
		}

		return FALSE;
	}

	result = find_notebook_and_tab_at_pos ((gint)event->x_root,
					       (gint)event->y_root,
					       &dest, 
					       &page_num);

	if (result != NOT_IN_APP_WINDOWS)
	{
		if (dest != notebook)
		{
			move_current_tab_to_another_notebook (notebook, 
							      dest,
						      	      event, 
						      	      page_num);
		}
		else
		{
			g_return_val_if_fail (page_num >= -1, FALSE);
			move_current_tab (notebook, page_num);
		}
	}

	return FALSE;
}

static void
move_current_tab_to_another_notebook (PlumaNotebook  *src,
				      PlumaNotebook  *dest,
				      GdkEventMotion *event,
				      gint            dest_position)
{
	PlumaTab *tab;
	gint cur_page;

	/* This is getting tricky, the tab was dragged in a notebook
	 * in another window of the same app, we move the tab
	 * to that new notebook, and let this notebook handle the
	 * drag
	 */
	g_return_if_fail (PLUMA_IS_NOTEBOOK (dest));
	g_return_if_fail (dest != src);

	cur_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (src));
	tab = PLUMA_TAB (gtk_notebook_get_nth_page (GTK_NOTEBOOK (src), 
						    cur_page));

	/* stop drag in origin window */
	/* ungrab the pointer if it's grabbed */
	drag_stop (src);
	if (gdk_pointer_is_grabbed ())
	{
		gdk_pointer_ungrab (event->time);
	}
	gtk_grab_remove (GTK_WIDGET (src));

	pluma_notebook_move_tab (src, dest, tab, dest_position);

	/* start drag handling in dest notebook */
	dest->priv->motion_notify_handler_id =
		g_signal_connect (G_OBJECT (dest),
				  "motion-notify-event",
				  G_CALLBACK (motion_notify_cb),
				  NULL);

	drag_start (dest, event->time);
}

static gboolean
button_release_cb (PlumaNotebook  *notebook,
		   GdkEventButton *event,
		   gpointer        data)
{
	if (notebook->priv->drag_in_progress)
	{
		gint cur_page_num;
		GtkWidget *cur_page;

		cur_page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
		cur_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook),
						      cur_page_num);

		/* CHECK: I don't follow the code here -- Paolo  */
		if (!is_in_notebook_window (notebook, event->x_root, event->y_root) &&
		    (gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook)) > 1))
		{
			/* Tab was detached */
			g_signal_emit (G_OBJECT (notebook),
				       signals[TAB_DETACHED], 
				       0, 
				       cur_page);
		}

		/* ungrab the pointer if it's grabbed */
		if (gdk_pointer_is_grabbed ())
		{
			gdk_pointer_ungrab (event->time);
		}
		gtk_grab_remove (GTK_WIDGET (notebook));
	}

	/* This must be called even if a drag isn't happening */
	drag_stop (notebook);

	return FALSE;
}

static gboolean
button_press_cb (PlumaNotebook  *notebook,
		 GdkEventButton *event,
		 gpointer        data)
{
	gint tab_clicked;

	if (notebook->priv->drag_in_progress)
		return TRUE;

	tab_clicked = find_tab_num_at_pos (notebook,
					   event->x_root,
					   event->y_root);
					   
	if ((event->button == 1) && 
	    (event->type == GDK_BUTTON_PRESS) && 
	    (tab_clicked >= 0))
	{
		notebook->priv->x_start = event->x_root;
		notebook->priv->y_start = event->y_root;
		
		notebook->priv->motion_notify_handler_id =
			g_signal_connect (G_OBJECT (notebook),
					  "motion-notify-event",
					  G_CALLBACK (motion_notify_cb), 
					  NULL);
	}
	else if ((event->type == GDK_BUTTON_PRESS) && 
		 (event->button == 3))
	{
		if (tab_clicked == -1)
		{
			// CHECK: do we really need it?
			
			/* consume event, so that we don't pop up the context menu when
			 * the mouse if not over a tab label
			 */
			return TRUE;
		}
		else
		{
			/* Switch to the page the mouse is over, but don't consume the event */
			gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 
						       tab_clicked);
		}
	}

	return FALSE;
}

/**
 * pluma_notebook_new:
 *
 * Creates a new #PlumaNotebook object.
 *
 * Returns: a new #PlumaNotebook
 */
GtkWidget *
pluma_notebook_new (void)
{
	return GTK_WIDGET (g_object_new (PLUMA_TYPE_NOTEBOOK, NULL));
}

static void
pluma_notebook_switch_page_cb (GtkNotebook     *notebook,
#if GTK_CHECK_VERSION (3, 0, 0)
                               GtkWidget       *page,
#else
                               GtkNotebookPage *page,
#endif
                               guint            page_num,
                               gpointer         data)
{
	PlumaNotebook *nb = PLUMA_NOTEBOOK (notebook);
	GtkWidget *child;
	PlumaView *view;

	child = gtk_notebook_get_nth_page (notebook, page_num);

	/* Remove the old page, we dont want to grow unnecessarily
	 * the list */
	if (nb->priv->focused_pages)
	{
		nb->priv->focused_pages =
			g_list_remove (nb->priv->focused_pages, child);
	}

	nb->priv->focused_pages = g_list_append (nb->priv->focused_pages,
						 child);

	/* give focus to the view */
	view = pluma_tab_get_view (PLUMA_TAB (child));
	gtk_widget_grab_focus (GTK_WIDGET (view));
}

/*
 * update_tabs_visibility: Hide tabs if there is only one tab
 * and the pref is not set.
 */
static void
update_tabs_visibility (PlumaNotebook *nb, 
			gboolean       before_inserting)
{
	gboolean show_tabs;
	guint num;

	num = gtk_notebook_get_n_pages (GTK_NOTEBOOK (nb));

	if (before_inserting) num++;

	show_tabs = (nb->priv->always_show_tabs || num > 1);

	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (nb), show_tabs);
}

static void
pluma_notebook_init (PlumaNotebook *notebook)
{
	notebook->priv = PLUMA_NOTEBOOK_GET_PRIVATE (notebook);

	notebook->priv->close_buttons_sensitive = TRUE;
	notebook->priv->tab_drag_and_drop_enabled = TRUE;
	
	gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);
	gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook), FALSE);
	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), FALSE);

	notebook->priv->always_show_tabs = TRUE;

	g_signal_connect (notebook, 
			  "button-press-event",
			  (GCallback)button_press_cb, 
			  NULL);
	g_signal_connect (notebook, 
			  "button-release-event",
			  (GCallback)button_release_cb,
			  NULL);
	gtk_widget_add_events (GTK_WIDGET (notebook), 
			       GDK_BUTTON1_MOTION_MASK);

	g_signal_connect_after (G_OBJECT (notebook), 
				"switch_page",
                                G_CALLBACK (pluma_notebook_switch_page_cb),
                                NULL);
}

static void
pluma_notebook_finalize (GObject *object)
{
	PlumaNotebook *notebook = PLUMA_NOTEBOOK (object);

	g_list_free (notebook->priv->focused_pages);

	G_OBJECT_CLASS (pluma_notebook_parent_class)->finalize (object);
}

/*
 * We need to override this because when we don't show the tabs, like in
 * fullscreen we need to have wrap around too
 */
static gboolean
pluma_notebook_change_current_page (GtkNotebook *notebook,
				    gint         offset)
{
	gboolean wrap_around;
	gint current;
	
	current = gtk_notebook_get_current_page (notebook);

	if (current != -1)
	{
		current = current + offset;
		
		g_object_get (gtk_widget_get_settings (GTK_WIDGET (notebook)),
			      "gtk-keynav-wrap-around", &wrap_around,
			      NULL);
		
		if (wrap_around)
		{
			if (current < 0)
			{
				current = gtk_notebook_get_n_pages (notebook) - 1;
			}
			else if (current >= gtk_notebook_get_n_pages (notebook))
			{
				current = 0;
			}
		}
		
		gtk_notebook_set_current_page (notebook, current);
	}
	else
	{
		gtk_widget_error_bell (GTK_WIDGET (notebook));
	}

	return TRUE;
}

static void
close_button_clicked_cb (PlumaTabLabel *tab_label, PlumaNotebook *notebook)
{
	PlumaTab *tab;

	tab = pluma_tab_label_get_tab (tab_label);
	g_signal_emit (notebook, signals[TAB_CLOSE_REQUEST], 0, tab);
}

static GtkWidget *
create_tab_label (PlumaNotebook *nb,
		  PlumaTab      *tab)
{
	GtkWidget *tab_label;

	tab_label = pluma_tab_label_new (tab);

	g_signal_connect (tab_label,
			  "close-clicked",
			  G_CALLBACK (close_button_clicked_cb),
			  nb);

	g_object_set_data (G_OBJECT (tab), "tab-label", tab_label);

	return tab_label;
}

static GtkWidget *
get_tab_label (PlumaTab *tab)
{
	GtkWidget *tab_label;

	tab_label = GTK_WIDGET (g_object_get_data (G_OBJECT (tab), "tab-label"));
	g_return_val_if_fail (tab_label != NULL, NULL);

	return tab_label;
}

static void
remove_tab_label (PlumaNotebook *nb,
		  PlumaTab      *tab)
{
	GtkWidget *tab_label;

	tab_label = get_tab_label (tab);

	g_signal_handlers_disconnect_by_func (tab_label,
					      G_CALLBACK (close_button_clicked_cb),
					      nb);

	g_object_set_data (G_OBJECT (tab), "tab-label", NULL);
}

/**
 * pluma_notebook_set_always_show_tabs:
 * @nb: a #PlumaNotebook
 * @show_tabs: %TRUE to always show the tabs
 *
 * Sets the visibility of the tabs in the @nb.
 */
void
pluma_notebook_set_always_show_tabs (PlumaNotebook *nb, 
				     gboolean       show_tabs)
{
	g_return_if_fail (PLUMA_IS_NOTEBOOK (nb));

	nb->priv->always_show_tabs = (show_tabs != FALSE);

	update_tabs_visibility (nb, FALSE);
}

/**
 * pluma_notebook_add_tab:
 * @nb: a #PlumaNotebook
 * @tab: a #PlumaTab
 * @position: the position where the @tab should be added
 * @jump_to: %TRUE to set the @tab as active
 *
 * Adds the specified @tab to the @nb.
 */
void
pluma_notebook_add_tab (PlumaNotebook *nb,
		        PlumaTab      *tab,
		        gint           position,
		        gboolean       jump_to)
{
	GtkWidget *tab_label;

	g_return_if_fail (PLUMA_IS_NOTEBOOK (nb));
	g_return_if_fail (PLUMA_IS_TAB (tab));

	tab_label = create_tab_label (nb, tab);
	gtk_notebook_insert_page (GTK_NOTEBOOK (nb), 
				  GTK_WIDGET (tab),
				  tab_label,
				  position);
	update_tabs_visibility (nb, TRUE);

	g_signal_emit (G_OBJECT (nb), signals[TAB_ADDED], 0, tab);

	/* The signal handler may have reordered the tabs */
	position = gtk_notebook_page_num (GTK_NOTEBOOK (nb), 
					  GTK_WIDGET (tab));

	if (jump_to)
	{
		PlumaView *view;
		
		gtk_notebook_set_current_page (GTK_NOTEBOOK (nb), position);
		g_object_set_data (G_OBJECT (tab), 
				   "jump_to",
				   GINT_TO_POINTER (jump_to));
		view = pluma_tab_get_view (tab);
		
		gtk_widget_grab_focus (GTK_WIDGET (view));
	}
}

static void
smart_tab_switching_on_closure (PlumaNotebook *nb,
				PlumaTab      *tab)
{
	gboolean jump_to;

	jump_to = GPOINTER_TO_INT (g_object_get_data
				   (G_OBJECT (tab), "jump_to"));

	if (!jump_to || !nb->priv->focused_pages)
	{
		gtk_notebook_next_page (GTK_NOTEBOOK (nb));
	}
	else
	{
		GList *l;
		GtkWidget *child;
		int page_num;

		/* activate the last focused tab */
		l = g_list_last (nb->priv->focused_pages);
		child = GTK_WIDGET (l->data);
		page_num = gtk_notebook_page_num (GTK_NOTEBOOK (nb),
						  child);
		gtk_notebook_set_current_page (GTK_NOTEBOOK (nb), 
					       page_num);
	}
}

static void
remove_tab (PlumaTab      *tab,
	    PlumaNotebook *nb)
{
	gint position;

	position = gtk_notebook_page_num (GTK_NOTEBOOK (nb), GTK_WIDGET (tab));

	/* we ref the tab so that it's still alive while the tabs_removed
	 * signal is processed.
	 */
	g_object_ref (tab);

	remove_tab_label (nb, tab);
	gtk_notebook_remove_page (GTK_NOTEBOOK (nb), position);
	update_tabs_visibility (nb, FALSE);

	g_signal_emit (G_OBJECT (nb), signals[TAB_REMOVED], 0, tab);

	g_object_unref (tab);
}

/**
 * pluma_notebook_remove_tab:
 * @nb: a #PlumaNotebook
 * @tab: a #PlumaTab
 *
 * Removes @tab from @nb.
 */
void
pluma_notebook_remove_tab (PlumaNotebook *nb,
			   PlumaTab      *tab)
{
	gint position, curr;

	g_return_if_fail (PLUMA_IS_NOTEBOOK (nb));
	g_return_if_fail (PLUMA_IS_TAB (tab));

	/* Remove the page from the focused pages list */
	nb->priv->focused_pages =  g_list_remove (nb->priv->focused_pages,
						  tab);

	position = gtk_notebook_page_num (GTK_NOTEBOOK (nb), GTK_WIDGET (tab));
	curr = gtk_notebook_get_current_page (GTK_NOTEBOOK (nb));

	if (position == curr)
	{
		smart_tab_switching_on_closure (nb, tab);
	}

	remove_tab (tab, nb);
}

/**
 * pluma_notebook_remove_all_tabs:
 * @nb: a #PlumaNotebook
 *
 * Removes all #PlumaTab from @nb.
 */
void
pluma_notebook_remove_all_tabs (PlumaNotebook *nb)
{	
	g_return_if_fail (PLUMA_IS_NOTEBOOK (nb));
	
	g_list_free (nb->priv->focused_pages);
	nb->priv->focused_pages = NULL;

	gtk_container_foreach (GTK_CONTAINER (nb),
			       (GtkCallback)remove_tab,
			       nb);
}

static void
set_close_buttons_sensitivity (PlumaTab      *tab,
                               PlumaNotebook *nb)
{
	GtkWidget *tab_label;

	tab_label = get_tab_label (tab);

	pluma_tab_label_set_close_button_sensitive (PLUMA_TAB_LABEL (tab_label),
						    nb->priv->close_buttons_sensitive);
}

/**
 * pluma_notebook_set_close_buttons_sensitive:
 * @nb: a #PlumaNotebook
 * @sensitive: %TRUE to make the buttons sensitive
 *
 * Sets whether the close buttons in the tabs of @nb are sensitive.
 */
void
pluma_notebook_set_close_buttons_sensitive (PlumaNotebook *nb,
					    gboolean       sensitive)
{
	g_return_if_fail (PLUMA_IS_NOTEBOOK (nb));

	sensitive = (sensitive != FALSE);

	if (sensitive == nb->priv->close_buttons_sensitive)
		return;

	nb->priv->close_buttons_sensitive = sensitive;

	gtk_container_foreach (GTK_CONTAINER (nb),
			       (GtkCallback)set_close_buttons_sensitivity,
			       nb);
}

/**
 * pluma_notebook_get_close_buttons_sensitive:
 * @nb: a #PlumaNotebook
 *
 * Whether the close buttons are sensitive.
 *
 * Returns: %TRUE if the close buttons are sensitive
 */
gboolean
pluma_notebook_get_close_buttons_sensitive (PlumaNotebook *nb)
{
	g_return_val_if_fail (PLUMA_IS_NOTEBOOK (nb), TRUE);

	return nb->priv->close_buttons_sensitive;
}

/**
 * pluma_notebook_set_tab_drag_and_drop_enabled:
 * @nb: a #PlumaNotebook
 * @enable: %TRUE to enable the drag and drop
 *
 * Sets whether drag and drop of tabs in the @nb is enabled.
 */
void
pluma_notebook_set_tab_drag_and_drop_enabled (PlumaNotebook *nb,
					      gboolean       enable)
{
	g_return_if_fail (PLUMA_IS_NOTEBOOK (nb));
	
	enable = (enable != FALSE);
	
	if (enable == nb->priv->tab_drag_and_drop_enabled)
		return;
		
	nb->priv->tab_drag_and_drop_enabled = enable;		
}

/**
 * pluma_notebook_get_tab_drag_and_drop_enabled:
 * @nb: a #PlumaNotebook
 *
 * Whether the drag and drop is enabled in the @nb.
 *
 * Returns: %TRUE if the drag and drop is enabled.
 */
gboolean	
pluma_notebook_get_tab_drag_and_drop_enabled (PlumaNotebook *nb)
{
	g_return_val_if_fail (PLUMA_IS_NOTEBOOK (nb), TRUE);
	
	return nb->priv->tab_drag_and_drop_enabled;
}

