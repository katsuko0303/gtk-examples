#include "image_loader.h"

struct _MyImageLoader {
	GObject parent_instance;
};
typedef	struct _MyImageLoaderPrivate {
	GFile *file;
	gboolean scaling;
	gint width;
	gint height;
}	MyImageLoaderPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyImageLoader, my_image_loader, G_TYPE_OBJECT)
#define	PRIVATE(self)	((MyImageLoaderPrivate *)my_image_loader_get_instance_private(MY_IMAGE_LOADER(self)))
#define	DECLARE_PRIVATE(self)	MyImageLoaderPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_FILE,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_object(&priv->file);

	(*G_OBJECT_CLASS(my_image_loader_parent_class)->dispose)(obj);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_FILE:
		g_value_set_object(value, priv->file);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void set_property(GObject *obj, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_FILE:
		g_clear_object(&priv->file);
		priv->file = g_value_dup_object(value);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void my_image_loader_class_init(MyImageLoaderClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;
	obj_class->set_property = set_property;

	properties[PROP_FILE] = g_param_spec_object("file", "", "", G_TYPE_FILE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);
}

static void my_image_loader_init(MyImageLoader *loader)
{
	DECLARE_PRIVATE(loader);

	priv->file = NULL;
	priv->scaling = FALSE;
}

static void thread_func(GTask *task, GObject *source, gpointer task_data, GCancellable *cancellable)
{
	DECLARE_PRIVATE(source);
	GValue value = { 0 };
	GError *error;
	GFileInputStream *stream;
	GdkPixbuf *pixbuf;
	GdkTexture *texture;

	error = NULL;
	stream = g_file_read(priv->file, cancellable, &error);
	if (error != NULL) {
		g_task_return_error(task, error);
		return ;
	}
	if (priv->scaling) {
		pixbuf = gdk_pixbuf_new_from_stream_at_scale(G_INPUT_STREAM(stream), priv->width, priv->height, TRUE, cancellable, &error);
	}
	else {
		pixbuf = gdk_pixbuf_new_from_stream(G_INPUT_STREAM(stream), cancellable, &error);
	}
	g_object_unref(G_OBJECT(stream));
	if (error != NULL) {
		g_task_return_error(task, error);
		return ;
	}
	texture = gdk_texture_new_for_pixbuf(pixbuf);
	g_value_init(&value, GDK_TYPE_TEXTURE);
	g_value_set_object(&value, texture);
	g_task_return_value(task, &value);
	g_object_unref(G_OBJECT(texture));
	g_value_reset(&value);
}

MyImageLoader * my_image_loader_new(GFile *file)
{
	return MY_IMAGE_LOADER(g_object_new(MY_TYPE_IMAGE_LOADER, "file", file, NULL));
}

void my_image_loader_load_async(MyImageLoader *loader, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer cb_data)
{
	DECLARE_PRIVATE(loader);
	GTask *task;

	priv->scaling = FALSE;
	task = g_task_new(G_OBJECT(loader), cancellable, callback, cb_data);
	g_task_run_in_thread(task, (GTaskThreadFunc)thread_func);
}

void my_image_loader_load_at_scale_async(MyImageLoader *loader, gint width, gint height, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer cb_data)
{
	DECLARE_PRIVATE(loader);
	GTask *task;

	priv->scaling = TRUE;
	priv->width = width;
	priv->height = height;
	task = g_task_new(G_OBJECT(loader), cancellable, callback, cb_data);
	g_task_run_in_thread(task, (GTaskThreadFunc)thread_func);
}

GdkTexture * my_image_loader_load_finish(MyImageLoader *loader, GAsyncResult *result, GError **error)
{
	GValue value = { 0 };
	GdkTexture *texture;
	gboolean ret;

	ret = g_task_propagate_value(G_TASK(result), &value, error);
	g_object_unref(G_OBJECT(result));
	if (! ret) {
		return NULL;
	}
	texture = GDK_TEXTURE(g_value_dup_object(&value));
	g_value_reset(&value);
	return texture;
}

