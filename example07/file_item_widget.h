#pragma once
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define	MY_TYPE_FILE_ITEM_WIDGET	my_file_item_widget_get_type()
G_DECLARE_FINAL_TYPE(MyFileItemWidget, my_file_item_widget, MY, FILE_ITEM_WIDGET, GtkBox)

GtkWidget * my_file_item_widget_new();

G_END_DECLS


