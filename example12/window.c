#include "window.h"
#include <string.h>
#include "../common/utils.h"

static void builder_scope_interface_init(GtkBuilderScopeInterface *iface);
static void enumerate_files_recursively(MyWindow *window);

struct _MyWindow {
	GtkWindow parent_instance;
};
typedef	struct _MyWindowPrivate {
	GListStore *file_model;
	GtkListView *list_view;
	GtkCustomFilter *filter;
	GtkSpinner *spinner;

	GFile *top_folder;
	gchar *search_text;
	gboolean loading;

	GList *folder_list;
}	MyWindowPrivate;
G_DEFINE_TYPE_WITH_CODE(MyWindow, my_window, GTK_TYPE_WINDOW,
		G_ADD_PRIVATE(MyWindow)
		G_IMPLEMENT_INTERFACE(GTK_TYPE_BUILDER_SCOPE, builder_scope_interface_init)
		)
#define	PRIVATE(self)	((MyWindowPrivate *)my_window_get_instance_private(MY_WINDOW(self)))
#define	DECLARE_PRIVATE(self)	MyWindowPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_TOP_FOLDER,
	PROP_SEARCH_TEXT,
	PROP_LOADING,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static GIcon * get_icon(GtkListItem *list_item, GObject *item)
{
	return (item != NULL? g_file_info_get_icon(G_FILE_INFO(item)): NULL);
}

static gchar * get_display_name(GtkListItem *list_item, GObject *item)
{
	return g_strdup((item != NULL? g_file_info_get_display_name(G_FILE_INFO(item)): ""));
}

static GClosure * create_closure(GtkBuilderScope *scope, GtkBuilder *builder, const gchar *function_name, GtkBuilderClosureFlags flags, GObject *obj, GError **error)
{
	GClosure *closure = NULL;

	if (strcmp(function_name, "get_icon") == 0) {
		closure = g_cclosure_new(G_CALLBACK(get_icon), NULL, NULL);
	}
	else if (strcmp(function_name, "get_display_name") == 0) {
		closure = g_cclosure_new(G_CALLBACK(get_display_name), NULL, NULL);
	}
	return closure;
}

static void builder_scope_interface_init(GtkBuilderScopeInterface *iface)
{
	iface->create_closure = create_closure;
}

static gboolean filter_func(GObject *item, gpointer cb_data)
{
	DECLARE_PRIVATE(cb_data);

	return strlen(priv->search_text) == 0 || strstr(g_file_info_get_display_name(G_FILE_INFO(item)), priv->search_text) != NULL;
}

static void on_next_files(GObject *source, GAsyncResult *result, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	DECLARE_PRIVATE(window);
	GError *error;
	GList *top;
	GList *list;

	error = NULL;
	top = g_file_enumerator_next_files_finish(G_FILE_ENUMERATOR(source), result, &error);
	if (top == NULL) {
		if (error != NULL) {
			g_printerr("%s(%d): %s.\n", __FILE__, __LINE__, error->message);
		}
		g_file_enumerator_close(G_FILE_ENUMERATOR(source), NULL, NULL);
		if (priv->folder_list != NULL) {
			enumerate_files_recursively(window);
		}
		else {
			g_object_set(G_OBJECT(window), "loading", FALSE, NULL);
		}
	}
	else {
		list = top;
		while (list != NULL) {
			g_list_store_append(priv->file_model, list->data);
			if (g_file_info_get_file_type(G_FILE_INFO(list->data)) == G_FILE_TYPE_DIRECTORY) {
				priv->folder_list = g_list_append(priv->folder_list, g_file_enumerator_get_child(G_FILE_ENUMERATOR(source), G_FILE_INFO(list->data)));
			}
			list = g_list_next(list);
		}
		g_list_free(top);
		g_file_enumerator_next_files_async(G_FILE_ENUMERATOR(source), 32, G_PRIORITY_DEFAULT, NULL, on_next_files, cb_data);
	}
}

static void on_enumerate_children(GObject *source, GAsyncResult *result, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	DECLARE_PRIVATE(window);
	GError *error;
	GFileEnumerator *enumerator;

	error = NULL;
	enumerator = g_file_enumerate_children_finish(G_FILE(source), result, &error);
	if (enumerator == NULL) {
		g_printerr("%s(%d): %s.\n", __FILE__, __LINE__, (error != NULL? error->message: "(no messages)"));
		if (priv->folder_list != NULL) {
			enumerate_files_recursively(window);
		}
		else {
			g_object_set(G_OBJECT(window), "loading", FALSE, NULL);
		}
	}
	else {
		g_file_enumerator_next_files_async(enumerator, 32, G_PRIORITY_DEFAULT, NULL, on_next_files, cb_data);
		g_object_unref(G_OBJECT(enumerator));
	}
	if (error != NULL)	g_error_free(error);
}

