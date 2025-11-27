#include <gtk/gtk.h>
#include <xed/xed-view.h>
#include <xed/xed-view-activatable.h>
#include <libpeas/peas.h>

#define MAX_FONT_SIZE 64
#define MIN_FONT_SIZE 4

#define TEXT_ZOOM_TYPE (text_zoom_get_type())
G_DECLARE_FINAL_TYPE(TextZoom, text_zoom, TEXT, ZOOM, GObject)

struct _TextZoom
{
	GObject parent_instance;
	XedView *view;
	gulong handler_scroll;
	gulong handler_key;
};

static void xed_view_activatable_iface_init(XedViewActivatableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE(TextZoom, text_zoom, G_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE(XED_TYPE_VIEW_ACTIVATABLE, xed_view_activatable_iface_init))

enum
{
	PROP_0,
	PROP_VIEW
};

static void
set_font_size(GtkTextView *view, int delta, gboolean reset)
{
	PangoFontDescription *default_desc =
		g_object_get_data(G_OBJECT(view), "zoom-default-font");
	gint *default_size_ptr =
		g_object_get_data(G_OBJECT(view), "zoom-default-size");
	gint *current_size_ptr =
		g_object_get_data(G_OBJECT(view), "zoom-current-size");

	if (!default_desc)
	{
		GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(view));
		PangoFontDescription *font_desc = NULL;

		gtk_style_context_get(context, GTK_STATE_FLAG_NORMAL,
			"font", &font_desc, NULL);

		if (font_desc)
		{
			default_desc = pango_font_description_copy(font_desc);
			pango_font_description_free(font_desc);
		}
		else
		{
			default_desc = pango_font_description_from_string("Monospace 10");
		}

		g_object_set_data_full(G_OBJECT(view), "zoom-default-font",
			default_desc, (GDestroyNotify)pango_font_description_free);

		gint default_size = pango_font_description_get_size(default_desc) / PANGO_SCALE;
		if (default_size == 0)
		{
			default_size = 10; // Fallback size
		}

		default_size_ptr = g_new(gint, 1);
		*default_size_ptr = default_size;
		g_object_set_data_full(G_OBJECT(view), "zoom-default-size",
			default_size_ptr, g_free);

		current_size_ptr = g_new(gint, 1);
		*current_size_ptr = default_size;
		g_object_set_data_full(G_OBJECT(view), "zoom-current-size",
			current_size_ptr, g_free);
	}

	gint current_size;
	if (reset)
	{
		current_size = *default_size_ptr;
	}
	else
	{
		current_size = *current_size_ptr + delta;
		if (current_size < MIN_FONT_SIZE)
		{
			current_size = MIN_FONT_SIZE;
		}
		if (current_size > MAX_FONT_SIZE)
		{
			current_size = MAX_FONT_SIZE;
		}
	}

	*current_size_ptr = current_size;

	PangoFontDescription *desc = pango_font_description_copy(default_desc);
	pango_font_description_set_size(desc, current_size * PANGO_SCALE);

	// Use CSS provider instead of the deprecated gtk_widget_override_font
	GtkCssProvider *provider = gtk_css_provider_new();
	gchar *css_data = g_strdup_printf(
		"textview { font-family: %s; font-size: %dpt; }",
		pango_font_description_get_family(desc) ? pango_font_description_get_family(desc) : "monospace",
		current_size);

	gtk_css_provider_load_from_data(provider, css_data, -1, NULL);

	GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(view));
	gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider),
		GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	g_free(css_data);
	g_object_unref(provider);
	pango_font_description_free(desc);
}

