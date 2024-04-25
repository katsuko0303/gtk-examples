#pragma once
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define	MY_TYPE_WINDOW	my_window_get_type()
G_DECLARE_FINAL_TYPE(MyWindow, my_window, MY, WINDOW, GtkApplicationWindow)

GtkWidget * my_window_new();

G_END_DECLS