static void enumerate_files_recursively(MyWindow *window)
{
	DECLARE_PRIVATE(window);
	GFile *folder;
	GList *folder_list;

	folder = G_FILE(priv->folder_list->data);
	folder_list = priv->folder_list;
	priv->folder_list = g_list_remove_link(priv->folder_list, folder_list);
	g_file_enumerate_children_async(folder, G_FILE_ATTRIBUTE_STANDARD_ICON "," G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME ","
			G_FILE_ATTRIBUTE_STANDARD_TYPE "," G_FILE_ATTRIBUTE_STANDARD_NAME,
			G_FILE_QUERY_INFO_NONE, G_PRIORITY_DEFAULT, NULL, 
			on_enumerate_children, window);
	g_list_free(folder_list);
	g_object_unref(G_OBJECT(folder));
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_object(&priv->top_folder);
	g_clear_pointer(&priv->search_text, g_free);

	(*G_OBJECT_CLASS(my_window_parent_class)->dispose)(obj);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_TOP_FOLDER:
		g_value_set_object(value, priv->top_folder);
		break;

	case PROP_SEARCH_TEXT:
		g_value_set_string(value, priv->search_text);
		break;

	case PROP_LOADING:
		g_value_set_boolean(value, priv->loading);
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
	case PROP_TOP_FOLDER:
		g_clear_object(&priv->top_folder);
		priv->top_folder = g_value_dup_object(value);
		priv->folder_list = g_list_append(NULL, g_object_ref(G_OBJECT(priv->top_folder)));
		enumerate_files_recursively(MY_WINDOW(obj));
		g_object_set(obj, "loading", TRUE, NULL);
		break;

	case PROP_SEARCH_TEXT:
		{
			const gchar *s = g_value_get_string(value);
			if (strcmp(s, priv->search_text) != 0) {
				g_free(priv->search_text);
				priv->search_text = g_strdup(s);
				g_object_notify_by_pspec(obj, pspec);

				gtk_filter_changed(GTK_FILTER(priv->filter), GTK_FILTER_CHANGE_DIFFERENT);
			}
		}
		break;

	case PROP_LOADING:
		priv->loading = g_value_get_boolean(value);
		if (priv->loading) {
			gtk_spinner_start(priv->spinner);
		}
		else {
			gtk_spinner_stop(priv->spinner);
		}
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

	properties[PROP_TOP_FOLDER] = g_param_spec_object("top-folder", "", "", G_TYPE_FILE, G_PARAM_READWRITE);
	properties[PROP_SEARCH_TEXT] = g_param_spec_string("search-text", "", "", "", G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);
	properties[PROP_LOADING] = g_param_spec_boolean("loading", "", "", FALSE, G_PARAM_READWRITE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example12/window.ui");
	gtk_widget_class_bind_template_child_full(widget_class, "file_model", FALSE, G_PRIVATE_OFFSET(MyWindow, file_model));
	gtk_widget_class_bind_template_child_full(widget_class, "list_view", FALSE, G_PRIVATE_OFFSET(MyWindow, list_view));
	gtk_widget_class_bind_template_child_full(widget_class, "filter", FALSE, G_PRIVATE_OFFSET(MyWindow, filter));
	gtk_widget_class_bind_template_child_full(widget_class, "spinner", FALSE, G_PRIVATE_OFFSET(MyWindow, spinner));
}

static void my_window_init(MyWindow *window)
{
	DECLARE_PRIVATE(window);
	GtkListItemFactory *factory;

	priv->top_folder = NULL;
	priv->search_text = g_strdup("");
	priv->loading = FALSE;

	gtk_widget_init_template(GTK_WIDGET(window));

	gtk_custom_filter_set_filter_func(priv->filter, (GtkCustomFilterFunc)filter_func, window, NULL);

	factory = gtk_builder_list_item_factory_new_from_resource(GTK_BUILDER_SCOPE(window), "/com/example/example12/list_item.ui");
	CHECK_OBJECT_UNREF(factory);
	gtk_list_view_set_factory(priv->list_view, factory);
	g_object_unref(G_OBJECT(factory));
}

GtkWidget * my_window_new(GFile *top_folder)
{
	return GTK_WIDGET(g_object_new(MY_TYPE_WINDOW, "top-folder", top_folder, NULL));
}

