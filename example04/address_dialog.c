#include "address_dialog.h"

struct _MyAddressDialog {
	GtkWindow parent_instance;
};
typedef	struct _MyAddressDialogPrivate {
	gchar *first_name;
	gchar *last_name;
	GDate *birth_day;
#if 0
	gint age;
#endif
	gchar *address;
	gchar *email;
	GTask *task;
	MyAddressData *target;
	GtkCalendar *birth_day_calendar;
}	MyAddressDialogPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyAddressDialog, my_address_dialog, GTK_TYPE_WINDOW)
#define	PRIVATE(self)	((MyAddressDialogPrivate *)my_address_dialog_get_instance_private(MY_ADDRESS_DIALOG(self)))
#define	DECLARE_PRIVATE(self)	MyAddressDialogPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_FIRST_NAME,
	PROP_LAST_NAME,
	PROP_BIRTH_DAY,
	PROP_DISPLAY_BIRTH_DAY,
#if 0
	PROP_AGE,
#endif
	PROP_ADDRESS,
	PROP_EMAIL,
	PROP_TARGET,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

enum {
	SIGNAL_CANCEL,
	NUM_SIGNALS
};
static guint signal_ids[NUM_SIGNALS];

static void on_cancel(GtkButton *button, gpointer cb_data)
{
	g_signal_emit(G_OBJECT(cb_data), signal_ids[SIGNAL_CANCEL], 0);
}

static void on_accept(GtkButton *button, gpointer cb_data)
{
	MyAddressDialog *dialog = MY_ADDRESS_DIALOG(cb_data);
	DECLARE_PRIVATE(dialog);
	GValue *value;
	MyAddressData *address_data;

	if (priv->task != NULL) {
		if (priv->target == NULL) {
			address_data = my_address_data_new();
		}
		else {
			address_data = MY_ADDRESS_DATA(g_object_ref(G_OBJECT(priv->target)));
		}
		my_address_data_set_first_name(address_data, priv->first_name);
		my_address_data_set_last_name(address_data, priv->last_name);
		my_address_data_set_birth_day(address_data, priv->birth_day);
		//my_address_data_set_age(address_data, priv->age);
		my_address_data_set_address(address_data, priv->address);
		my_address_data_set_email(address_data, priv->email);
		value = g_new0(GValue, 1);
		g_value_init(value, MY_TYPE_ADDRESS_DATA);
		g_value_take_object(value, address_data);
		g_task_return_value(priv->task, value);
	}
	gtk_window_destroy(GTK_WINDOW(dialog));
}

static void cancel_real(MyAddressDialog *dialog)
{
	DECLARE_PRIVATE(dialog);
	GError *error;

	if (priv->task != NULL) {
		error = g_error_new_literal(gtk_dialog_error_quark(), GTK_DIALOG_ERROR_DISMISSED, "Dismissed by user");
		g_task_return_error(priv->task, error);
	}
	gtk_window_destroy(GTK_WINDOW(dialog));
}

