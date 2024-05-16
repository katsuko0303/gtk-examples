#pragma once
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define	MY_TYPE_NO_UI_WINDOW	my_no_ui_window_get_type()
G_DECLARE_FINAL_TYPE(MyNoUIWindow, my_no_ui_window, MY, NO_UI_WINDOW, GtkWindow)

GtkWidget * my_no_ui_window_new();

G_END_DECLS


