#include "window.h"

struct _MyWindow {
	GtkWindow parent_instance;
};
typedef	struct _MyWindowPrivate {
	GtkTextBuffer *buffer;
}	MyWindowPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyWindow, my_window, GTK_TYPE_WINDOW)
#define	PRIVATE(self)	((MyWindowPrivate *)my_window_get_instance_private(MY_WINDOW(self)))
#define	DECLARE_PRIVATE(self)	MyWindowPrivate *priv = PRIVATE(self)

static void on_load_contents(GObject *source, GAsyncResult *result, gpointer cb_data)
{
	DECLARE_PRIVATE(cb_data);
	GError *error;
	gchar *contents;
	gsize size;
	gboolean ret;

	error = NULL;
	ret = g_file_load_contents_finish(G_FILE(source), result, &contents, &size, NULL, &error);
	if (! ret) {
		g_printerr("%s(%d): error: %s.\n", __FILE__, __LINE__, (error != NULL? error->message: "(no messages)"));
	}
	else {
		gtk_text_buffer_set_text(priv->buffer, contents, size);
		g_free(contents);
	}
	if (error != NULL) {
		g_error_free(error);
	}
}

static void on_select_file(GObject *source, GAsyncResult *result, gpointer cb_data)
{
	GError *error;
	GFile *file;

	error = NULL;
	file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(source), result, &error);
	if (file == NULL) {
		g_printerr("%s(%d): error: %s.\n", __FILE__, __LINE__, (error != NULL? error->message: "(no messages)"));
	}
	else {
		g_file_load_contents_async(file, NULL, on_load_contents, cb_data);
		g_object_unref(G_OBJECT(file));
	}
	if (error != NULL)	g_error_free(error);
}

static void on_open(GtkButton *button, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	GtkFileDialog *dialog;

	dialog = gtk_file_dialog_new();
	gtk_file_dialog_set_modal(dialog, TRUE);
	gtk_file_dialog_open(dialog, GTK_WINDOW(window), NULL, on_select_file, cb_data);
	g_object_unref(G_OBJECT(dialog));
}

static void my_window_class_init(MyWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example10/window.ui");
	gtk_widget_class_bind_template_child_full(widget_class, "buffer", FALSE, G_PRIVATE_OFFSET(MyWindow, buffer));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_open", G_CALLBACK(on_open));
}

static void my_window_init(MyWindow *window)
{
	gtk_widget_init_template(GTK_WIDGET(window));
}

GtkWidget * my_window_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_WINDOW, NULL));
}

