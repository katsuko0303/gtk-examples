#include "file_item.h"

struct _MyFileItem {
	GObject parent_instance;
};
typedef	struct _MyFileItemPrivate {
	GFileInfo *file_info;
}	MyFileItemPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyFileItem, my_file_item, G_TYPE_OBJECT)
#define	PRIVATE(self)	((MyFileItemPrivate *)my_file_item_get_instance_private(MY_FILE_ITEM(self)))
#define	DECLARE_PRIVATE(self)	MyFileItemPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_FILE_INFO,
	PROP_ICON,
	PROP_DISPLAY_NAME,
	PROP_TYPE_DESCRIPTION,
	PROP_DISPLAY_MODIFIED_TIME,
	PROP_MODIFIED_TIME,
	PROP_SIZE,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_object(&priv->file_info);

	(*G_OBJECT_CLASS(my_file_item_parent_class)->dispose)(obj);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_FILE_INFO:
		g_value_set_object(value, priv->file_info);
		break;

	case PROP_ICON:
		g_value_set_object(value, (priv->file_info != NULL? g_file_info_get_icon(priv->file_info): NULL));
		break;

	case PROP_DISPLAY_NAME:
		g_value_set_string(value, (priv->file_info != NULL? g_file_info_get_display_name(priv->file_info): ""));
		break;

	case PROP_TYPE_DESCRIPTION:
		{
			gchar *description = NULL;
			const gchar *content_type;

			if (priv->file_info != NULL) {
				content_type = g_file_info_get_content_type(priv->file_info);
				if (content_type != NULL) {
					description = g_content_type_get_description(content_type);
				}
			}
			if (description == NULL) {
				g_value_set_string(value, "");
			}
			else {
				g_value_set_string(value, description);
				g_free(description);
			}
		}
		break;

	case PROP_DISPLAY_MODIFIED_TIME:
		{
			gchar *text = NULL;
			GDateTime *modified_time;

			if (priv->file_info != NULL) {
				modified_time = g_file_info_get_modification_date_time(priv->file_info);
				if (modified_time != NULL) {
					text = g_date_time_format(modified_time, "%c");
					g_date_time_unref(modified_time);
				}
			}
			if (text == NULL) {
				g_value_set_string(value, "");
			}
			else {
				g_value_set_string(value, text);
				g_free(text);
			}
		}
		break;

	case PROP_MODIFIED_TIME:
		{
			gint64 time = 0;
			GDateTime *modified_time;

			if (priv->file_info != NULL) {
				modified_time = g_file_info_get_modification_date_time(priv->file_info);
				if (modified_time != NULL) {
					time = g_date_time_to_unix(modified_time);
					g_date_time_unref(modified_time);
				}
			}
			g_value_set_int64(value, time);
		}
		break;

	case PROP_SIZE:
		g_value_set_uint64(value, (guint64)(priv->file_info != NULL? g_file_info_get_size(priv->file_info): 0));
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
	case PROP_FILE_INFO:
		g_clear_object(&priv->file_info);
		priv->file_info = g_value_dup_object(value);
		g_object_notify_by_pspec(obj, properties[PROP_ICON]);
		g_object_notify_by_pspec(obj, properties[PROP_DISPLAY_NAME]);
		g_object_notify_by_pspec(obj, properties[PROP_TYPE_DESCRIPTION]);
		g_object_notify_by_pspec(obj, properties[PROP_DISPLAY_MODIFIED_TIME]);
		g_object_notify_by_pspec(obj, properties[PROP_MODIFIED_TIME]);
		g_object_notify_by_pspec(obj, properties[PROP_SIZE]);
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

	properties[PROP_FILE_INFO] = g_param_spec_object("file-info", "", "", G_TYPE_FILE_INFO, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	properties[PROP_ICON] = g_param_spec_object("icon", "", "", G_TYPE_ICON, G_PARAM_READABLE);
	properties[PROP_DISPLAY_NAME] = g_param_spec_string("display-name", "", "", "", G_PARAM_READABLE);
	properties[PROP_TYPE_DESCRIPTION] = g_param_spec_string("type-description", "", "", "", G_PARAM_READABLE);
	properties[PROP_DISPLAY_MODIFIED_TIME] = g_param_spec_string("display-modified-time", "", "", "", G_PARAM_READABLE);
	properties[PROP_MODIFIED_TIME] = g_param_spec_int64("modified-time", "", "", G_MININT64, G_MAXINT64, 0, G_PARAM_READABLE);
	properties[PROP_SIZE] = g_param_spec_uint64("size", "", "", 0, G_MAXUINT64, 0, G_PARAM_READABLE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);
}

static void my_file_item_init(MyFileItem *item)
{
	DECLARE_PRIVATE(item);

	priv->file_info = NULL;
}

MyFileItem * my_file_item_new(GFileInfo *file_info)
{
	return MY_FILE_ITEM(g_object_new(MY_TYPE_FILE_ITEM, "file-info", file_info, NULL));
}

gboolean my_file_item_is_directory(MyFileItem *item)
{
	DECLARE_PRIVATE(item);

	return (priv->file_info != NULL && g_file_info_get_file_type(priv->file_info) == G_FILE_TYPE_DIRECTORY);
}

const gchar * my_file_item_get_name(MyFileItem *item)
{
	DECLARE_PRIVATE(item);

	g_return_val_if_fail(priv->file_info != NULL, "");
	return g_file_info_get_name(priv->file_info);
}

