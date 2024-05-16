#include "window.h"
#include "file_item.h"

struct _MyWindow {
	GtkWindow parent_instance;
};
typedef	struct _MyWindowPrivate {
	GValue data;
	GCancellable *cancellable;
}	MyWindowPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyWindow, my_window, GTK_TYPE_WINDOW)
#define	PRIVATE(self)	((MyWindowPrivate *)my_window_get_instance_private(MY_WINDOW(self)))
#define	DECLARE_PRIVATE(self)	MyWindowPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_DATA_IS_NONE,
	PROP_DATA_IS_TEXT,
	PROP_TEXT_DATA,
	PROP_DATA_IS_IMAGE,
	PROP_IMAGE_DATA,
	PROP_DATA_IS_FILES,
	PROP_FILES_DATA,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static void notify_data_update(MyWindow *window)
{
	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_DATA_IS_NONE]);
	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_DATA_IS_TEXT]);
	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_TEXT_DATA]);
	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_DATA_IS_IMAGE]);
	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_IMAGE_DATA]);
	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_DATA_IS_FILES]);
	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_FILES_DATA]);
}

static void on_read_text(GObject *source, GAsyncResult *result, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	DECLARE_PRIVATE(window);
	GError *error;
	gchar *text;

	error = NULL;
	text = gdk_clipboard_read_text_finish(GDK_CLIPBOARD(source), result, &error);
	if (error != NULL) {
		if (g_error_matches(error, g_io_error_quark(), G_IO_ERROR_CANCELLED)) {
			g_error_free(error);
			return ;
		}
		g_printerr("%s(%d): failed to read text from clipboard. (%s)", __FILE__, __LINE__, error->message);
		g_error_free(error);
	}
	else {
		g_value_unset(&priv->data);
		g_value_init(&priv->data, G_TYPE_STRING);
		g_value_set_string(&priv->data, text);
		notify_data_update(window);
	}
	if (text != NULL) {
		g_free(text);
	}
	g_clear_object(&priv->cancellable);
}

static void on_read_texture(GObject *source, GAsyncResult *result, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	DECLARE_PRIVATE(window);
	GError *error;
	GdkTexture *texture;

	error = NULL;
	texture = gdk_clipboard_read_texture_finish(GDK_CLIPBOARD(source), result, &error);
	if (error != NULL) {
		if (g_error_matches(error, g_io_error_quark(), G_IO_ERROR_CANCELLED)) {
			g_error_free(error);
			return ;
		}
		g_printerr("%s(%d): failed to read texture from clipboard. (%s)", __FILE__, __LINE__, error->message);
		g_error_free(error);
	}
	else {
		g_value_unset(&priv->data);
		g_value_init(&priv->data, GDK_TYPE_TEXTURE);
		g_value_set_object(&priv->data, texture);
		notify_data_update(window);
	}
	g_clear_object(&texture);
	g_clear_object(&priv->cancellable);
}

static void on_paste(GtkButton *button, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	DECLARE_PRIVATE(window);
	GdkClipboard *clipboard;
	GdkContentFormats *formats;

	if (priv->cancellable != NULL) {
		g_cancellable_cancel(priv->cancellable);
		g_clear_object(&priv->cancellable);
	}
	g_value_unset(&priv->data);
	notify_data_update(window);

	clipboard = gdk_display_get_primary_clipboard(gdk_display_get_default());
	formats = gdk_clipboard_get_formats(clipboard);
	if (gdk_content_formats_contain_gtype(formats, G_TYPE_STRING)) {
		priv->cancellable = g_cancellable_new();
		gdk_clipboard_read_text_async(clipboard, priv->cancellable, on_read_text, window);
	}
	else if (gdk_content_formats_contain_gtype(formats, GDK_TYPE_TEXTURE)) {
		priv->cancellable = g_cancellable_new();
		gdk_clipboard_read_texture_async(clipboard, priv->cancellable, on_read_texture, window);
	}
}

