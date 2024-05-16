# coding: utf-8

import sys, os, traceback, enum
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, GObject, Gdk, GLib, Gio

#----------------------------------------------------------------------
#	file_item

class FileItem(GObject.GObject):
	__gtype_name__ = 'MyFileItem'

	@GObject.Property(type=Gio.File, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.CONSTRUCT_ONLY))
	def file(self):
		return self._file if hasattr(self, '_file') else None
	@file.setter
	def file(self, value):
		self._file = value
		self._file_info = None

	@GObject.Property(type=GObject.TYPE_STRING)
	def display_name(self):
		return self._file_info.get_display_name() if self._file_info is not None else ''

	@GObject.Property(type=Gio.Icon)
	def icon(self):
		return self._file_info.get_icon() if self._file_info is not None else None

	def query_info(self, cancellable):
		def callback(source, result):
			try:
				self._file_info = source.query_info_finish(result)
			except GLib.Error as e:
				if e.matches(Gio.io_error_quark(), Gio.IOErrorEnum.CANCELLED):
					return
				traceback.print_exc()
			self.notify('display-name')
			self.notify('icon')
		self._file.query_info_async(','.join([Gio.FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME, Gio.FILE_ATTRIBUTE_STANDARD_ICON]),
					Gio.FileQueryInfoFlags.NONE, GLib.PRIORITY_DEFAULT, cancellable, callback)

#----------------------------------------------------------------------
#	window

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'window.ui'))
class Window(Gtk.Window):
	__gtype_name__ = 'MyWindow'

	@GObject.Property(type=GObject.TYPE_BOOLEAN, default=True)
	def data_is_none(self):
		return self._data is None

	@GObject.Property(type=GObject.TYPE_BOOLEAN, default=False)
	def data_is_text(self):
		return isinstance(self._data, str)

	@GObject.Property(type=GObject.TYPE_STRING)
	def text_data(self):
		return self._data if isinstance(self._data, str) else ''

	@GObject.Property(type=GObject.TYPE_BOOLEAN, default=False)
	def data_is_image(self):
		return isinstance(self._data, Gdk.Texture)

	@GObject.Property(type=Gdk.Paintable)
	def image_data(self):
		return self._data if isinstance(self._data, Gdk.Texture) else None

	@GObject.Property(type=GObject.TYPE_BOOLEAN, default=False)
	def data_is_files(self):
		return isinstance(self._data, Gio.ListStore)

	@GObject.Property(type=Gio.ListModel)
	def files_data(self):
		return self._data if isinstance(self._data, Gio.ListStore) else None

	def __init__(self, **kwargs):
		self._data = None
		self._cancellable = None
		super().__init__(**kwargs)

	def _notify_data_update(self):
		self.notify('data-is-none')
		self.notify('data-is-text')
		self.notify('text-data')
		self.notify('data-is-image')
		self.notify('image-data')
		self.notify('data-is-files')
		self.notify('files-data')

	@Gtk.Template.Callback('on_paste')
	def _on_paste(self, button):
		if self._cancellable is not None:
			self._cancellable.cancel()
			self._cancellable = None
		self._data = None
		self._notify_data_update()

		clipboard = Gdk.Display.get_default().get_primary_clipboard()
		formats = clipboard.get_formats()
		if formats.contain_gtype(GObject.TYPE_STRING):
			self._cancellable = Gio.Cancellable()
			def callback(source, result):
				try:
					self._data = source.read_text_finish(result)
					self._notify_data_update()
				except GLib.Error as e:
					if e.matches(Gio.io_error_quark(), Gio.IOErrorEnum.CANCELLED):
						return
					traceback.print_exc()
				self._cancellable = None
			clipboard.read_text_async(self._cancellable, callback)
		elif formats.contain_gtype(Gdk.Texture):
			self._cancellable = Gio.Cancellable()
			def callback(source, result):
				try:
					self._data = source.read_texture_finish(result)
					self._notify_data_update()
				except GLib.Error as e:
					if e.matches(Gio.io_error_quark(), Gio.IOErrorEnum.CANCELLED):
						return
					traceback.print_exc()
				self._cancellable = None
			clipboard.read_texture_async(self._cancellable, callback)

	@Gtk.Template.Callback('on_drop')
	def _on_drop(self, drop_target, value, x, y):
		if self._cancellable is not None:
			self._cancellable.cancel()
			self._cancellable = None
		self._data = None
		self._notify_data_update()

		if isinstance(value, str) or isinstance(value, Gdk.Texture):
			self._data = value
			self._notify_data_update()
			return True
		elif isinstance(value, Gdk.FileList):
			list_store = Gio.ListStore(item_type=FileItem)
			self._cancellable = Gio.Cancellable()
			for file in value.get_files():
				item = FileItem(file=file)
				item.query_info(self._cancellable)
				list_store.append(item)
			self._data = list_store
			self._notify_data_update()
			return True
		return False

#----------------------------------------------------------------------
#	main

def on_activate(app):
	window = Window(application=app)
	window.set_visible(True)

if __name__ == '__main__':
	app = Gtk.Application()
	app.connect('activate', on_activate)
	sys.exit(app.run(sys.argv))

