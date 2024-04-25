#include "window.h"
#include "paintable.h"
#include "line_drawer.h"
#include "rect_drawer.h"
#include "erase_drawer.h"

struct _MyWindow {
	GtkWindow parent_instance;
};
typedef	struct _MyWindowPrivate {
	MyPaintable *paintable;
	GListStore *drawer_model;
	GtkBox *drawer_box;
	GtkWidget *drawer_form;
	MyDrawer *drawer;
	GtkPicture *picture;
}	MyWindowPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyWindow, my_window, GTK_TYPE_WINDOW)
#define	PRIVATE(self)	((MyWindowPrivate *)my_window_get_instance_private(MY_WINDOW(self)))
#define	DECLARE_PRIVATE(self)	MyWindowPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_PAINTABLE,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static void on_drawer_drop_down_notify_selected(GObject *obj, GParamSpec *pspec, gpointer cb_data)
{
	DECLARE_PRIVATE(cb_data);
	GtkDropDown *drop_down = GTK_DROP_DOWN(obj);

	if (priv->drawer_form != NULL) {
		gtk_box_remove(priv->drawer_box, priv->drawer_form);
		priv->drawer_form = NULL;
	}

	if (priv->drawer != NULL) {
		g_object_unref(G_OBJECT(priv->drawer));
	}
	priv->drawer = MY_DRAWER(g_list_model_get_item(G_LIST_MODEL(gtk_drop_down_get_model(drop_down)), gtk_drop_down_get_selected(drop_down)));

	priv->drawer_form = my_drawer_make_form(priv->drawer);
	g_return_if_fail(priv->drawer_form != NULL);
	gtk_box_append(priv->drawer_box, priv->drawer_form);
}

static void on_picture_drag_begin(GtkGestureDrag *drag, gdouble start_x, gdouble start_y, gpointer cb_data)
{
	DECLARE_PRIVATE(cb_data);

	if (priv->drawer != NULL) {
		g_signal_emit_by_name(G_OBJECT(priv->drawer), "drag-begin", priv->paintable, drag, start_x, start_y);
		gdk_paintable_invalidate_contents(GDK_PAINTABLE(priv->paintable));
	}
}

static void on_picture_drag_update(GtkGestureDrag *drag, gdouble offset_x, gdouble offset_y, gpointer cb_data)
{
	DECLARE_PRIVATE(cb_data);

	if (priv->drawer != NULL) {
		g_signal_emit_by_name(G_OBJECT(priv->drawer), "drag-update", priv->paintable, drag, offset_x, offset_y);
		gdk_paintable_invalidate_contents(GDK_PAINTABLE(priv->paintable));
	}
}

static void on_picture_drag_end(GtkGestureDrag *drag, gdouble offset_x, gdouble offset_y, gpointer cb_data)
{
	DECLARE_PRIVATE(cb_data);

	if (priv->drawer != NULL) {
		g_signal_emit_by_name(G_OBJECT(priv->drawer), "drag-end", priv->paintable, drag, offset_x, offset_y);
		gdk_paintable_invalidate_contents(GDK_PAINTABLE(priv->paintable));
	}
}

static void on_paintable_draw_overlapped(MyPaintable *paintable, cairo_t *cr, gpointer cb_data)
{
	DECLARE_PRIVATE(cb_data);

	if (priv->drawer != NULL) {
		my_drawer_draw_overlapped(priv->drawer, cr);
	}
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_object(&priv->paintable);
	g_clear_object(&priv->drawer);
	g_clear_object(&priv->drawer_form);

	(*G_OBJECT_CLASS(my_window_parent_class)->dispose)(obj);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_PAINTABLE:
		g_value_set_object(value, priv->paintable);
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

	my_drawer_get_type();

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;

	properties[PROP_PAINTABLE] = g_param_spec_object("paintable", "", "", GDK_TYPE_PAINTABLE, G_PARAM_READABLE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example06/window.ui");
	gtk_widget_class_bind_template_child_full(widget_class, "drawer_model", FALSE, G_PRIVATE_OFFSET(MyWindow, drawer_model));
	gtk_widget_class_bind_template_child_full(widget_class, "drawer_box", FALSE, G_PRIVATE_OFFSET(MyWindow, drawer_box));
	gtk_widget_class_bind_template_child_full(widget_class, "picture", FALSE, G_PRIVATE_OFFSET(MyWindow, picture));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_drawer_drop_down_notify_selected", G_CALLBACK(on_drawer_drop_down_notify_selected));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_picture_drag_begin", G_CALLBACK(on_picture_drag_begin));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_picture_drag_update", G_CALLBACK(on_picture_drag_update));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_picture_drag_end", G_CALLBACK(on_picture_drag_end));
}

static void my_window_init(MyWindow *window)
{
	DECLARE_PRIVATE(window);
	MyDrawer *drawer;

	priv->paintable = my_paintable_new();
	g_signal_connect(G_OBJECT(priv->paintable), "draw-overlapped", G_CALLBACK(on_paintable_draw_overlapped), window);
	priv->drawer_form = NULL;
	priv->drawer = NULL;

	gtk_widget_init_template(GTK_WIDGET(window));

	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_PAINTABLE]);

	{
		MyDrawer *(*drawer_constructors[])() = {
			my_line_drawer_new,
			my_rect_drawer_new,
			my_erase_drawer_new,
		};
		for (gint i = 0; i < G_N_ELEMENTS(drawer_constructors); ++i) {
			drawer = (*drawer_constructors[i])();
			g_list_store_append(priv->drawer_model, drawer);
			g_object_unref(G_OBJECT(drawer));
		}
	}
}

GtkWidget * my_window_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_WINDOW, NULL));
}