static gboolean
on_scroll_event(GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
{
	guint state = event->state & gtk_accelerator_get_default_mod_mask();

	if (state == GDK_CONTROL_MASK)
	{
		switch (event->direction)
		{
			case GDK_SCROLL_UP:
				set_font_size(GTK_TEXT_VIEW(widget), +1, FALSE);
				return TRUE;
			case GDK_SCROLL_DOWN:
				set_font_size(GTK_TEXT_VIEW(widget), -1, FALSE);
				return TRUE;
			case GDK_SCROLL_SMOOTH:
				if (event->delta_y < 0)
				{
					set_font_size(GTK_TEXT_VIEW(widget), +1, FALSE);
				}
				else if (event->delta_y > 0)
				{
					set_font_size(GTK_TEXT_VIEW(widget), -1, FALSE);
				}
				return TRUE;
			default:
				break;
		}
	}
	return FALSE;
}

static gboolean
on_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	if ((event->state & GDK_CONTROL_MASK) != 0)
	{
		switch (event->keyval)
		{
			case GDK_KEY_plus:
			case GDK_KEY_KP_Add:
			case GDK_KEY_equal:
				set_font_size(GTK_TEXT_VIEW(widget), +1, FALSE);
				return TRUE;
			case GDK_KEY_minus:
			case GDK_KEY_KP_Subtract:
				set_font_size(GTK_TEXT_VIEW(widget), -1, FALSE);
				return TRUE;
			case GDK_KEY_0:
			case GDK_KEY_KP_0:
				set_font_size(GTK_TEXT_VIEW(widget), 0, TRUE);
				return TRUE;
		}
	}
	return FALSE;
}

static void
text_zoom_activate(XedViewActivatable *activatable)
{
	TextZoom *self = TEXT_ZOOM(activatable);

	gtk_widget_set_can_focus(GTK_WIDGET(self->view), TRUE);
	gtk_widget_add_events(GTK_WIDGET(self->view),
		GDK_SCROLL_MASK | GDK_KEY_PRESS_MASK);

	self->handler_scroll = g_signal_connect(self->view, "scroll-event",
		G_CALLBACK(on_scroll_event), self);
	self->handler_key = g_signal_connect(self->view, "key-press-event",
		G_CALLBACK(on_key_press_event), self);
}

static void
text_zoom_deactivate(XedViewActivatable *activatable)
{
	TextZoom *self = TEXT_ZOOM(activatable);

	if (self->handler_scroll)
	{
		g_signal_handler_disconnect(self->view, self->handler_scroll);
	}
	if (self->handler_key)
	{
		g_signal_handler_disconnect(self->view, self->handler_key);
	}

	self->handler_scroll = 0;
	self->handler_key = 0;
}

static void
text_zoom_get_property(GObject *object, guint prop_id,
	GValue *value, GParamSpec *pspec)
{
	TextZoom *self = TEXT_ZOOM(object);
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

static void
text_zoom_set_property(GObject *object, guint prop_id,
	const GValue *value, GParamSpec *pspec)
{
	TextZoom *self = TEXT_ZOOM(object);

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

static void
text_zoom_dispose(GObject *object)
{
	TextZoom *self = TEXT_ZOOM(object);

	if (self->handler_scroll && self->view)
		g_signal_handler_disconnect(self->view, self->handler_scroll);
	if (self->handler_key && self->view)
		g_signal_handler_disconnect(self->view, self->handler_key);

	self->handler_scroll = 0;
	self->handler_key = 0;
	self->view = NULL;

	G_OBJECT_CLASS(text_zoom_parent_class)->dispose(object);
}

static void
text_zoom_class_init(TextZoomClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->get_property = text_zoom_get_property;
	object_class->set_property = text_zoom_set_property;
	object_class->dispose = text_zoom_dispose;

	g_object_class_override_property(object_class, PROP_VIEW, "view");
}

static void
text_zoom_init(TextZoom *self)
{
	self->view = NULL;
	self->handler_scroll = 0;
	self->handler_key = 0;
}

static void
xed_view_activatable_iface_init(XedViewActivatableInterface *iface)
{
	iface->activate = text_zoom_activate;
	iface->deactivate = text_zoom_deactivate;
}

G_MODULE_EXPORT void
peas_register_types(PeasObjectModule *module)
{
	peas_object_module_register_extension_type(module,
		XED_TYPE_VIEW_ACTIVATABLE,
		TEXT_ZOOM_TYPE);
}
