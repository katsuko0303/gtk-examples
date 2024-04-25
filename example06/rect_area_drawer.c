#include "rect_area_drawer.h"

typedef	struct _MyRectAreaDrawerPrivate {
	gboolean dragging;
	gdouble drag_start_x;
	gdouble drag_start_y;
	gdouble drag_end_x;
	gdouble drag_end_y;
}	MyRectAreaDrawerPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyRectAreaDrawer, my_rect_area_drawer, MY_TYPE_DRAWER)
#define	PRIVATE(self)	((MyRectAreaDrawerPrivate *)my_rect_area_drawer_get_instance_private(MY_RECT_AREA_DRAWER(self)))
#define	DECLARE_PRIVATE(self)	MyRectAreaDrawerPrivate *priv = PRIVATE(self)

enum {
	SIGNAL_UPDATE_SURFACE,
	NUM_SIGNALS
};
static guint signal_ids[NUM_SIGNALS];

static GtkWidget * make_form(MyDrawer *drawer)
{
	return gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
}

static void drag_begin(MyDrawer *drawer, MyPaintable *paintable, GtkGestureDrag *drag, gdouble start_x, gdouble start_y)
{
	DECLARE_PRIVATE(drawer);
	gdouble x, y;
	GtkWidget *widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(drag));

	if (my_paintable_convert_to_surface_coordinate(paintable, start_x, start_y, gtk_widget_get_width(widget), gtk_widget_get_height(widget), &x, &y)) {
		priv->drag_start_x = x;
		priv->drag_start_y = y;
		priv->drag_end_x = x;
		priv->drag_end_y = y;
		priv->dragging = TRUE;
	}
}

static void drag_update(MyDrawer *drawer, MyPaintable *paintable, GtkGestureDrag *drag, gdouble offset_x, gdouble offset_y)
{
	DECLARE_PRIVATE(drawer);
	GtkWidget *widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(drag));
	gdouble sx, sy;

	if (priv->dragging) {
		gtk_gesture_drag_get_start_point(drag, &sx, &sy);
		my_paintable_convert_to_surface_coordinate(paintable, sx + offset_x, sy + offset_y, gtk_widget_get_width(widget), gtk_widget_get_height(widget),
				&priv->drag_end_x, &priv->drag_end_y);
	}
}

static void drag_end(MyDrawer *drawer, MyPaintable *paintable, GtkGestureDrag *drag, gdouble offset_x, gdouble offset_y)
{
	DECLARE_PRIVATE(drawer);
	GtkWidget *widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(drag));
	gdouble sx, sy;
	gdouble ex, ey;
	gdouble x, y, width, height;
	cairo_t *cr;
	MyRectAreaDrawerClass *drawer_class = MY_RECT_AREA_DRAWER_GET_CLASS(drawer);

	g_return_if_fail(drawer_class->update_surface != NULL);

	if (priv->dragging) {
		gtk_gesture_drag_get_start_point(drag, &sx, &sy);
		my_paintable_convert_to_surface_coordinate(paintable, sx + offset_x, sy + offset_y, gtk_widget_get_width(widget), gtk_widget_get_height(widget), &ex, &ey);

		if (priv->drag_start_x < ex) {
			x = priv->drag_start_x;
			width = ex - priv->drag_start_x;
		}
		else {
			x = ex;
			width = priv->drag_start_x - ex;
		}
		if (priv->drag_start_y < ey) {
			y = priv->drag_start_y;
			height = ey - priv->drag_start_y;
		}
		else {
			y = ey;
			height = priv->drag_start_y - ey;
		}
		if (width > 0 && height > 0) {
			cr = my_paintable_make_cairo_context(paintable);

#if 0
			cairo_rectangle(cr, x, y, width, height);
			cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
			gdk_cairo_set_source_rgba(cr, &priv->color);
			cairo_fill(cr);
#elif 0
			(*drawer_class->update_surface)(MY_RECT_AREA_DRAWER(drawer), cr, x, y, width, height);
#else
			g_signal_emit(G_OBJECT(drawer), signal_ids[SIGNAL_UPDATE_SURFACE], 0, cr, x, y, width, height);
#endif

			cairo_destroy(cr);
		}
		priv->dragging = FALSE;
	}
}

static void draw_overlapped(MyDrawer *drawer, cairo_t *cr)
{
	DECLARE_PRIVATE(drawer);
	gdouble x, y, width, height;

	if (! priv->dragging) {
		return ;
	}
	cairo_save(cr);

	if (priv->drag_start_x < priv->drag_end_x) {
		x = priv->drag_start_x;
		width = priv->drag_end_x - priv->drag_start_x;
	}
	else {
		x = priv->drag_end_x;
		width = priv->drag_start_x - priv->drag_end_x;
	}
	if (priv->drag_start_y < priv->drag_end_y) {
		y = priv->drag_start_y;
		height = priv->drag_end_y - priv->drag_start_y;
	}
	else {
		y = priv->drag_end_y;
		height = priv->drag_start_y - priv->drag_end_y;
	}
	if (width > 0 && height > 0) {
		cairo_rectangle(cr, x, y, width, height);

		cairo_set_operator(cr, CAIRO_OPERATOR_ADD);
		cairo_set_source_rgba(cr, 1, 1, 1, 0.5);
		cairo_fill_preserve(cr);

		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
		cairo_set_source_rgba(cr, 0, 0, 0, 1);
		cairo_set_line_width(cr, 2);
		cairo_stroke(cr);
	}

	cairo_restore(cr);
}

static void my_rect_area_drawer_class_init(MyRectAreaDrawerClass *klass)
{
	MyDrawerClass *drawer_class = MY_DRAWER_CLASS(klass);

	drawer_class->make_form = make_form;
	drawer_class->drag_begin = drag_begin;
	drawer_class->drag_update = drag_update;
	drawer_class->drag_end = drag_end;
	drawer_class->draw_overlapped = draw_overlapped;

	signal_ids[SIGNAL_UPDATE_SURFACE] = g_signal_new("update-surface", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET(MyRectAreaDrawerClass, update_surface), NULL, NULL, NULL, G_TYPE_NONE,
			5, G_TYPE_POINTER, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE);
}

static void my_rect_area_drawer_init(MyRectAreaDrawer *drawer)
{
	DECLARE_PRIVATE(drawer);

	priv->dragging = FALSE;
}

