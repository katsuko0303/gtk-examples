#include "paintable.h"
#include <gtk/gtk.h>
#include <cairo.h>

#define	DEFAULT_SURFACE_WIDTH	256
#define	DEFAULT_SURFACE_HEIGHT	256

static void paintable_interface_init(GdkPaintableInterface *iface);

struct _MyPaintable {
	GObject parent_instance;
};
typedef	struct _MyPaintablePrivate {
	cairo_surface_t *surface;
	gint surface_width;
	gint surface_height;
	guint idle_id;
}	MyPaintablePrivate;
G_DEFINE_TYPE_WITH_CODE(MyPaintable, my_paintable, G_TYPE_OBJECT,
		G_ADD_PRIVATE(MyPaintable)
		G_IMPLEMENT_INTERFACE(GDK_TYPE_PAINTABLE, paintable_interface_init)
		)
#define	PRIVATE(self)	((MyPaintablePrivate *)my_paintable_get_instance_private(MY_PAINTABLE(self)))
#define	DECLARE_PRIVATE(self)	MyPaintablePrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_SURFACE_WIDTH,
	PROP_SURFACE_HEIGHT,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

enum {
	SIGNAL_DRAW_OVERLAPPED,
	NUM_SIGNALS
};
static guint signal_ids[NUM_SIGNALS];

static gboolean on_idle(gpointer cb_data)
{
	MyPaintable *paintable = MY_PAINTABLE(cb_data);
	DECLARE_PRIVATE(paintable);
	cairo_t *cr;

	if (priv->surface != NULL) {
		cairo_surface_destroy(priv->surface);
	}

	priv->surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, priv->surface_width, priv->surface_height);
	cr = cairo_create(priv->surface);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_rectangle(cr, 0, 0, priv->surface_width, priv->surface_height);
	cairo_fill(cr);
	cairo_destroy(cr);

	gdk_paintable_invalidate_contents(GDK_PAINTABLE(paintable));
	priv->idle_id = 0;
	return G_SOURCE_REMOVE;
}

static void snapshot(GdkPaintable *paintable, GdkSnapshot *ss, gdouble width, gdouble height)
{
	DECLARE_PRIVATE(paintable);
	GdkRGBA color = { 0.2, 0.2, 0.2, 1.0 };
	graphene_rect_t bounds = GRAPHENE_RECT_INIT(0, 0, width, height);
	cairo_t *cr;

	gtk_snapshot_save(GTK_SNAPSHOT(ss));

	gtk_snapshot_append_color(GTK_SNAPSHOT(ss), &color, &bounds);
	if (priv->surface != NULL) {
		cr = gtk_snapshot_append_cairo(GTK_SNAPSHOT(ss), &bounds);
		cairo_translate(cr, width / 2.0, height / 2.0);
		cairo_translate(cr, -(gdouble)priv->surface_width / 2.0, -(gdouble)priv->surface_height / 2.0);

		cairo_set_source_surface(cr, priv->surface, 0, 0);
		cairo_paint(cr);

		g_signal_emit(G_OBJECT(paintable), signal_ids[SIGNAL_DRAW_OVERLAPPED], 0, cr);

		cairo_destroy(cr);
	}

	gtk_snapshot_restore(GTK_SNAPSHOT(ss));
}

static GdkPaintableFlags get_flags(GdkPaintable *paintable)
{
	return GDK_PAINTABLE_STATIC_SIZE;
}

static void paintable_interface_init(GdkPaintableInterface *iface)
{
	iface->snapshot = snapshot;
	iface->get_flags = get_flags;
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	if (priv->surface != NULL) {
		cairo_surface_destroy(priv->surface);
		priv->surface = NULL;
	}

	(*G_OBJECT_CLASS(my_paintable_parent_class)->dispose)(obj);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_SURFACE_WIDTH:
		g_value_set_int(value, priv->surface_width);
		break;

	case PROP_SURFACE_HEIGHT:
		g_value_set_int(value, priv->surface_height);
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
	case PROP_SURFACE_WIDTH:
		priv->surface_width = g_value_get_int(value);
		if (priv->idle_id == 0) {
			priv->idle_id = g_idle_add(on_idle, obj);
		}
		break;

	case PROP_SURFACE_HEIGHT:
		priv->surface_height = g_value_get_int(value);
		if (priv->idle_id == 0) {
			priv->idle_id = g_idle_add(on_idle, obj);
		}
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void my_paintable_class_init(MyPaintableClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;
	obj_class->set_property = set_property;

	properties[PROP_SURFACE_WIDTH] = g_param_spec_int("surface-width", "", "", 1, G_MAXINT, DEFAULT_SURFACE_WIDTH, G_PARAM_READWRITE);
	properties[PROP_SURFACE_HEIGHT] = g_param_spec_int("surface-height", "", "", 1, G_MAXINT, DEFAULT_SURFACE_HEIGHT, G_PARAM_READWRITE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	{
		GType arg_types[] = { G_TYPE_POINTER };
		signal_ids[SIGNAL_DRAW_OVERLAPPED] = g_signal_newv("draw-overlapped", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST,
				NULL, NULL, NULL, NULL, G_TYPE_NONE, G_N_ELEMENTS(arg_types), arg_types);
	}
}

static void my_paintable_init(MyPaintable *paintable)
{
	DECLARE_PRIVATE(paintable);

	priv->surface_width = DEFAULT_SURFACE_WIDTH;
	priv->surface_height = DEFAULT_SURFACE_HEIGHT;
	priv->surface = NULL;

	priv->idle_id = g_idle_add(on_idle, paintable);
}

MyPaintable * my_paintable_new()
{
	return MY_PAINTABLE(g_object_new(MY_TYPE_PAINTABLE, NULL));
}

cairo_t * my_paintable_make_cairo_context(MyPaintable *paintable)
{
	DECLARE_PRIVATE(paintable);

	return (priv->surface != NULL? cairo_create(priv->surface): NULL);
}

gboolean my_paintable_convert_to_surface_coordinate(MyPaintable *paintable, gdouble x, gdouble y, gdouble width, gdouble height, gdouble *sx, gdouble *sy)
{
	DECLARE_PRIVATE(paintable);
	gdouble _sx, _sy;

	_sx = x - width / 2.0 + priv->surface_width / 2.0;
	if (sx != NULL)	*sx = _sx;
	_sy = y - height / 2.0 + priv->surface_height / 2.0;
	if (sy != NULL)	*sy = _sy;
	return (_sx >= 0 && _sx < priv->surface_width && _sy >= 0 && _sy < priv->surface_height);
}

