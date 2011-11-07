/*
 * gedit-notebook.c
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */
 
/*
 * Modified by the gedit Team, 2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
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

#include "gedit-notebook.h"
#include "gedit-tab.h"
#include "gedit-tab-label.h"
#include "gedit-marshal.h"
#include "gedit-window.h"

#ifdef BUILD_SPINNER
#include "gedit-spinner.h"
#endif

#define AFTER_ALL_TABS -1
#define NOT_IN_APP_WINDOWS -2

#define GEDIT_NOTEBOOK_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GEDIT_TYPE_NOTEBOOK, GeditNotebookPrivate))

struct _GeditNotebookPrivate
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

G_DEFINE_TYPE(GeditNotebook, gedit_notebook, GTK_TYPE_NOTEBOOK)

static void gedit_notebook_finalize (GObject *object);

static gboolean gedit_notebook_change_current_page (GtkNotebook *notebook,
						    gint         offset);

static void move_current_tab_to_another_notebook  (GeditNotebook  *src,
						   GeditNotebook  *dest,
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
gedit_notebook_destroy (GtkObject *object)
{
	GeditNotebook *notebook = GEDIT_NOTEBOOK (object);

	if (!notebook->priv->destroy_has_run)
	{
		GList *children, *l;

		children = gtk_container_get_children (GTK_CONTAINER (notebook));

		for (l = children; l != NULL; l = g_list_next (l))
		{
			gedit_notebook_remove_tab (notebook,
						   GEDIT_TAB (l->data));
		}

		g_list_free (children);
		notebook->priv->destroy_has_run = TRUE;
	}

	GTK_OBJECT_CLASS (gedit_notebook_parent_class)->destroy (object);
}

static void
gedit_notebook_class_init (GeditNotebookClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkObjectClass *gtkobject_class = GTK_OBJECT_CLASS (klass);
	GtkNotebookClass *notebook_class = GTK_NOTEBOOK_CLASS (klass);

	object_class->finalize = gedit_notebook_finalize;
	gtkobject_class->destroy = gedit_notebook_destroy;
	
	notebook_class->change_current_page = gedit_notebook_change_current_page;

	signals[TAB_ADDED] =
		g_signal_new ("tab_added",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (GeditNotebookClass, tab_added),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE,
			      1,
			      GEDIT_TYPE_TAB);
	signals[TAB_REMOVED] =
		g_signal_new ("tab_removed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (GeditNotebookClass, tab_removed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE,
			      1,
			      GEDIT_TYPE_TAB);
	signals[TAB_DETACHED] =
		g_signal_new ("tab_detached",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (GeditNotebookClass, tab_detached),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE,
			      1,
			      GEDIT_TYPE_TAB);
	signals[TABS_REORDERED] =
		g_signal_new ("tabs_reordered",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (GeditNotebookClass, tabs_reordered),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
	signals[TAB_CLOSE_REQUEST] =
		g_signal_new ("tab-close-request",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GeditNotebookClass, tab_close_request),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE,
			      1,
			      GEDIT_TYPE_TAB);

	g_type_class_add_private (object_class, sizeof(GeditNotebookPrivate));
}

static GeditNotebook *
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

	/* toplevel should be an GeditWindow */
	if ((toplevel != NULL) && 
	    GEDIT_IS_WINDOW (toplevel))
	{
		return GEDIT_NOTEBOOK (_gedit_window_get_notebook
						(GEDIT_WINDOW (toplevel)));
	}

	/* We are outside all windows containing a notebook */
	return NULL;
}

static gboolean
is_in_notebook_window (GeditNotebook *notebook,
		       gint           abs_x, 
		       gint           abs_y)
{
	GeditNotebook *nb_at_pointer;

	g_return_val_if_fail (notebook != NULL, FALSE);

	nb_at_pointer = find_notebook_at_pointer (abs_x, abs_y);

	return (nb_at_pointer == notebook);
}

