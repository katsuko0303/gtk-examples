#include "window.h"
#include "time_paintable.h"

struct _MyWindow {
	GtkWindow parent_instance;
};
typedef	struct _MyWindowPrivate {
	MyTimePaintable *paintable;
	guint tick_id;
}	MyWindowPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyWindow, my_window, GTK_TYPE_WINDOW)
#define	PRIVATE(self)	((MyWindowPrivate *)my_window_get_instance_private(MY_WINDOW(self)))
#define	DECLARE_PRIVATE(self)	MyWindowPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_PLAY_ICON_NAME,
	PROP_PAINTABLE,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static gboolean on_animate(GtkWidget *widget, GdkFrameClock *clock, gpointer cb_data)
{
	gdk_paintable_invalidate_contents(GDK_PAINTABLE(PRIVATE(widget)->paintable));
	return G_SOURCE_CONTINUE;
}

static void on_start_or_stop(GtkButton *button, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	DECLARE_PRIVATE(window);

	if (my_time_paintable_is_running(priv->paintable)) {
		my_time_paintable_stop(priv->paintable);
		if (priv->tick_id != 0) {
			gtk_widget_remove_tick_callback(GTK_WIDGET(window), priv->tick_id);
			priv->tick_id = 0;
		}
	}
	else {
		my_time_paintable_start(priv->paintable);
		if (priv->tick_id == 0) {
			priv->tick_id = gtk_widget_add_tick_callback(GTK_WIDGET(window), on_animate, NULL, NULL);
		}
	}
	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_PLAY_ICON_NAME]);
}

static void on_reset(GtkButton * button, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	DECLARE_PRIVATE(window);

	my_time_paintable_reset(priv->paintable);
	if (priv->tick_id != 0) {
		gtk_widget_remove_tick_callback(GTK_WIDGET(window), priv->tick_id);
		priv->tick_id = 0;
	}
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_PLAY_ICON_NAME:
		g_value_set_string(value, (! my_time_paintable_is_running(priv->paintable)? "media-playback-start-symbolic": "media-playback-pause-symbolic"));
		break;

	case PROP_PAINTABLE:
		g_value_set_object(value, priv->paintable);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_object(&priv->paintable);

	(*G_OBJECT_CLASS(my_window_parent_class)->dispose)(obj);
}

static void my_window_class_init(MyWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;

	properties[PROP_PLAY_ICON_NAME] = g_param_spec_string("play-icon-name", "", "", "media-playback-start-symbolic", G_PARAM_READABLE);
	properties[PROP_PAINTABLE] = g_param_spec_object("paintable", "", "", GDK_TYPE_PAINTABLE, G_PARAM_READABLE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example03/window.ui");
	gtk_widget_class_bind_template_callback_full(widget_class, "on_reset", G_CALLBACK(on_reset));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_start_or_stop", G_CALLBACK(on_start_or_stop));
}

static void my_window_init(MyWindow *window)
{
	DECLARE_PRIVATE(window);

	priv->paintable = my_time_paintable_new();
	priv->tick_id = 0;

	gtk_widget_init_template(GTK_WIDGET(window));

	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_PAINTABLE]);
}

GtkWidget * my_window_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_WINDOW, NULL));
}
