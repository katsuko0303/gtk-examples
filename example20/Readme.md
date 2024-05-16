# ドロップダウンサンプル

GTK4.10より、`GtkComboBox`がdeprecatedになり、代わりに`GtkDropDown`を使用するよう促されています。
というわけで、4つのサンプルを用意しました。

更に、uiファイルを使用したパターンと、使用しないパターンも用意しました。
実行時に「`noui`」という引数を指定すると、使用しないパターンのコードが動作します。

## シンプルな文字列のリスト

`GtkComboBoxText`のようなものです。

```xml
	<object class="GtkDropDown">
		<property name="model">
			<object class="GtkStringList">
				<items>
					<item>Hoge</item>
					<item>Piyo</item>
					<item>Fuga</item>
				</items>
			</object>
		</property>
	</object>
```

重要なのは、`GtkDropDown.model`プロパティです。
これは、選択項目の一覧を示す`Gio.ListModel`インターフェイスを実装したオブジェクトです。
`Gio.ListModel`インターフェイスは、項目数を取得する`get_n_items`メソッド、インデックスに対する項目のオブジェクトを取得する`get_item`メソッドがあります。

今回のように文字列を扱うだけであれば、`Gtk.StringList`というクラスが用意されています。

なお、コードでドロップダウンウィジェットを作成するサンプルでは、

```python
		model = Gtk.StringList.new(['Hoge', 'Piyo', 'Fuga'])
		drop_down = Gtk.DropDown(model=model)
```

としていますが、一括で処理する`Gtk.DropDown.new_from_strings`というメソッドもあります。

## 自分で用意した項目クラスを使用するリスト

単純な文字列ではなく、複雑な構造のデータを項目として扱うのであれば、`GObject`を派生したクラスを自分で用意します。

```python
class ListItem(GObject.GObject):
	__gtype_name__ = 'MyListItem'

	@GObject.Property(type=GObject.TYPE_INT)
	def value(self):
		return self._value if hasattr(self, '_value') else 0
	@value.setter
	def value(self, v):
		self._value = v
		self.notify('label')

	@GObject.Property(type=GObject.TYPE_STRING)
	def label(self):
		return f'Item {self.value}'
```

このサンプルでは、データとしては整数値を持ち(それを`value`プロパティとして定義し)、実際にUIに表示する文字列を`label`プロパティとしました。

`Gtk.DropDown.model`プロパティには`Gio.ListModel`インターフェイスを実装したオブジェクトですが、このような汎用的なオブジェクトを管理するリストクラスが、`Gio.ListStore`というクラスとして用意されています。

```python
	self._custom_item_list_model = Gio.ListStore(item_type=ListItem)
	for i in range(5):
		item = ListItem(value=i)
		self._custom_item_list_model.append(item)
```

また、アイテムの型は自分で用意した`ListItem`クラスですが、UIで表示するには文字列にしなければなりません。
先ほど説明したとおり、今回は`ListItem.label`プロパティにそれが設定されているので、`Gtk.DropDown`でプロパティの値を見るように、`Gtk.DropDown.expression`プロパティで設定します。

```xml
	<object class="GtkDropDown">
		<property name="expression">
			<lookup name="label" type="MyListItem" />
		</property>
	</object>
```

## ディレクトリリスト

特定のディレクトリ以下のファイル一覧を管理するリストクラスとして、`Gtk.DirectoryList`というクラスが用意されています。
おおよそ以下のようにファイル情報を取得し、`Gio.FileInfo`クラスの項目として管理するリストオブジェクトです。
(実際には非同期で処理されるはずなので、多少違ってます。)

```python
	enumerator = file.enumerate_children(attributes, Gio.FileQueryInfoFlags.NONE, None)
	while True:
		file_info = enumerator.next_file(None)
		if file_info is None:
			break
		append_to_list(file_info)
```

上記のコードの「`file`」「`attributes`」に相当するものは、それぞれ`Gtk.DirectoryList`のプロパティに指定します。

