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
			self.result = f'{index}: {repr(layout.get_text().encode("utf-8")[index:].decode("utf-8")[0])}'
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

