# ダイアログサンプル

GTK4.10では、`GtkDialog`がdeprecatedになりました。
…、別にすぐになくなるわけではないですが、ダイアログってGUIにおいてそれなりに重要な要素だと思うのですけどねぇ。
どのような意図があるのかわからないのですが、しかしマニュアルには

> Use GtkWindow instead.

としか書かれてないんで、代わりにどのような対処を行えばいいのかわからないんですよねぇ…。

とりあえず従来のダイアログの動作をする`GtkWindow`を作ろう、というのが、今回の目的です。

## responseの代わりは？

GTK4になった時点で`gtk_dialog_run`が無くなったので、代わりに`GtkDialog::response`シグナルを受け取って処理する、という対処を行いましたが、今回はそれすらも使えないわけです。
代わりにどうしたらいいか、ここは4.10で追加された`GtkFileDialog`を参考にしましょう。

`GtkFileDialog`でファイルを選択するには、以下のような手順になります。

1. `GtkFileDialog`のオブジェクトを作成する。
2. `open`メソッドを呼んで、ダイアログを表示する。
3. ダイアログが閉じられると`open`メソッドで指定したコールバックが呼ばれるので、その中で`open_finish`メソッドを呼んで結果を受け取る。

これと同じ処理を行います。

```python
	def _on_clicked(self, button):
		dialog = NameDialog(modal=True)
		dialog.ask(self, None, self._on_ask_name, button)

	def _on_ask_name(self, source, result, button):
		try:
			name = source.ask_finish(result)
			button.set_label(f'My name is {name}.')
		except GLib.Error as e:
			return
```

さて、この`ask`メソッドと`ask_finish`メソッドの中身はどうなっているか、です。
まずは、`ask`メソッド。

```c
void my_name_dialog_ask(MyNameDialog *dialog, GtkWindow *parent, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer cb_data)
{
	DECLARE_PRIVATE(dialog);

	gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
	if (priv->task == NULL) {
		priv->task = g_task_new(G_OBJECT(dialog), cancellable, callback, cb_data);
	}
	gtk_widget_set_visible(GTK_WIDGET(dialog), TRUE);
}
```

```python
	def ask(self, parent, cancellable, callback, *args):
		self.set_transient_for(parent)
		self._task = Gio.Task.new(self, cancellable, callback, *args)
		self.set_visible(True)
```

メソッドの中で、`GTask`(`Gio.Task`)クラスのオブジェクトを作成します。
`new`メソッドの引数に、渡されてきたキャンセラブルオブジェクト、コールバックとデータをそのまま渡します。

`ask_finish`メソッドは、以下のようになってます。

```c
gchar * my_name_dialog_ask_finish(MyNameDialog *dialog, GAsyncResult *result, GError **error)
{
	GValue value = { 0 };
	gchar *name = NULL;
	gboolean ret;

	ret = g_task_propagate_value(G_TASK(result), &value, error);
	if (ret) {
		name = g_value_dup_string(&value);
		g_value_reset(&value);
	}
	return name;
}
```

```python
	def ask_finish(self, result):
		ret, obj = result.propagate_value()
		return obj if ret else None
```

コールバックで渡されてきた`GAsyncResult`のオブジェクトは、実際は`ask`メソッドの中で作成された`GTask`オブジェクトです。
このオブジェクトに対して`propagate_value`メソッドを呼び出すことによって、後ほど説明するダイアログ内で設定した結果を受け取ることができます。

## OKボタンの代わりは？

OKボタンの処理、つまり入力を受け付けた際の処理です。
サンプルでは、以下のようになっています。

```c
static void on_ok(GtkButton *button, gpointer cb_data)
{
	MyNameDialog *dialog = MY_NAME_DIALOG(cb_data);
	DECLARE_PRIVATE(dialog);
	GValue value = { 0 };

	if (priv->task != NULL) {
		g_value_init(&value, G_TYPE_STRING);
		g_value_set_string(&value, priv->name);
		g_task_return_value(priv->task, &value);
		g_value_reset(&value);
		g_clear_object(&priv->task);
	}
	gtk_window_destroy(GTK_WINDOW(dialog));
}
```

```python
	@Gtk.Template.Callback('on_ok')
	def _on_ok(self, button):
		if self._task is not None:
			value = GObject.Value(value_type=GObject.TYPE_STRING)
			value.set_string(self.name)
			self._task.return_value(value)
			self._task = None
		self.destroy()
```

