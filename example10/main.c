#include <gtk/gtk.h>
#include "window.h"

static void on_activate(GApplication *app, gpointer cb_data)
{
	GtkWidget *window;

	window = my_window_new();
	gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(app));
	gtk_widget_set_visible(window, TRUE);
}

int main(int argc, char *argv[])
{
	GtkApplication *app;

	app = gtk_application_new(NULL, 0);
	g_signal_connect(G_OBJECT(app), "activate", G_CALLBACK(on_activate), NULL);
	return g_application_run(G_APPLICATION(app), argc, argv);
}

