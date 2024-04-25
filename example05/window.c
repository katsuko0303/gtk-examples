#include "window.h"
#include "file_item.h"

struct _MyWindow {
	GtkWindow parent_instance;
};
typedef	struct _MyWindowPrivate {
	GListStore *file_model;
	GFile *current_folder;
	GCancellable *cancellable;
	GtkColumnViewColumn *default_column;
	GtkColumnView *file_list;
}	MyWindowPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyWindow, my_window, GTK_TYPE_WINDOW)
#define	PRIVATE(self)	((MyWindowPrivate *)my_window_get_instance_private(MY_WINDOW(self)))
#define	DECLARE_PRIVATE(self)	MyWindowPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_FILE_MODEL,
	PROP_CURRENT_FOLDER,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static void on_file_list_activate(GtkColumnView *column_view, guint position, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	DECLARE_PRIVATE(window);
	MyFileItem *item;
	GFile *child;
	gchar *uri;

	if (position != GTK_INVALID_LIST_POSITION) {
		item = MY_FILE_ITEM(g_list_model_get_item(G_LIST_MODEL(gtk_column_view_get_model(column_view)), position));
		child = g_file_get_child(priv->current_folder, my_file_item_get_name(item));
		if (my_file_item_is_directory(item)) {
			g_object_set(G_OBJECT(window), "current-folder", child, NULL);
		}
		else {
			uri = g_file_get_uri(child);
			g_app_info_launch_default_for_uri(uri, NULL, NULL);
			g_free(uri);
		}
		g_object_unref(G_OBJECT(child));
	}
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

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_object(&priv->file_model);
	g_clear_object(&priv->current_folder);

	(*G_OBJECT_CLASS(my_window_parent_class)->dispose)(obj);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_FILE_MODEL:
		g_value_set_object(value, priv->file_model);
		break;

	case PROP_CURRENT_FOLDER:
		g_value_set_object(value, priv->current_folder);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void on_next_files(GObject *source, GAsyncResult *result, gpointer cb_data)
{
	GFileEnumerator *enumerator = G_FILE_ENUMERATOR(source);
	DECLARE_PRIVATE(cb_data);
	GError *error;
	GList *top, *file_list;
	MyFileItem *item;
	GFileInfo *file_info;

	error = NULL;
	top = g_file_enumerator_next_files_finish(enumerator, result, &error);
	if (top == NULL) {
		if (error != NULL) {
			if (! g_error_matches(error, g_io_error_quark(), G_IO_ERROR_CANCELLED)) {
				g_printerr("%s(%d): failed to get files: %s.\n", __FILE__, __LINE__, error->message);
			}
			g_error_free(error);
		}
		priv->cancellable = NULL;
		g_file_enumerator_close(enumerator, NULL, NULL);
		g_object_unref(G_OBJECT(enumerator));
	}
	else {
		file_list = top;
		while (file_list != NULL) {
			file_info = G_FILE_INFO(file_list->data);
			item = my_file_item_new(file_info);
			g_list_store_append(priv->file_model, item);
			g_object_unref(G_OBJECT(item));
			g_object_unref(G_OBJECT(file_info));

			file_list = g_list_next(file_list);
		}
		g_list_free(top);
		g_file_enumerator_next_files_async(enumerator, 32, G_PRIORITY_DEFAULT, priv->cancellable, on_next_files, cb_data);
	}
}

static void on_enumerate_children(GObject *source, GAsyncResult *result, gpointer cb_data)
{
	DECLARE_PRIVATE(cb_data);
	GError *error;
	GFileEnumerator *enumerator;

	error = NULL;
	enumerator = g_file_enumerate_children_finish(G_FILE(source), result, &error);
	if (enumerator == NULL) {
		if (error == NULL || ! g_error_matches(error, g_io_error_quark(), G_IO_ERROR_CANCELLED)) {
			g_printerr("%s(%d): failed to enumerate children: %s.\n", __FILE__, __LINE__, (error != NULL? error->message: "(no messages)"));
			priv->cancellable = NULL;
		}
	}
	else {
		g_file_enumerator_next_files_async(enumerator, 32, G_PRIORITY_DEFAULT, priv->cancellable, on_next_files, cb_data);
	}
	if (error != NULL) {
		g_error_free(error);
	}
}

static void set_property(GObject *obj, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_CURRENT_FOLDER:
		g_clear_object(&priv->current_folder);
		priv->current_folder = g_value_dup_object(value);
		g_list_store_remove_all(priv->file_model);
		if (priv->cancellable != NULL) {
			g_cancellable_cancel(priv->cancellable);
			priv->cancellable = NULL;
		}
		priv->cancellable = g_cancellable_new();
		g_file_enumerate_children_async(priv->current_folder, MY_FILE_ITEM_FILE_ATTRIBUTES, G_FILE_QUERY_INFO_NONE, G_PRIORITY_DEFAULT,
				priv->cancellable, on_enumerate_children, obj);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void my_window_class_init(MyWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	my_file_item_get_type();

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;
	obj_class->set_property = set_property;

	properties[PROP_FILE_MODEL] = g_param_spec_object("file-model", "", "", G_TYPE_LIST_MODEL, G_PARAM_READABLE);
	properties[PROP_CURRENT_FOLDER] = g_param_spec_object("current-folder", "", "", G_TYPE_FILE, G_PARAM_READWRITE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example05/window.ui");
	gtk_widget_class_bind_template_callback_full(widget_class, "on_file_list_activate", G_CALLBACK(on_file_list_activate));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_go_up", G_CALLBACK(on_go_up));
	gtk_widget_class_bind_template_child_full(widget_class, "default_column", FALSE, G_PRIVATE_OFFSET(MyWindow, default_column));
	gtk_widget_class_bind_template_child_full(widget_class, "file_list", FALSE, G_PRIVATE_OFFSET(MyWindow, file_list));
}

static void my_window_init(MyWindow *window)
{
	DECLARE_PRIVATE(window);

	priv->current_folder = NULL;
	priv->file_model = g_list_store_new(MY_TYPE_FILE_ITEM);

	gtk_widget_init_template(GTK_WIDGET(window));

	{
		GFile *file = g_file_new_for_path(".");
		g_object_set(G_OBJECT(window), "current-folder", file, NULL);
		g_object_unref(G_OBJECT(file));
	}

	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_FILE_MODEL]);

	gtk_column_view_sort_by_column(priv->file_list, priv->default_column, GTK_SORT_ASCENDING);
}

GtkWidget * my_window_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_WINDOW, NULL));
}

