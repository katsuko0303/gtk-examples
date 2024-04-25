#include "window.h"
#include "file_item.h"
#include "file_item_widget.h"
#include "image_loader.h"

struct _MyWindow {
	GtkWindow parent_instance;
};
typedef	struct _MyWindowPrivate {
	GFile *current_folder;
	GListStore *file_model;
	GTask *task;
	gboolean image_active;
	GdkPaintable *image_paintable;
}	MyWindowPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyWindow, my_window, GTK_TYPE_WINDOW)
#define	PRIVATE(self)	((MyWindowPrivate *)my_window_get_instance_private(MY_WINDOW(self)))
#define	DECLARE_PRIVATE(self)	MyWindowPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_CURRENT_FOLDER,
	PROP_FILE_MODEL,
	PROP_IMAGE_ACTIVE,
	PROP_IMAGE_PAINTABLE,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static void on_close_image(GtkButton *button, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	DECLARE_PRIVATE(window);

	priv->image_active = FALSE;
	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_IMAGE_ACTIVE]);
}

static void on_go_up(GtkButton *button, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	DECLARE_PRIVATE(window);
	GFile *parent;

	parent = g_file_get_parent(priv->current_folder);
	if (parent != NULL) {
		g_object_set(G_OBJECT(window), "current-folder", parent, NULL);
		g_object_unref(G_OBJECT(parent));
	}
}

static void on_load_image(GObject *source, GAsyncResult *result, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	DECLARE_PRIVATE(window);
	GError *error = NULL;
	GdkTexture *texture;

	texture = my_image_loader_load_finish(MY_IMAGE_LOADER(source), result, &error);
	g_object_unref(G_OBJECT(source));
	if (error != NULL) {
		g_printerr("%s(%d): failed to load image: %s.\n", __FILE__, __LINE__, error->message);
		g_error_free(error);
	}
	else {
		priv->image_paintable = GDK_PAINTABLE(texture);
		g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_IMAGE_PAINTABLE]);
	}
}

static void on_file_list_activate(GtkGridView *grid_view, guint position, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	DECLARE_PRIVATE(window);
	MyFileItem *item;
	MyImageLoader *loader;

	if (position != GTK_INVALID_LIST_POSITION) {
		item = MY_FILE_ITEM(g_list_model_get_item(G_LIST_MODEL(gtk_grid_view_get_model(grid_view)), position));
		if (my_file_item_is_directory(item)) {
			g_object_set(G_OBJECT(window), "current-folder", my_file_item_get_file(item), NULL);
		}
		else if (my_file_item_is_image_file(item)) {
			priv->image_active = TRUE;
			g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_IMAGE_ACTIVE]);

			loader = my_image_loader_new(my_file_item_get_file(item));
			my_image_loader_load_async(loader, NULL, on_load_image, cb_data);
		}
		g_object_unref(G_OBJECT(item));
	}
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_object(&priv->current_folder);
	g_clear_object(&priv->file_model);
	g_clear_object(&priv->task);
	g_clear_object(&priv->image_paintable);

	(*G_OBJECT_CLASS(my_window_parent_class)->dispose)(obj);
}

