#include "name_dialog.h"

struct _MyNameDialog {
	GtkWindow parent_instance;
};
typedef	struct _MyNameDialogPrivate {
	gchar *name;
	GTask *task;
}	MyNameDialogPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyNameDialog, my_name_dialog, GTK_TYPE_WINDOW)
#define	PRIVATE(self)	((MyNameDialogPrivate *)my_name_dialog_get_instance_private(MY_NAME_DIALOG(self)))
#define	DECLARE_PRIVATE(self)	MyNameDialogPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_NAME,
	PROP_CAN_ACCEPT,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

enum {
	SIGNAL_CANCEL,
	NUM_SIGNALS
};
static guint signal_ids[NUM_SIGNALS];

static void cancel_real(MyNameDialog *dialog)
{
	DECLARE_PRIVATE(dialog);

	if (priv->task != NULL) {
		g_task_return_error(priv->task, g_error_new_literal(gtk_dialog_error_quark(), GTK_DIALOG_ERROR_DISMISSED, "Dismissed by user"));
		g_clear_object(&priv->task);
	}
	gtk_window_destroy(GTK_WINDOW(dialog));
}

static void on_cancel(GtkButton *button, gpointer cb_data)
{
	g_signal_emit(G_OBJECT(cb_data), signal_ids[SIGNAL_CANCEL], 0);
}

static void on_ok(GtkButton *button, gpointer cb_data)
{
	MyNameDialog *dialog = MY_NAME_DIALOG(cb_data);
	DECLARE_PRIVATE(dialog);
	GValue value = { 0 };

	if (priv->task != NULL) {
		g_value_init(&value, G_TYPE_STRING);
		g_value_set_string(&value, priv->name);
		g_task_return_value(priv->task, &value);
		g_value_reset(&value);
		g_clear_object(&priv->task);
	}
	gtk_window_destroy(GTK_WINDOW(dialog));
}

static gboolean close_request(GtkWindow *window)
{
	g_signal_emit(G_OBJECT(window), signal_ids[SIGNAL_CANCEL], 0);
	return FALSE;
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_pointer(&priv->name, g_free);

	(*G_OBJECT_CLASS(my_name_dialog_parent_class)->dispose)(obj);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_NAME:
		g_value_set_string(value, priv->name);
		break;

	case PROP_CAN_ACCEPT:
		g_value_set_boolean(value, (strlen(priv->name) > 0));
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
	case PROP_NAME:
		{
			const gchar *s = g_value_get_string(value);
			if (strcmp(s, priv->name) != 0) {
				g_clear_pointer(&priv->name, g_free);
				priv->name = g_strdup(s);
				g_object_notify_by_pspec(obj, pspec);
				g_object_notify_by_pspec(obj, properties[PROP_CAN_ACCEPT]);
			}
		}
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void my_name_dialog_class_init(MyNameDialogClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GtkWindowClass *window_class = GTK_WINDOW_CLASS(klass);
	GtkShortcutTrigger *trigger;
	GtkShortcutAction *action;
	GClosure *closure;

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;
	obj_class->set_property = set_property;

	window_class->close_request = close_request;

	properties[PROP_NAME] = g_param_spec_string("name", "", "", "", G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);
	properties[PROP_CAN_ACCEPT] = g_param_spec_boolean("can-accept", "", "", FALSE, G_PARAM_READABLE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	closure = g_cclosure_new(G_CALLBACK(cancel_real), NULL, NULL);
	signal_ids[SIGNAL_CANCEL] = g_signal_newv("cancel", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			closure, NULL, NULL, NULL, G_TYPE_NONE, 0, NULL);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example11/name_dialog.ui");
	gtk_widget_class_bind_template_callback_full(widget_class, "on_cancel", G_CALLBACK(on_cancel));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_ok", G_CALLBACK(on_ok));

	trigger = gtk_keyval_trigger_new(GDK_KEY_Escape, 0);
	action = gtk_signal_action_new("cancel");
	gtk_widget_class_add_shortcut(widget_class, gtk_shortcut_new(trigger, action));
}

static void my_name_dialog_init(MyNameDialog *dialog)
{
	DECLARE_PRIVATE(dialog);

	priv->name = g_strdup("");

	gtk_widget_init_template(GTK_WIDGET(dialog));
}

GtkWidget * my_name_dialog_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_NAME_DIALOG, NULL));
}

void my_name_dialog_ask(MyNameDialog *dialog, GtkWindow *parent, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer cb_data)
{
	DECLARE_PRIVATE(dialog);

	gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
	if (priv->task == NULL) {
		priv->task = g_task_new(G_OBJECT(dialog), cancellable, callback, cb_data);
	}
	gtk_widget_set_visible(GTK_WIDGET(dialog), TRUE);
}

gchar * my_name_dialog_ask_finish(MyNameDialog *dialog, GAsyncResult *result, GError **error)
{
	GValue value = { 0 };
	gchar *name = NULL;
	gboolean ret;

	ret = g_task_propagate_value(G_TASK(result), &value, error);
	if (ret) {
		name = g_value_dup_string(&value);
		g_value_reset(&value);
	}
	return name;
}

