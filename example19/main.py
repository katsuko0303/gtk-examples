# coding: utf-8

import sys, os
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, Pango, GObject

# GType参照をしておかないと、window.uiの中で使用している`PangoFontFamily`が「Invalid type」と言われてしまう。
Pango.FontFamily.__gtype__

#----------------------------------------------------------------------
#	window

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'window.ui'))
class Window(Gtk.Window):
	__gtype_name__ = 'MyWindow'

	@GObject.Property(type=Pango.AttrList)
	def label_attributes(self):
		attr_list = Pango.AttrList()
		attr_list.insert(Pango.attr_size_new(Pango.SCALE * 32))
		font_family = self._drop_down.get_selected_item()
		if font_family is not None:
			attr_list.insert(Pango.attr_family_new(font_family.get_name()))
		return attr_list

	_label = Gtk.Template.Child('label')
	_drop_down = Gtk.Template.Child('drop_down')

	def __init__(self, **kwargs):
		super().__init__(**kwargs)
		pctx = self._label.get_pango_context()
		font_map = pctx.get_font_map()
		self._drop_down.set_model(font_map)
		family = pctx.get_font_description().get_family()
		selected = Gtk.INVALID_LIST_POSITION
		for i in range(font_map.get_n_items()):
			if font_map.get_item(i).get_name() == family:
				selected = i
				break
		self._drop_down.set_selected(selected)

	#@Gtk.Template.Callback('get_font_family_label')
	#def _get_font_family_label(self, font_family):
	#	return font_family.get_name()

	@Gtk.Template.Callback('on_drop_down_notify_selected')
	def _on_drop_down_notify_selected(self, obj, pspec):
		self.notify('label-attributes')

#----------------------------------------------------------------------
#	main

def on_activate(app):
	window = Window(application=app)
	window.set_visible(True)

if __name__ == '__main__':
	app = Gtk.Application()
	app.connect('activate', on_activate)
	sys.exit(app.run(sys.argv))

