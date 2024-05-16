#pragma once
#include <glib-object.h>

G_BEGIN_DECLS

#define	MY_TYPE_LIST_ITEM	my_list_item_get_type()
G_DECLARE_FINAL_TYPE(MyListItem, my_list_item, MY, LIST_ITEM, GObject)

MyListItem * my_list_item_new(gint value);
gint my_list_item_get_value(MyListItem *item);
gchar * my_list_item_get_label(MyListItem *item);

G_END_DECLS


