#pragma once
#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define	MY_FILE_ITEM_FILE_ATTRIBUTES	\
	G_FILE_ATTRIBUTE_STANDARD_NAME "," G_FILE_ATTRIBUTE_STANDARD_ICON "," G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME "," \
	G_FILE_ATTRIBUTE_STANDARD_TYPE "," G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE "," G_FILE_ATTRIBUTE_TIME_MODIFIED "," \
	G_FILE_ATTRIBUTE_STANDARD_SIZE

#define	MY_TYPE_FILE_ITEM	my_file_item_get_type()
G_DECLARE_FINAL_TYPE(MyFileItem, my_file_item, MY, FILE_ITEM, GObject)

MyFileItem * my_file_item_new(GFileInfo *file_info);
gboolean my_file_item_is_directory(MyFileItem *item);
const gchar * my_file_item_get_name(MyFileItem *item);

G_END_DECLS


