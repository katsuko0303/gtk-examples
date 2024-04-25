#include "window.h"

struct _MyWindow {
	GtkWindow parent_instance;
};
typedef	struct _MyWindowPrivate {
	GFile *current_folder;
}	MyWindowPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyWindow, my_window, GTK_TYPE_WINDOW)
#define	PRIVATE(self)	my_window_get_instance_private(MY_WINDOW(self))
#define	DECLARE_PRIVATE(self)	MyWindowPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_CURRENT_FOLDER,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static void on_setup_list_item(GtkSignalListItemFactory *factory, GtkListItem *list_item, gpointer cb_data)
{
	GtkWidget *hbox;
	GtkWidget *image;
	GtkWidget *label;

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_widget_set_margin_start(hbox, 4);
	gtk_widget_set_margin_end(hbox, 4);

	image = gtk_image_new();
	gtk_box_append(GTK_BOX(hbox), image);

	label = gtk_label_new("");
	gtk_widget_set_halign(label, GTK_ALIGN_FILL);
	gtk_widget_set_hexpand(label, TRUE);
	gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
	gtk_box_append(GTK_BOX(hbox), label);

	gtk_list_item_set_child(list_item, hbox);
}

static void on_bind_list_item(GtkSignalListItemFactory *factory, GtkListItem *list_item, gpointer cb_data)
{
	GtkWidget *hbox = gtk_list_item_get_child(list_item);
	GFileInfo *file_info = G_FILE_INFO(gtk_list_item_get_item(list_item));
	GtkWidget *child;

	child = gtk_widget_get_first_child(hbox);
	while (child != NULL) {
		if (GTK_IS_IMAGE(child)) {
			gtk_image_set_from_gicon(GTK_IMAGE(child), g_file_info_get_icon(file_info));
		}
		else if (GTK_IS_LABEL(child)) {
			gtk_label_set_label(GTK_LABEL(child), g_file_info_get_display_name(file_info));
		}
		child = gtk_widget_get_next_sibling(child);
	}
}

static void on_go_up(GtkButton *button, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	DECLARE_PRIVATE(window);
	GFile *parent;

	parent = g_file_get_parent(priv->current_folder);
	if (parent != NULL) {
		g_clear_object(&priv->current_folder);
		priv->current_folder = parent;
		g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_CURRENT_FOLDER]);
	}
}

static void on_list_view_activate(GtkListView *list_view, guint position, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	DECLARE_PRIVATE(window);
	GFile *folder;
	GFileInfo *file_info;

	if (position != GTK_INVALID_LIST_POSITION) {
		file_info = G_FILE_INFO(g_list_model_get_item(G_LIST_MODEL(gtk_list_view_get_model(list_view)), position));
		if (g_file_info_get_file_type(file_info) == G_FILE_TYPE_DIRECTORY) {
			folder = g_file_get_child(priv->current_folder, g_file_info_get_name(file_info));
			g_clear_object(&priv->current_folder);
			priv->current_folder = folder;
			g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_CURRENT_FOLDER]);
		}
		g_object_unref(G_OBJECT(file_info));
	}
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_object(&priv->current_folder);

	(*G_OBJECT_CLASS(my_window_parent_class)->dispose)(obj);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_CURRENT_FOLDER:
		g_value_set_object(value, priv->current_folder);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void my_window_class_init(MyWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;

	properties[PROP_CURRENT_FOLDER] = g_param_spec_object("current-folder", "", "", G_TYPE_FILE, G_PARAM_READABLE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example02/window.ui");
	gtk_widget_class_bind_template_callback_full(widget_class, "on_setup_list_item", G_CALLBACK(on_setup_list_item));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_bind_list_item", G_CALLBACK(on_bind_list_item));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_go_up", G_CALLBACK(on_go_up));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_list_view_activate", G_CALLBACK(on_list_view_activate));
}

static void my_window_init(MyWindow *window)
{
	DECLARE_PRIVATE(window);

	priv->current_folder = g_file_new_for_path(".");

	gtk_widget_init_template(GTK_WIDGET(window));

	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_CURRENT_FOLDER]);
}

GtkWidget * my_window_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_WINDOW, NULL));
}

