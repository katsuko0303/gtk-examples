#include "window.h"

#define	LABEL	"Hello, world!"
#define	KEYWORD	"world"

struct _MyWindow {
	GtkWindow parent_instance;
};
G_DEFINE_TYPE(MyWindow, my_window, GTK_TYPE_WINDOW)

enum {
	PROP_0,
	PROP_LABEL,
	PROP_ATTRIBUTES,
	PROP_ATTRIBUTES_BY_STRING,
	PROP_MARKUP_LABEL,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	switch (prop_id) {
	case PROP_LABEL:
		g_value_set_static_string(value, LABEL);
		break;

	case PROP_ATTRIBUTES:
		{
			PangoAttrList *attr_list;
			const gchar *label = LABEL;
			gint start;
			gchar *p;
			gint end;
			PangoAttribute *attr;

			attr_list = pango_attr_list_new();
			pango_attr_list_insert(attr_list, pango_attr_foreground_new(65535, 0, 0));
			pango_attr_list_insert(attr_list, pango_attr_size_new(PANGO_SCALE * 24));
			pango_attr_list_insert(attr_list, pango_attr_family_new("Serif"));
			start = 0;
			while (TRUE) {
				p = strstr(&label[start], KEYWORD);
				if (p == NULL) {
					break;
				}
				end = (gint)(p - &label[start]);
				attr = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
				attr->start_index = start;
				attr->end_index = end;
				pango_attr_list_insert(attr_list, attr);
				start = end + 1;
			}
			g_value_take_boxed(value, attr_list);
		}
		break;

	case PROP_ATTRIBUTES_BY_STRING:
		{
#if PANGO_VERSION_MAJOR < 1 || (PANGO_VERSION_MAJOR == 1 && PANGO_VERSION_MINOR < 50)
			g_print("%s(%d): warning: pango version too old.\n", __FILE__, __LINE__);
#else
			GStrvBuilder *builder = g_strv_builder_new();
			gchar *s;
			const gchar *label = LABEL;
			gint start;
			gchar *p;
			gint end;

			s = g_strdup_printf("0 -1 size %u", PANGO_SCALE * 24);
			g_strv_builder_add_many(builder, "0 -1 foreground #0000ff", s, "0 -1 family Serif", NULL);
			g_free(s);
			GStrv array;

			start = 0;
			while (TRUE) {
				p = strstr(&label[start], KEYWORD);
				if (p == NULL) {
					break;
				}
				end = (gint)(p - &label[start]);
				s = g_strdup_printf("%d %d weight bold", start, end);
				g_strv_builder_add(builder, s);
				g_free(s);
				start = end + 1;
			}
			array = g_strv_builder_end(builder);
			g_strv_builder_unref(builder);
			s = g_strjoinv(", ", array);
			g_strfreev(array);
			g_value_take_boxed(value, pango_attr_list_from_string(s));
			g_free(s);
#endif
		}
		break;

	case PROP_MARKUP_LABEL:
		{
			GStrvBuilder *builder = g_strv_builder_new();
			const gchar *start = LABEL;
			gchar *s;
			GStrv array;

			g_strv_builder_add(builder, "<span foreground=\"#c0c000\" size=\"24pt\" font_family=\"Serif\">");
			while (TRUE) {
				gchar *p = strstr(start, KEYWORD);
				if (p == NULL) {
					g_strv_builder_add(builder, start);
					break;
				}
				if (start != p) {
					s = g_strndup(start, p - start);
					g_strv_builder_add(builder, s);
					g_free(s);
				}
				g_strv_builder_add_many(builder, "<span font_weight=\"bold\">", KEYWORD, "</span>", NULL);
				start = p + strlen(KEYWORD);
			}
			g_strv_builder_add(builder, "</span>");
			array = g_strv_builder_end(builder);
			g_strv_builder_unref(builder);
			s = g_strjoinv(NULL, array);
			g_strfreev(array);
			g_value_set_string(value, s);
			g_free(s);
		}
		break;
	}
}

static void my_window_class_init(MyWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = get_property;

	properties[PROP_LABEL] = g_param_spec_string("label", "", "", "", G_PARAM_READABLE);
	properties[PROP_ATTRIBUTES] = g_param_spec_boxed("attributes", "", "", PANGO_TYPE_ATTR_LIST, G_PARAM_READABLE);
	properties[PROP_ATTRIBUTES_BY_STRING] = g_param_spec_boxed("attributes-by-string", "", "", PANGO_TYPE_ATTR_LIST, G_PARAM_READABLE);
	properties[PROP_MARKUP_LABEL] = g_param_spec_string("markup-label", "", "", "", G_PARAM_READABLE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example16/window.ui");
}

static void my_window_init(MyWindow *window)
{
	gtk_widget_init_template(GTK_WIDGET(window));

	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_LABEL]);
	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_MARKUP_LABEL]);
	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_ATTRIBUTES]);
	g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_ATTRIBUTES_BY_STRING]);
}

GtkWidget * my_window_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_WINDOW, NULL));
}
