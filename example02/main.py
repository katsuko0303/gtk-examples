# coding: utf-8

import sys, os
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, GObject, Gio

#----------------------------------------------------------------------
#	Window

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'window.ui'))
class MyWindow(Gtk.Window):
	__gtype_name__ = 'MyWindow'

	@GObject.Property(type=Gio.File)
	def current_folder(self):
		return self._current_folder

	def __init__(self, **kwargs):
		self._current_folder = Gio.File.new_for_path('.')
		super().__init__(**kwargs)
		self.notify('current-folder')

	@Gtk.Template.Callback('on_setup_list_item')
	def _on_setup_list_item(self, factory, list_item):
		hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=4, margin_start=4, margin_end=4)
		image = Gtk.Image()
		hbox.append(image)
		label = Gtk.Label(halign=Gtk.Align.FILL, hexpand=True, xalign=0.0)
		hbox.append(label)
		list_item.set_child(hbox)

	@Gtk.Template.Callback('on_bind_list_item')
	def _on_bind_list_item(self, factory, list_item):
		hbox = list_item.get_child()
		file_info = list_item.get_item()

		child = hbox.get_first_child()
		while child is not None:
			if child.__gtype__ == Gtk.Image.__gtype__:
				child.set_from_gicon(file_info.get_icon())
			elif child.__gtype__ == Gtk.Label.__gtype__:
				child.set_label(file_info.get_display_name())
			child = child.get_next_sibling()

	@Gtk.Template.Callback('on_go_up')
	def _on_go_up(self, button):
		parent = self._current_folder.get_parent()
		if parent is not None:
			self._current_folder = parent
			self.notify('current-folder')

	@Gtk.Template.Callback('on_list_view_activate')
	def _on_list_view_activate(self, list_view, position):
		if position != Gtk.INVALID_LIST_POSITION:
			file_info = list_view.get_model().get_item(position)
			if file_info.get_file_type() == Gio.FileType.DIRECTORY:
				folder = self._current_folder.get_child(file_info.get_name())
				self._current_folder = folder
				self.notify('current-folder')

#----------------------------------------------------------------------
#	main

def on_activate(app):
	window = MyWindow(application=app)
	window.set_visible(True)

if __name__ == '__main__':
	app = Gtk.Application()
	app.connect('activate', on_activate)
	sys.exit(app.run(sys.argv))

