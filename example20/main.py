# coding: utf-8

import sys, os
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, GObject, Gio, Pango

#----------------------------------------------------------------------
#	ListItem

class ListItem(GObject.GObject):
	__gtype_name__ = 'MyListItem'

	@GObject.Property(type=GObject.TYPE_INT)
	def value(self):
		return self._value if hasattr(self, '_value') else 0
	@value.setter
	def value(self, v):
		self._value = v
		self.notify('label')

	@GObject.Property(type=GObject.TYPE_STRING)
	def label(self):
		return f'Item {self.value}'

#----------------------------------------------------------------------
#	window

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'window.ui'))
class Window(Gtk.Window, Gtk.BuilderScope):
	__gtype_name__ = 'MyWindow'

	@GObject.Property(type=Gio.File)
	def target_directory(self):
		return self._target_directory

	@GObject.Property(type=Gio.ListModel)
	def custom_item_list_model(self):
		if not hasattr(self, '_custom_item_list_model'):
			self._custom_item_list_model = Gio.ListStore(item_type=ListItem)
			for i in range(5):
				item = ListItem(value=i)
				self._custom_item_list_model.append(item)
		return self._custom_item_list_model

	def __init__(self, **kwargs):
		super().__init__(**kwargs)
		self._target_directory = Gio.File.new_for_path('.')
		self.notify('target-directory')

		self.notify('custom-item-list-model')

	@Gtk.Template.Callback('get_display_name')
	def _get_display_name(self, obj):
		return obj.get_display_name()

#----------------------------------------------------------------------
#	no_ui_window

class NoUIWindow(Gtk.Window):
	__gtype_name__ = 'MyNoUIWindow'

	def __init__(self, **kwargs):
		super().__init__(**kwargs)

		self._target_directory = Gio.File.new_for_path('.')

		self._custom_item_list_model = Gio.ListStore(item_type=ListItem)
		for i in range(5):
			item = ListItem(value=i)
			self._custom_item_list_model.append(item)

		vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=8, margin_start=8, margin_end=8, margin_top=8, margin_bottom=8)
		self.set_child(vbox)

		vbox.append(self._make_simple_string_drop_down())
		vbox.append(self._make_drop_down_using_custom_item())
		vbox.append(self._make_directory_list_drop_down())
		vbox.append(self._make_drop_down_using_list_item_factory())

	def _make_simple_string_drop_down(self):
		hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)

		label = Gtk.Label(label='Simple String List')
		hbox.append(label)

		model = Gtk.StringList.new(['Hoge', 'Piyo', 'Fuga'])
		drop_down = Gtk.DropDown(model=model)
		hbox.append(drop_down)

		return hbox

	def _make_drop_down_using_custom_item(self):
		hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)

		label = Gtk.Label(label='Use Custom Item')
		hbox.append(label)

		expression = Gtk.PropertyExpression.new(ListItem, None, 'label')
		drop_down = Gtk.DropDown(model=self._custom_item_list_model, expression=expression)
		hbox.append(drop_down)

		return hbox

	def _make_directory_list_drop_down(self):
		hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)

		label = Gtk.Label(label='Directory List')
		hbox.append(label)

		model = Gtk.DirectoryList(attributes=Gio.FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME, file=self._target_directory)
		expression = Gtk.ClosureExpression.new(GObject.TYPE_STRING, self._get_display_name, None)
		drop_down = Gtk.DropDown(model=model, expression=expression)
		hbox.append(drop_down)

		return hbox

	def _make_drop_down_using_list_item_factory(self):
		hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)

		label = Gtk.Label(label='Use List Item Factory')
		hbox.append(label)

		factory = Gtk.SignalListItemFactory()
		factory.connect('setup', self._on_factory_setup)
		factory.connect('bind', self._on_factory_bind)
		drop_down = Gtk.DropDown(model=self._custom_item_list_model, factory=factory)
		hbox.append(drop_down)

		return hbox

	def _get_display_name(self, obj):
		return obj.get_display_name()

	def _on_factory_setup(self, factory, list_item):
		hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)

		attributes = Pango.AttrList()
		attributes.insert(Pango.attr_weight_new(Pango.Weight.BOLD))
		label = Gtk.Label(attributes=attributes)
		hbox.append(label)
		hbox.label = label

		separator = Gtk.Separator(orientation=Gtk.Orientation.VERTICAL)
		hbox.append(separator)

		label = Gtk.Label()
		hbox.append(label)
		hbox.value = label

		list_item.set_child(hbox)

	def _on_factory_bind(self, factory, list_item):
		child = list_item.get_child()
		item = list_item.get_item()

		child.label.set_label(item.label)
		child.value.set_label(str(item.value))

#----------------------------------------------------------------------
#	main

def on_command_line(app, command_line):
	args = command_line.get_arguments()
	use_ui = 'noui' not in args[1:]
	if use_ui:
		window = Window(application=app)
	else:
		window = NoUIWindow(application=app)
	window.set_visible(True)
	return 0

if __name__ == '__main__':
	app = Gtk.Application(flags=Gio.ApplicationFlags.HANDLES_COMMAND_LINE)
	app.connect('command-line', on_command_line)
	sys.exit(app.run(sys.argv))

