#pragma once
#include <gtk/gtk.h>
#include <cairo.h>
#include "paintable.h"

G_BEGIN_DECLS

#define	MY_TYPE_DRAWER	my_drawer_get_type()
G_DECLARE_DERIVABLE_TYPE(MyDrawer, my_drawer, MY, DRAWER, GObject)

struct _MyDrawerClass {
	GObjectClass parent_class;

	GtkWidget * (*make_form)(MyDrawer *drawer);
	void (*drag_begin)(MyDrawer *drawer, MyPaintable *paintable, GtkGestureDrag *drag, gdouble start_x, gdouble start_y);
	void (*drag_update)(MyDrawer *drawer, MyPaintable *paintable, GtkGestureDrag *drag, gdouble offset_x, gdouble offset_y);
	void (*drag_end)(MyDrawer *drawer, MyPaintable *paintable, GtkGestureDrag *drag, gdouble offset_x, gdouble offset_y);
	void (*draw_overlapped)(MyDrawer *drawer, cairo_t *cr);

	gpointer padding[12];
};

GtkWidget * my_drawer_make_form(MyDrawer *drawer);
void my_drawer_draw_overlapped(MyDrawer *drawer, cairo_t *cr);

G_END_DECLS