static gint
find_tab_num_at_pos (GeditNotebook *notebook, 
		     gint           abs_x, 
		     gint           abs_y)
{
	GtkPositionType tab_pos;
	int page_num = 0;
	GtkNotebook *nb = GTK_NOTEBOOK (notebook);
	GtkWidget *page;

	tab_pos = gtk_notebook_get_tab_pos (GTK_NOTEBOOK (notebook));

	if (GTK_NOTEBOOK (notebook)->first_tab == NULL)
	{
		return AFTER_ALL_TABS;
	}

	/* For some reason unfullscreen + quick click can
	   cause a wrong click event to be reported to the tab */
	if (!is_in_notebook_window (notebook, abs_x, abs_y))
	{
		return NOT_IN_APP_WINDOWS;
	}

	while ((page = gtk_notebook_get_nth_page (nb, page_num)) != NULL)
	{
		GtkWidget *tab;
		gint max_x, max_y;
		gint x_root, y_root;

		tab = gtk_notebook_get_tab_label (nb, page);
		g_return_val_if_fail (tab != NULL, AFTER_ALL_TABS);

		if (!GTK_WIDGET_MAPPED (GTK_WIDGET (tab)))
		{
			++page_num;
			continue;
		}

		gdk_window_get_origin (GDK_WINDOW (tab->window),
				       &x_root, &y_root);

		max_x = x_root + tab->allocation.x + tab->allocation.width;
		max_y = y_root + tab->allocation.y + tab->allocation.height;

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
			      GeditNotebook **notebook,
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
 * gedit_notebook_move_tab:
 * @src: a #GeditNotebook
 * @dest: a #GeditNotebook
 * @tab: a #GeditTab
 * @dest_position: the position for @tab
 *
 * Moves @tab from @src to @dest.
 * If dest_position is greater than or equal to the number of tabs 
 * of the destination nootebook or negative, tab will be moved to the 
 * end of the tabs.
 */
void
gedit_notebook_move_tab (GeditNotebook *src,
			 GeditNotebook *dest,
			 GeditTab      *tab,
			 gint           dest_position)
{
	g_return_if_fail (GEDIT_IS_NOTEBOOK (src));	
	g_return_if_fail (GEDIT_IS_NOTEBOOK (dest));
	g_return_if_fail (src != dest);
	g_return_if_fail (GEDIT_IS_TAB (tab));

	/* make sure the tab isn't destroyed while we move it */
	g_object_ref (tab);
	gedit_notebook_remove_tab (src, tab);
	gedit_notebook_add_tab (dest, tab, dest_position, TRUE);
	g_object_unref (tab);
}

/**
 * gedit_notebook_reorder_tab:
 * @src: a #GeditNotebook
 * @tab: a #GeditTab
 * @dest_position: the position for @tab
 *
 * Reorders the page containing @tab, so that it appears in @dest_position position.
 * If dest_position is greater than or equal to the number of tabs 
 * of the destination notebook or negative, tab will be moved to the 
 * end of the tabs.
 */
void
gedit_notebook_reorder_tab (GeditNotebook *src,
			    GeditTab      *tab,
			    gint           dest_position)
{
	gint old_position;
	
	g_return_if_fail (GEDIT_IS_NOTEBOOK (src));	
	g_return_if_fail (GEDIT_IS_TAB (tab));

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
drag_start (GeditNotebook *notebook,
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
drag_stop (GeditNotebook *notebook)
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
move_current_tab (GeditNotebook *notebook,
	          gint           dest_position)
{
	gint cur_page_num;

	cur_page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));

	if (dest_position != cur_page_num)
	{
		GtkWidget *cur_tab;
		
		cur_tab = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook),
						     cur_page_num);
						     
		gedit_notebook_reorder_tab (GEDIT_NOTEBOOK (notebook),
					    GEDIT_TAB (cur_tab),
					    dest_position);
	}
}

static gboolean
motion_notify_cb (GeditNotebook  *notebook,
		  GdkEventMotion *event,
		  gpointer        data)
{
	GeditNotebook *dest;
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
move_current_tab_to_another_notebook (GeditNotebook  *src,
				      GeditNotebook  *dest,
				      GdkEventMotion *event,
				      gint            dest_position)
{
	GeditTab *tab;
	gint cur_page;

	/* This is getting tricky, the tab was dragged in a notebook
	 * in another window of the same app, we move the tab
	 * to that new notebook, and let this notebook handle the
	 * drag
	 */
	g_return_if_fail (GEDIT_IS_NOTEBOOK (dest));
	g_return_if_fail (dest != src);

	cur_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (src));
	tab = GEDIT_TAB (gtk_notebook_get_nth_page (GTK_NOTEBOOK (src), 
						    cur_page));

	/* stop drag in origin window */
	/* ungrab the pointer if it's grabbed */
	drag_stop (src);
	if (gdk_pointer_is_grabbed ())
	{
		gdk_pointer_ungrab (event->time);
	}
	gtk_grab_remove (GTK_WIDGET (src));

	gedit_notebook_move_tab (src, dest, tab, dest_position);

	/* start drag handling in dest notebook */
	dest->priv->motion_notify_handler_id =
		g_signal_connect (G_OBJECT (dest),
				  "motion-notify-event",
				  G_CALLBACK (motion_notify_cb),
				  NULL);

	drag_start (dest, event->time);
}

