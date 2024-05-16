#include <gtk/gtk.h>
#include "window.h"
#include "no_ui_window.h"

static gint on_command_line(GApplication *app, GApplicationCommandLine *command_line, gpointer cb_data)
{
	gint argc;
	gchar **argv;
	GtkWidget *window;
	gboolean use_ui = TRUE;
	gint i;

	argv = g_application_command_line_get_arguments(command_line, &argc);
	for (i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "noui") == 0) {
			use_ui = FALSE;
			break;
		}
	}
	if (use_ui) {
		window = my_window_new();
	}
	else {
		window = my_no_ui_window_new();
	}
	gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(app));
	gtk_widget_set_visible(window, TRUE);

	g_strfreev(argv);
	return 0;
}

int main(int argc, char *argv[])
{
	GtkApplication *app;

	app = gtk_application_new(NULL, G_APPLICATION_HANDLES_COMMAND_LINE);
	g_signal_connect(G_OBJECT(app), "command-line", G_CALLBACK(on_command_line), NULL);
	return g_application_run(G_APPLICATION(app), argc, argv);
}