今回のサンプルでは、`Gio.FileInfo.get_display_name`メソッドで取得するファイルの表示名をUIに表示します。
マニュアルには、属性に「`G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME`」(実態は、「`standard::display-name`」という文字列)を指定しないとエラーになる、と書かれているので、`Gtk.DirectoryList.attributes`プロパティに指定します。

`Gtk.DropDown`で、アイテムから表示文字列に変換するために、`expression`プロパティを設定します。
今回はアイテムクラスのプロパティではないので、

```xml
	<object class="GtkDropDown">
		<property name="expression">
			<closure type="gchararray" function="get_display_name" />
		</property>
	</object>
```

というように、`Gtk.ClosureExpression`でメソッドを呼び出すようにし、

```python
@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'window.ui'))
class Window(Gtk.Window, Gtk.BuilderScope):
	__gtype_name__ = 'MyWindow'

	@Gtk.Template.Callback('get_display_name')
	def _get_display_name(self, obj):
		return obj.get_display_name()
```

というように、コードで呼び出されるメソッドを用意し、その中でアイテムに対して`Gio.FileInfo.get_display_name`メソッドを呼び出します。

## ファクトリを使ったリスト

今までのように、UIに単純に文字列を表示するものであれば、`Gtk.DropDown.expression`プロパティを使えばいいですが、もう少し凝ったUIにする場合には、`Gtk.DropDown.factory`プロパティでファクトリを指定します。

今回のサンプルでは、以前使用したカスタムリストアイテム('ListItem')を使用し、`label`プロパティだけでなく`value`プロパティの値も表示しています。

`factory`プロパティには、UIファイルを使用してUIを構築する`Gtk.BuilderListItemFactory`か、コードでUIを構築する`Gtk.SignalListItemFactory`が用意されています。
今回のサンプルでは、UIファイルを使うパターンで`Gtk.BuilderListItemFactory`を、UIファイルを使わないパターンで`Gtk.SignalListItemFactory`を使用しています。
もちろん、そのようにしなければならない決まりはなく、UIファイルの中で`Gtk.SignalListItemFactory`を使ってもいいし、その逆も問題ありません。

### GtkBuilderListItemFactory

`Gtk.BuilderListItemFactory`では、`bytes`プロパティか`resource`プロパティでUIの定義を指定します。
`bytes`プロパティはUI定義を示すテキストを、`resource`プロパティはUI定義を示すファイルのリソース内のパスを指定します。

今回のサンプルでは、

```xml
	<object class="GtkBuilderListItemFactory">
		<property name="bytes"><![CDATA[
<interface>
	<template class="GtkListItem">
		<property name="child">
			<object class="GtkBox">
				<property name="orientation">horizontal</property>
				<property name="spacing">8</property>
				<child>
					<object class="GtkLabel" id="label">
						<property name="attributes">0 -1 weight bold</property>
						<binding name="label">
							<lookup name="label" type="MyListItem">
								<lookup name="item">GtkListItem</lookup>
							</lookup>
						</binding>
					</object>
				</child>
				<child>
					<object class="GtkSeparator">
						<property name="orientation">vertical</property>
					</object>
				</child>
				<child>
					<object class="GtkLabel" id="value">
						<binding name="label">
							<lookup name="value" type="MyListItem">
								<lookup name="item">GtkListItem</lookup>
							</lookup>
						</binding>
					</object>
				</child>
			</object>
		</property>
	</template>
</interface>
		]]></property>
	</object>
```

というように、UIファイルの中にUI定義を埋め込む形になっていますが、管理としてはあまり良い方法ではないと思いますので、できればリソースとして管理した方がいいでしょう。

リスト項目のUIファイルの内容ですが、

```xml
<interface>
	<template class="GtkListItem">
		<property name="child">
		
		<!-- ここに、リスト項目のウィジェットを記述する -->
		
		</property>
	</template>
</interface>
```

