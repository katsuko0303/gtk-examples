# coding: utf-8

import sys, os, random
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, GObject, Gio

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

		list_model = Gtk.StringList.new([f'Item {i}' for i in range(5)])
		def create_model_func(item):
			return Gtk.StringList.new([f'Sub Item {i}' for i in range(random.randint(2, 10))])
		self._root_model = Gtk.TreeListModel.new(list_model, False, False, create_model_func)
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

