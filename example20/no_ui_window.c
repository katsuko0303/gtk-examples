#include "no_ui_window.h"
#include "list_item.h"

struct _MyNoUIWindow {
	GtkWindow parent_instance;
};
G_DEFINE_TYPE(MyNoUIWindow, my_no_ui_window, GTK_TYPE_WINDOW)

static GtkWidget * make_simple_string_drop_down()
{
	static const gchar *labels[] = {
		"Hoge", "Piyo", "Fuga", NULL,
	};
	GtkWidget *hbox;
	GtkWidget *label;
	GtkStringList *model;
	GtkWidget *drop_down;

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

	label = gtk_label_new("Simple String List");
	gtk_box_append(GTK_BOX(hbox), label);

	model = gtk_string_list_new(labels);
	drop_down = gtk_drop_down_new(G_LIST_MODEL(model), NULL);
	gtk_box_append(GTK_BOX(hbox), drop_down);

	return hbox;
}

static GtkWidget * make_drop_down_using_custom_item(GListModel *model)
{
	GtkWidget *hbox;
	GtkWidget *label;
	GtkExpression *expression;
	GtkWidget *drop_down;

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

	label = gtk_label_new("Use Custom Item");
	gtk_box_append(GTK_BOX(hbox), label);

	expression = gtk_property_expression_new(MY_TYPE_LIST_ITEM, NULL, "label");
	drop_down = gtk_drop_down_new(G_LIST_MODEL(g_object_ref(G_OBJECT(model))), expression);
	gtk_box_append(GTK_BOX(hbox), drop_down);

	return hbox;
}

static gchar * get_display_name(GObject *obj, gpointer cb_data)
{
	return g_strdup(g_file_info_get_display_name(G_FILE_INFO(obj)));
}

static GtkWidget * make_directory_list_drop_down(GFile *target_directory)
{
	GtkWidget *hbox;
	GtkWidget *label;
	GtkDirectoryList *model;
	GtkExpression *expression;
	GtkWidget *drop_down;

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

	label = gtk_label_new("Directory List");
	gtk_box_append(GTK_BOX(hbox), label);

	model = gtk_directory_list_new(G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME, target_directory);
	expression = gtk_cclosure_expression_new(G_TYPE_STRING, NULL, 0, NULL, G_CALLBACK(get_display_name), NULL, NULL);
	drop_down = gtk_drop_down_new(G_LIST_MODEL(model), expression);
	gtk_box_append(GTK_BOX(hbox), drop_down);

	return hbox;
}

static void on_factory_setup(GtkSignalListItemFactory *factory, GtkListItem *list_item)
{
	GtkWidget *hbox;
	PangoAttrList *attributes;
	GtkWidget *label;
	GtkWidget *separator;

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

	attributes = pango_attr_list_new();
	pango_attr_list_insert(attributes, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
	label = gtk_label_new("");
	gtk_label_set_attributes(GTK_LABEL(label), attributes);
	gtk_box_append(GTK_BOX(hbox), label);
	g_object_set_data(G_OBJECT(hbox), "label", label);

	separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
	gtk_box_append(GTK_BOX(hbox), separator);

	label = gtk_label_new("");
	gtk_box_append(GTK_BOX(hbox), label);
	g_object_set_data(G_OBJECT(hbox), "value", label);

	gtk_list_item_set_child(list_item, hbox);

	pango_attr_list_unref(attributes);
}

static void on_factory_bind(GtkSignalListItemFactory *factory, GtkListItem *list_item)
{
	GtkWidget *child;
	GObject *item;
	gchar *label;

	child = gtk_list_item_get_child(list_item);
	item = gtk_list_item_get_item(list_item);

	label = my_list_item_get_label(MY_LIST_ITEM(item));
	gtk_label_set_label(GTK_LABEL(g_object_get_data(G_OBJECT(child), "label")), label);
	g_free(label);

	label = g_strdup_printf("%d", my_list_item_get_value(MY_LIST_ITEM(item)));
	gtk_label_set_label(GTK_LABEL(g_object_get_data(G_OBJECT(child), "value")), label);
	g_free(label);
}

static GtkWidget * make_drop_down_using_list_item_factory(GListModel *model)
{
	GtkWidget *hbox;
	GtkWidget *label;
	GtkListItemFactory *factory;
	GtkWidget *drop_down;

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

	label = gtk_label_new("Use List Item Factory");
	gtk_box_append(GTK_BOX(hbox), label);

	factory = gtk_signal_list_item_factory_new();
	g_signal_connect(G_OBJECT(factory), "setup", G_CALLBACK(on_factory_setup), NULL);
	g_signal_connect(G_OBJECT(factory), "bind", G_CALLBACK(on_factory_bind), NULL);
	drop_down = gtk_drop_down_new(G_LIST_MODEL(g_object_ref(G_OBJECT(model))), NULL);
	gtk_drop_down_set_factory(GTK_DROP_DOWN(drop_down), factory);
	gtk_box_append(GTK_BOX(hbox), drop_down);

	g_object_unref(G_OBJECT(factory));
	return hbox;
}

static void my_no_ui_window_class_init(MyNoUIWindowClass *klass)
{
}

static void my_no_ui_window_init(MyNoUIWindow *window)
{
	GFile *target_directory;
	GListStore *custom_item_list_model;
	GtkWidget *vbox;
	MyListItem *item;
	gint i;

	target_directory = g_file_new_for_path(".");

	custom_item_list_model = g_list_store_new(MY_TYPE_LIST_ITEM);
	for (i = 0; i < 5; ++i) {
		item = my_list_item_new(i);
		g_list_store_append(custom_item_list_model, item);
		g_object_unref(G_OBJECT(item));
	}

	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
	gtk_widget_set_margin_start(vbox, 8);
	gtk_widget_set_margin_end(vbox, 8);
	gtk_widget_set_margin_top(vbox, 8);
	gtk_widget_set_margin_bottom(vbox, 8);
	gtk_window_set_child(GTK_WINDOW(window), vbox);

	gtk_box_append(GTK_BOX(vbox), make_simple_string_drop_down());
	gtk_box_append(GTK_BOX(vbox), make_drop_down_using_custom_item(G_LIST_MODEL(custom_item_list_model)));
	gtk_box_append(GTK_BOX(vbox), make_directory_list_drop_down(target_directory));
	gtk_box_append(GTK_BOX(vbox), make_drop_down_using_list_item_factory(G_LIST_MODEL(custom_item_list_model)));

	g_object_unref(G_OBJECT(custom_item_list_model));
	g_object_unref(G_OBJECT(target_directory));
}

GtkWidget * my_no_ui_window_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_NO_UI_WINDOW, NULL));
}

