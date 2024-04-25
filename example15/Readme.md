# GtkLabel�̍��W����e�L�X�g�ʒu���擾

`GtkLabel`�̍��W(�Ⴆ�΁A�}�E�X�ŃN���b�N�����ʒu)����A�����ɏ�����Ă���e�L�X�g�𔲂��o���T���v���ł��B

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

�܂��킩��Α債���b�ł͂���܂��񂪁A

1. `gtk_label_get_layout`���\�b�h�ŁAPango���C�A�E�g���擾
2. `gtk_label_get_layout_offsets`���\�b�h�ŁA�E�B�W�F�b�g���̃��C�A�E�g�̕`��ʒu���擾
3. `pango_layout_xy_to_index`���\�b�h�ŁA���C�A�E�g���̍��W���當���̃C���f�b�N�X���擾

�Ƃ����菇�Ȃ�ł����A���ӂ���_�Ƃ��Ă͈ȉ��̓_�ł��B

* �}�[�N�A�b�v���g���Ă���ꍇ�́A`GtkLable::label`�v���p�e�B�͑������܂񂾃e�L�X�g�Ȃ̂Ŏg��Ȃ��B
* ���C�A�E�g�̈ʒu���l������B(2.�̘b)
* Pango�ł̍��W��`PANGO_SCALE`��P�ʂƂ���B
* `pango_layout_xy_to_index`�Ŏ擾���镶���ʒu��**byte-index**�Ȃ̂ŁAPython�̏ꍇ�͂�������UTF-8�ɕϊ�����B

## ���܂�: GtkLabel::attributes

`GtkLabel::attributes`�v���p�e�B�́A���x���̕����������w�肷��v���p�e�B�A�Ƃ����̂͂Ȃ�ƂȂ��킩��̂ł����A[�}�j���A��](https://docs.gtk.org/gtk4/property.Label.attributes.html)�ɂ͂قƂ�ǐ������Ȃ��āA�ǂ�����Ďw�肷��񂾁H�Ǝv���Ă����̂����ǁA�ǂ����[`pango_attr_list_to_string`](https://docs.gtk.org/Pango/method.AttrList.to_string.html)���\�b�h�̐����ɏ�����Ă���悤�Ɏw�肷��炵���ł��B

```xml
					<object class="GtkLabel">
						<property name="attributes">0 -1 size 24576, 0 -1 weight bold</property>
						<property name="label" bind-source="MyWindow" bind-property="result" />
					</object>
```

�����𕡐��w�肷��ꍇ�ɂ́A�u`,`�v�ŋ�؂�܂��B

��̑����́A�u&lt;�����J�n�ʒu&gt; &lt;�����I���ʒu&gt; &lt;�����^�C�v��&gt; &lt;�����l&gt;�v�Ƃ����悤�Ɏw�肵�܂��B
�����I���ʒu�́A�u-1�v���w�肷��ƁA�����̏I�����Ӗ����܂��B
�����^�C�v���́A`PangoAttrType`�񋓌^��nick_name���w�肵�܂��B

�ŁA�Ō�Ɉ������������͕̂����̃T�C�Y�ł���usize�v�����̎w��̎d���B
���낢��Ƃ���Ă��G���[�ɂȂ�̂����ǁA�ǂ����uxx-large�v�Ƃ��u120%�v�Ȃǂ̎w��͂ł����APango�̒P�ʂł̒l(��̗�ł���u`24576`�v�́u`24 * PANGO_SCALE`�v�̒l�ł�)�łȂ���΂����Ȃ��悤�ł��B

