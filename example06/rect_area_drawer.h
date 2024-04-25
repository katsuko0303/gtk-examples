#pragma once
#include "drawer.h"
#include <cairo.h>

G_BEGIN_DECLS

#define	MY_TYPE_RECT_AREA_DRAWER	my_rect_area_drawer_get_type()
G_DECLARE_DERIVABLE_TYPE(MyRectAreaDrawer, my_rect_area_drawer, MY, RECT_AREA_DRAWER, MyDrawer)

struct _MyRectAreaDrawerClass {
	MyDrawerClass parent_class;

	void (*update_surface)(MyRectAreaDrawer *drawer, cairo_t *cr, gdouble x, gdouble y, gdouble width, gdouble height);

	gpointer padding[12];
};

G_END_DECLS


