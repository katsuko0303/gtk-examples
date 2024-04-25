#include "file_item.h"

#define	FILE_ATTRIBUTES	\
	G_FILE_ATTRIBUTE_STANDARD_NAME "," G_FILE_ATTRIBUTE_STANDARD_ICON "," G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME "," G_FILE_ATTRIBUTE_STANDARD_TYPE

struct _MyFileItem {
	GObject parent_instance;
};
typedef	struct _MyFileItemPrivate {
	GFile *file;
	GFileInfo *file_info;
}	MyFileItemPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyFileItem, my_file_item, G_TYPE_OBJECT)
#define	PRIVATE(self)	((MyFileItemPrivate *)my_file_item_get_instance_private(MY_FILE_ITEM(self)))
#define	DECLARE_PRIVATE(self)	MyFileItemPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_FILE,
	PROP_FILE_INFO,
	PROP_ICON,
	PROP_DISPLAY_NAME,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static void on_next_files(GObject *source, GAsyncResult *result, gpointer cb_data)
{
	GListStore *list_store = G_LIST_STORE(cb_data);
	GError *error;
	GList *top, *list;
	GFileInfo *file_info;
	GFile *child;
	MyFileItem *item;

	error = NULL;
	top = g_file_enumerator_next_files_finish(G_FILE_ENUMERATOR(source), result, &error);
	if (top == NULL) {
		if (error != NULL) {
			g_printerr("%s(%d): error: %s.\n", __FILE__, __LINE__, error->message);
		}
		g_file_enumerator_close(G_FILE_ENUMERATOR(source), NULL, NULL);
		g_object_unref(G_OBJECT(list_store));
	}
	else {
		list = top;
		while (list != NULL) {
			file_info = G_FILE_INFO(list->data);
			child = g_file_enumerator_get_child(G_FILE_ENUMERATOR(source), file_info);

			item = MY_FILE_ITEM(g_object_new(MY_TYPE_FILE_ITEM, "file", child, "file-info", file_info, NULL));
			g_list_store_append(list_store, item);
			g_object_unref(G_OBJECT(item));

			g_object_unref(G_OBJECT(child));
			g_object_unref(G_OBJECT(file_info));
			list = g_list_next(list);
		}
		g_list_free(top);

		g_file_enumerator_next_files_async(G_FILE_ENUMERATOR(source), 32, G_PRIORITY_DEFAULT, NULL, on_next_files, list_store);
	}
}

static void on_enumerate_children(GObject *source, GAsyncResult *result, gpointer cb_data)
{
	GListStore *list_store = G_LIST_STORE(cb_data);
	GFileEnumerator *enumerator;
	GError *error;

	error = NULL;
	enumerator = g_file_enumerate_children_finish(G_FILE(source), result, &error);
	if (enumerator == NULL) {
		g_printerr("%s(%d): error: %s.\n", __FILE__, __LINE__, (error != NULL? error->message: "(no messages)"));
		g_object_unref(G_OBJECT(list_store));
	}
	else {
		g_file_enumerator_next_files_async(enumerator, 32, G_PRIORITY_DEFAULT, NULL, on_next_files, list_store);
	}
	if (error != NULL)	g_error_free(error);
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_object(&priv->file);
	g_clear_object(&priv->file_info);

	(*G_OBJECT_CLASS(my_file_item_parent_class)->dispose)(obj);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_FILE:
		g_value_set_object(value, priv->file);
		break;

	case PROP_FILE_INFO:
		g_value_set_object(value, priv->file_info);
		break;

	case PROP_ICON:
		g_value_set_object(value, (priv->file_info != NULL? g_file_info_get_icon(priv->file_info): NULL));
		break;

	case PROP_DISPLAY_NAME:
		g_value_set_string(value, (priv->file_info != NULL? g_file_info_get_display_name(priv->file_info): NULL));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void set_property(GObject *obj, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_FILE:
		g_clear_object(&priv->file);
		priv->file = g_value_dup_object(value);
		break;

	case PROP_FILE_INFO:
		g_clear_object(&priv->file_info);
		priv->file_info = g_value_dup_object(value);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void my_file_item_class_init(MyFileItemClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;
	obj_class->set_property = set_property;

	properties[PROP_FILE] = g_param_spec_object("file", "", "", G_TYPE_FILE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	properties[PROP_FILE_INFO] = g_param_spec_object("file-info", "", "", G_TYPE_FILE_INFO, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	properties[PROP_ICON] = g_param_spec_object("icon", "", "", G_TYPE_ICON, G_PARAM_READABLE);
	properties[PROP_DISPLAY_NAME] = g_param_spec_string("display-name", "", "", "", G_PARAM_READABLE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);
}

static void my_file_item_init(MyFileItem *item)
{
	DECLARE_PRIVATE(item);

	priv->file = NULL;
	priv->file_info = NULL;
}

GListModel * my_file_item_create_root_model(GFile *root_dir)
{
	GListStore *list_store;

	list_store = g_list_store_new(MY_TYPE_FILE_ITEM);
	g_file_enumerate_children_async(root_dir, FILE_ATTRIBUTES, G_FILE_QUERY_INFO_NONE, G_PRIORITY_DEFAULT, NULL, on_enumerate_children, g_object_ref(G_OBJECT(list_store)));
	return G_LIST_MODEL(list_store);
}

GListModel * my_file_item_create_child_model(MyFileItem *item)
{
	DECLARE_PRIVATE(item);

	return (priv->file != NULL && priv->file_info != NULL && g_file_info_get_file_type(priv->file_info) == G_FILE_TYPE_DIRECTORY?
			my_file_item_create_root_model(priv->file): NULL);
}

