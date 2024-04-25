# coding: utf-8

import sys
import gi
gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, GObject, Gio, GLib

def on_new_tab(action, parameter, window):
	create_tab(window)

def on_close(action, parameter, window):
	window.close()

def on_new_window_clicked(button, window):
	create_window(window.get_application())

def on_new_tab_clicked(button, window):
	create_tab(window)

def create_tab(window):
	notebook = window.notebook

	child = Gtk.Label(label=GLib.DateTime.new_now_local().format('created at %c'))
	tab_label = Gtk.Label(label=f'Page {1 + window.page_count}')

	page_index = notebook.append_page(child, tab_label)
	notebook.set_current_page(page_index)

	window.page_count += 1

def create_window(app):
	window = Gtk.ApplicationWindow(application=app)
	window.set_default_size(400, 300)
	window.page_count = 0

	notebook = Gtk.Notebook()
	window.set_child(notebook)
	window.notebook = notebook

	titlebar = Gtk.HeaderBar()
	window.set_titlebar(titlebar)

	button = Gtk.Button(icon_name='window-new-symbolic')
	button.connect('clicked', on_new_window_clicked, window)
	titlebar.pack_start(button)

	button = Gtk.Button(icon_name='tab-new-symbolic')
	button.connect('clicked', on_new_tab_clicked, window)
	titlebar.pack_start(button)

	menu = Gio.Menu()

	section_menu = Gio.Menu()
	menu_item = Gio.MenuItem.new('New Window', 'app.new_window')
	section_menu.append_item(menu_item)
	menu_item = Gio.MenuItem.new('New Tab', 'win.new_tab')
	section_menu.append_item(menu_item)
	menu.append_section(None, section_menu)

	section_menu = Gio.Menu()
	menu_item = Gio.MenuItem.new('Close This Window', 'win.close')
	section_menu.append_item(menu_item)
	menu_item = Gio.MenuItem.new('Quit', 'app.quit')
	section_menu.append_item(menu_item)
	menu.append_section(None, section_menu)

	button = Gtk.MenuButton(icon_name='open-menu-symbolic', menu_model=menu)
	titlebar.pack_end(button)

	window.add_action_entries([
		('new_tab', on_new_tab, None, None, None),
		('close', on_close, None, None, None),
		], window)

	create_tab(window)

	window.set_visible(True)

def on_quit(action, parameter, app):
	app.quit()

def on_new_window(action, parameter, app):
	create_window(app)

def on_activate(app):
	create_window(app)

if __name__ == '__main__':
	app = Gtk.Application(application_id='com.example.example01')
	for name, callback in [('quit', on_quit), ('new_window', on_new_window)]:
		action = Gio.SimpleAction.new(name, None)
		action.connect('activate', callback, app)
		app.add_action(action)
	app.connect('activate', on_activate)
	sys.exit(app.run(sys.argv))

