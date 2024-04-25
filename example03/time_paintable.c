#include "time_paintable.h"
#include <gtk/gtk.h>

static void paintable_interface_init(GdkPaintableInterface *iface);

struct _MyTimePaintable {
	GObject parent_instance;
};
typedef	struct _MyTimePaintablePrivate {
	GTimer *timer;
	GdkRGBA color;
}	MyTimePaintablePrivate;
G_DEFINE_TYPE_WITH_CODE(MyTimePaintable, my_time_paintable, G_TYPE_OBJECT,
		G_ADD_PRIVATE(MyTimePaintable)
		G_IMPLEMENT_INTERFACE(GDK_TYPE_PAINTABLE, paintable_interface_init)
		)
#define	PRIVATE(self)	my_time_paintable_get_instance_private(MY_TIME_PAINTABLE(self))
#define	DECLARE_PRIVATE(self)	MyTimePaintablePrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_RUNNING,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

enum {
	SIGNAL_START,
	SIGNAL_STOP,
	SIGNAL_RESET,
	NUM_SIGNALS
};
static guint signal_ids[NUM_SIGNALS];

static void start_real(MyTimePaintable *paintable)
{
	DECLARE_PRIVATE(paintable);

	if (priv->timer == NULL) {
		priv->timer = g_timer_new();
	}
	else {
		if (! g_timer_is_active(priv->timer)) {
			g_timer_continue(priv->timer);
		}
	}
	gdk_paintable_invalidate_contents(GDK_PAINTABLE(paintable));
}

static void stop_real(MyTimePaintable *paintable)
{
	DECLARE_PRIVATE(paintable);

	if (priv->timer != NULL && g_timer_is_active(priv->timer)) {
		g_timer_stop(priv->timer);
		gdk_paintable_invalidate_contents(GDK_PAINTABLE(paintable));
	}
}

static void reset_real(MyTimePaintable *paintable)
{
	DECLARE_PRIVATE(paintable);

	if (priv->timer != NULL) {
		g_timer_destroy(priv->timer);
		priv->timer = NULL;
		gdk_paintable_invalidate_contents(GDK_PAINTABLE(paintable));
	}
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	if (priv->timer != NULL) {
		g_timer_destroy(priv->timer);
		priv->timer = NULL;
	}

	(*G_OBJECT_CLASS(my_time_paintable_parent_class)->dispose)(obj);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_RUNNING:
		g_value_set_boolean(value, priv->timer != NULL && g_timer_is_active(priv->timer));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void set_property(GObject *obj, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void my_time_paintable_class_init(MyTimePaintableClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;
	obj_class->set_property = set_property;

	properties[PROP_RUNNING] = g_param_spec_boolean("running", "", "", FALSE, G_PARAM_READABLE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	{
		GClosure *closure = g_cclosure_new(G_CALLBACK(start_real), NULL, NULL);
		signal_ids[SIGNAL_START] = g_signal_newv("start", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST,
				closure, NULL, NULL, NULL, G_TYPE_NONE, 0, NULL);
	}
	{
		GClosure *closure = g_cclosure_new(G_CALLBACK(stop_real), NULL, NULL);
		signal_ids[SIGNAL_STOP] = g_signal_newv("stop", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST,
				closure, NULL, NULL, NULL, G_TYPE_NONE, 0, NULL);
	}
	{
		GClosure *closure = g_cclosure_new(G_CALLBACK(reset_real), NULL, NULL);
		signal_ids[SIGNAL_RESET] = g_signal_newv("reset", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST,
				closure, NULL, NULL, NULL, G_TYPE_NONE, 0, NULL);
	}
}

static void my_time_paintable_init(MyTimePaintable *paintable)
{
	DECLARE_PRIVATE(paintable);

	priv->timer = NULL;
	gdk_rgba_parse(&priv->color, "black");
}

static GdkPaintableFlags get_flags(GdkPaintable *paintable)
{
	return GDK_PAINTABLE_STATIC_SIZE;
}

static void snapshot(GdkPaintable *paintable, GdkSnapshot *snapshot, gdouble width, gdouble height)
{
	DECLARE_PRIVATE(paintable);
	PangoFontDescription *font_desc;
	gchar *text;
	gulong milliseconds;
	PangoLayout *layout;

	PangoContext *ctx = pango_font_map_create_context(pango_cairo_font_map_get_default());
	layout = pango_layout_new(ctx);

	font_desc = pango_font_description_from_string("Sans");
	pango_font_description_set_absolute_size(font_desc, height * PANGO_SCALE);
	pango_layout_set_font_description(layout, font_desc);

	if (priv->timer != NULL) {
		milliseconds = (gulong)(g_timer_elapsed(priv->timer, NULL) * 1000);
	}
	else {
		milliseconds = 0ul;
	}
	text = g_strdup_printf("%u:%02u.%02u", (guint)(milliseconds / (60 * 1000)), (guint)(milliseconds / 1000 % 60), (guint)(milliseconds / 10 % 100));
	pango_layout_set_text(layout, text, -1);
	g_free(text);

	pango_layout_set_width(layout, width * PANGO_SCALE);
	pango_layout_set_height(layout, height * PANGO_SCALE);
	pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);

	gtk_snapshot_append_layout(GTK_SNAPSHOT(snapshot), layout, &priv->color);

	g_object_unref(G_OBJECT(ctx));
	g_object_unref(G_OBJECT(layout));
}

static void paintable_interface_init(GdkPaintableInterface *iface)
{
	iface->get_flags = get_flags;
	iface->snapshot = snapshot;
}

MyTimePaintable * my_time_paintable_new()
{
	return MY_TIME_PAINTABLE(g_object_new(MY_TYPE_TIME_PAINTABLE, NULL));
}

gboolean my_time_paintable_is_running(MyTimePaintable *paintable)
{
	DECLARE_PRIVATE(paintable);

	return priv->timer != NULL && g_timer_is_active(priv->timer);
}

void my_time_paintable_start(MyTimePaintable *paintable)
{
	g_signal_emit(G_OBJECT(paintable), signal_ids[SIGNAL_START], 0);
}

void my_time_paintable_stop(MyTimePaintable *paintable)
{
	g_signal_emit(G_OBJECT(paintable), signal_ids[SIGNAL_STOP], 0);
}

void my_time_paintable_reset(MyTimePaintable *paintable)
{
	g_signal_emit(G_OBJECT(paintable), signal_ids[SIGNAL_RESET], 0);
}

