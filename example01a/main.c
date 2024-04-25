#include <gtk/gtk.h>

typedef	struct _WindowData {
	GtkWidget *notebook;
	gint page_count;
}	WindowData;

static void create_tab(GtkWidget *window);
static void create_window(GtkApplication *app);

static void on_new_tab(GSimpleAction *action, GVariant *parameter, gpointer cb_data)
{
	create_tab(GTK_WIDGET(cb_data));
}

static void on_close(GSimpleAction *action, GVariant *parameter, gpointer cb_data)
{
	gtk_window_close(GTK_WINDOW(cb_data));
}

static void create_tab(GtkWidget *window)
{
	WindowData *window_data = (WindowData *)g_object_get_data(G_OBJECT(window), "data");
	GDateTime *datetime;
	GtkWidget *child;
	GtkWidget *tab_label;
	gint page_index;
	gchar *label_text;
	gchar *tab_label_text;

	datetime = g_date_time_new_now_local();
	label_text = g_date_time_format(datetime, "created at %c.");
	child = gtk_label_new(label_text);

	tab_label_text = g_strdup_printf("Page %d", 1 + window_data->page_count);
	tab_label = gtk_label_new(tab_label_text);

	page_index = gtk_notebook_append_page(GTK_NOTEBOOK(window_data->notebook), child, tab_label);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(window_data->notebook), page_index);

	g_date_time_unref(datetime);
	g_free(label_text);
	g_free(tab_label_text);

	++window_data->page_count;
}

static void on_new_window_clicked(GtkButton *button, gpointer cb_data)
{
	create_window(GTK_APPLICATION(gtk_window_get_application(GTK_WINDOW(cb_data))));
}

static void on_new_tab_clicked(GtkButton *button, gpointer cb_data)
{
	create_tab(GTK_WIDGET(cb_data));
}

static void create_window(GtkApplication *app)
{
	GtkWidget *window;
	WindowData *window_data;
	GtkWidget *titlebar;
	GtkWidget *button;
	GMenu *menu;
	GMenu *section_menu;
	GMenuItem *menu_item;
	const GActionEntry entries[] = {
		{ "new_tab", on_new_tab, NULL, NULL, NULL },
		{ "close", on_close, NULL, NULL, NULL },
	};

	window = gtk_application_window_new(app);
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
	window_data = g_new(WindowData, 1);
	window_data->page_count = 0;
	g_object_set_data_full(G_OBJECT(window), "data", window_data, g_free);

	window_data->notebook = gtk_notebook_new();
	gtk_window_set_child(GTK_WINDOW(window), window_data->notebook);

	titlebar = gtk_header_bar_new();
	gtk_window_set_titlebar(GTK_WINDOW(window), titlebar);

	button = gtk_button_new_from_icon_name("window-new-symbolic");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(on_new_window_clicked), window);
	gtk_header_bar_pack_start(GTK_HEADER_BAR(titlebar), button);

	button = gtk_button_new_from_icon_name("tab-new-symbolic");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(on_new_tab_clicked), window);
	gtk_header_bar_pack_start(GTK_HEADER_BAR(titlebar), button);

	menu = g_menu_new();

	section_menu = g_menu_new();
	menu_item = g_menu_item_new("New Window", "app.new_window");
	g_menu_append_item(section_menu, menu_item);
	menu_item = g_menu_item_new("New Tab", "win.new_tab");
	g_menu_append_item(section_menu, menu_item);
	g_menu_append_section(menu, NULL, G_MENU_MODEL(section_menu));

	section_menu = g_menu_new();
	menu_item = g_menu_item_new("Close This Window", "win.close");
	g_menu_append_item(section_menu, menu_item);
	menu_item = g_menu_item_new("Quit", "app.quit");
	g_menu_append_item(section_menu, menu_item);
	g_menu_append_section(menu, NULL, G_MENU_MODEL(section_menu));

	button = gtk_menu_button_new();
	gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(button), "open-menu-symbolic");
	gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(button), G_MENU_MODEL(menu));
	gtk_header_bar_pack_end(GTK_HEADER_BAR(titlebar), button);

	g_action_map_add_action_entries(G_ACTION_MAP(window), entries, G_N_ELEMENTS(entries), window);

	create_tab(window);

	gtk_widget_set_visible(window, TRUE);

}

static void on_activate(GApplication *app, gpointer cb_data)
{
	create_window(GTK_APPLICATION(app));
}

static void on_quit(GSimpleAction *action, GVariant *parameter, gpointer cb_data)
{
	g_application_quit(G_APPLICATION(cb_data));
}

static void on_new_window(GSimpleAction *action, GVariant *parameter, gpointer cb_data)
{
	create_window(GTK_APPLICATION(cb_data));
}

int main(int argc, char *argv[])
{
	GtkApplication *app;
	const GActionEntry entries[] = {
		{ "quit", on_quit, NULL, NULL, NULL },
		{ "new_window", on_new_window, NULL, NULL, NULL },
	};

	app = gtk_application_new("com.example.example01", 0);
	g_action_map_add_action_entries(G_ACTION_MAP(app), entries, G_N_ELEMENTS(entries), app);

	g_signal_connect(G_OBJECT(app), "activate", G_CALLBACK(on_activate), NULL);
	return g_application_run(G_APPLICATION(app), argc, argv);
}

