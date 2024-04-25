#include "window.h"

struct _MyWindow {
	GtkWindow parent_instance;
};
typedef	struct _MyWindowPrivate {
	gchar *result;
}	MyWindowPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyWindow, my_window, GTK_TYPE_WINDOW)
#define	PRIVATE(self)	((MyWindowPrivate *)my_window_get_instance_private(MY_WINDOW(self)))
#define	DECLARE_PRIVATE(self)	MyWindowPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_RESULT,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static void on_released(GtkGestureClick *gesture, gint n_press, gdouble x, gdouble y, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	GtkWidget *label;
	gint lx, ly;
	gint index;
	gint trailing;
	PangoLayout *layout;
	gboolean inside;
	gchar *text;
	const gchar *layout_text;
	const gchar *start, *end;
	gchar *s;

	label = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));
	layout = gtk_label_get_layout(GTK_LABEL(label));
	gtk_label_get_layout_offsets(GTK_LABEL(label), &lx, &ly);
	inside = pango_layout_xy_to_index(layout, PANGO_SCALE * ((int)x - lx), PANGO_SCALE * ((int)y - ly), &index, &trailing);
	if (inside) {
		layout_text = pango_layout_get_text(layout);
		start = layout_text + index;
		end = start + 1;
		while ((((unsigned char)*end) & 0xc0) == 0x80)	++end;
		s = g_strndup(start, (gsize)(end - start));

		text = g_strdup_printf("%d: %s", index, s);
		g_object_set(G_OBJECT(window), "result", text, NULL);
		g_free(s);
		g_free(text);
	}
	else {
		g_object_set(G_OBJECT(window), "result", "(clicked outside)", NULL);
	}
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_pointer(&priv->result, g_free);

	(*G_OBJECT_CLASS(my_window_parent_class)->dispose)(obj);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_RESULT:
		g_value_set_string(value, priv->result);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void set_property(GObject *obj, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_RESULT:
		g_clear_pointer(&priv->result, g_free);
		priv->result = g_value_dup_string(value);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void my_window_class_init(MyWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;
	obj_class->set_property = set_property;

	properties[PROP_RESULT] = g_param_spec_string("result", "", "", "", G_PARAM_READWRITE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example15/window.ui");
	gtk_widget_class_bind_template_callback_full(widget_class, "on_released", G_CALLBACK(on_released));
}

static void my_window_init(MyWindow *window)
{
	DECLARE_PRIVATE(window);

	priv->result = g_strdup("");

	gtk_widget_init_template(GTK_WIDGET(window));
}

GtkWidget * my_window_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_WINDOW, NULL));
}

