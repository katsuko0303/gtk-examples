#pragma once
#include <gdk/gdk.h>

G_BEGIN_DECLS

#define	MY_TYPE_TIME_PAINTABLE	my_time_paintable_get_type()
G_DECLARE_FINAL_TYPE(MyTimePaintable, my_time_paintable, MY, TIME_PAINTABLE, GObject)

MyTimePaintable * my_time_paintable_new();
gboolean my_time_paintable_is_running(MyTimePaintable *paintable);
void my_time_paintable_start(MyTimePaintable *paintable);
void my_time_paintable_stop(MyTimePaintable *paintable);
void my_time_paintable_reset(MyTimePaintable *paintable);

G_END_DECLS


