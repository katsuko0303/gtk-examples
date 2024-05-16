#pragma once
#include <gio/gio.h>

G_BEGIN_DECLS

#define	MY_TYPE_FILE_ITEM	my_file_item_get_type()
G_DECLARE_FINAL_TYPE(MyFileItem, my_file_item, MY, FILE_ITEM, GObject)

MyFileItem * my_file_item_new(GFile *file);
void my_file_item_query_info(MyFileItem *item, GCancellable *cancellable);

G_END_DECLS


