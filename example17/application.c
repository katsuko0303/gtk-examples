#include "application.h"
#include "window.h"

struct _MyApplication {
	GtkApplication parent_instance;
};
typedef	struct _MyApplicationPrivate {
	GtkCssProvider *css_provider;
	GdkRGBA label_background;
}	MyApplicationPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyApplication, my_application, GTK_TYPE_APPLICATION)
#define	PRIVATE(self)	((MyApplicationPrivate *)my_application_get_instance_private(MY_APPLICATION(self)))
#define	DECLARE_PRIVATE(self)	MyApplicationPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_LABEL_BACKGROUND,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static void update_style_sheet(MyApplication *app)
{
	DECLARE_PRIVATE(app);
	GdkDisplay *display = gdk_display_get_default();
	gchar *s;
	gchar *source;
	GBytes *bytes;

	if (priv->css_provider != NULL) {
		gtk_style_context_remove_provider_for_display(display, GTK_STYLE_PROVIDER(priv->css_provider));
		g_clear_object(&priv->css_provider);
	}

	priv->css_provider = gtk_css_provider_new();
	s = gdk_rgba_to_string(&priv->label_background);
	source = g_strdup_printf(".custom_label { background-color: %s; }", s);
	g_free(s);
	bytes = g_bytes_new(source, strlen(source));
	gtk_css_provider_load_from_bytes(priv->css_provider, bytes);
	gtk_style_context_add_provider_for_display(display, GTK_STYLE_PROVIDER(priv->css_provider), GTK_STYLE_PROVIDER_PRIORITY_FALLBACK);
	g_bytes_unref(bytes);
	g_free(source);
}

static void startup(GApplication *app)
{
	(*G_APPLICATION_CLASS(my_application_parent_class)->startup)(app);
	update_style_sheet(MY_APPLICATION(app));
}

static void activate(GApplication *app)
{
	GtkWidget *window;

	window = my_window_new();
	gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(app));
	gtk_widget_set_visible(window, TRUE);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_LABEL_BACKGROUND:
		g_value_set_boxed(value, &priv->label_background);
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
	case PROP_LABEL_BACKGROUND:
		memcpy(&priv->label_background, g_value_get_boxed(value), sizeof(GdkRGBA));
		update_style_sheet(MY_APPLICATION(obj));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_object(&priv->css_provider);

	(*G_OBJECT_CLASS(my_application_parent_class)->dispose)(obj);
}

static void my_application_class_init(MyApplicationClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GApplicationClass *app_class = G_APPLICATION_CLASS(klass);

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;
	obj_class->set_property = set_property;

	app_class->startup = startup;
	app_class->activate = activate;

	properties[PROP_LABEL_BACKGROUND] = g_param_spec_boxed("label-background", "", "", GDK_TYPE_RGBA, G_PARAM_READWRITE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);
}

static void my_application_init(MyApplication *app)
{
	DECLARE_PRIVATE(app);

	priv->css_provider = NULL;

	priv->label_background.red = 0.0;
	priv->label_background.green = 0.0;
	priv->label_background.blue = 0.0;
	priv->label_background.alpha = 0.0;
}

MyApplication * my_application_new()
{
	return MY_APPLICATION(g_object_new(MY_TYPE_APPLICATION, NULL));
}

const GdkRGBA * my_application_get_label_background(MyApplication *app)
{
	return &PRIVATE(app)->label_background;
}

void my_application_set_label_background(MyApplication *app, const GdkRGBA *background)
{
	g_object_set(G_OBJECT(app), "label-background", background, NULL);
}