static void load_folder_thread_func(GTask *task, GObject *source, gpointer task_data, GCancellable *cancellable)
{
	GError *error;
	GFileEnumerator *enumerator;
	GListStore *list_store;
	GFileInfo *file_info;
	GFile *child;
	MyFileItem *item;
	GValue value = { 0 };

	error = NULL;
	enumerator = g_file_enumerate_children(G_FILE(task_data), MY_FILE_ITEM_FILE_ATTRIBUTES, G_FILE_QUERY_INFO_NONE, cancellable, &error);
	if (error != NULL) {
		g_task_return_error(task, error);
		g_error_free(error);
		return ;
	}
	list_store = g_list_store_new(MY_TYPE_FILE_ITEM);
	while (TRUE) {
		error = NULL;
		file_info = g_file_enumerator_next_file(enumerator, cancellable, &error);
		if (error != NULL) {
			g_file_enumerator_close(enumerator, NULL, NULL);
			g_object_unref(G_OBJECT(enumerator));
			g_task_return_error(task, error);
			g_error_free(error);
			g_object_unref(G_OBJECT(list_store));
			return ;
		}
		if (file_info == NULL) {
			break;
		}
		child = g_file_enumerator_get_child(enumerator, file_info);
		item = my_file_item_new(child, file_info);
		g_list_store_append(list_store, item);
		g_object_unref(G_OBJECT(item));
		g_object_unref(G_OBJECT(child));
		g_object_unref(G_OBJECT(file_info));
	}
	g_file_enumerator_close(enumerator, NULL, NULL);
	g_value_init(&value, G_TYPE_LIST_STORE);
	g_value_set_object(&value, list_store);
	g_task_return_value(task, &value);
	g_value_reset(&value);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_CURRENT_FOLDER:
		g_value_set_object(value, priv->current_folder);
		break;

	case PROP_FILE_MODEL:
		g_value_set_object(value, priv->file_model);
		break;

	case PROP_IMAGE_ACTIVE:
		g_value_set_boolean(value, priv->image_active);
		break;

	case PROP_IMAGE_PAINTABLE:
		g_value_set_object(value, priv->image_paintable);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void on_loaded_folder(GObject *source, GAsyncResult *result)
{
	DECLARE_PRIVATE(source);
	GError *error;
	GValue value = { 0 };
	gboolean ret;

	error = NULL;
	ret = g_task_propagate_value(G_TASK(result), &value, &error);
	if (! ret) {
		if (error == NULL || ! g_error_matches(error, g_io_error_quark(), G_IO_ERROR_CANCELLED)) {
			g_printerr("%s(%d): failed to load folder: %s.\n", __FILE__, __LINE__, (error != NULL? error->message: "(no messages)"));
			g_clear_object(&priv->task);
		}
	}
	else {
		priv->file_model = G_LIST_STORE(g_value_dup_object(&value));
		g_object_notify_by_pspec(source, properties[PROP_FILE_MODEL]);
		g_value_reset(&value);
		g_clear_object(&priv->task);
	}
}

static void set_property(GObject *obj, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_CURRENT_FOLDER:
		g_clear_object(&priv->current_folder);
		priv->current_folder = g_value_dup_object(value);

		if (priv->task != NULL) {
			g_cancellable_cancel(g_task_get_cancellable(priv->task));
			g_clear_object(&priv->task);
		}
		priv->task = g_task_new(obj, g_cancellable_new(), (GAsyncReadyCallback)on_loaded_folder, NULL);
		g_task_set_task_data(priv->task, g_object_ref(G_OBJECT(priv->current_folder)), g_object_unref);
		g_task_run_in_thread(priv->task, (GTaskThreadFunc)load_folder_thread_func);
		break;


	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void my_window_class_init(MyWindowClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	my_file_item_get_type();
	my_file_item_widget_get_type();

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;
	obj_class->set_property = set_property;

	properties[PROP_CURRENT_FOLDER] = g_param_spec_object("current-folder", "", "", G_TYPE_FILE, G_PARAM_READWRITE);
	properties[PROP_FILE_MODEL] = g_param_spec_object("file-model", "", "", G_TYPE_LIST_MODEL, G_PARAM_READABLE);
	properties[PROP_IMAGE_ACTIVE] = g_param_spec_boolean("image-active", "", "", FALSE, G_PARAM_READABLE);
	properties[PROP_IMAGE_PAINTABLE] = g_param_spec_object("image-paintable", "", "", GDK_TYPE_PAINTABLE, G_PARAM_READABLE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example07/window.ui");
	gtk_widget_class_bind_template_callback_full(widget_class, "on_go_up", G_CALLBACK(on_go_up));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_file_list_activate", G_CALLBACK(on_file_list_activate));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_close_image", G_CALLBACK(on_close_image));
}

static void my_window_init(MyWindow *window)
{
	DECLARE_PRIVATE(window);
	GFile *folder;

	priv->file_model = NULL;
	priv->current_folder = NULL;
	priv->task = NULL;
	priv->image_active = FALSE;
	priv->image_paintable = NULL;

	gtk_widget_init_template(GTK_WIDGET(window));

	folder = g_file_new_for_path(".");
	g_object_set(G_OBJECT(window), "current-folder", folder, NULL);
	g_object_unref(G_OBJECT(folder));
}

GtkWidget * my_window_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_WINDOW, NULL));
}

