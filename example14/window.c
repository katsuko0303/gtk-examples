#include "window.h"
#include <webkit/webkit.h>

struct _MyWindow {
	GtkWindow parent_instance;
};
typedef	struct _MyWindowPrivate {
	WebKitWebView *web_view;
	GtkSpinner *spinner;
}	MyWindowPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyWindow, my_window, GTK_TYPE_WINDOW)
#define	PRIVATE(self)	((MyWindowPrivate *)my_window_get_instance_private(MY_WINDOW(self)))
#define	DECLARE_PRIVATE(self)	MyWindowPrivate *priv = PRIVATE(self)

static void on_web_view_notify_is_loading(GObject *obj, GParamSpec *pspec, gpointer cb_data)
{
	DECLARE_PRIVATE(cb_data);

	if (webkit_web_view_is_loading(WEBKIT_WEB_VIEW(obj))) {
		gtk_spinner_start(priv->spinner);
	}
	else {
		gtk_spinner_stop(priv->spinner);
	}
}

static void my_window_class_init(MyWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	webkit_web_view_get_type();

	gtk_widget_class_set_template_from_resource(widget_class, "/com/example/example14/window.ui");
	gtk_widget_class_bind_template_child_full(widget_class, "web_view", FALSE, G_PRIVATE_OFFSET(MyWindow, web_view));
	gtk_widget_class_bind_template_child_full(widget_class, "spinner", FALSE, G_PRIVATE_OFFSET(MyWindow, spinner));
	gtk_widget_class_bind_template_callback_full(widget_class, "on_web_view_notify_is_loading", G_CALLBACK(on_web_view_notify_is_loading));
}

static void my_window_init(MyWindow *window)
{
	DECLARE_PRIVATE(window);

	gtk_widget_init_template(GTK_WIDGET(window));

	webkit_web_view_load_uri(priv->web_view, "https://www.gtk.org/");
}

GtkWidget * my_window_new()
{
	return GTK_WIDGET(g_object_new(MY_TYPE_WINDOW, NULL));
}

