#include "file_item_widget.h"
#include "file_item.h"

struct _MyFileItemWidget {
	GtkBox parent_instance;
};
typedef	struct _MyFileItemWidgetPrivate {
	MyFileItem *item;
	GtkWidget *image;
	GtkWidget *label;
}	MyFileItemWidgetPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyFileItemWidget, my_file_item_widget, GTK_TYPE_BOX)
#define	PRIVATE(self)	((MyFileItemWidgetPrivate *)my_file_item_widget_get_instance_private(MY_FILE_ITEM_WIDGET(self)))
#define	DECLARE_PRIVATE(self)	MyFileItemWidgetPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_ITEM,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static void on_image_map(GtkWidget *widget, gpointer cb_data)
{
	DECLARE_PRIVATE(cb_data);

	if (priv->item != NULL) {
		my_file_item_load_icon_texture(priv->item);
	}
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_object(&priv->item);

	(*G_OBJECT_CLASS(my_file_item_widget_parent_class)->dispose)(obj);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_ITEM:
		g_value_set_object(value, priv->item);
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
	case PROP_ITEM:
		g_clear_object(&priv->item);
		priv->item = g_value_dup_object(value);

		if (priv->item != NULL) {
			if (priv->label != NULL) {
				gtk_label_set_label(GTK_LABEL(priv->label), my_file_item_get_display_name(priv->item));
				g_object_bind_property(G_OBJECT(priv->item), "display-name", G_OBJECT(priv->label), "label", G_BINDING_DEFAULT);
			}
			if (priv->image != NULL) {
				gtk_image_set_from_paintable(GTK_IMAGE(priv->image), my_file_item_get_icon(priv->item));
				g_object_bind_property(G_OBJECT(priv->item), "icon", G_OBJECT(priv->image), "paintable", G_BINDING_DEFAULT);
				if (gtk_widget_get_mapped(priv->image)) {
					my_file_item_load_icon_texture(priv->item);
				}
			}
		}

		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void my_file_item_widget_class_init(MyFileItemWidgetClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;
	obj_class->set_property = set_property;

	properties[PROP_ITEM] = g_param_spec_object("item", "", "", MY_TYPE_FILE_ITEM, G_PARAM_READWRITE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);
}

static void my_file_item_widget_init(MyFileItemWidget *widget)
{
	DECLARE_PRIVATE(widget);

	priv->item = NULL;

	priv->image = gtk_image_new_from_paintable((priv->item != NULL? my_file_item_get_icon(priv->item): NULL));
	gtk_widget_set_size_request(priv->image, 128, 128);
	g_signal_connect(G_OBJECT(priv->image), "map", G_CALLBACK(on_image_map), widget);
	if (priv->item != NULL) {
		g_object_bind_property(G_OBJECT(priv->item), "icon", G_OBJECT(priv->image), "paintable", G_BINDING_DEFAULT);
	}
	gtk_box_append(GTK_BOX(widget), priv->image);

	priv->label = gtk_label_new((priv->item != NULL? my_file_item_get_display_name(priv->item): ""));
	if (priv->item != NULL) {
		g_object_bind_property(G_OBJECT(priv->item), "display-name", G_OBJECT(priv->label), "label", G_BINDING_DEFAULT);
	}
	gtk_box_append(GTK_BOX(widget), priv->label);
}

GtkWidget * my_file_item_widget_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_FILE_ITEM_WIDGET,
				"orientation", GTK_ORIENTATION_VERTICAL,
				"spacing", 4,
				NULL));
}

