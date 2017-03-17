#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xed-paned.h"

#define ANIMATION_TIME 125

struct _XedPanedPrivate
{
    gint start_pos;
    gint current_pos;
    gint target_pos;

    gint64 start_time;
    gint64 end_time;

    guint tick_id;

    gboolean animating;
    gboolean show_child;
    gboolean is_vertical;
    gint pane_number;
};

G_DEFINE_TYPE_WITH_PRIVATE (XedPaned, xed_paned, GTK_TYPE_PANED)

static void
xed_paned_dispose (GObject *object)
{
    XedPaned *paned = XED_PANED (object);

    if (paned->priv->tick_id != 0)
    {
        gtk_widget_remove_tick_callback (GTK_WIDGET (paned), paned->priv->tick_id);
    }
    paned->priv->tick_id = 0;

    G_OBJECT_CLASS (xed_paned_parent_class)->dispose (object);
}

static void
xed_paned_class_init (XedPanedClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = xed_paned_dispose;
}

static void
xed_paned_init (XedPaned *paned)
{
    paned->priv = xed_paned_get_instance_private (paned);

    paned->priv->animating = FALSE;
}

GtkWidget *
xed_paned_new (GtkOrientation orientation)
{
    return GTK_WIDGET (g_object_new (XED_TYPE_PANED,
                                     "orientation", orientation,
                                     NULL));
}

static void
animate_step (XedPaned *paned,
              gint64    now)
{

    gdouble t;
    gint difference;

    if ((paned->priv->show_child && paned->priv->pane_number == 1) ||
         (!paned->priv->show_child && paned->priv->pane_number == 2))
    {
        difference = paned->priv->target_pos - paned->priv->start_pos;
        if (now < paned->priv->end_time)
        {
            t = ((gdouble) (now - paned->priv->start_time) / (gdouble) (paned->priv->end_time - paned->priv->start_time));
        }
        else
        {
            t = 1.0;
        }

        paned->priv->current_pos = paned->priv->start_pos + (difference * t);
    }
    else
    {
        difference = paned->priv->start_pos - paned->priv->target_pos;
        if (now < paned->priv->end_time)
        {
            t = ((gdouble) (now - paned->priv->start_time) / (gdouble) (paned->priv->end_time - paned->priv->start_time));
        }
        else
        {
            t = 1.0;
        }

        paned->priv->current_pos = paned->priv->start_pos - (difference * t);
    }

    gtk_paned_set_position (GTK_PANED (paned), paned->priv->current_pos);
    gtk_widget_queue_draw (GTK_WIDGET (paned));
}

static gboolean
animate_cb (GtkWidget     *widget,
            GdkFrameClock *frame_clock,
            gpointer       user_data)
{
    XedPaned *paned = XED_PANED (widget);
    gint64 now;

    now = gdk_frame_clock_get_frame_time (frame_clock);

    animate_step (paned, now);

    if (paned->priv->current_pos == paned->priv->target_pos)
    {
        paned->priv->tick_id = 0;

        if (!paned->priv->show_child)
        {
            if (paned->priv->pane_number == 1)
            {
                gtk_widget_hide (gtk_paned_get_child1 (GTK_PANED (paned)));
            }
            else
            {
                gtk_widget_hide (gtk_paned_get_child2 (GTK_PANED (paned)));
            }
        }

        paned->priv->animating = FALSE;
        return G_SOURCE_REMOVE;
    }

    return G_SOURCE_CONTINUE;
}

static void
calculate_target_postion (XedPaned *paned,
                          gint      target_position)
{
    if (paned->priv->show_child)
    {
        if (target_position < 0)
        {
            paned->priv->target_pos = 0;
        }
        else
        {
            paned->priv->target_pos = target_position;
        }
    }
    else
    {
        if (paned->priv->pane_number == 1)
        {
            paned->priv->target_pos = 0;
        }
        else
        {
            gint max_pos;

            g_object_get (G_OBJECT (paned), "max-position", &max_pos, NULL);
            paned->priv->target_pos = max_pos;
        }
    }
}

static void
calculate_start_position (XedPaned *paned)
{
    if (paned->priv->show_child && paned->priv->pane_number == 1)
    {
        paned->priv->start_pos = 0;
    }
    else if (paned->priv->show_child && paned->priv->pane_number == 2)
    {
        gint max_pos;

        g_object_get (G_OBJECT (paned), "max-position", &max_pos, NULL);
        paned->priv->start_pos = max_pos;
    }
    else if (paned->priv->pane_number == 1 || paned->priv->pane_number == 2)
    {
        paned->priv->start_pos = gtk_paned_get_position (GTK_PANED (paned));
    }

    paned->priv->current_pos = paned->priv->start_pos;
}

static void
setup_animation (XedPaned  *paned,
                 gint       target_position)
{
    if (!gtk_widget_get_mapped (GTK_WIDGET (paned)))
    {
        return;
    }

    if (gtk_orientable_get_orientation (GTK_ORIENTABLE (paned)) == GTK_ORIENTATION_HORIZONTAL)
    {
        paned->priv->is_vertical = FALSE;
    }
    else
    {
        paned->priv->is_vertical = TRUE;
    }

    calculate_start_position (paned);
    calculate_target_postion (paned, target_position);

    paned->priv->start_time = gdk_frame_clock_get_frame_time (gtk_widget_get_frame_clock (GTK_WIDGET (paned)));
    paned->priv->end_time = paned->priv->start_time + (ANIMATION_TIME * 1000);

    if (paned->priv->tick_id == 0)
    {
        paned->priv->animating = TRUE;
        paned->priv->tick_id = gtk_widget_add_tick_callback (GTK_WIDGET (paned),
                                                             animate_cb,
                                                             NULL,
                                                             NULL);
    }

    if (paned->priv->show_child)
    {
        gtk_widget_show (gtk_paned_get_child1 (GTK_PANED (paned)));
    }

    animate_step (paned, paned->priv->start_time);
}

void
xed_paned_close (XedPaned *paned,
                 gint      pane_number)
{
    g_return_if_fail (XED_IS_PANED (paned));
    g_return_if_fail (pane_number == 1 || pane_number == 2);

    paned->priv->show_child = FALSE;
    paned->priv->pane_number = pane_number;
    setup_animation (paned, -1);
}

void
xed_paned_open (XedPaned *paned,
                gint      pane_number,
                gint      target_position)
{
    g_return_if_fail (XED_IS_PANED (paned));
    g_return_if_fail (pane_number == 1 || pane_number == 2);

    paned->priv->show_child = TRUE;
    paned->priv->pane_number = pane_number;
    setup_animation (paned, target_position);
}

gboolean
xed_paned_get_is_animating (XedPaned *paned)
{
    return paned->priv->animating;
}
