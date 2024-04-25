#include "window.h"

static GtkContentFit content_fit_values[] = {
	GTK_CONTENT_FIT_FILL,
	GTK_CONTENT_FIT_CONTAIN,
	GTK_CONTENT_FIT_COVER,
	GTK_CONTENT_FIT_SCALE_DOWN,
	};

struct _MyWindow {
	GtkWindow parent_instance;
};
typedef	struct _MyWindowPrivate {
	gboolean can_shrink;
	GtkContentFit content_fit;
	GdkTexture *paintable;
	GtkStringList *content_fit_model;
}	MyWindowPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyWindow, my_window, GTK_TYPE_WINDOW)
#define	PRIVATE(self)	((MyWindowPrivate *)my_window_get_instance_private(MY_WINDOW(self)))
#define	DECLARE_PRIVATE(self)	MyWindowPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_CAN_SHRINK,
	PROP_CONTENT_FIT_MODEL,
	PROP_CONTENT_FIT_SELECTED,
	PROP_CONTENT_FIT,
	PROP_PAINTABLE,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static gboolean on_drop_image(GtkDropTarget *drop_target, const GValue *value, gdouble x, gdouble y, gpointer cb_data)
{
	DECLARE_PRIVATE(cb_data);

	if (G_VALUE_HOLDS(value, G_TYPE_FILE)) {
		g_clear_object(&priv->paintable);
		priv->paintable = gdk_texture_new_from_file(G_FILE(g_value_get_object(value)), NULL);
		return TRUE;
	}
	else if (G_VALUE_HOLDS(value, GDK_TYPE_PIXBUF)) {
		g_clear_object(&priv->paintable);
		priv->paintable = gdk_texture_new_for_pixbuf(GDK_PIXBUF(g_value_get_object(value)));
		return TRUE;
	}
	return FALSE;
}

static void on_select_image_file(GObject *source, GAsyncResult *result, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	DECLARE_PRIVATE(window);
	GFile *file;

	g_clear_object(&priv->paintable);

	file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(source), result, NULL);
	if (file != NULL) {
		priv->paintable = gdk_texture_new_from_file(file, NULL);
		g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_PAINTABLE]);
		g_object_unref(G_OBJECT(file));
	}
}

static void on_open(GtkButton *button, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	GtkFileDialog *dialog;
	GListStore *filters;
	GtkFileFilter *file_filter;

	dialog = gtk_file_dialog_new();
	gtk_file_dialog_set_modal(dialog, TRUE);

	filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
	file_filter = gtk_file_filter_new();
	gtk_file_filter_set_name(file_filter, "Image Files");
	gtk_file_filter_add_pixbuf_formats(file_filter);
	g_list_store_append(filters, file_filter);
	g_object_unref(G_OBJECT(file_filter));
	gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(filters));
	g_object_unref(G_OBJECT(filters));

	gtk_file_dialog_open(dialog, GTK_WINDOW(window), NULL, on_select_image_file, cb_data);
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_object(&priv->paintable);
	g_clear_object(&priv->content_fit_model);

	(*G_OBJECT_CLASS(my_window_parent_class)->dispose)(obj);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_CAN_SHRINK:
		g_value_set_boolean(value, priv->can_shrink);
		break;

	case PROP_CONTENT_FIT_MODEL:
		g_value_set_object(value, priv->content_fit_model);
		break;

	case PROP_CONTENT_FIT_SELECTED:
		{
			guint position = GTK_INVALID_LIST_POSITION;
			guint num;
			const gchar *name;
			GEnumValue *enum_value;
			guint i;
			if (priv->content_fit_model != NULL) {
				num = g_list_model_get_n_items(G_LIST_MODEL(priv->content_fit_model));
				for (i = 0; i < num; ++i) {
					name = gtk_string_list_get_string(priv->content_fit_model, i);
					enum_value = g_enum_get_value_by_nick(g_type_class_ref(GTK_TYPE_CONTENT_FIT), name);
					if (enum_value->value == (gint)priv->content_fit) {
						position = i;
						break;
					}
				}
			}
			g_value_set_uint(value, position);
		}
		break;

	case PROP_CONTENT_FIT:
		g_value_set_enum(value, priv->content_fit);
		break;

	case PROP_PAINTABLE:
		g_value_set_object(value, priv->paintable);
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
	case PROP_CAN_SHRINK:
		priv->can_shrink = g_value_get_boolean(value);
		g_object_notify_by_pspec(obj, pspec);
		break;

	case PROP_CONTENT_FIT_SELECTED:
		{
			guint position = g_value_get_uint(value);
			if (position != GTK_INVALID_LIST_POSITION) {
				priv->content_fit = (GtkContentFit)(g_enum_get_value_by_nick(g_type_class_ref(GTK_TYPE_CONTENT_FIT), gtk_string_list_get_string(priv->content_fit_model, position))->value);
				g_object_notify_by_pspec(obj, pspec);
				g_object_notify_by_pspec(obj, properties[PROP_CONTENT_FIT]);
			}
		}
		break;

	case PROP_CONTENT_FIT:
		priv->content_fit = (GtkContentFit)g_value_get_enum(value);
		g_object_notify_by_pspec(obj, pspec);
		g_object_notify_by_pspec(obj, properties[PROP_CONTENT_FIT_SELECTED]);
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
	obj_class->set_property = set_property;

	properties[PROP_CAN_SHRINK] = g_param_spec_boolean("can-shrink", "", "", TRUE, G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);
	properties[PROP_CONTENT_FIT_MODEL] = g_param_spec_object("content-fit-model", "", "", G_TYPE_LIST_MODEL, G_PARAM_READABLE);
	properties[PROP_CONTENT_FIT_SELECTED] = g_param_spec_uint("content-fit-selected", "", "", 0, G_MAXUINT, GTK_INVALID_LIST_POSITION, G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);
	properties[PROP_CONTENT_FIT] = g_param_spec_enum("content-fit", "", "", GTK_TYPE_CONTENT_FIT, GTK_CONTENT_FIT_CONTAIN, G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);
	properties[PROP_PAINTABLE] = g_param_spec_object("paintable", "", "", GDK_TYPE_PAINTABLE, G_PARAM_READABLE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example13/window.ui");
	gtk_widget_class_bind_template_callback_full(widget_class, "on_open", G_CALLBACK(on_open));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_drop_image", G_CALLBACK(on_drop_image));
}

static void my_window_init(MyWindow *window)
{
	DECLARE_PRIVATE(window);
	gint i;

	priv->can_shrink = TRUE;
	priv->paintable = NULL;
	priv->content_fit = GTK_CONTENT_FIT_CONTAIN;
	priv->content_fit_model = gtk_string_list_new(NULL);
	for (i = 0; i < G_N_ELEMENTS(content_fit_values); ++i) {
		gtk_string_list_append(priv->content_fit_model, g_enum_get_value(g_type_class_ref(GTK_TYPE_CONTENT_FIT), content_fit_values[i])->value_nick);
	}

	gtk_widget_init_template(GTK_WIDGET(window));

	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_CONTENT_FIT_MODEL]);
	g_object_set(G_OBJECT(window), "content-fit", GTK_CONTENT_FIT_CONTAIN, NULL);
}

GtkWidget * my_window_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_WINDOW, NULL));
}

