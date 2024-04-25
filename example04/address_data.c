#include "address_data.h"

struct _MyAddressData {
	GObject parent_instance;
};
typedef	struct _MyAddressDataPrivate {
	gchar *first_name;
	gchar *last_name;
	GDate *birth_day;
	gint age;
	gchar *address;
	gchar *email;
}	MyAddressDataPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyAddressData, my_address_data, G_TYPE_OBJECT)
#define	PRIVATE(self)	((MyAddressDataPrivate *)my_address_data_get_instance_private(MY_ADDRESS_DATA(self)))
#define	DECLARE_PRIVATE(self)	MyAddressDataPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_FIRST_NAME,
	PROP_LAST_NAME,
	PROP_BIRTH_DAY,
	PROP_AGE,
	PROP_ADDRESS,
	PROP_EMAIL,
	PROP_FULL_NAME,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static gint get_age(GDate *date)
{
	GDateTime *now;
	gint age;

	now = g_date_time_new_now_local();
	age = g_date_time_get_year(now) - g_date_get_year(date) - (g_date_time_get_day_of_year(now) < g_date_get_day_of_year(date)? 1: 0);
	g_date_time_unref(now);
	return age;
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_pointer(&priv->first_name, g_free);
	g_clear_pointer(&priv->last_name, g_free);
	g_clear_pointer(&priv->address, g_free);
	g_clear_pointer(&priv->email, g_free);
	g_clear_pointer(&priv->birth_day, g_date_free);

	(*G_OBJECT_CLASS(my_address_data_parent_class)->dispose)(obj);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_FIRST_NAME:
		g_value_set_string(value, priv->first_name);
		break;

	case PROP_LAST_NAME:
		g_value_set_string(value, priv->last_name);
		break;

	case PROP_BIRTH_DAY:
		g_value_set_boxed(value, priv->birth_day);
		break;

	case PROP_AGE:
		g_value_set_int(value, get_age(priv->birth_day));
		break;

	case PROP_ADDRESS:
		g_value_set_string(value, priv->address);
		break;

	case PROP_EMAIL:
		g_value_set_string(value, priv->email);
		break;

	case PROP_FULL_NAME:
		{
			if (strlen(priv->first_name) == 0) {
				g_value_set_string(value, priv->last_name);
			}
			else if (strlen(priv->last_name) == 0) {
				g_value_set_string(value, priv->first_name);
			}
			else {
				gchar *full_name = g_strdup_printf("%s %s", priv->first_name, priv->last_name);
				g_value_set_string(value, full_name);
				g_free(full_name);
			}
		}
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
	case PROP_FIRST_NAME:
		g_clear_pointer(&priv->first_name, g_free);
		priv->first_name = g_value_dup_string(value);
		g_object_notify_by_pspec(obj, properties[PROP_FULL_NAME]);
		break;

	case PROP_LAST_NAME:
		g_clear_pointer(&priv->last_name, g_free);
		priv->last_name = g_value_dup_string(value);
		g_object_notify_by_pspec(obj, properties[PROP_FULL_NAME]);
		break;

	case PROP_BIRTH_DAY:
		g_clear_pointer(&priv->birth_day, g_date_free);
		priv->birth_day = g_value_dup_boxed(value);
		break;

#if 0
	case PROP_AGE:
		priv->age = g_value_get_int(value);
		break;
#endif

	case PROP_ADDRESS:
		g_clear_pointer(&priv->address, g_free);
		priv->address = g_value_dup_string(value);
		break;

	case PROP_EMAIL:
		g_clear_pointer(&priv->email, g_free);
		priv->email = g_value_dup_string(value);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void my_address_data_class_init(MyAddressDataClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;
	obj_class->set_property = set_property;

	properties[PROP_FIRST_NAME] = g_param_spec_string("first-name", "", "", "", G_PARAM_READWRITE);
	properties[PROP_LAST_NAME] = g_param_spec_string("last-name", "", "", "", G_PARAM_READWRITE);
	properties[PROP_BIRTH_DAY] = g_param_spec_boxed("birth-day", "", "", G_TYPE_DATE, G_PARAM_READWRITE);
	properties[PROP_AGE] = g_param_spec_int("age", "", "", G_MININT, G_MAXINT, 0, G_PARAM_READABLE);
	properties[PROP_ADDRESS] = g_param_spec_string("address", "", "", "", G_PARAM_READWRITE);
	properties[PROP_EMAIL] = g_param_spec_string("email", "", "", "", G_PARAM_READWRITE);
	properties[PROP_FULL_NAME] = g_param_spec_string("full-name", "", "", "", G_PARAM_READABLE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);
}

static void my_address_data_init(MyAddressData *address_data)
{
	DECLARE_PRIVATE(address_data);

	priv->first_name = g_strdup("");
	priv->last_name = g_strdup("");
	priv->birth_day = g_date_new();
#if 0
	priv->age = 0;
#endif
	priv->address = g_strdup("");
	priv->email = g_strdup("");
}

MyAddressData * my_address_data_new()
{
	return MY_ADDRESS_DATA(g_object_new(MY_TYPE_ADDRESS_DATA, NULL));
}

const gchar * my_address_data_get_first_name(MyAddressData *address_data)
{
	return PRIVATE(address_data)->first_name;
}

void my_address_data_set_first_name(MyAddressData *address_data, const gchar *first_name)
{
	g_object_set(G_OBJECT(address_data), "first-name", first_name, NULL);
}

const gchar * my_address_data_get_last_name(MyAddressData *address_data)
{
	return PRIVATE(address_data)->last_name;
}

void my_address_data_set_last_name(MyAddressData *address_data, const gchar *last_name)
{
	g_object_set(G_OBJECT(address_data), "last-name", last_name, NULL);
}

const GDate * my_address_data_get_birth_day(MyAddressData *address_data)
{
	return PRIVATE(address_data)->birth_day;
}

void my_address_data_set_birth_day(MyAddressData *address_data, const GDate *birth_day)
{
	g_object_set(G_OBJECT(address_data), "birth-day", birth_day, NULL);
}

gint my_address_data_get_age(MyAddressData *address_data)
{
#if 0
	return PRIVATE(address_data)->age;
#else
	return get_age(PRIVATE(address_data)->birth_day);
#endif
}

#if 0
void my_address_data_set_age(MyAddressData *address_data, gint age)
{
	g_object_set(G_OBJECT(address_data), "age", age, NULL);
}
#endif

const gchar * my_address_data_get_address(MyAddressData *address_data)
{
	return PRIVATE(address_data)->address;
}

void my_address_data_set_address(MyAddressData *address_data, const gchar *address)
{
	g_object_set(G_OBJECT(address_data), "address", address, NULL);
}

const gchar * my_address_data_get_email(MyAddressData *address_data)
{
	return PRIVATE(address_data)->email;
}

void my_address_data_set_email(MyAddressData *address_data, const gchar *email)
{
	g_object_set(G_OBJECT(address_data), "email", email, NULL);
}

