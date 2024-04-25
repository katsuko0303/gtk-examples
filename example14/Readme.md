## WebKitGTKについて

https://webkitgtk.org/

公式サイトのDocumentationには、[StableバージョンのWebKit2GTK](https://webkitgtk.org/reference/webkit2gtk/stable/index.html)と、[UnstableバージョンのWebKitGTK]((https://webkitgtk.org/reference/webkitgtk/unstable/index.html)が存在します。(2024/04現在)

「WebKit2GTK」は「2」というからにはこちらが新しいのだろう、とか、Stableバージョンなんだから安定しているだろう、と思ってしまいますが、**これはGTK3向けのバージョン**です。
GTK4であれば、Unstableバージョンの「WebKitGTK-6.0」を使います。

## WebKitWebViewがuiで認識されない。

`window.c`の

```c
static void my_window_class_init(MyWindowClass *klass)
{
	webkit_web_view_get_type();
}
```

と、`main.py`の

```python
WebKit.WebView.__gtype__
```

について、これだけ見ると無意味のコードに見えますが、このコードを消すと、

```
example14:10883): Gtk-CRITICAL **: 10:53:57.518: Error building template class 'MyWindow' for an instance of type 'MyWindow': .:0:0 Invalid object type 'WebKitWebView'
```

というエラーが出てしまいます。

UIファイル内でクラス名を参照する際には、あらかじめクラス名を登録しておく必要があります。
大抵の場合、クラスの`_get_type`のサフィックス名のついた関数か、`_TYPE_`の名のついたマクロを一度でも呼べばOKです。(`MyWindow'であれば、`my_window_get_type`か`MY_TYPE_WINDOW`になります。)
要は`GType`を取得すればいいので、Pythonの場合、`__gtype__`属性を参照すればいいでしょう。


