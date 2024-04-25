#include "window.h"
#include "name_dialog.h"

struct _MyWindow {
	GtkWindow parent_instance;
};
G_DEFINE_TYPE(MyWindow, my_window, GTK_TYPE_WINDOW)

static void on_ask_name(GObject *source, GAsyncResult *result, gpointer cb_data)
{
	GError *error;
	gchar *name;
	gchar *label;

	error = NULL;
	name = my_name_dialog_ask_finish(MY_NAME_DIALOG(source), result, &error);
	if (name == NULL) {
		g_printerr("%s(%d): %s.\n", __FILE__, __LINE__, (error != NULL? error->message: "error (no messages)"));
	}
	else {
		label = g_strdup_printf("My name is %s.", name);
		gtk_button_set_label(GTK_BUTTON(cb_data), label);
		g_free(label);
		g_free(name);
	}
	if (error != NULL) {
		g_error_free(error);
	}
}

static void on_clicked(GtkButton *button, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	GtkWidget *dialog;

	dialog = my_name_dialog_new();
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	my_name_dialog_ask(MY_NAME_DIALOG(dialog), GTK_WINDOW(window), NULL, on_ask_name, button);
}

static void my_window_class_init(MyWindowClass *klass)
{
}

static void my_window_init(MyWindow *window)
{
	GtkWidget *button;

	button = gtk_button_new_with_label("Push");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(on_clicked), window);
	gtk_window_set_child(GTK_WINDOW(window), button);
}

GtkWidget * my_window_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_WINDOW, "default-width", 400, "default-height", 300, NULL));
}

