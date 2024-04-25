#include "application.h"
#include "window.h"

struct _MyApplication {
	GtkApplication parent_instance;
};
G_DEFINE_TYPE(MyApplication, my_application, GTK_TYPE_APPLICATION)

enum {
	SIGNAL_NEW_WINDOW,
	NUM_SIGNALS
};
static guint signal_ids[NUM_SIGNALS];

static void new_window_real(MyApplication *app)
{
	GtkWidget *window;

	window = my_window_new();
	gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(app));
	gtk_widget_set_visible(window, TRUE);
}

static void on_quit(GSimpleAction *action, GVariant *parameter, gpointer cb_data)
{
	g_application_quit(G_APPLICATION(cb_data));
}

static void on_new_window(GSimpleAction *action, GVariant *parameter, gpointer cb_data)
{
	g_signal_emit(G_OBJECT(cb_data), signal_ids[SIGNAL_NEW_WINDOW], 0);
}

static void activate(GApplication *app)
{
	g_signal_emit(G_OBJECT(app), signal_ids[SIGNAL_NEW_WINDOW], 0);
}

static void my_application_class_init(MyApplicationClass *klass)
{
	GApplicationClass *app_class = G_APPLICATION_CLASS(klass);
	GClosure *closure;

	closure = g_cclosure_new(G_CALLBACK(new_window_real), NULL, NULL);
	signal_ids[SIGNAL_NEW_WINDOW] = g_signal_newv("new-window", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST, closure, NULL, NULL, NULL,
			G_TYPE_NONE, 0, NULL);

	app_class->activate = activate;
}

static void my_application_init(MyApplication *app)
{
	const GActionEntry entries[] = {
		{ "quit", on_quit, NULL, NULL, NULL },
		{ "new_window", on_new_window, NULL, NULL, NULL },
	};

	g_action_map_add_action_entries(G_ACTION_MAP(app), entries, G_N_ELEMENTS(entries), app);
}

MyApplication * my_application_new()
{
	return MY_APPLICATION(g_object_new(MY_TYPE_APPLICATION,
				"application-id", "com.example.example01",
				NULL));
}