static gboolean
button_release_cb (GeditNotebook  *notebook,
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
button_press_cb (GeditNotebook  *notebook,
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
 * gedit_notebook_new:
 *
 * Creates a new #GeditNotebook object.
 *
 * Returns: a new #GeditNotebook
 */
GtkWidget *
gedit_notebook_new (void)
{
	return GTK_WIDGET (g_object_new (GEDIT_TYPE_NOTEBOOK, NULL));
}

static void
gedit_notebook_switch_page_cb (GtkNotebook     *notebook,
                               GtkNotebookPage *page,
                               guint            page_num,
                               gpointer         data)
{
	GeditNotebook *nb = GEDIT_NOTEBOOK (notebook);
	GtkWidget *child;
	GeditView *view;

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
	view = gedit_tab_get_view (GEDIT_TAB (child));
	gtk_widget_grab_focus (GTK_WIDGET (view));
}

/*
 * update_tabs_visibility: Hide tabs if there is only one tab
 * and the pref is not set.
 */
static void
update_tabs_visibility (GeditNotebook *nb, 
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
gedit_notebook_init (GeditNotebook *notebook)
{
	notebook->priv = GEDIT_NOTEBOOK_GET_PRIVATE (notebook);

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
                                G_CALLBACK (gedit_notebook_switch_page_cb),
                                NULL);
}

static void
gedit_notebook_finalize (GObject *object)
{
	GeditNotebook *notebook = GEDIT_NOTEBOOK (object);

	g_list_free (notebook->priv->focused_pages);

	G_OBJECT_CLASS (gedit_notebook_parent_class)->finalize (object);
}

/*
 * We need to override this because when we don't show the tabs, like in
 * fullscreen we need to have wrap around too
 */
static gboolean
gedit_notebook_change_current_page (GtkNotebook *notebook,
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
close_button_clicked_cb (GeditTabLabel *tab_label, GeditNotebook *notebook)
{
	GeditTab *tab;

	tab = gedit_tab_label_get_tab (tab_label);
	g_signal_emit (notebook, signals[TAB_CLOSE_REQUEST], 0, tab);
}

static GtkWidget *
create_tab_label (GeditNotebook *nb,
		  GeditTab      *tab)
{
	GtkWidget *tab_label;

	tab_label = gedit_tab_label_new (tab);

	g_signal_connect (tab_label,
			  "close-clicked",
			  G_CALLBACK (close_button_clicked_cb),
			  nb);

	g_object_set_data (G_OBJECT (tab), "tab-label", tab_label);

	return tab_label;
}

static void
remove_tab_label (GeditNotebook *nb,
		  GeditTab      *tab)
{
	GtkWidget *tab_label;

	tab_label = gedit_tab_label_new (tab);

	g_signal_handlers_disconnect_by_func (tab_label,
					      G_CALLBACK (close_button_clicked_cb),
					      nb);

	g_object_set_data (G_OBJECT (tab), "tab-label", NULL);
}

static GtkWidget *
get_tab_label (GeditTab *tab)
{
	GtkWidget *tab_label;

	tab_label = GTK_WIDGET (g_object_get_data (G_OBJECT (tab), "tab-label"));
	g_return_val_if_fail (tab_label != NULL, NULL);

	return tab_label;
}

/**
 * gedit_notebook_set_always_show_tabs:
 * @nb: a #GeditNotebook
 * @show_tabs: %TRUE to always show the tabs
 *
 * Sets the visibility of the tabs in the @nb.
 */
void
gedit_notebook_set_always_show_tabs (GeditNotebook *nb, 
				     gboolean       show_tabs)
{
	g_return_if_fail (GEDIT_IS_NOTEBOOK (nb));

	nb->priv->always_show_tabs = (show_tabs != FALSE);

	update_tabs_visibility (nb, FALSE);
}

/**
 * gedit_notebook_add_tab:
 * @nb: a #GeditNotebook
 * @tab: a #GeditTab
 * @position: the position where the @tab should be added
 * @jump_to: %TRUE to set the @tab as active
 *
 * Adds the specified @tab to the @nb.
 */
void
gedit_notebook_add_tab (GeditNotebook *nb,
		        GeditTab      *tab,
		        gint           position,
		        gboolean       jump_to)
{
	GtkWidget *tab_label;

	g_return_if_fail (GEDIT_IS_NOTEBOOK (nb));
	g_return_if_fail (GEDIT_IS_TAB (tab));

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
		GeditView *view;
		
		gtk_notebook_set_current_page (GTK_NOTEBOOK (nb), position);
		g_object_set_data (G_OBJECT (tab), 
				   "jump_to",
				   GINT_TO_POINTER (jump_to));
		view = gedit_tab_get_view (tab);
		
		gtk_widget_grab_focus (GTK_WIDGET (view));
	}
}

static void
smart_tab_switching_on_closure (GeditNotebook *nb,
				GeditTab      *tab)
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
remove_tab (GeditTab      *tab,
	    GeditNotebook *nb)
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
 * gedit_notebook_remove_tab:
 * @nb: a #GeditNotebook
 * @tab: a #GeditTab
 *
 * Removes @tab from @nb.
 */
void
gedit_notebook_remove_tab (GeditNotebook *nb,
			   GeditTab      *tab)
{
	gint position, curr;

	g_return_if_fail (GEDIT_IS_NOTEBOOK (nb));
	g_return_if_fail (GEDIT_IS_TAB (tab));

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
 * gedit_notebook_remove_all_tabs:
 * @nb: a #GeditNotebook
 *
 * Removes all #GeditTab from @nb.
 */
void
gedit_notebook_remove_all_tabs (GeditNotebook *nb)
{	
	g_return_if_fail (GEDIT_IS_NOTEBOOK (nb));
	
	g_list_free (nb->priv->focused_pages);
	nb->priv->focused_pages = NULL;

	gtk_container_foreach (GTK_CONTAINER (nb),
			       (GtkCallback)remove_tab,
			       nb);
}

static void
set_close_buttons_sensitivity (GeditTab      *tab,
                               GeditNotebook *nb)
{
	GtkWidget *tab_label;

	tab_label = get_tab_label (tab);

	gedit_tab_label_set_close_button_sensitive (GEDIT_TAB_LABEL (tab_label),
						    nb->priv->close_buttons_sensitive);
}

/**
 * gedit_notebook_set_close_buttons_sensitive:
 * @nb: a #GeditNotebook
 * @sensitive: %TRUE to make the buttons sensitive
 *
 * Sets whether the close buttons in the tabs of @nb are sensitive.
 */
void
gedit_notebook_set_close_buttons_sensitive (GeditNotebook *nb,
					    gboolean       sensitive)
{
	g_return_if_fail (GEDIT_IS_NOTEBOOK (nb));

	sensitive = (sensitive != FALSE);

	if (sensitive == nb->priv->close_buttons_sensitive)
		return;

	nb->priv->close_buttons_sensitive = sensitive;

	gtk_container_foreach (GTK_CONTAINER (nb),
			       (GtkCallback)set_close_buttons_sensitivity,
			       nb);
}

/**
 * gedit_notebook_get_close_buttons_sensitive:
 * @nb: a #GeditNotebook
 *
 * Whether the close buttons are sensitive.
 *
 * Returns: %TRUE if the close buttons are sensitive
 */
gboolean
gedit_notebook_get_close_buttons_sensitive (GeditNotebook *nb)
{
	g_return_val_if_fail (GEDIT_IS_NOTEBOOK (nb), TRUE);

	return nb->priv->close_buttons_sensitive;
}

/**
 * gedit_notebook_set_tab_drag_and_drop_enabled:
 * @nb: a #GeditNotebook
 * @enable: %TRUE to enable the drag and drop
 *
 * Sets whether drag and drop of tabs in the @nb is enabled.
 */
void
gedit_notebook_set_tab_drag_and_drop_enabled (GeditNotebook *nb,
					      gboolean       enable)
{
	g_return_if_fail (GEDIT_IS_NOTEBOOK (nb));
	
	enable = (enable != FALSE);
	
	if (enable == nb->priv->tab_drag_and_drop_enabled)
		return;
		
	nb->priv->tab_drag_and_drop_enabled = enable;		
}

/**
 * gedit_notebook_get_tab_drag_and_drop_enabled:
 * @nb: a #GeditNotebook
 *
 * Whether the drag and drop is enabled in the @nb.
 *
 * Returns: %TRUE if the drag and drop is enabled.
 */
gboolean	
gedit_notebook_get_tab_drag_and_drop_enabled (GeditNotebook *nb)
{
	g_return_val_if_fail (GEDIT_IS_NOTEBOOK (nb), TRUE);
	
	return nb->priv->tab_drag_and_drop_enabled;
}

