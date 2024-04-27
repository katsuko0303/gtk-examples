#include <gtk/gtk.h>
#include "window.h"

static void on_startup(GApplication *app, gpointer cb_data)
{
	GtkCssProvider *provider;

	provider = gtk_css_provider_new();
	gtk_css_provider_load_from_resource(provider, "/com/example/example16/app.css");
	gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_FALLBACK);
	g_object_unref(G_OBJECT(provider));
}

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
	g_signal_connect(G_OBJECT(app), "startup", G_CALLBACK(on_startup), NULL);
	g_signal_connect(G_OBJECT(app), "activate", G_CALLBACK(on_activate), NULL);
	return g_application_run(G_APPLICATION(app), argc, argv);
}

