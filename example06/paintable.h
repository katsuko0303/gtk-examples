#pragma once
#include <gdk/gdk.h>

G_BEGIN_DECLS

#define	MY_TYPE_PAINTABLE	my_paintable_get_type()
G_DECLARE_FINAL_TYPE(MyPaintable, my_paintable, MY, PAINTABLE, GObject)

MyPaintable * my_paintable_new();
cairo_t * my_paintable_make_cairo_context(MyPaintable *paintable);
gboolean my_paintable_convert_to_surface_coordinate(MyPaintable *paintable, gdouble x, gdouble y, gdouble width, gdouble height, gdouble *sx, gdouble *sy);

G_END_DECLS


