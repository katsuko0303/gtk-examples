#include "file_item.h"

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
	PROP_DISPLAY_NAME,
	PROP_ICON,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static void on_query_info(GObject *source, GAsyncResult *result, gpointer cb_data)
{
	MyFileItem *item = MY_FILE_ITEM(cb_data);
	DECLARE_PRIVATE(item);
	GError *error;

	g_clear_object(&priv->file_info);
	error = NULL;
	priv->file_info = g_file_query_info_finish(G_FILE(source), result, &error);
	if (priv->file_info == NULL) {
		g_printerr("%s(%d): failed to query file info. (%s)\n", __FILE__, __LINE__, (error != NULL? error->message : "(no messages)"));
	}
	if (error != NULL) {
		g_error_free(error);
	}
	g_object_notify_by_pspec(G_OBJECT(item), properties[PROP_DISPLAY_NAME]);
	g_object_notify_by_pspec(G_OBJECT(item), properties[PROP_ICON]);
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

	case PROP_DISPLAY_NAME:
		g_value_set_string(value, (priv->file_info != NULL? g_file_info_get_display_name(priv->file_info): ""));
		break;

	case PROP_ICON:
		g_value_set_object(value, (priv->file_info != NULL? g_file_info_get_icon(priv->file_info): NULL));
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
		g_clear_object(&priv->file_info);
		priv->file = g_value_dup_object(value);
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
	properties[PROP_DISPLAY_NAME] = g_param_spec_string("display-name", "", "", "", G_PARAM_READABLE);
	properties[PROP_ICON] = g_param_spec_object("icon", "", "", G_TYPE_ICON, G_PARAM_READABLE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);
}

static void my_file_item_init(MyFileItem *item)
{
	DECLARE_PRIVATE(item);

	priv->file = NULL;
	priv->file_info = NULL;
}

MyFileItem * my_file_item_new(GFile *file)
{
	return MY_FILE_ITEM(g_object_new(MY_TYPE_FILE_ITEM, "file", file, NULL));
}

void my_file_item_query_info(MyFileItem *item, GCancellable *cancellable)
{
	DECLARE_PRIVATE(item);

	g_return_if_fail(priv->file_info == NULL);
	g_return_if_fail(priv->file != NULL);
	g_file_query_info_async(priv->file, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME "," G_FILE_ATTRIBUTE_STANDARD_ICON,
			G_FILE_QUERY_INFO_NONE, G_PRIORITY_DEFAULT, cancellable, on_query_info, item);
}

