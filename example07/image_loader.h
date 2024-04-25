#pragma once
#include <glib-object.h>
#include <gdk/gdk.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define	MY_TYPE_IMAGE_LOADER	my_image_loader_get_type()
G_DECLARE_FINAL_TYPE(MyImageLoader, my_image_loader, MY, IMAGE_LOADER, GObject)

MyImageLoader * my_image_loader_new(GFile *file);
void my_image_loader_load_async(MyImageLoader *loader, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer cb_data);
void my_image_loader_load_at_scale_async(MyImageLoader *loader, gint width, gint height, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer cb_data);
GdkTexture * my_image_loader_load_finish(MyImageLoader *loader, GAsyncResult *result, GError **error);

G_END_DECLS

