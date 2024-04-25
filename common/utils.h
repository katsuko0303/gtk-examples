#pragma once
#include <glib-object.h>

G_BEGIN_DECLS

typedef	struct _UtilFileInfo {
	const char *file;
	int line;
}	UtilFileInfo;

/**
 *	@brief オブジェクトの解放チェック
 *	@param obj GObject
 *
 *	指定されたGObjectの参照が全て無くなったら、メッセージを表示します。
 */
#define	CHECK_OBJECT_UNREF(obj)	\
	{\
		static UtilFileInfo fi = { __FILE__, __LINE__ };\
		g_object_weak_ref(G_OBJECT(obj), (GWeakNotify)_util_notify_weak, &fi);\
	}

static void _util_notify_weak(gpointer data, GObject *obj)
{
	const UtilFileInfo *fi = (UtilFileInfo *)data;
	g_print("%s(%d): notify weak %s.\n", fi->file, fi->line, G_OBJECT_TYPE_NAME(obj));
}

G_END_DECLS


