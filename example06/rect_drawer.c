#include "rect_drawer.h"

struct _MyRectDrawer {
	MyRectAreaDrawer parent_instance;
};
typedef	struct _MyRectDrawerPrivate {
	GdkRGBA color;
}	MyRectDrawerPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyRectDrawer, my_rect_drawer, MY_TYPE_RECT_AREA_DRAWER)
#define	PRIVATE(self)	((MyRectDrawerPrivate *)my_rect_drawer_get_instance_private(MY_RECT_DRAWER(self)))
#define	DECLARE_PRIVATE(self)	MyRectDrawerPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_COLOR,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static GtkWidget * make_form(MyDrawer *drawer)
{
	DECLARE_PRIVATE(drawer);
	GtkBuilder *builder;
	GtkColorDialogButton *color_button;
	GtkWidget *form;
	GError *error = NULL;

	builder = gtk_builder_new();
	gtk_builder_add_from_resource(builder, "/com/example/example06/rect_drawer.ui", &error);
	if (error != NULL) {
		g_printerr("%s(%d): UI build error: %s.\n", __FILE__, __LINE__, error->message);
		g_error_free(error);
		return NULL;
	}

	color_button = GTK_COLOR_DIALOG_BUTTON(gtk_builder_get_object(builder, "color_button"));
	gtk_color_dialog_button_set_rgba(color_button, &priv->color);
	g_object_bind_property(G_OBJECT(drawer), "color", color_button, "rgba", G_BINDING_BIDIRECTIONAL);

	form = GTK_WIDGET(gtk_builder_get_object(builder, "root"));
	g_object_ref(G_OBJECT(form));
	g_object_unref(G_OBJECT(builder));
	return form;
}

static void update_surface(MyRectAreaDrawer *drawer, cairo_t *cr, gdouble x, gdouble y, gdouble width, gdouble height)
{
	DECLARE_PRIVATE(drawer);

	cairo_rectangle(cr, x, y, width, height);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	gdk_cairo_set_source_rgba(cr, &priv->color);
	cairo_fill(cr);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_COLOR:
		g_value_set_boxed(value, &priv->color);
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
	case PROP_COLOR:
		{
			GdkRGBA *rgba = (GdkRGBA *)g_value_get_boxed(value);
			if (! gdk_rgba_equal(rgba, &priv->color)) {
				memcpy(&priv->color, rgba, sizeof(GdkRGBA));
				g_object_notify_by_pspec(obj, pspec);
			}
		}
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void my_rect_drawer_class_init(MyRectDrawerClass *klass)
{
	MyDrawerClass *drawer_class = MY_DRAWER_CLASS(klass);
	MyRectAreaDrawerClass *rect_area_drawer_class = MY_RECT_AREA_DRAWER_CLASS(klass);
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = get_property;
	obj_class->set_property = set_property;

	properties[PROP_COLOR] = g_param_spec_boxed("color", "", "", GDK_TYPE_RGBA, G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	rect_area_drawer_class->update_surface = update_surface;

	drawer_class->make_form = make_form;
}

static void my_rect_drawer_init(MyRectDrawer *drawer)
{
	DECLARE_PRIVATE(drawer);

	gdk_rgba_parse(&priv->color, "black");
}

MyDrawer * my_rect_drawer_new()
{
	return MY_DRAWER(g_object_new(MY_TYPE_RECT_DRAWER, "label", "Rectangle", NULL));
}

