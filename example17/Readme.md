# スタイルシートの動的変更

Example16では「GtkLabelの文字装飾」をテーマにしましたが、では背景色は？というのが今回のテーマの始まりです。

Pangoの属性にbackgroundがあるのですが、これはspan単位、つまり文字の背景のみで、例えば改行などして各行でで文字数が違うと、改行した後の背景色が変わりません。
そうではなくて、ブロック単位で背景色を変えたいんです。

色々調べたのですが、どうもスタイルシート変えるほかなさそうだ、という結論にいたり、今回のサンプルになりました。

まぁそれほど難しい話ではなく、`gtk_style_context_add_provider_for_display`で追加したのだから、`gtk_style_context_remove_provider_for_display`で削除して追加しなおせばいいだけです。
ただ、既に作られたウィジェットに即時に適用されるのかな、という点が懸念点だったのですが、試してみると問題なさそうでした。
