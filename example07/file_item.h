#pragma once
#include <glib-object.h>
#include <gio/gio.h>
#include <gdk/gdk.h>

G_BEGIN_DECLS

#define	MY_FILE_ITEM_FILE_ATTRIBUTES	\
	G_FILE_ATTRIBUTE_STANDARD_NAME "," G_FILE_ATTRIBUTE_STANDARD_ICON "," G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME "," \
	G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE "," G_FILE_ATTRIBUTE_STANDARD_TYPE

#define	MY_TYPE_FILE_ITEM	my_file_item_get_type()
G_DECLARE_FINAL_TYPE(MyFileItem, my_file_item, MY, FILE_ITEM, GObject)

MyFileItem * my_file_item_new(GFile *file, GFileInfo *file_info);
const gchar * my_file_item_get_display_name(MyFileItem *item);
GdkPaintable * my_file_item_get_icon(MyFileItem *item);
void my_file_item_load_icon_texture(MyFileItem *item);
gboolean my_file_item_is_directory(MyFileItem *item);
GFile * my_file_item_get_file(MyFileItem *item);
gboolean my_file_item_is_image_file(MyFileItem *item);

G_END_DECLS


