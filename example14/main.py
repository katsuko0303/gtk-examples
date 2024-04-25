# coding: utf-8

import sys, os
import gi
gi.require_version('Gtk', '4.0')
gi.require_version('WebKit', '6.0')
from gi.repository import Gtk, WebKit, GObject

WebKit.WebView.__gtype__

#----------------------------------------------------------------------
#	window

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'window.ui'))
class Window(Gtk.Window):
	__gtype_name__ = 'MyWindow'

	_web_view = Gtk.Template.Child('web_view')
	_spinner = Gtk.Template.Child('spinner')

	def __init__(self, **kwargs):
		super().__init__(**kwargs)

		self._web_view.load_uri('https://www.gtk.org/')

	@Gtk.Template.Callback('on_web_view_notify_is_loading')
	def _on_web_view_notify_is_loading(self, obj, pspec):
		if obj.is_loading():
			self._spinner.start()
		else:
			self._spinner.stop()

#----------------------------------------------------------------------
#	main

def on_activate(app):
	window = Window(application=app)
	window.set_visible(True)

if __name__ == '__main__':
	app = Gtk.Application()
	app.connect('activate', on_activate)
	sys.exit(app.run(sys.argv))

