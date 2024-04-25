#include "window.h"
#include "file_item.h"

struct _MyWindow {
	GtkWindow parent_instance;
};
typedef	struct _MyWindowPrivate {
	GtkTreeListModel *root_model;
}	MyWindowPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyWindow, my_window, GTK_TYPE_WINDOW)
#define	PRIVATE(self)	((MyWindowPrivate *)my_window_get_instance_private(MY_WINDOW(self)))
#define	DECLARE_PRIVATE(self)	MyWindowPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_ROOT_MODEL,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static GListModel * create_model_func(GObject *item, gpointer cb_data)
{
	return my_file_item_create_child_model(MY_FILE_ITEM(item));
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_object(&priv->root_model);

	(*G_OBJECT_CLASS(my_window_parent_class)->dispose)(obj);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_ROOT_MODEL:
		g_value_set_object(value, priv->root_model);
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

	properties[PROP_ROOT_MODEL] = g_param_spec_object("root-model", "", "", G_TYPE_LIST_MODEL, G_PARAM_READABLE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example09/window.ui");
}

static void my_window_init(MyWindow *window)
{
	DECLARE_PRIVATE(window);
	GFile *home_dir;
	GListModel *list_model;

	priv->root_model = NULL;

	gtk_widget_init_template(GTK_WIDGET(window));

	home_dir = g_file_new_for_path(g_get_home_dir());
	list_model = my_file_item_create_root_model(home_dir);
	priv->root_model = gtk_tree_list_model_new(list_model, FALSE, FALSE, (GtkTreeListModelCreateModelFunc)create_model_func, NULL, NULL);
	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_ROOT_MODEL]);

	g_object_unref(G_OBJECT(home_dir));
}

GtkWidget * my_window_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_WINDOW, NULL));
}