static gboolean on_drop(GtkDropTarget *drop_target, GValue *value, gdouble x, gdouble y, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	DECLARE_PRIVATE(window);
	GdkFileList *file_list;
	GListStore *list_store;
	GSList *top;
	GSList *node;
	MyFileItem *item;

	if (priv->cancellable != NULL) {
		g_cancellable_cancel(priv->cancellable);
		g_clear_object(&priv->cancellable);
	}
	g_value_unset(&priv->data);
	notify_data_update(window);

	if (value->g_type == G_TYPE_STRING || value->g_type == GDK_TYPE_TEXTURE) {
		g_value_init(&priv->data, value->g_type);
		g_value_copy(value, &priv->data);
		notify_data_update(window);
		return TRUE;
	}
	else if (value->g_type == GDK_TYPE_FILE_LIST) {
		file_list = (GdkFileList *)g_value_get_boxed(value);
		list_store = g_list_store_new(MY_TYPE_FILE_ITEM);
		priv->cancellable = g_cancellable_new();
		top = gdk_file_list_get_files(file_list);
		node = top;
		while (node != NULL) {
			item = my_file_item_new(G_FILE(node->data));
			my_file_item_query_info(item, priv->cancellable);
			g_list_store_append(list_store, item);

			g_object_unref(G_OBJECT(node->data));
			node = g_slist_next(node);
		}
		g_slist_free(top);
		g_value_init(&priv->data, G_TYPE_LIST_STORE);
		g_value_take_object(&priv->data, list_store);
		notify_data_update(window);
		return TRUE;
	}
	return FALSE;
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_value_unset(&priv->data);
	g_clear_object(&priv->cancellable);

	(*G_OBJECT_CLASS(my_window_parent_class)->dispose)(obj);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_DATA_IS_NONE:
		g_value_set_boolean(value, priv->data.g_type == G_TYPE_INVALID);
		break;

	case PROP_DATA_IS_TEXT:
		g_value_set_boolean(value, priv->data.g_type == G_TYPE_STRING);
		break;

	case PROP_TEXT_DATA:
		g_value_set_string(value, (priv->data.g_type == G_TYPE_STRING? g_value_get_string(&priv->data): ""));
		break;

	case PROP_DATA_IS_IMAGE:
		g_value_set_boolean(value, priv->data.g_type == GDK_TYPE_TEXTURE);
		break;

	case PROP_IMAGE_DATA:
		g_value_set_object(value, (priv->data.g_type == GDK_TYPE_TEXTURE? g_value_get_object(&priv->data): NULL));
		break;

	case PROP_DATA_IS_FILES:
		g_value_set_boolean(value, priv->data.g_type == G_TYPE_LIST_STORE);
		break;

	case PROP_FILES_DATA:
		g_value_set_object(value, (priv->data.g_type == G_TYPE_LIST_STORE? g_value_get_object(&priv->data): NULL));
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

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;

	properties[PROP_DATA_IS_NONE] = g_param_spec_boolean("data-is-none", "", "", TRUE, G_PARAM_READABLE);
	properties[PROP_DATA_IS_TEXT] = g_param_spec_boolean("data-is-text", "", "", FALSE, G_PARAM_READABLE);
	properties[PROP_TEXT_DATA] = g_param_spec_string("text-data", "", "", "", G_PARAM_READABLE);
	properties[PROP_DATA_IS_IMAGE] = g_param_spec_boolean("data-is-image", "", "", FALSE, G_PARAM_READABLE);
	properties[PROP_IMAGE_DATA] = g_param_spec_object("image-data", "", "", GDK_TYPE_PAINTABLE, G_PARAM_READABLE);
	properties[PROP_DATA_IS_FILES] = g_param_spec_boolean("data-is-files", "", "", FALSE, G_PARAM_READABLE);
	properties[PROP_FILES_DATA] = g_param_spec_object("files-data", "", "", G_TYPE_LIST_MODEL, G_PARAM_READABLE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example18/window.ui");
	gtk_widget_class_bind_template_callback_full(widget_class, "on_paste", G_CALLBACK(on_paste));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_drop", G_CALLBACK(on_drop));
}

static void my_window_init(MyWindow *window)
{
	DECLARE_PRIVATE(window);

	memset(&priv->data, 0, sizeof(priv->data));
	priv->cancellable = NULL;

	gtk_widget_init_template(GTK_WIDGET(window));
}

GtkWidget * my_window_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_WINDOW, NULL));
}

