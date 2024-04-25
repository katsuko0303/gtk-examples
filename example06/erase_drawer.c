#include "erase_drawer.h"

struct _MyEraseDrawer {
	MyRectAreaDrawer parent_instance;
};
G_DEFINE_TYPE(MyEraseDrawer, my_erase_drawer, MY_TYPE_RECT_AREA_DRAWER)

static void update_surface(MyRectAreaDrawer *drawer, cairo_t *cr, gdouble x, gdouble y, gdouble width, gdouble height)
{
	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_rectangle(cr, x, y, width, height);
	cairo_fill(cr);
}

static GtkWidget * make_form(MyDrawer *drawer)
{
	GtkWidget *vbox;
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
	return GTK_WIDGET(g_object_ref(G_OBJECT(vbox)));
}

static void my_erase_drawer_class_init(MyEraseDrawerClass *klass)
{
	MyRectAreaDrawerClass *rect_area_drawer_class = MY_RECT_AREA_DRAWER_CLASS(klass);
	MyDrawerClass *drawer_class = MY_DRAWER_CLASS(klass);

	rect_area_drawer_class->update_surface = update_surface;

	drawer_class->make_form = make_form;
}

static void my_erase_drawer_init(MyEraseDrawer *drawer)
{
}

MyDrawer * my_erase_drawer_new()
{
	return MY_DRAWER(g_object_new(MY_TYPE_ERASE_DRAWER, "label", "Erase", NULL));
}

