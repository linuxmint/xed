#include <gtk/gtk.h>
#include <xed/xed-view.h>
#include <xed/xed-view-activatable.h>
#include <libpeas/peas.h>

#define BRACKET_COMPLETION_TYPE (bracket_completion_get_type())
G_DECLARE_FINAL_TYPE(BracketCompletion, bracket_completion, BRACKET, COMPLETION, GObject)

struct _BracketCompletion
{
	GObject parent_instance;
	XedView *view;

	GtkTextBuffer *buffer;
	GtkTextMark *mark_begin;
	GtkTextMark *mark_end;
	GList *stack;
	gboolean relocate_marks;
	gulong handler_key_press;
	gulong handler_editable;
};

static void xed_view_activatable_iface_init(XedViewActivatableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE(BracketCompletion, bracket_completion, G_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE(XED_TYPE_VIEW_ACTIVATABLE, xed_view_activatable_iface_init))

enum
{
	PROP_0,
	PROP_VIEW
};

static const gchar* get_closing_bracket(gchar c)
{
	switch (c)
	{
		case '(': return ")";
		case '[': return "]";
		case '{': return "}";
		case '"': return "\"";
		case '\'': return "'";
		default: return NULL;
	}
}

static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	BracketCompletion *self = BRACKET_COMPLETION(user_data);
	GtkTextIter iter;

	if (!gtk_text_view_get_editable(GTK_TEXT_VIEW(self->view)))
	{
		return FALSE;
	}

	gtk_text_buffer_get_iter_at_mark(self->buffer, &iter, gtk_text_buffer_get_insert(self->buffer));

	if (event->keyval == GDK_KEY_BackSpace)
	{
		if (self->stack != NULL)
		{
			GList *last = g_list_last(self->stack);
			g_free(last->data);
			self->stack = g_list_delete_link(self->stack, last);
		}
		return FALSE;
	}

	gchar key = gdk_keyval_to_unicode(event->keyval);
	if (key == 0)
	{
		return FALSE;
	}

	const gchar *closing = get_closing_bracket(key);
	if (closing)
	{
		gtk_text_buffer_insert_at_cursor(self->buffer, closing, -1);

		gtk_text_buffer_get_iter_at_mark(self->buffer, &iter, gtk_text_buffer_get_insert(self->buffer));
		gtk_text_iter_backward_char(&iter);
		gtk_text_buffer_place_cursor(self->buffer, &iter);

		/* Store the opening bracket */
		self->stack = g_list_append(self->stack, g_strdup_printf("%c", key));
	}
	
	return FALSE;
}

static void on_notify_editable(GObject *obj, GParamSpec *pspec, gpointer user_data)
{
	BracketCompletion *self = BRACKET_COMPLETION(user_data);
	gboolean editable = gtk_text_view_get_editable(GTK_TEXT_VIEW(self->view));

	if (!editable)
	{
		g_list_free_full(self->stack, g_free);
		self->stack = NULL;
	}
}

static void bracket_completion_activate(XedViewActivatable *activatable)
{
	BracketCompletion *self = BRACKET_COMPLETION(activatable);
	GtkTextIter iter;

	self->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->view));

	gtk_text_buffer_get_iter_at_mark(self->buffer, &iter, gtk_text_buffer_get_insert(self->buffer));
	self->mark_begin = gtk_text_buffer_create_mark(self->buffer, NULL, &iter, TRUE);
	self->mark_end = gtk_text_buffer_create_mark(self->buffer, NULL, &iter, FALSE);

	self->handler_key_press = g_signal_connect(self->view, "key-press-event", 
		G_CALLBACK(on_key_press), self);
	self->handler_editable = g_signal_connect(self->view, "notify::editable", 
		G_CALLBACK(on_notify_editable), self);
}

static void bracket_completion_deactivate(XedViewActivatable *activatable)
{
	BracketCompletion *self = BRACKET_COMPLETION(activatable);

	if (self->handler_key_press != 0)
	{
		g_signal_handler_disconnect(self->view, self->handler_key_press);
		self->handler_key_press = 0;
	}

	if (self->handler_editable != 0)
	{
		g_signal_handler_disconnect(self->view, self->handler_editable);
		self->handler_editable = 0;
	}
	
	if (self->buffer)
	{
		if (self->mark_begin)
		{
			gtk_text_buffer_delete_mark(self->buffer, self->mark_begin);
			self->mark_begin = NULL;
		}
		if (self->mark_end)
		{
			gtk_text_buffer_delete_mark(self->buffer, self->mark_end);
			self->mark_end = NULL;
		}
	}

	g_list_free_full(self->stack, g_free);
	self->stack = NULL;
}

static void bracket_completion_get_property(GObject *object, guint prop_id, 
	GValue *value, GParamSpec *pspec)
	{
	BracketCompletion *self = BRACKET_COMPLETION(object);
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

static void bracket_completion_set_property(GObject *object, guint prop_id, 
	const GValue *value, GParamSpec *pspec)
{
	BracketCompletion *self = BRACKET_COMPLETION(object);

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

static void bracket_completion_dispose(GObject *object)
{
	BracketCompletion *self = BRACKET_COMPLETION(object);
	g_clear_object(&self->view);
	G_OBJECT_CLASS(bracket_completion_parent_class)->dispose(object);
}

static void bracket_completion_finalize(GObject *object)
{
	BracketCompletion *self = BRACKET_COMPLETION(object);
	g_list_free_full(self->stack, g_free);
	self->stack = NULL;
	G_OBJECT_CLASS(bracket_completion_parent_class)->finalize(object);
}

static void bracket_completion_init(BracketCompletion *self)
{
	self->stack = NULL;
	self->relocate_marks = TRUE;
	self->handler_key_press = 0;
	self->handler_editable = 0;
}

static void bracket_completion_class_init(BracketCompletionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->get_property = bracket_completion_get_property;
	object_class->set_property = bracket_completion_set_property;
	object_class->dispose = bracket_completion_dispose;
	object_class->finalize = bracket_completion_finalize;

	g_object_class_override_property(object_class, PROP_VIEW, "view");
}

static void xed_view_activatable_iface_init(XedViewActivatableInterface *iface)
{
	iface->activate = bracket_completion_activate;
	iface->deactivate = bracket_completion_deactivate;
}

G_MODULE_EXPORT void
peas_register_types(PeasObjectModule *module)
{
	peas_object_module_register_extension_type(module,
		XED_TYPE_VIEW_ACTIVATABLE,
		BRACKET_COMPLETION_TYPE);
}