#include "list_item.h"

struct _MyListItem {
	GObject parent_instance;
};
typedef	struct _MyListItemPrivate {
	gint value;
}	MyListItemPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyListItem, my_list_item, G_TYPE_OBJECT)
#define	PRIVATE(self)	((MyListItemPrivate *)my_list_item_get_instance_private(MY_LIST_ITEM(self)))
#define	DECLARE_PRIVATE(self)	MyListItemPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_VALUE,
	PROP_LABEL,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static gchar * get_label(MyListItem *item)
{
	DECLARE_PRIVATE(item);

	return g_strdup_printf("Item %d", priv->value);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_VALUE:
		g_value_set_int(value, priv->value);
		break;

	case PROP_LABEL:
		g_value_take_string(value, get_label(MY_LIST_ITEM(obj)));
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
	case PROP_VALUE:
		priv->value = g_value_get_int(value);
		g_object_notify_by_pspec(obj, properties[PROP_LABEL]);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void my_list_item_class_init(MyListItemClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = get_property;
	obj_class->set_property = set_property;

	properties[PROP_VALUE] = g_param_spec_int("value", "", "", G_MININT, G_MAXINT, 0, G_PARAM_READWRITE);
	properties[PROP_LABEL] = g_param_spec_string("label", "", "", "", G_PARAM_READABLE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);
}

static void my_list_item_init(MyListItem *item)
{
	DECLARE_PRIVATE(item);

	priv->value = 0;
}

MyListItem * my_list_item_new(gint value)
{
	return MY_LIST_ITEM(g_object_new(MY_TYPE_LIST_ITEM, "value", value, NULL));
}

gint my_list_item_get_value(MyListItem *item)
{
	return PRIVATE(item)->value;
}

gchar * my_list_item_get_label(MyListItem *item)
{
	return get_label(item);
}

