# GtkLabelの座標からテキスト位置を取得

`GtkLabel`の座標(例えば、マウスでクリックした位置)から、そこに書かれているテキストを抜き出すサンプルです。

```python
# coding: utf-8

import sys, os
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, Pango, GObject

#----------------------------------------------------------------------
#	window

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'window.ui'))
class Window(Gtk.Window):
	__gtype_name__ = 'MyWindow'

	@GObject.Property(type=GObject.TYPE_STRING)
	def result(self):
		return self._result if hasattr(self, '_result') else ''
	@result.setter
	def result(self, value):
		self._result = value

	@Gtk.Template.Callback('on_released')
	def _on_released(self, gesture, n_press, x, y):
		label = gesture.get_widget()
		layout = label.get_layout()
		lx, ly = label.get_layout_offsets()
		inside, index, trailing = layout.xy_to_index(Pango.SCALE * (x - lx), Pango.SCALE * (y - ly))
		if inside:
			self.result = f'{index}: {repr(layout.get_text().encode("utf-8")[index:].decode("utf-8"))}'
		else:
			self.result = '(clicked outside)'

#----------------------------------------------------------------------
#	main

def on_activate(app):
	window = Window(application=app)
	window.set_visible(True)

if __name__ == '__main__':
	app = Gtk.Application()
	app.connect('activate', on_activate)
	sys.exit(app.run(sys.argv))

```

まぁわかれば大した話ではありませんが、

1. `gtk_label_get_layout`メソッドで、Pangoレイアウトを取得
2. `gtk_label_get_layout_offsets`メソッドで、ウィジェット内のレイアウトの描画位置を取得
3. `pango_layout_xy_to_index`メソッドで、レイアウト内の座標から文字のインデックスを取得

という手順なんですが、注意する点としては以下の点です。

* マークアップを使っている場合は、`GtkLable::label`プロパティは属性を含んだテキストなので使わない。
* レイアウトの位置を考慮する。(2.の話)
* Pangoでの座標は`PANGO_SCALE`を単位とする。
* `pango_layout_xy_to_index`で取得する文字位置は**byte-index**なので、Pythonの場合はいったんUTF-8に変換する。

## おまけ: GtkLabel::attributes

`GtkLabel::attributes`プロパティは、ラベルの文字属性を指定するプロパティ、というのはなんとなくわかるのですが、[マニュアル](https://docs.gtk.org/gtk4/property.Label.attributes.html)にはほとんど説明がなくて、どうやって指定するんだ？と思っていたのだけど、どうやら[`pango_attr_list_to_string`](https://docs.gtk.org/Pango/method.AttrList.to_string.html)メソッドの説明に書かれているように指定するらしいです。

```xml
					<object class="GtkLabel">
						<property name="attributes">0 -1 size 24576, 0 -1 weight bold</property>
						<property name="label" bind-source="MyWindow" bind-property="result" />
					</object>
```

属性を複数指定する場合には、「`,`」で区切ります。

一つの属性は、「&lt;文字開始位置&gt; &lt;文字終了位置&gt; &lt;属性タイプ名&gt; &lt;属性値&gt;」というように指定します。
文字終了位置は、「-1」を指定すると、文字の終わりを意味します。
属性タイプ名は、`PangoAttrType`列挙型のnick_nameを指定します。

で、最後に引っかかったのは文字のサイズである「size」属性の指定の仕方。
いろいろとやってもエラーになるのだけど、どうやら「xx-large」とか「120%」などの指定はできず、Pangoの単位での値(上の例である「`24576`」は「`24 * PANGO_SCALE`」の値です)でなければいけないようです。

