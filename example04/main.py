# coding: utf-8

import sys, os
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, GObject, Gio, GLib, Gdk

#----------------------------------------------------------------------
#	address_data

class MyAddressData(GObject.GObject):
	__gtype_name__ = 'MyAddressData'

	@GObject.Property(type=GObject.TYPE_STRING)
	def first_name(self):
		return self._first_name if hasattr(self, '_first_name') else ''
	@first_name.setter
	def first_name(self, value):
		self._first_name = value
		self.notify('full-name')

	@GObject.Property(type=GObject.TYPE_STRING)
	def last_name(self):
		return self._last_name if hasattr(self, '_last_name') else ''
	@last_name.setter
	def last_name(self, value):
		self._last_name = value
		self.notify('full-name')

	@GObject.Property(type=GLib.Date)
	def birth_day(self):
		if hasattr(self, '_birth_day'):
			return self._birth_day
		date_time = GLib.DateTime.new_now_local()
		return GLib.Date.new_dmy(date_time.get_day_of_month(), date_time.get_month(), date_time.get_year())
	@birth_day.setter
	def birth_day(self, value):
		self._birth_day = value

	@GObject.Property(type=GObject.TYPE_INT)
	def age(self):
		now = GLib.DateTime.new_now_local()
		return now.get_year() - self.birth_day.get_year() - (1 if now.get_day_of_year() < self.birth_day.get_day_of_year() else 0)

	@GObject.Property(type=GObject.TYPE_STRING)
	def address(self):
		return self._address if hasattr(self, '_address') else ''
	@address.setter
	def address(self, value):
		self._address = value

	@GObject.Property(type=GObject.TYPE_STRING)
	def email(self):
		return self._email if hasattr(self, '_email') else ''
	@email.setter
	def email(self, value):
		self._email = value

	@GObject.Property(type=GObject.TYPE_STRING)
	def full_name(self):
		return self.last_name if not self.first_name else self.first_name if not self.last_name else ' '.join([self.first_name, self.last_name])

