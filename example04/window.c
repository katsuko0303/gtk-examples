#include "window.h"
#include "address_dialog.h"

struct _MyWindow {
	GtkWindow parent_instance;
};
typedef	struct _MyWindowPrivate {
	GListStore *address_model;
}	MyWindowPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyWindow, my_window, GTK_TYPE_WINDOW)
#define	PRIVATE(self)	((MyWindowPrivate *)my_window_get_instance_private(MY_WINDOW(self)))
#define	DECLARE_PRIVATE(self)	MyWindowPrivate *priv = PRIVATE(self)

static void on_edit_finish(GObject *source, GAsyncResult *result, gpointer cb_data)
{
	MyAddressData *address_data;
	DECLARE_PRIVATE(cb_data);

	address_data = my_address_dialog_edit_finish(MY_ADDRESS_DIALOG(source), result, NULL);
	if (address_data != NULL) {
		if (my_address_dialog_get_target(MY_ADDRESS_DIALOG(source)) == NULL) {
			g_list_store_append(priv->address_model, address_data);
		}
		g_object_unref(G_OBJECT(address_data));
	}
}

static void on_new_address(GtkButton *button, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	GtkWidget *dialog;

	dialog = my_address_dialog_new();
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	my_address_dialog_edit_async(MY_ADDRESS_DIALOG(dialog), GTK_WINDOW(window), NULL, on_edit_finish, window);
}

static void on_address_list_activate(GtkColumnView *column_view, guint position, gpointer cb_data)
{
	MyWindow *window = MY_WINDOW(cb_data);
	MyAddressData *address_data;
	GtkWidget *dialog;

	if (position == GTK_INVALID_LIST_POSITION) {
		return ;
	}
	address_data = MY_ADDRESS_DATA(g_list_model_get_item(G_LIST_MODEL(gtk_column_view_get_model(column_view)), position));
	if (address_data == NULL) {
		return ;
	}
	dialog = my_address_dialog_new();
	my_address_dialog_set_target(MY_ADDRESS_DIALOG(dialog), address_data);
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	my_address_dialog_edit_async(MY_ADDRESS_DIALOG(dialog), GTK_WINDOW(window), NULL, on_edit_finish, window);
	g_object_unref(G_OBJECT(address_data));
}

static void my_window_class_init(MyWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	my_address_data_get_type();

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example04/window.ui");
	gtk_widget_class_bind_template_callback_full(widget_class, "on_new_address", G_CALLBACK(on_new_address));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_address_list_activate", G_CALLBACK(on_address_list_activate));
	gtk_widget_class_bind_template_child_full(widget_class, "address_model", FALSE, G_PRIVATE_OFFSET(MyWindow, address_model));
}

static void my_window_init(MyWindow *window)
{
	gtk_widget_init_template(GTK_WIDGET(window));
}

GtkWidget * my_window_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_WINDOW, NULL));
}

