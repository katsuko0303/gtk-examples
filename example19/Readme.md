# GtkDropDownサンプル

フォントファミリを選択して、選択されたフォントをラベルに反映させるサンプルです。

## フォントについて

使えるフォントファミリの一覧の取得は、`PangoFontMap`オブジェクトから取得します。
`PangoFontMap`は、`PangoFontFamily`を要素とした`GListModel`インターフェイスのオブジェクトなので、`GListModel`のインターフェイスに従ってフォントファミリを取得します。

では`PangoFontMap`オブジェクトはどこから持ってくるか、ですが、使用するウィジェットに対して`gtk_widget_get_pango_context`を呼び出して`PangoContext`オブジェクトを取得し、更に`pango_context_get_font_map`を呼び出して`PangoFontMap`オブジェクトを取得します。
なお、ウィジェットに依存したくない場合には、`pango_cairo_font_map_get_default`を呼び出せばいいようです。

## ドロップダウンについて

GTK4.10より、`GtkComboBox`がdeprecatedになり、代わりに`GtkDropDown`を使用するようになります。

選択項目の一覧は、`GtkDropDown.model`プロパティに`GListModel`インターフェイスを実装したオブジェクトを指定します。
先程述べたとおり、`PangoFontMap`は`GListModel`インターフェイスを実装しているので、そのまま渡します。

`PangoFontMap`の各要素の型は`PangoFontFamily`ですが、UIで表示するために文字列にしなければならないので、`GtkDropDown.expression`プロパティで`PangoFontFamily`から文字列に変換する式を指定します。
サンプルでは、`PangoFontFamily.name`プロパティを参照するようにしています。

なおPythonでは、uiファイルを読み込む前に、

```python
# GType参照をしておかないと、window.uiの中で使用している`PangoFontFamily`が「Invalid type」と言われてしまう。
Pango.FontFamily.__gtype__
```

と、`PangoFontFamily`の`GType`を参照しておいて、`PangoFontFamily`の型を認識しておく必要があります。

