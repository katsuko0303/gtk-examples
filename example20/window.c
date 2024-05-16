#include "window.h"
#include "list_item.h"

struct _MyWindow {
	GtkWindow parent_instance;
};
typedef	struct _MyWindowPrivate {
	GFile *target_directory;
	GListStore *custom_item_list_model;
}	MyWindowPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyWindow, my_window, GTK_TYPE_WINDOW)
#define	PRIVATE(self)	((MyWindowPrivate *)my_window_get_instance_private(MY_WINDOW(self)))
#define	DECLARE_PRIVATE(self)	MyWindowPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_TARGET_DIRECTORY,
	PROP_CUSTOM_ITEM_LIST_MODEL,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static gchar * get_display_name(GObject *obj, gpointer cb_data)
{
	return g_strdup(g_file_info_get_display_name(G_FILE_INFO(obj)));
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_object(&priv->target_directory);
	g_clear_object(&priv->custom_item_list_model);

	(*G_OBJECT_CLASS(my_window_parent_class)->dispose)(obj);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_TARGET_DIRECTORY:
		g_value_set_object(value, G_OBJECT(priv->target_directory));
		break;

	case PROP_CUSTOM_ITEM_LIST_MODEL:
		{
			MyListItem *item;
			gint i;

			if (priv->custom_item_list_model == NULL) {
				priv->custom_item_list_model = g_list_store_new(MY_TYPE_LIST_ITEM);
				for (i = 0; i < 5; ++i) {
					item = my_list_item_new(i);
					g_list_store_append(priv->custom_item_list_model, G_OBJECT(item));
					g_object_unref(G_OBJECT(item));
				}
			}
			g_value_set_object(value, G_OBJECT(priv->custom_item_list_model));
		}
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void my_window_class_init(MyWindowClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	my_list_item_get_type();

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;

	properties[PROP_TARGET_DIRECTORY] = g_param_spec_object("target-directory", "", "", G_TYPE_FILE, G_PARAM_READABLE);
	properties[PROP_CUSTOM_ITEM_LIST_MODEL] = g_param_spec_object("custom-item-list-model", "", "", G_TYPE_LIST_MODEL, G_PARAM_READABLE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example20/window.ui");
	gtk_widget_class_bind_template_callback_full(widget_class, "get_display_name", G_CALLBACK(get_display_name));
}

static void my_window_init(MyWindow *window)
{
	DECLARE_PRIVATE(window);

	priv->target_directory = NULL;
	priv->custom_item_list_model = NULL;

	gtk_widget_init_template(GTK_WIDGET(window));

	priv->target_directory = g_file_new_for_path(".");
	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_TARGET_DIRECTORY]);

	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_CUSTOM_ITEM_LIST_MODEL]);
}

GtkWidget * my_window_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_WINDOW, NULL));
}

