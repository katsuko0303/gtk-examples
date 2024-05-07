#include "window.h"
#include "application.h"

struct _MyWindow {
	GtkWindow parent_instance;
};
G_DEFINE_TYPE(MyWindow, my_window, GTK_TYPE_WINDOW)

enum {
	PROP_0,
	PROP_LABEL_BACKGROUND,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	switch (prop_id) {
	case PROP_LABEL_BACKGROUND:
		{
			MyApplication *app = MY_APPLICATION(gtk_window_get_application(GTK_WINDOW(obj)));
			if (app != NULL) {
				g_value_set_boxed(value, my_application_get_label_background(app));
			}
			else {
				GdkRGBA rgba = { 0.0, 0.0, 0.0, 0.0 };
				g_value_set_boxed(value, &rgba);
			}
		}
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void set_property(GObject *obj, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	switch (prop_id) {
	case PROP_LABEL_BACKGROUND:
		{
			MyApplication *app = MY_APPLICATION(gtk_window_get_application(GTK_WINDOW(obj)));
			const GdkRGBA *new_background = (GdkRGBA *)g_value_get_boxed(value);
			const GdkRGBA *background = my_application_get_label_background(app);
			if (! gdk_rgba_equal(background, new_background)) {
				my_application_set_label_background(app, new_background);
			}
		}
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void my_window_class_init(MyWindowClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->get_property = get_property;
	obj_class->set_property = set_property;

	properties[PROP_LABEL_BACKGROUND] = g_param_spec_boxed("label-background", "", "", GDK_TYPE_RGBA, G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example17/window.ui");
}

static void my_window_init(MyWindow *window)
{
	gtk_widget_init_template(GTK_WIDGET(window));

	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_LABEL_BACKGROUND]);
}

GtkWidget * my_window_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_WINDOW, NULL));
}

