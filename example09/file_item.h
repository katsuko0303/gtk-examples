#pragma once
#include <gio/gio.h>

G_BEGIN_DECLS

#define	MY_TYPE_FILE_ITEM	my_file_item_get_type()
G_DECLARE_FINAL_TYPE(MyFileItem, my_file_item, MY, FILE_ITEM, GObject)

GListModel * my_file_item_create_root_model(GFile *root_dir);
GListModel * my_file_item_create_child_model(MyFileItem *item);

G_END_DECLS


