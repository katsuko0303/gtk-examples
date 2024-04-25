#include "file_item.h"
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "image_loader.h"

#define	ICON_SIZE	128

struct _MyFileItem {
	GObject parent_instance;
};
typedef	struct _MyFileItemPrivate {
	GFile *file;
	GFileInfo *file_info;
	GdkTexture *icon_texture;
#if 0
	GTask *task;
#else
	MyImageLoader *loader;
#endif
}	MyFileItemPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(MyFileItem, my_file_item, G_TYPE_OBJECT)
#define	PRIVATE(self)	((MyFileItemPrivate *)my_file_item_get_instance_private(MY_FILE_ITEM(self)))
#define	DECLARE_PRIVATE(self)	MyFileItemPrivate *priv = PRIVATE(self)

enum {
	PROP_0,
	PROP_FILE,
	PROP_FILE_INFO,
	PROP_ICON,
	PROP_DISPLAY_NAME,
	NUM_PROPERTIES
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL };

static void on_loaded_icon_texture(GObject *source, GAsyncResult *result, gpointer cb_data)
{
	MyFileItem *item = MY_FILE_ITEM(cb_data);
	DECLARE_PRIVATE(item);
	GError *error;
	GdkTexture *texture;

	error = NULL;
	texture = my_image_loader_load_finish(MY_IMAGE_LOADER(source), result, &error);
	if (error != NULL) {
		g_printerr("%s(%d): failed to load texture: %s.\n", __FILE__, __LINE__, error->message);
		g_error_free(error);
		g_object_unref(G_OBJECT(priv->loader));
		priv->loader = NULL;
		return ;
	}
	g_clear_object(&priv->icon_texture);
	priv->icon_texture = texture;
	g_clear_object(&priv->loader);
	g_object_notify_by_pspec(G_OBJECT(item), properties[PROP_ICON]);
}

static void dispose(GObject *obj)
{
	DECLARE_PRIVATE(obj);

	g_clear_object(&priv->file);
	g_clear_object(&priv->file_info);
	g_clear_object(&priv->icon_texture);
	g_clear_object(&priv->loader);

	(*G_OBJECT_CLASS(my_file_item_parent_class)->dispose)(obj);
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
	DECLARE_PRIVATE(obj);

	switch (prop_id) {
	case PROP_FILE:
		g_value_set_object(value, priv->file);
		break;

	case PROP_FILE_INFO:
		g_value_set_object(value, priv->file_info);
		break;

	case PROP_ICON:
		{
			GdkPaintable *paintable = NULL;
			GIcon *icon;
			GtkIconTheme *icon_theme;

			if (priv->icon_texture != NULL) {
				paintable = GDK_PAINTABLE(g_object_ref(G_OBJECT(priv->icon_texture)));
			}
			else {
				if (priv->file_info != NULL) {
					icon = g_file_info_get_icon(priv->file_info);
					if (icon != NULL) {
						icon_theme = gtk_icon_theme_get_for_display(gdk_display_get_default());
						paintable = GDK_PAINTABLE(gtk_icon_theme_lookup_by_gicon(icon_theme, icon, ICON_SIZE, 1, GTK_TEXT_DIR_NONE, 0));
					}
				}
			}
			g_value_set_object(value, paintable);
			g_clear_object(&paintable);
		}
		break;

	case PROP_DISPLAY_NAME:
		g_value_set_string(value, (priv->file_info != NULL? g_file_info_get_display_name(priv->file_info): ""));
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
	case PROP_FILE:
		g_clear_object(&priv->file);
		priv->file = g_value_dup_object(value);
		break;

	case PROP_FILE_INFO:
		g_clear_object(&priv->file_info);
		priv->file_info = g_value_dup_object(value);
		g_object_notify_by_pspec(obj, properties[PROP_ICON]);
		g_object_notify_by_pspec(obj, properties[PROP_DISPLAY_NAME]);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
		break;
	}
}

static void my_file_item_class_init(MyFileItemClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->dispose = dispose;
	obj_class->get_property = get_property;
	obj_class->set_property = set_property;

	properties[PROP_FILE] = g_param_spec_object("file", "", "", G_TYPE_FILE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	properties[PROP_FILE_INFO] = g_param_spec_object("file-info", "", "", G_TYPE_FILE_INFO, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	properties[PROP_ICON] = g_param_spec_object("icon", "", "", GDK_TYPE_PAINTABLE, G_PARAM_READABLE);
	properties[PROP_DISPLAY_NAME] = g_param_spec_string("display-name", "", "", "", G_PARAM_READABLE);
	g_object_class_install_properties(obj_class, NUM_PROPERTIES, properties);
}

static void my_file_item_init(MyFileItem *item)
{
	DECLARE_PRIVATE(item);

	priv->file = NULL;
	priv->file_info = NULL;
	priv->icon_texture = NULL;
	priv->loader = NULL;
}

MyFileItem * my_file_item_new(GFile *file, GFileInfo *file_info)
{
	return MY_FILE_ITEM(g_object_new(MY_TYPE_FILE_ITEM, "file", file, "file-info", file_info, NULL));
}

const gchar * my_file_item_get_display_name(MyFileItem *item)
{
	DECLARE_PRIVATE(item);

	return (priv->file_info != NULL? g_file_info_get_display_name(priv->file_info): "");
}

GdkPaintable * my_file_item_get_icon(MyFileItem *item)
{
	GdkPaintable *paintable = NULL;

	g_object_get(G_OBJECT(item), "icon", &paintable, NULL);
	return paintable;
}

void my_file_item_load_icon_texture(MyFileItem *item)
{
	DECLARE_PRIVATE(item);

	if (priv->icon_texture != NULL) {
		return ;
	}
	if (priv->loader != NULL) {
		return ;
	}
	g_return_if_fail(priv->file_info != NULL);
	if (! my_file_item_is_image_file(item)) {
		return ;
	}

	priv->loader = my_image_loader_new(priv->file);
	my_image_loader_load_at_scale_async(priv->loader, ICON_SIZE, ICON_SIZE, NULL, (GAsyncReadyCallback)on_loaded_icon_texture, item);
}

gboolean my_file_item_is_directory(MyFileItem *item)
{
	DECLARE_PRIVATE(item);

	return (priv->file_info != NULL && g_file_info_get_file_type(priv->file_info) == G_FILE_TYPE_DIRECTORY);
}

GFile * my_file_item_get_file(MyFileItem *item)
{
	return PRIVATE(item)->file;
}

gboolean my_file_item_is_image_file(MyFileItem *item)
{
	DECLARE_PRIVATE(item);
	const gchar *content_type;

	if (priv->file_info == NULL) {
		return FALSE;
	}
	content_type = g_file_info_get_content_type(priv->file_info);
	return (content_type != NULL && strncmp(g_content_type_get_mime_type(content_type), "image/", 6) == 0);
}

