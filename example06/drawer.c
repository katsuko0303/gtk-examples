#include "drawer.h"

typedef	struct _MyDrawerPrivate {
	gchar *label;
}	MyDrawerPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyDrawer, my_drawer, G_TYPE_OBJECT)
#define	PRIVATE(self)	((MyDrawerPrivate *)my_drawer_get_instance_private(MY_DRAWER(self)))
#define	DECLARE_PRIVATE(self)	MyDrawerPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_LABEL,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

enum {
	SIGNAL_DRAG_BEGIN,
	SIGNAL_DRAG_UPDATE,
	SIGNAL_DRAG_END,
	SIGNAL_MAKE_FORM,
	SIGNAL_DRAW_OVERLAPPED,
	NUM_SIGNALS
};
static guint signal_ids[NUM_SIGNALS];

static void drag_begin(MyDrawer *drawer, MyPaintable *paintable, GtkGestureDrag *drag, gdouble start_x, gdouble start_y)
{
	//g_print("%s(%d): drag begin: %f, %f.\n", __FILE__, __LINE__, start_x, start_y);
}

static void drag_update(MyDrawer *drawer, MyPaintable *paintable, GtkGestureDrag *drag, gdouble offset_x, gdouble offset_y)
{
	//g_print("%s(%d): drag update: %f, %f.\n", __FILE__, __LINE__, offset_x, offset_y);
}

static void drag_end(MyDrawer *drawer, MyPaintable *paintable, GtkGestureDrag *drag, gdouble offset_x, gdouble offset_y)
{
	//g_print("%s(%d): drag end: %f, %f.\n", __FILE__, __LINE__, offset_x, offset_y);
}

static void draw_overlapped(MyDrawer *drawer, cairo_t *cr)
{
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_pointer(&priv->label, g_free);

	(*G_OBJECT_CLASS(my_drawer_parent_class)->dispose)(obj);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_LABEL:
		g_value_set_string(value, priv->label);
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
	case PROP_LABEL:
		g_clear_pointer(&priv->label, g_free);
		priv->label = g_value_dup_string(value);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void my_drawer_class_init(MyDrawerClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;
	obj_class->set_property = set_property;

	properties[PROP_LABEL] = g_param_spec_string("label", "", "", "", G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	signal_ids[SIGNAL_DRAG_BEGIN] = g_signal_new("drag-begin", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET(MyDrawerClass, drag_begin), NULL, NULL, NULL, G_TYPE_NONE,
			4, MY_TYPE_PAINTABLE, GTK_TYPE_GESTURE_DRAG, G_TYPE_DOUBLE, G_TYPE_DOUBLE);
	signal_ids[SIGNAL_DRAG_UPDATE] = g_signal_new("drag-update", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET(MyDrawerClass, drag_update), NULL, NULL, NULL, G_TYPE_NONE,
			4, MY_TYPE_PAINTABLE, GTK_TYPE_GESTURE_DRAG, G_TYPE_DOUBLE, G_TYPE_DOUBLE);
	signal_ids[SIGNAL_DRAG_END] = g_signal_new("drag-end", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET(MyDrawerClass, drag_end), NULL, NULL, NULL, G_TYPE_NONE,
			4, MY_TYPE_PAINTABLE, GTK_TYPE_GESTURE_DRAG, G_TYPE_DOUBLE, G_TYPE_DOUBLE);
	signal_ids[SIGNAL_MAKE_FORM] = g_signal_new("make-form", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET(MyDrawerClass, make_form), NULL, NULL, NULL, GTK_TYPE_WIDGET,
			0);
	signal_ids[SIGNAL_DRAW_OVERLAPPED] = g_signal_new("draw-overlapped", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET(MyDrawerClass, draw_overlapped), NULL, NULL, NULL, G_TYPE_NONE,
			1, G_TYPE_POINTER);

	klass->make_form = NULL;
	klass->drag_begin = drag_begin;
	klass->drag_update = drag_update;
	klass->drag_end = drag_end;
	klass->draw_overlapped = draw_overlapped;
}

static void my_drawer_init(MyDrawer *drawer)
{
	DECLARE_PRIVATE(drawer);

	priv->label = g_strdup("");
}

GtkWidget * my_drawer_make_form(MyDrawer *drawer)
{
	GtkWidget *widget;

	g_signal_emit(G_OBJECT(drawer), signal_ids[SIGNAL_MAKE_FORM], 0, &widget);
	return widget;
}

void my_drawer_draw_overlapped(MyDrawer *drawer, cairo_t *cr)
{
	g_signal_emit(G_OBJECT(drawer), signal_ids[SIGNAL_DRAW_OVERLAPPED], 0, cr);
}

