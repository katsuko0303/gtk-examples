# coding: utf-8

import sys, os
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, GObject, Gdk, GLib

#----------------------------------------------------------------------
#	window

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'window.ui'))
class Window(Gtk.Window):
	__gtype_name__ = 'MyWindow'

	@GObject.Property(type=Gdk.RGBA, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.EXPLICIT_NOTIFY))
	def label_background(self):
		return self.get_application().label_background
	@label_background.setter
	def label_background(self, value):
		label_background = self.get_application().label_background
		if not Gdk.RGBA.equal(label_background, value):
			self.get_application().label_background = value

	def __init__(self, **kwargs):
		super().__init__(**kwargs)

		self.notify('label-background')

#----------------------------------------------------------------------
#	application

class Application(Gtk.Application):
	__gtype_name__ = 'MyApplication'

	@GObject.Property(type=Gdk.RGBA)
	def label_background(self):
		if not hasattr(self, '_label_background'):
			rgba = Gdk.RGBA()
			rgba.red, rgba.green, rgba.blue, rgba.alpha = 0.0, 0.0, 0.0, 0.0
			return rgba
		return self._label_background
	@label_background.setter
	def label_background(self, value):
		self._label_background = value
		self._update_style_sheet()

	def __init__(self, **kwargs):
		self._css_provider = None
		super().__init__(**kwargs)

	def do_startup(self):
		Gtk.Application.do_startup(self)
		self._update_style_sheet()

	def do_activate(self):
		window = Window(application=self)
		window.set_visible(True)

	def _update_style_sheet(self):
		if self._css_provider is not None:
			Gtk.StyleContext.remove_provider_for_display(Gdk.Display.get_default(), self._css_provider)
		self._css_provider = Gtk.CssProvider()
		self._css_provider.load_from_bytes(GLib.Bytes.new(f'.custom_label {{ background-color: {self.label_background.to_string()}; }}'.encode('utf-8')))
		Gtk.StyleContext.add_provider_for_display(Gdk.Display.get_default(), self._css_provider, Gtk.STYLE_PROVIDER_PRIORITY_FALLBACK)

#----------------------------------------------------------------------
#	main

if __name__ == '__main__':
	app = Application()
	sys.exit(app.run(sys.argv))

