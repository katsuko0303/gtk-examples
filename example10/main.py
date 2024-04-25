# coding: utf-8

import sys, os, traceback
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, GObject, GLib

#----------------------------------------------------------------------
#	window

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'window.ui'))
class Window(Gtk.Window):
	__gtype_name__ = 'MyWindow'

	_buffer = Gtk.Template.Child('buffer')

	@Gtk.Template.Callback('on_open')
	def _on_open(self, button):
		dialog = Gtk.FileDialog(modal=True)
		dialog.open(self, None, self._on_select_file)

	def _on_select_file(self, source, result):
		try:
			file = source.open_finish(result)
		except GLib.Error as e:
			return

		file.load_contents_async(None, self._on_load_contents)

	def _on_load_contents(self, source, result):
		try:
			ret, contents, etag = source.load_contents_finish(result)
		except GLib.Error as e:
			traceback.print_exc()
			return
		if ret:
			self._buffer.set_text(contents.decode('utf-8'))

#----------------------------------------------------------------------
#	main

def on_activate(app):
	window = Window(application=app)
	window.set_visible(True)

if __name__ == '__main__':
	app = Gtk.Application()
	app.connect('activate', on_activate)
	sys.exit(app.run(sys.argv))

