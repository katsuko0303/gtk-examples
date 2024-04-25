# coding: utf-8

import sys, os
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, GObject, Gio, GLib

#----------------------------------------------------------------------
#	Window

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'window.ui'))
class MyWindow(Gtk.ApplicationWindow):
	__gtype_name__ = 'MyWindow'

	@GObject.Signal(flags=GObject.SignalFlags.RUN_FIRST)
	def new_tab(self):
		builder = Gtk.Builder()
		builder.add_from_file(os.path.join(os.path.dirname(__file__), 'page.ui'))

		child = builder.get_object('page')
		child.set_label(GLib.DateTime.new_now_local().format('created at %c'))

		tab_label = Gtk.Label(label=f'Page {1 + self._page_count}')

		page_index = self._notebook.append_page(child, tab_label)
		self._notebook.set_current_page(page_index)

		self._page_count += 1

	_notebook = Gtk.Template.Child('notebook')

	def __init__(self, **kwargs):
		self._page_count = 0
		super().__init__(**kwargs)

		self.add_action_entries([
			('new_tab', self._on_new_tab, None, None, None),
			('close', self._on_close, None, None, None),
			])

		self.emit('new-tab')

	def _on_new_tab(self, action, parameter, *args):
		self.emit('new-tab')

	def _on_close(self, action, parameter, *args):
		self.close()

	@Gtk.Template.Callback('on_new_tab_clicked')
	def _on_new_tab_clicked(self, button):
		self.emit('new-tab')

	@Gtk.Template.Callback('on_new_window_clicked')
	def _on_new_window_clicked(self, button):
		self.get_application().activate_action('new_window', None)

#----------------------------------------------------------------------
#	Application

class MyApplication(Gtk.Application):
	__gtype_name__ = 'MyApplication'

	@GObject.Signal(flags=GObject.SignalFlags.RUN_FIRST)
	def new_window(self):
		window = MyWindow(application=self)
		window.set_visible(True)

	def __init__(self, **kwargs):
		super().__init__(application_id='com.example.example01', **kwargs)

		for name, callback in [
				('quit', self._on_quit),
				('new_window', self._on_new_window),
				]:
			action = Gio.SimpleAction.new(name, None)
			action.connect('activate', callback)
			self.add_action(action)

	def do_activate(self):
		self.emit('new-window')

	def _on_quit(self, action, parameter, *args):
		self.quit()

	def _on_new_window(self, action, parameter, *args):
		self.emit('new-window')

#----------------------------------------------------------------------
#	main

if __name__ == '__main__':
	app = MyApplication()
	sys.exit(app.run(sys.argv))

