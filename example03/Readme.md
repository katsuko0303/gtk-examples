# ストップウォッチサンプル

ストップウォッチのサンプルです。
このサンプルの要点は、`GdkPaintable`の使い方です。

`GdkPaintable`とは、GTK4で追加されたもので、描画処理を抽象化したインターフェイスです。
従来`GtkImage`などで`GdkPixbuf`など画像を渡して表示していましたが、`GdkPaintable`を実装したオブジェクトを渡せることによって、もっと柔軟な描画処理を行うことができるようになりました。

なお、`GdkPaintable`はインターフェイスなので、実態はありません。
サンプルのように自分で実態を定義するか、もし単純に画像を扱うのであれば`GdkTexture`というクラスを使います。

## GdkPaintable

`GdkPaintable`インターフェイスを実装する場合には、以下のメソッドを実装します。

### get_flags

`GdkPaintable`の動作を示す`GdkPaintableFlags`フラグを返します。

* `GDK_PAINTABLE_STATIC_SIZE` - サイズが固定していることを示します。
* `GDK_PAINTABLE_STATIC_CONTENTS` - 描画内容が固定していることを示します。

例えばアニメーション描画する際には、**`GDK_PAINTABLE_STATIC_CONTENTS`を指定してはいけません**。

### snapshot

実際の描画処理を行うメソッドです。
引数として、スナップショットオブジェクトと描画対象の幅と高さが渡されます。

スナップショットオブジェクトの型は、マニュアルでは`GdkSnapshot`となっていますが、GTKで使う分には`GtkSnapshot`として扱って問題ないようです。

### get_intrinsic_width / get_intrinsic_height

描画対象の「好ましい」サイズを返します。
「好ましい」なので、必ずしもこのサイズになるとは限りませんが、このサイズを元に実際のサイズが求められます。
もし特に指定がないのであれば、0を返します。

## GtkImageとGtkPicture

従来よりイメージを表示するウィジェットとして`GtkImage`がありましたが、GTK4で`GtkPicture`というウィジェットが追加されました。
`GtkImage`は主にアイコンなど、サイズがUIで決められているもの、`GtkPicture`は画像など、プログラムで変化があるもの、というような切り分けがあるようです。

## gtk_widget_add_tick_callback

従来より繰り返し処理を行う処理を行う際に、`g_idle_add`や`g_timeout_add`などの関数がありましたが、GTK4では`gtk_widget_add_tick_callback`というメソッドが追加されました。
アニメーション処理を行うために適しているタイミングでコールバックを呼び出すようです。