#----------------------------------------------------------------------
#	address_dialog

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'address_dialog.ui'))
class MyAddressDialog(Gtk.Window):
	__gtype_name__ = 'MyAddressDialog'

	@GObject.Property(type=GObject.TYPE_STRING, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.EXPLICIT_NOTIFY))
	def first_name(self):
		return self._first_name if hasattr(self, '_first_name') else ''
	@first_name.setter
	def first_name(self, value):
		if not hasattr(self, '_first_name') or value != self._first_name:
			self._first_name = value
			self.notify('first-name')

	@GObject.Property(type=GObject.TYPE_STRING, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.EXPLICIT_NOTIFY))
	def last_name(self):
		return self._last_name if hasattr(self, '_last_name') else ''
	@last_name.setter
	def last_name(self, value):
		if not hasattr(self, '_last_name') or value != self._last_name:
			self._last_name = value
			self.notify('last-name')

	@GObject.Property(type=GLib.Date, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.EXPLICIT_NOTIFY))
	def birth_day(self):
		if hasattr(self, '_birth_day'):
			return self._birth_day
		now = GLib.DateTime.new_now_local()
		return GLib.Date.new_dmy(now.get_day_of_month(), now.get_month(), now.get_year())
	@birth_day.setter
	def birth_day(self, value):
		if not hasattr(self, '_birth_day') or GLib.Date.compare(value, self._birth_day) != 0:
			self._birth_day = value
			self.notify('birth-day')
			self.notify('display-birth-day')

	@GObject.Property(type=GObject.TYPE_STRING)
	def display_birth_day(self):
		return f'{int(self.birth_day.get_month())}/{self.birth_day.get_day()}/{self.birth_day.get_year()}'

	@GObject.Property(type=GObject.TYPE_STRING, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.EXPLICIT_NOTIFY))
	def address(self):
		return self._address if hasattr(self, '_address') else ''
	@address.setter
	def address(self, value):
		if not hasattr(self, '_address') or value != self._address:
			self._address = value
			self.notify('address')

	@GObject.Property(type=GObject.TYPE_STRING, flags=(GObject.ParamFlags.READWRITE | GObject.ParamFlags.EXPLICIT_NOTIFY))
	def email(self):
		return self._email if hasattr(self, '_email') else ''
	@email.setter
	def email(self, value):
		if not hasattr(self, '_email') or value != self._email:
			self._email = value
			self.notify('email')

	@GObject.Property(type=MyAddressData)
	def target(self):
		return self._target if hasattr(self, '_target') else None
	@target.setter
	def target(self, value):
		self._target = value
		if self._target is not None:
			self.first_name = self._target.first_name
			self.last_name = self._target.last_name
			self.birth_day = self._target.birth_day
			self.address = self._target.address
			self.email = self._target.email

	@GObject.Signal(flags=(GObject.SignalFlags.RUN_FIRST | GObject.SignalFlags.ACTION))
	def cancel(self):
		if self._task is not None:
			self._task.return_error(GLib.Error.new_literal(Gtk.DialogError.quark(), 'Dismissed by user', Gtk.DialogError.DISMISSED))
		self.destroy()

	_birth_day_calendar = Gtk.Template.Child('birth_day_calendar')

	def __init__(self, **kwargs):
		self._task = None
		super().__init__(**kwargs)

		date_time = GLib.DateTime.new_local(self.birth_day.get_year(), self.birth_day.get_month(), self.birth_day.get_day(), 0, 0, 0)
		self._birth_day_calendar.select_day(date_time)

		self.notify('display-birth-day')

	def do_close_request(self):
		self.emit('cancel')
		return False

	def edit_async(self, parent, cancellable, callback, *args):
		assert self._task is None
		self.set_transient_for(parent)
		self._task = Gio.Task.new(self, cancellable, callback, *args)
		self.set_visible(True)

	def edit_finish(self, result):
		ret, obj = result.propagate_value()
		return obj if ret else None

	@Gtk.Template.Callback('on_cancel')
	def _on_cancel(self, button):
		self.emit('cancel')

	@Gtk.Template.Callback('on_accept')
	def _on_accept(self, button):
		if self._task is not None:
			address_data = self.target if self.target is not None else MyAddressData()
			address_data.first_name = self.first_name
			address_data.last_name = self.last_name
			address_data.birth_day = self.birth_day
			address_data.address = self.address
			address_data.email = self.email

			value = GObject.Value(value_type=MyAddressData)
			value.set_object(address_data)
			self._task.return_value(value)
		self.destroy()

	@Gtk.Template.Callback('on_birth_day_selected')
	def _on_birth_day_selected(self, calendar):
		date_time = calendar.get_date()
		self.birth_day = GLib.Date.new_dmy(date_time.get_day_of_month(), date_time.get_month(), date_time.get_year())

MyAddressDialog.add_shortcut(Gtk.Shortcut.new(Gtk.KeyvalTrigger.new(Gdk.KEY_Escape, 0), Gtk.SignalAction.new('cancel')))

#----------------------------------------------------------------------
#	window

@Gtk.Template(filename=os.path.join(os.path.dirname(__file__), 'window.ui'))
class MyWindow(Gtk.Window):
	__gtype_name__ = 'MyWindow'

	_address_model = Gtk.Template.Child('address_model')

	def __init__(self, **kwargs):
		super().__init__(**kwargs)

	def _on_edit_finish(self, source, result):
		try:
			address_data = source.edit_finish(result)
			if address_data is None:
				return
		except GLib.Error as e:
			return
		if source.target is None:
			self._address_model.append(address_data)

	@Gtk.Template.Callback('on_new_address')
	def _on_new_address(self, button):
		dialog = MyAddressDialog(modal=True)
		dialog.edit_async(self, None, self._on_edit_finish)

	@Gtk.Template.Callback('on_address_list_activate')
	def _on_address_list_activate(self, list_view, position):
		if position == Gtk.INVALID_LIST_POSITION:
			return
		address_data = list_view.get_model().get_item(position)
		if address_data is None:
			return
		dialog = MyAddressDialog(modal=True, target=address_data)
		dialog.edit_async(self, None, self._on_edit_finish)

#----------------------------------------------------------------------
#	main

def on_activate(app):
	window = MyWindow(application=app)
	window.set_visible(True)

if __name__ == '__main__':
	app = Gtk.Application()
	app.connect('activate', on_activate)
	sys.exit(app.run(sys.argv))

