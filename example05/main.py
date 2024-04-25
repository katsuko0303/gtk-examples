# coding: utf-8

import sys, os, traceback
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, GObject, Gio, GLib

#----------------------------------------------------------------------
#	FileItem

class FileItem(GObject.GObject):
	__gtype_name__ = 'MyFileItem'

	FILE_ATTRIBUTES = ','.join([
		Gio.FILE_ATTRIBUTE_STANDARD_NAME,
		Gio.FILE_ATTRIBUTE_STANDARD_ICON,
		Gio.FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
		Gio.FILE_ATTRIBUTE_STANDARD_TYPE,
		Gio.FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
		Gio.FILE_ATTRIBUTE_TIME_MODIFIED,
		Gio.FILE_ATTRIBUTE_STANDARD_SIZE,
		])

	@GObject.Property(type=Gio.FileInfo, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.CONSTRUCT_ONLY))
	def file_info(self):
		return self._file_info if hasattr(self, '_file_info') else None
	@file_info.setter
	def file_info(self, value):
		self._file_info = value
		self.notify('icon')
		self.notify('display-name')
		self.notify('type-description')
		self.notify('display-modified-time')
		self.notify('modified-time')
		self.notify('size')

	@GObject.Property(type=Gio.Icon)
	def icon(self):
		return self.file_info.get_icon() if self.file_info is not None else None

	@GObject.Property(type=GObject.TYPE_STRING)
	def display_name(self):
		return self.file_info.get_display_name() if self.file_info is not None else ''

	@GObject.Property(type=GObject.TYPE_STRING)
	def type_description(self):
		description = ''
		if self.file_info is not None:
			content_type = self.file_info.get_content_type()
			if content_type:
				description = Gio.content_type_get_description(content_type)
		return description

	@GObject.Property(type=GObject.TYPE_STRING)
	def display_modified_time(self):
		text = ''
		if self.file_info is not None:
			modified_time = self.file_info.get_modification_date_time()
			if modified_time is not None:
				text = modified_time.format('%c')
		return text

	@GObject.Property(type=GObject.TYPE_INT64)
	def modified_time(self):
		time = 0
		if self.file_info is not None:
			modified_time = self.file_info.get_modification_date_time()
			if modified_time is not None:
				time = modified_time.to_unix()
		return time

	@GObject.Property(type=GObject.TYPE_UINT64)
	def size(self):
		size = 0
		if self.file_info is not None:
			size = self.file_info.get_size()
		return size

	@GObject.Property(type=GObject.TYPE_BOOLEAN, default=False)
	def is_directory(self):
		return self.file_info is not None and self.file_info.get_file_type() == Gio.FileType.DIRECTORY

	@GObject.Property(type=GObject.TYPE_STRING)
	def name(self):
		return self.file_info.get_name() if self.file_info is not None else ''

#----------------------------------------------------------------------
#	window

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'window.ui'))
class Window(Gtk.Window):
	__gtype_name__ = 'MyWindow'

	@GObject.Property(type=Gio.ListModel)
	def file_model(self):
		return self._file_model

	@GObject.Property(type=Gio.File)
	def current_folder(self):
		return self._current_folder if hasattr(self, '_current_folder') else None
	@current_folder.setter
	def current_folder(self, value):
		self._current_folder = value
		self._file_model.remove_all()
		if self._cancellable is not None:
			self._cancellable.cancel()
		self._cancellable = Gio.Cancellable()
		self._current_folder.enumerate_children_async(FileItem.FILE_ATTRIBUTES, Gio.FileQueryInfoFlags.NONE, GLib.PRIORITY_DEFAULT,
					self._cancellable, self._on_enumerate_children)

	_file_list = Gtk.Template.Child('file_list')
	_default_column = Gtk.Template.Child('default_column')

	def __init__(self, **kwargs):
		self._file_model = Gio.ListStore(item_type=FileItem)
		self._cancellable = None
		super().__init__(**kwargs)

		self.current_folder = Gio.File.new_for_path('.')

		self.notify('file-model')

		self._file_list.sort_by_column(self._default_column, Gtk.SortType.ASCENDING)

	def _on_enumerate_children(self, source, result):
		try:
			enumerator = source.enumerate_children_finish(result)
		except GLib.Error as e:
			if not e.matches(Gio.io_error_quark(), Gio.IOErrorEnum.CANCELLED):
				traceback.print_exc()
				self._cancellable = None
			return
		enumerator.next_files_async(32, GLib.PRIORITY_DEFAULT, self._cancellable, self._on_next_files)

	def _on_next_files(self, source, result):
		try:
			file_infos = source.next_files_finish(result)
		except GLib.Error as e:
			if not e.matches(Gio.io_error_quark(), Gio.IOErrorEnum.CANCELLED):
				self._cancellable = None
			source.close(None)
			return
		if not file_infos:
			source.close()
			self._cancellable = None
		else:
			for file_info in file_infos:
				item = FileItem(file_info=file_info)
				self._file_model.append(item)
			source.next_files_async(32, GLib.PRIORITY_DEFAULT, self._cancellable, self._on_next_files)

	@Gtk.Template.Callback('on_go_up')
	def _on_go_up(self, button):
		parent = self.current_folder.get_parent()
		if parent is not None:
			self.current_folder = parent

	@Gtk.Template.Callback('on_file_list_activate')
	def _on_file_list_activate(self, column_view, position):
		if position != Gtk.INVALID_LIST_POSITION:
			item = column_view.get_model().get_item(position)
			child = self.current_folder.get_child(item.name)
			if item.is_directory:
				self.current_folder = child
			else:
				try:
					Gio.AppInfo.launch_default_for_uri(child.get_uri(), None)
				except GLib.Error as e:
					traceback.print_exc()

#----------------------------------------------------------------------
#	main

def on_activate(app):
	window = Window(application=app)
	window.set_visible(True)

if __name__ == '__main__':
	app = Gtk.Application()
	app.connect('activate', on_activate)
	sys.exit(app.run(sys.argv))

