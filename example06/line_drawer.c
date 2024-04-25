#include "line_drawer.h"
#include "paintable.h"
#include <cairo.h>
#include <math.h>

#define	DEFAULT_LINE_WIDTH	2

struct _MyLineDrawer {
	MyDrawer parent_instance;
};
typedef	struct _MyLineDrawerPrivate {
	GdkRGBA color;
	gint line_width;

	gboolean dragging;
	gdouble drag_last_x;
	gdouble drag_last_y;

}	MyLineDrawerPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyLineDrawer, my_line_drawer, MY_TYPE_DRAWER)
#define	PRIVATE(self)	((MyLineDrawerPrivate *)my_line_drawer_get_instance_private(MY_LINE_DRAWER(self)))
#define	DECLARE_PRIVATE(self)	MyLineDrawerPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_COLOR,
	PROP_LINE_WIDTH,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static GtkWidget * make_form(MyDrawer *drawer)
{
	DECLARE_PRIVATE(drawer);
	GtkBuilder *builder;
	GtkColorDialogButton *color_button;
	GtkAdjustment *line_width_adjustment;
	GtkWidget *form;
	GError *error = NULL;

	builder = gtk_builder_new();
	gtk_builder_add_from_resource(builder, "/com/example/example06/line_drawer.ui", &error);
	if (error != NULL) {
		g_printerr("%s(%d): UI build error: %s.\n", __FILE__, __LINE__, error->message);
		g_error_free(error);
		return NULL;
	}

	color_button = GTK_COLOR_DIALOG_BUTTON(gtk_builder_get_object(builder, "color_button"));
	gtk_color_dialog_button_set_rgba(color_button, &priv->color);
	g_object_bind_property(G_OBJECT(drawer), "color", color_button, "rgba", G_BINDING_BIDIRECTIONAL);

	line_width_adjustment = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "line_width_adjustment"));
	gtk_adjustment_set_value(line_width_adjustment, priv->line_width);
	g_object_bind_property(G_OBJECT(drawer), "line-width", line_width_adjustment, "value", G_BINDING_BIDIRECTIONAL);

	form = GTK_WIDGET(gtk_builder_get_object(builder, "root"));
	g_object_ref(G_OBJECT(form));
	g_object_unref(G_OBJECT(builder));
	return form;
}

static void drag_begin(MyDrawer *drawer, MyPaintable *paintable, GtkGestureDrag *drag, gdouble start_x, gdouble start_y)
{
	DECLARE_PRIVATE(drawer);
	GtkWidget *widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(drag));
	gdouble x, y;

	if (my_paintable_convert_to_surface_coordinate(paintable, start_x, start_y, gtk_widget_get_width(widget), gtk_widget_get_height(widget), &x, &y)) {
		priv->drag_last_x = x;
		priv->drag_last_y = y;
		priv->dragging = TRUE;
	}
}

static void _update_for_dragging(MyDrawer *drawer, MyPaintable *paintable, GtkGestureDrag *drag, gdouble offset_x, gdouble offset_y)
{
	DECLARE_PRIVATE(drawer);
	GtkWidget *widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(drag));
	gdouble sx, sy;
	gdouble x, y;
	cairo_t *cr;

	if (! priv->dragging) {
		return ;
	}
	gtk_gesture_drag_get_start_point(drag, &sx, &sy);
	my_paintable_convert_to_surface_coordinate(paintable, sx + offset_x, sy + offset_y, gtk_widget_get_width(widget), gtk_widget_get_height(widget), &x, &y);

	cr = my_paintable_make_cairo_context(paintable);
	if (cr != NULL) {
		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
		gdk_cairo_set_source_rgba(cr, &priv->color);

		cairo_set_line_width(cr, priv->line_width);
		cairo_move_to(cr, priv->drag_last_x, priv->drag_last_y);
		cairo_line_to(cr, x, y);
		cairo_stroke(cr);

		cairo_arc(cr, priv->drag_last_x, priv->drag_last_y, priv->line_width / 2.0, 0, 2.0 * M_PI);
		cairo_arc(cr, x, y, priv->line_width / 2.0, 0, 2.0 * M_PI);
		cairo_fill(cr);

		cairo_destroy(cr);
	}
	priv->drag_last_x = x;
	priv->drag_last_y = y;
}

static void drag_update(MyDrawer *drawer, MyPaintable *paintable, GtkGestureDrag *drag, gdouble offset_x, gdouble offset_y)
{
	_update_for_dragging(drawer, paintable, drag, offset_x, offset_y);
}

static void drag_end(MyDrawer *drawer, MyPaintable *paintable, GtkGestureDrag *drag, gdouble offset_x, gdouble offset_y)
{
	DECLARE_PRIVATE(drawer);

	_update_for_dragging(drawer, paintable, drag, offset_x, offset_y);
	priv->dragging = FALSE;
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_COLOR:
		g_value_set_boxed(value, &priv->color);
		break;

	case PROP_LINE_WIDTH:
		g_value_set_int(value, priv->line_width);
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
				memcpy(&priv->color, g_value_get_boxed(value), sizeof(GdkRGBA));
				g_object_notify_by_pspec(obj, pspec);
			}
		}
		break;

	case PROP_LINE_WIDTH:
		{
			gint line_width = g_value_get_int(value);
			if (line_width != priv->line_width) {
				priv->line_width = g_value_get_int(value);
				g_object_notify_by_pspec(obj, pspec);
			}
		}
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void dispose(GObject *obj)
{
	(*G_OBJECT_CLASS(my_line_drawer_parent_class)->dispose)(obj);
}

static void my_line_drawer_class_init(MyLineDrawerClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	MyDrawerClass *drawer_class = MY_DRAWER_CLASS(klass);

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;
	obj_class->set_property = set_property;

	properties[PROP_COLOR] = g_param_spec_boxed("color", "", "", GDK_TYPE_RGBA, G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);
	properties[PROP_LINE_WIDTH] = g_param_spec_int("line-width", "", "", 1, G_MAXINT, DEFAULT_LINE_WIDTH, G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	drawer_class->make_form = make_form;
	drawer_class->drag_begin = drag_begin;
	drawer_class->drag_update = drag_update;
	drawer_class->drag_end = drag_end;
}

static void my_line_drawer_init(MyLineDrawer *drawer)
{
	DECLARE_PRIVATE(drawer);

	gdk_rgba_parse(&priv->color, "black");
	priv->line_width = DEFAULT_LINE_WIDTH;
	priv->dragging = FALSE;
}

MyDrawer * my_line_drawer_new()
{
	return MY_DRAWER(g_object_new(MY_TYPE_LINE_DRAWER, "label", "Line", NULL));
}

