#include <gtk/gtk.h>
#include "window.h"

static gint on_command_line(GApplication *app, GApplicationCommandLine *command_line, gpointer cb_data)
{
	gchar **argv;
	gint argc;
	GFile *top_folder;
	const gchar *home_dir;
	GtkWidget *window;

	argv = g_application_command_line_get_arguments(command_line, &argc);
	if (argc >= 2) {
		top_folder = g_file_new_for_path(argv[1]);
	}
	else {
		home_dir = g_get_home_dir();
		top_folder = g_file_new_for_path(home_dir);
	}
	window = my_window_new(top_folder);
	gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(app));
	gtk_widget_set_visible(window, TRUE);

	g_object_unref(top_folder);
	return 0;
}

int main(int argc, char *argv[])
{
	GtkApplication *app;

	app = gtk_application_new(NULL, G_APPLICATION_HANDLES_COMMAND_LINE);
	g_signal_connect(G_OBJECT(app), "command-line", G_CALLBACK(on_command_line), NULL);
	return g_application_run(G_APPLICATION(app), argc, argv);
}