ここまではテンプレートだと思ってください。
(説明が長くなるので、省略します)

ウィジェットとリスト項目との関連付けですが、

```xml
	<object class="GtkLabel" id="label">
		<binding name="label">
			<lookup name="label" type="MyListItem">
				<lookup name="item">GtkListItem</lookup>
			</lookup>
		</binding>
	</object>
```

というように記述すると、`Gtk.ListItem`オブジェクトの`item`プロパティの値を取得し、更にその値の`MyListItem.label`プロパティの値を、`Gtk.Label.label`プロパティに設定することになります。
コードで記述すると、以下のようなコードになります。

```python
    def bind(label: Gtk.Label, list_item: Gtk.ListItem):
        label.set_label(list_item.get_item().label)
```

(`Gtk.ListItem`に関しては、次の`GtkSignalListItemFactory`の説明もご覧ください)

## GtkSignalListItemFactory

`Gtk.SignalListItemFactory`では、`setup`と`bind`というシグナルをハンドリングすることによって、UIを構築します。

`setup`シグナルでは、ウィジェットを作成します。
引数に`Gtk.ListItem`のオブジェクトが渡されてくるので、`Gtk.ListItem.child`プロパティに作成したウィジェットを設定します。(`Gtk.ListItem.set_child`メソッドを使用するといいでしょう)

`bind`シグナルでは、ウィジェットとリスト項目の関連付け(リスト項目の内容をウィジェットに反映させる)を行います。
`setup`シグナルと同様に引数に`Gtk.ListItem`のオブジェクトが渡されてくるので、`Gtk.ListItem.get_child`メソッドで`setup`シグナルで作成したウィジェットを、`Gtk.ListItem.get_item`メソッドでリスト項目のオブジェクトを取得し、処理します。

```python
	def _on_factory_setup(self, factory, list_item):
		hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)

		attributes = Pango.AttrList()
		attributes.insert(Pango.attr_weight_new(Pango.Weight.BOLD))
		label = Gtk.Label(attributes=attributes)
		hbox.append(label)
		hbox.label = label

		separator = Gtk.Separator(orientation=Gtk.Orientation.VERTICAL)
		hbox.append(separator)

		label = Gtk.Label()
		hbox.append(label)
		hbox.value = label

		list_item.set_child(hbox)

	def _on_factory_bind(self, factory, list_item):
		child = list_item.get_child()
		item = list_item.get_item()

		child.label.set_label(item.label)
		child.value.set_label(str(item.value))

	factory = Gtk.SignalListItemFactory()
	factory.connect('setup', on_factory_setup)
	factory.connect('bind', on_factory_bind)
```

## その他、知っておいた方がいいこと。

今回のサンプルでは書かれていませんが、知っておいた方がいいことです。

### 選択項目の取得・設定

現在何が選択されているか、`Gtk.DropDown.selected`プロパティを参照します。(`Gtk.DropDown.get_selected`メソッドで取得できます)
このプロパティには、選択されている項目のインデックスが設定されています。

`Gtk.DropDown.set_selected`メソッドで、選択項目を設定することもできます。

何も選択されていない状態は、インデックス値は`Gtk.INVALID_LIST_POSITION`になります。

インデックスが取得できれば、`Gtk.DropDown.get_model`メソッドでモデルを取得し、`Gio.ListModel.get_item`メソッドで項目オブジェクトを取得できますが、`Gtk.DropDown.get_selected_item`メソッドで一括で処理することもできます。

### 選択項目の変更通知

`Gtk.ComboBox`では、`changed`シグナルで選択項目が変更されたことを知ることができましたが、`Gtk.DropDown`自体にはそのようなシグナルは存在しません。

代わりに、`notify::selected`シグナルで`selected`プロパティの値が変更されたことで知ることになります。

```python
	drop_down = Gtk.DropDown()
	def callback(obj, pspec):
		print(f'changed to {obj.get_selected()}.')
	drop_down.connect('notify::selected', callback)
```

