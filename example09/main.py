# coding: utf-8

import sys, os, traceback
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, GObject, Gio, GLib

#----------------------------------------------------------------------
#	file_item

class FileItem(GObject.GObject):
	__gtype_name__ = 'MyFileItem'

	FILE_ATTRIBUTES = ','.join([
		Gio.FILE_ATTRIBUTE_STANDARD_NAME, Gio.FILE_ATTRIBUTE_STANDARD_ICON, Gio.FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
		Gio.FILE_ATTRIBUTE_STANDARD_TYPE,
		])

	@GObject.Property(type=Gio.File, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.CONSTRUCT_ONLY))
	def file(self):
		return self._file if hasattr(self, '_file') else None
	@file.setter
	def file(self, value):
		self._file = value

	@GObject.Property(type=Gio.FileInfo, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.CONSTRUCT_ONLY))
	def file_info(self):
		return self._file_info if hasattr(self, '_file_info') else None
	@file_info.setter
	def file_info(self, value):
		self._file_info = value
		self.notify('icon')
		self.notify('display-name')

	@GObject.Property(type=Gio.Icon)
	def icon(self):
		return self.file_info.get_icon() if self.file_info is not None else None

	@GObject.Property(type=GObject.TYPE_STRING)
	def display_name(self):
		return self.file_info.get_display_name() if self.file_info is not None else ''

	@classmethod
	def create_root_model(cls, file):
		list_store = Gio.ListStore(item_type=FileItem)
		file.enumerate_children_async(cls.FILE_ATTRIBUTES, Gio.FileQueryInfoFlags.NONE, GLib.PRIORITY_DEFAULT, None, cls._on_enumerate_children, list_store)
		return list_store

	def create_child_model(self):
		if self.file is None or self.file_info is None or self.file_info.get_file_type() != Gio.FileType.DIRECTORY:
			return None
		return self.create_root_model(self.file)

	@classmethod
	def _on_enumerate_children(cls, source, result, list_store):
		try:
			enumerator = source.enumerate_children_finish(result)
		except GLib.Error as e:
			traceback.print_exc()
			return
		enumerator.next_files_async(32, GLib.PRIORITY_DEFAULT, None, cls._on_next_files, list_store)

	@classmethod
	def _on_next_files(cls, source, result, list_store):
		try:
			file_infos = source.next_files_finish(result)
		except GLib.Error as e:
			traceback.print_exc()
			return
		if not file_infos:
			source.close(None)
		else:
			for file_info in file_infos:
				child = source.get_child(file_info)
				item = cls(file=child, file_info=file_info)
				list_store.append(item)
			source.next_files_async(32, GLib.PRIORITY_DEFAULT, None, cls._on_next_files, list_store)

#----------------------------------------------------------------------
#	window

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'window.ui'))
class Window(Gtk.Window):
	__gtype_name__ = 'MyWindow'

	@GObject.Property(type=Gio.ListModel)
	def root_model(self):
		return self._root_model

	def __init__(self, **kwargs):
		self._root_model = None
		super().__init__(**kwargs)

		self._root_model = Gtk.TreeListModel.new(FileItem.create_root_model(Gio.File.new_for_path(GLib.get_home_dir())), False, False, lambda item: item.create_child_model())
		self.notify('root-model')

#----------------------------------------------------------------------
#	main

def on_activate(app):
	window = Window(application=app)
	window.set_visible(True)

if __name__ == '__main__':
	app = Gtk.Application()
	app.connect('activate', on_activate)
	sys.exit(app.run(sys.argv))

