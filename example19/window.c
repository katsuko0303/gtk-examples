#include "window.h"

struct _MyWindow {
	GtkWindow parent_instance;
};
typedef	struct _MyWindowPrivate {
	GtkWidget *label;
	GtkWidget *drop_down;
}	MyWindowPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyWindow, my_window, GTK_TYPE_WINDOW)
#define	PRIVATE(self)	((MyWindowPrivate *)my_window_get_instance_private(MY_WINDOW(self)))
#define	DECLARE_PRIVATE(self)	MyWindowPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_LABEL_ATTRIBUTES,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

/*
static gchar * get_font_family_label(PangoFontFamily *font_family, gpointer cb_data)
{
	return g_strdup(pango_font_family_get_name(font_family));
}
*/

static void on_drop_down_notify_selected(GObject *obj, GParamSpec *pspec, gpointer cb_data)
{
	g_object_notify_by_pspec(G_OBJECT(cb_data), properties[PROP_LABEL_ATTRIBUTES]);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_LABEL_ATTRIBUTES:
		{
			PangoAttrList *attr_list;
			GObject *selected_item;

			attr_list = pango_attr_list_new();
			pango_attr_list_insert(attr_list, pango_attr_size_new(PANGO_SCALE * 32));
			selected_item = gtk_drop_down_get_selected_item(GTK_DROP_DOWN(priv->drop_down));
			if (selected_item != NULL) {
				pango_attr_list_insert(attr_list, pango_attr_family_new(pango_font_family_get_name(PANGO_FONT_FAMILY(selected_item))));
			}
			g_value_take_boxed(value, attr_list);
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

	obj_class->get_property = get_property;

	properties[PROP_LABEL_ATTRIBUTES] = g_param_spec_boxed("label-attributes", "", "", PANGO_TYPE_ATTR_LIST, G_PARAM_READABLE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example19/window.ui");
	gtk_widget_class_bind_template_child_full(widget_class, "label", FALSE, G_PRIVATE_OFFSET(MyWindow, label));
	gtk_widget_class_bind_template_child_full(widget_class, "drop_down", FALSE, G_PRIVATE_OFFSET(MyWindow, drop_down));
	//gtk_widget_class_bind_template_callback_full(widget_class, "get_font_family_label", G_CALLBACK(get_font_family_label));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_drop_down_notify_selected", G_CALLBACK(on_drop_down_notify_selected));
}

static void my_window_init(MyWindow *window)
{
	DECLARE_PRIVATE(window);
	PangoContext *pctx;
	PangoFontMap *font_map;
	const gchar *family;
	guint selected;
	guint num_items;
	PangoFontFamily *font_family;
	guint i;

	g_print("%s(%d): window %p.\n", __FILE__, __LINE__, window);
	gtk_widget_init_template(GTK_WIDGET(window));

	pctx = gtk_widget_get_pango_context(priv->label);
	font_map = pango_context_get_font_map(pctx);
	gtk_drop_down_set_model(GTK_DROP_DOWN(priv->drop_down), G_LIST_MODEL(font_map));
	family = pango_font_description_get_family(pango_context_get_font_description(pctx));
	selected = GTK_INVALID_LIST_POSITION;
	num_items = g_list_model_get_n_items(G_LIST_MODEL(font_map));
	for (i = 0; i < num_items && selected == GTK_INVALID_LIST_POSITION; ++i) {
		font_family = PANGO_FONT_FAMILY(g_list_model_get_item(G_LIST_MODEL(font_map), i));
		if (strcmp(pango_font_family_get_name(font_family), family) == 0) {
			selected = i;
		}
		g_object_unref(G_OBJECT(font_family));
	}
	gtk_drop_down_set_selected(GTK_DROP_DOWN(priv->drop_down), selected);
}

GtkWidget * my_window_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_WINDOW, NULL));
}

