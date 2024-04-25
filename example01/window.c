#include "window.h"

struct _MyWindow {
	GtkApplicationWindow parent_instance;
};
typedef	struct _MyWindowPrivate {
	int page_count;
	GtkNotebook *notebook;
}	MyWindowPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyWindow, my_window, GTK_TYPE_APPLICATION_WINDOW)
#define	PRIVATE(self)	my_window_get_instance_private(MY_WINDOW(self))
#define	DECLARE_PRIVATE(self)	MyWindowPrivate *priv = PRIVATE(self)

enum {
	SIGNAL_NEW_TAB,
	NUM_SIGNALS
};
static guint signal_ids[NUM_SIGNALS];

static void new_tab_real(MyWindow *window)
{
	DECLARE_PRIVATE(window);
	GtkBuilder *builder;
	GtkWidget *child;
	GtkWidget *tab_label;
	GDateTime *datetime;
	gchar *tab_label_text;
	gint page_index;
	gchar *label_text;

	builder = gtk_builder_new();
	gtk_builder_add_from_resource(builder, "/com/example/example01/page.ui", NULL);

	child = GTK_WIDGET(gtk_builder_get_object(builder, "page"));
	datetime = g_date_time_new_now_local();
	label_text = g_date_time_format(datetime, "created at %c");
	gtk_label_set_label(GTK_LABEL(child), label_text);

	tab_label_text = g_strdup_printf("Page %d", 1 + priv->page_count);
	tab_label = gtk_label_new(tab_label_text);

	page_index = gtk_notebook_append_page(priv->notebook, child, tab_label);
	gtk_notebook_set_current_page(priv->notebook, page_index);

	g_free(tab_label_text);
	g_free(label_text);
	g_date_time_unref(datetime);
	g_object_unref(G_OBJECT(builder));
	++priv->page_count;
}

static void on_new_window_clicked(GtkButton *button, gpointer cb_data)
{
	g_action_group_activate_action(G_ACTION_GROUP(gtk_window_get_application(GTK_WINDOW(cb_data))), "new_window", NULL);
}

static void on_new_tab_clicked(GtkButton *button, gpointer cb_data)
{
	g_signal_emit(G_OBJECT(cb_data), signal_ids[SIGNAL_NEW_TAB], 0);
}

static void on_new_tab(GSimpleAction *action, GVariant *parameter, gpointer cb_data)
{
	g_signal_emit(G_OBJECT(cb_data), signal_ids[SIGNAL_NEW_TAB], 0);
}

static void on_close(GSimpleAction *action, GVariant *parameter, gpointer cb_data)
{
	gtk_window_close(GTK_WINDOW(cb_data));
}

static void my_window_class_init(MyWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GClosure *closure;

	closure = g_cclosure_new(G_CALLBACK(new_tab_real), NULL, NULL);
	signal_ids[SIGNAL_NEW_TAB] = g_signal_newv("new-tab", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST,
			closure, NULL, NULL, NULL, G_TYPE_NONE, 0, NULL);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example01/window.ui");
	gtk_widget_class_bind_template_callback_full(widget_class, "on_new_window_clicked", G_CALLBACK(on_new_window_clicked));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_new_tab_clicked", G_CALLBACK(on_new_tab_clicked));
	gtk_widget_class_bind_template_child_full(widget_class, "notebook", FALSE, G_PRIVATE_OFFSET(MyWindow, notebook));
}

static void my_window_init(MyWindow *window)
{
	DECLARE_PRIVATE(window);
	const GActionEntry entries[] = {
		{ "new_tab", on_new_tab, NULL, NULL, NULL },
		{ "close", on_close, NULL, NULL, NULL },
	};

	priv->page_count = 0;

	gtk_widget_init_template(GTK_WIDGET(window));

	g_action_map_add_action_entries(G_ACTION_MAP(window), entries, G_N_ELEMENTS(entries), window);

	g_signal_emit(G_OBJECT(window), signal_ids[SIGNAL_NEW_TAB], 0);
}

GtkWidget * my_window_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_WINDOW, NULL));
}