static void on_birth_day_selected(GtkCalendar *calendar, gpointer cb_data)
{
	MyAddressDialog *dialog = MY_ADDRESS_DIALOG(cb_data);
	GDateTime *date_time;
	GDate *date;

	date_time = gtk_calendar_get_date(calendar);
	date = g_date_new_dmy(g_date_time_get_day_of_month(date_time), g_date_time_get_month(date_time), g_date_time_get_year(date_time));
	g_object_set(G_OBJECT(dialog), "birth-day", date, NULL);
	g_date_free(date);
	g_date_time_unref(date_time);
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_pointer(&priv->first_name, g_free);
	g_clear_pointer(&priv->last_name, g_free);
	g_clear_pointer(&priv->birth_day, g_date_free);
	g_clear_pointer(&priv->address, g_free);
	g_clear_pointer(&priv->email, g_free);
	g_clear_object(&priv->target);

	(*G_OBJECT_CLASS(my_address_dialog_parent_class)->dispose)(obj);
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

	case PROP_DISPLAY_BIRTH_DAY:
		{
			gchar *s;

			s = g_strdup_printf("%u/%u/%u", (guint)g_date_get_month(priv->birth_day), (guint)g_date_get_day(priv->birth_day), (guint)g_date_get_year(priv->birth_day));
			g_value_set_string(value, s);
			g_free(s);
		}
		break;

#if 0
	case PROP_AGE:
		g_value_set_int(value, priv->age);
		break;
#endif

	case PROP_ADDRESS:
		g_value_set_string(value, priv->address);
		break;

	case PROP_EMAIL:
		g_value_set_string(value, priv->email);
		break;

	case PROP_TARGET:
		g_value_set_object(value, priv->target);
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
		{
			const gchar *s = g_value_get_string(value);
			if (strcmp(s, priv->first_name) != 0) {
				g_clear_pointer(&priv->first_name, g_free);
				priv->first_name = g_strdup(s);
				g_object_notify_by_pspec(obj, pspec);
			}
		}
		break;

	case PROP_LAST_NAME:
		{
			const gchar *s = g_value_get_string(value);
			if (strcmp(s, priv->last_name) != 0) {
				g_clear_pointer(&priv->last_name, g_free);
				priv->last_name = g_strdup(s);
				g_object_notify_by_pspec(obj, pspec);
			}
		}
		break;

	case PROP_BIRTH_DAY:
		{
			GDateTime *date_time;

			if (g_date_compare((const GDate *)g_value_get_boxed(value), priv->birth_day) != 0) {
				g_clear_pointer(&priv->birth_day, g_date_free);
				priv->birth_day = (GDate *)g_value_dup_boxed(value);

				date_time = g_date_time_new_local(g_date_get_year(priv->birth_day), (gint)g_date_get_month(priv->birth_day), g_date_get_day(priv->birth_day), 0, 0, 0);
				gtk_calendar_select_day(priv->birth_day_calendar, date_time);
				g_date_time_unref(date_time);

				g_object_notify_by_pspec(obj, properties[PROP_DISPLAY_BIRTH_DAY]);
			}
		}
		break;

#if 0
	case PROP_AGE:
		{
			gint i = g_value_get_int(value);
			if (i != priv->age) {
				priv->age = i;
				g_object_notify_by_pspec(obj, pspec);
			}
		}
		break;
#endif

	case PROP_ADDRESS:
		{
			const gchar *s = g_value_get_string(value);
			if (strcmp(s, priv->address) != 0) {
				g_clear_pointer(&priv->address, g_free);
				priv->address = g_strdup(s);
				g_object_notify_by_pspec(obj, pspec);
			}
		}
		break;

	case PROP_EMAIL:
		{
			const gchar *s = g_value_get_string(value);
			if (strcmp(s, priv->email) != 0) {
				g_clear_pointer(&priv->email, g_free);
				priv->email = g_strdup(s);
				g_object_notify_by_pspec(obj, pspec);
			}
		}
		break;

	case PROP_TARGET:
		g_clear_object(&priv->target);
		priv->target = g_value_dup_object(value);
		if (priv->target != NULL) {
			g_object_set(obj,
					"first-name", my_address_data_get_first_name(priv->target),
					"last-name", my_address_data_get_last_name(priv->target),
					"birth-day", my_address_data_get_birth_day(priv->target),
					//"age", my_address_data_get_age(priv->target),
					"address", my_address_data_get_address(priv->target),
					"email", my_address_data_get_email(priv->target),
					NULL);
		}
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static gboolean close_request(GtkWindow *window)
{
	g_signal_emit(G_OBJECT(window), signal_ids[SIGNAL_CANCEL], 0);
	return FALSE;
}

static void my_address_dialog_class_init(MyAddressDialogClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GtkWindowClass *window_class = GTK_WINDOW_CLASS(klass);

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;
	obj_class->set_property = set_property;

	properties[PROP_FIRST_NAME] = g_param_spec_string("first-name", "", "", "", G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);
	properties[PROP_LAST_NAME] = g_param_spec_string("last-name", "", "", "", G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);
	properties[PROP_BIRTH_DAY] = g_param_spec_boxed("birth-day", "", "", G_TYPE_DATE, G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);
	properties[PROP_DISPLAY_BIRTH_DAY] = g_param_spec_string("display-birth-day", "", "", "", G_PARAM_READABLE);
#if 0
	properties[PROP_AGE] = g_param_spec_int("age", "", "", 0, 999, 0, G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);
#endif
	properties[PROP_ADDRESS] = g_param_spec_string("address", "", "", "", G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);
	properties[PROP_EMAIL] = g_param_spec_string("email", "", "", "", G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);
	properties[PROP_TARGET] = g_param_spec_object("target", "", "", MY_TYPE_ADDRESS_DATA, G_PARAM_READWRITE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	{
		GClosure *closure = g_cclosure_new(G_CALLBACK(cancel_real), NULL, NULL);
		signal_ids[SIGNAL_CANCEL] = g_signal_newv("cancel", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
				closure, NULL, NULL, NULL, G_TYPE_NONE, 0, NULL);
	}

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example04/address_dialog.ui");
	gtk_widget_class_bind_template_callback_full(widget_class, "on_cancel", G_CALLBACK(on_cancel));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_accept", G_CALLBACK(on_accept));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_birth_day_selected", G_CALLBACK(on_birth_day_selected));
	gtk_widget_class_bind_template_child_full(widget_class, "birth_day_calendar", FALSE, G_PRIVATE_OFFSET(MyAddressDialog, birth_day_calendar));

	{
		GtkShortcutTrigger *trigger;
		GtkShortcutAction * action;

		trigger = gtk_keyval_trigger_new(GDK_KEY_Escape, 0);
		action = gtk_signal_action_new("cancel");
		gtk_widget_class_add_shortcut(widget_class, gtk_shortcut_new(trigger, action));
	}

	window_class->close_request = close_request;
}

static void my_address_dialog_init(MyAddressDialog *dialog)
{
	DECLARE_PRIVATE(dialog);
	GDateTime *date_time;

	priv->first_name = g_strdup("");
	priv->last_name = g_strdup("");
	date_time = g_date_time_new_now_local();
	priv->birth_day = g_date_new_dmy(g_date_time_get_day_of_month(date_time), g_date_time_get_month(date_time), g_date_time_get_year(date_time));
	g_date_time_unref(date_time);
#if 0
	priv->age = 0;
#endif
	priv->address = g_strdup("");
	priv->email = g_strdup("");
	priv->task = NULL;
	priv->target = NULL;

	gtk_widget_init_template(GTK_WIDGET(dialog));

	date_time = g_date_time_new_local(g_date_get_year(priv->birth_day), (gint)g_date_get_month(priv->birth_day), g_date_get_day(priv->birth_day), 0, 0, 0);
	gtk_calendar_select_day(priv->birth_day_calendar, date_time);
	g_date_time_unref(date_time);

	g_object_notify_by_pspec(G_OBJECT(dialog), properties[PROP_DISPLAY_BIRTH_DAY]);
}

GtkWidget * my_address_dialog_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_ADDRESS_DIALOG, NULL));
}

void my_address_dialog_edit_async(MyAddressDialog *dialog, GtkWindow *parent, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer cb_data)
{
	DECLARE_PRIVATE(dialog);

	g_return_if_fail(priv->task == NULL);
	gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
	priv->task = g_task_new(G_OBJECT(dialog), cancellable, callback, cb_data);
	gtk_widget_set_visible(GTK_WIDGET(dialog), TRUE);
}

MyAddressData * my_address_dialog_edit_finish(MyAddressDialog *dialog, GAsyncResult *result, GError **error)
{
	DECLARE_PRIVATE(dialog);
	GValue value = { 0 };
	MyAddressData *address_data = NULL;

	g_return_val_if_fail(priv->task != NULL, NULL);
	if (g_task_propagate_value(priv->task, &value, error)) {
		address_data = g_value_dup_object(&value);
		g_value_reset(&value);
	}
	return address_data;
}

MyAddressData * my_address_dialog_get_target(MyAddressDialog *dialog)
{
	return PRIVATE(dialog)->target;
}

void my_address_dialog_set_target(MyAddressDialog *dialog, MyAddressData *target)
{
	g_object_set(G_OBJECT(dialog), "target", target, NULL);
}