`ask`メソッドで作成した`GTask`オブジェクトに対して、`return_value`メソッドを呼んで入力結果を渡します。
ここで渡したデータが、先ほど説明した`propagate_value`メソッドの戻り値になるわけです。

また、このようなボタンは従来のダイアログのUI定義では、

```xml
    <template class="MyNameDialog">
        <action-widgets>
            <action-widget response="ok" default="true">ok_button</action-widget>
        </action-widgets>
    </template>
```

というように、ボタンがデフォルトの処理として定義しましたが、同様の処理を行うように、

```xml
    <template class="MyNameDialog">
        <property name="default-widget">ok_button</property>
    </template>
```

と定義します。また、見栄えも同様に、

```xml
					<object class="GtkButton" id="ok_button">
						<property name="label">OK</property>
						<style>
							<class name="suggested-action" />
						</style>
					</object>
```

というように、スタイルのクラス名を指定しておきます。

## キャンセル処理の代わりは？

キャンセル処理、と言っても、ユーザの操作としては、

* キャンセルボタンを押す。
* エスケープキーを押す。
* ウィンドウを閉じる。(一般的なウィンドウシステムであればALT+F4を押す、など)

というパターンがあります。

まずはどのパターンでも同様の処理を行うように、cancelシグナルを定義し、そのシグナルで以下のような処理を行うようにします。

```c
	if (priv->task != NULL) {
		g_task_return_error(priv->task, g_error_new_literal(gtk_dialog_error_quark(), GTK_DIALOG_ERROR_DISMISSED, "Dismissed by user"));
		g_clear_object(&priv->task);
	}
	gtk_window_destroy(GTK_WINDOW(dialog));
```

```python
		if self._task is not None:
			self._task.return_error(GLib.Error.new_literal(Gtk.DialogError.quark(), 'Dismissed by user', Gtk.DialogError.DISMISSED))
			self._task = None
		self.destroy()
```

`GTask`オブジェクトに対して`return_error`メソッドを呼ぶことによって、`propagate_value`メソッドが呼ばれたらエラーを起こすようにします。

次に、各操作に対する処理です。
キャンセルボタンは、普通に`GtkButton::clicked`シグナルを受け取って、先程の`cancel`シグナルを発生させます。

ウィンドウを閉じる処理ですが、`GtkWindow`の`close_request`メソッドをオーバーライドして、そこでシグナルを発生させます。

```c
static gboolean close_request(GtkWindow *window)
{
	g_signal_emit(G_OBJECT(window), signal_ids[SIGNAL_CANCEL], 0);
	return FALSE;
}

static void my_name_dialog_class_init(MyNameDialogClass *klass)
{
	GtkWindowClass *window_class = GTK_WINDOW_CLASS(klass);

	window_class->close_request = close_request;
}
```

```python
class NameDialog(Gtk.Window):
	def do_close_request(self):
		self.emit('cancel')
		return False
```

エスケープキーの処理ですが、こちらはちょっと複雑です。
GTK4では、**ダイアログのクラスに対して**ショートカットを登録します。

```c
static void my_name_dialog_class_init(MyNameDialogClass *klass)
{
	trigger = gtk_keyval_trigger_new(GDK_KEY_Escape, 0);
	action = gtk_signal_action_new("cancel");
	gtk_widget_class_add_shortcut(widget_class, gtk_shortcut_new(trigger, action));
}
```

```python
class NameDialog(Gtk.Window):
    pass
NameDialog.add_shortcut(Gtk.Shortcut(trigger=Gtk.KeyvalTrigger(keyval=Gdk.KEY_Escape, modifiers=0), action=Gtk.SignalAction(signal_name='cancel')))
```

なお、ここで`GtkSignalAction`で`cancel`シグナルを発生させるために、`cancel`シグナルの作成時にフラグに`G_SIGNAL_ACTION`を指定する必要があります。

```c
	signal_ids[SIGNAL_CANCEL] = g_signal_newv("cancel", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			closure, NULL, NULL, NULL, G_TYPE_NONE, 0, NULL);
```

```python
	@GObject.Signal(flags=(GObject.SignalFlags.RUN_FIRST | GObject.SignalFlags.ACTION))
	def cancel(self):
        pass
```

