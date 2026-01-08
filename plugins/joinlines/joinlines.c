#include <gtk/gtk.h>
#include <xed/xed-view.h>
#include <xed/xed-view-activatable.h>
#include <libpeas/peas.h>

#define JOIN_LINES_TYPE (join_lines_get_type())
G_DECLARE_FINAL_TYPE(JoinLines, join_lines, JOIN, LINES, GObject)

struct _JoinLines
{
	GObject parent_instance;
	XedView *view;
	GtkTextBuffer *buffer;
	gulong handler_key_press;
};

static void xed_view_activatable_iface_init(XedViewActivatableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE(JoinLines, join_lines, G_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE(XED_TYPE_VIEW_ACTIVATABLE, xed_view_activatable_iface_init))

enum
{
	PROP_0,
	PROP_VIEW
};

static void do_join_lines(JoinLines *self)
{
	GtkTextIter start, end;
	GtkTextMark *end_mark;

	if (!self->buffer)
	{
		return;
	}

	gtk_text_buffer_begin_user_action(self->buffer);

	if (!gtk_text_buffer_get_selection_bounds(self->buffer, &start, &end))
	{
		gtk_text_buffer_get_iter_at_mark(self->buffer, &start,
			gtk_text_buffer_get_insert(self->buffer));
		end = start;
		gtk_text_iter_forward_line(&end);
	}

	end_mark = gtk_text_buffer_create_mark(self->buffer, NULL, &end, FALSE);

	if (!gtk_text_iter_ends_line(&start))
	{
		gtk_text_iter_forward_to_line_end(&start);
	}

	while (gtk_text_iter_backward_char(&start))
	{
		gunichar ch = gtk_text_iter_get_char(&start);
		if (ch != ' ' && ch != '\t')
		{
			gtk_text_iter_forward_char(&start);
			break;
		}
	}

	while (TRUE)
	{
		GtkTextIter iter_end;
		gtk_text_buffer_get_iter_at_mark(self->buffer, &iter_end, end_mark);

		if (gtk_text_iter_compare(&start, &iter_end) >= 0)
		{
			break;
		}

		GtkTextIter tmp_end = start;
		while (1)
		{
			gunichar ch = gtk_text_iter_get_char(&tmp_end);
			if (ch == '\r' || ch == '\n' || ch == ' ' || ch == '\t')
			{
				gtk_text_iter_forward_char(&tmp_end);
			}
			else
			{
				break;
			}
		}

		gtk_text_buffer_delete(self->buffer, &start, &tmp_end);
		gtk_text_buffer_insert(self->buffer, &start, " ", -1);
		gtk_text_iter_forward_to_line_end(&start);
	}

	gtk_text_buffer_delete_mark(self->buffer, end_mark);
	gtk_text_buffer_end_user_action(self->buffer);
}


static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	JoinLines *self = JOIN_LINES(user_data);

	if ((event->state & GDK_CONTROL_MASK) && event->keyval == GDK_KEY_j)
	{
		do_join_lines(self);
		return TRUE;
	}

	return FALSE;
}


static void join_lines_activate(XedViewActivatable *activatable)
{
	JoinLines *self = JOIN_LINES(activatable);

	self->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->view));
	self->handler_key_press = g_signal_connect(self->view, "key-press-event",
		G_CALLBACK(on_key_press), self);
}

static void join_lines_deactivate(XedViewActivatable *activatable)
{
	JoinLines *self = JOIN_LINES(activatable);

	if (self->handler_key_press != 0)
	{
		g_signal_handler_disconnect(self->view, self->handler_key_press);
		self->handler_key_press = 0;
	}

	self->buffer = NULL;
}


static void join_lines_get_property(GObject *object, guint prop_id,
	GValue *value, GParamSpec *pspec)
{
	JoinLines *self = JOIN_LINES(object);
	switch (prop_id)
	{
	case PROP_VIEW:
		g_value_set_object(value, self->view);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void join_lines_set_property(GObject *object, guint prop_id,
	const GValue *value, GParamSpec *pspec)
{
	JoinLines *self = JOIN_LINES(object);

	switch (prop_id)
	{
	case PROP_VIEW:
		self->view = XED_VIEW(g_value_get_object(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void join_lines_dispose(GObject *object)
{
	JoinLines *self = JOIN_LINES(object);

	if (self->handler_key_press != 0 && self->view != NULL)
	{
		g_signal_handler_disconnect(self->view, self->handler_key_press);
		self->handler_key_press = 0;
	}

	self->view = NULL;
	self->buffer = NULL;
	
	G_OBJECT_CLASS(join_lines_parent_class)->dispose(object);
}

static void join_lines_init(JoinLines *self)
{
	self->buffer = NULL;
	self->handler_key_press = 0;
}

static void join_lines_class_init(JoinLinesClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->get_property = join_lines_get_property;
	object_class->set_property = join_lines_set_property;
	object_class->dispose = join_lines_dispose;

	g_object_class_override_property(object_class, PROP_VIEW, "view");
}

static void xed_view_activatable_iface_init(XedViewActivatableInterface *iface)
{
	iface->activate = join_lines_activate;
	iface->deactivate = join_lines_deactivate;
}

G_MODULE_EXPORT void
peas_register_types(PeasObjectModule *module)
{
	peas_object_module_register_extension_type(module,
		XED_TYPE_VIEW_ACTIVATABLE,
		JOIN_LINES_TYPE);
}
