# Copyright (C) 2007-2008 Martin Szulecki
# Copyright (C) 2011 Jean-Philippe Fleury <contact@jpfleury.net>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

'''
Adds context menu item to open an URI at the pointer position
'''

from gi.repository import Gtk, Xed, Gio, GObject, GtkSource
import gettext
import re
import sys
import os
import subprocess
import string

gettext.install("xed")

ACCEPTED_SCHEMES = ['file', 'ftp', 'sftp', 'smb', 'dav', 'davs', 'ssh', 'http', 'https']
RE_DELIM = re.compile(r'[\w#/\?:%@&\=\+\.\\~-]+', re.UNICODE|re.MULTILINE)
RE_URI_RFC2396 = re.compile("((([a-zA-Z][0-9a-zA-Z+\\-\\.]*):)?/{0,2}([0-9a-zA-Z;:,/\?@&=\+\$\.\-_!~\*'\(\)%]+))?(#[0-9a-zA-Z;,/\?:@&\=+$\.\\-_!~\*'\(\)%]+)?")

class OpenURIContextMenuPlugin(GObject.Object, Xed.WindowActivatable):
	__gtype_name__ = "OpenURIContextMenuPlugin"
	window = GObject.property(type=Xed.Window)

	def __init__(self):
		GObject.Object.__init__(self)
		self.uri = ""
		self.window = None
		self.encoding = GtkSource.Encoding.get_from_charset("UTF-8")

	def do_activate(self):
		handler_ids = []
		for signal in ('tab-added', 'tab-removed'):
			method = getattr(self, 'on_window_' + signal.replace('-', '_'))
			handler_ids.append(self.window.connect(signal, method))
		self.window.OpenURIContextMenuPluginID = handler_ids
		for view in self.window.get_views():
			self.connect_view(view)

	def do_deactivate(self):
		widgets = [self.window] + self.window.get_views()
		for widget in widgets:
			handler_ids = widget.OpenURIContextMenuPluginID
			if not handler_ids is None:
				for handler_id in handler_ids:
					widget.disconnect(handler_id)
			widget.OpenURIContextMenuPluginID = None
		self.window = None

	def connect_view(self, view):
		handler_id = view.connect('populate-popup', self.on_view_populate_popup)
		view.OpenURIContextMenuPluginID = [handler_id]

	def update_ui(self, window):
		pass

	def browse_url(self, menu_item, url):
		command = ['xdg-open', url]
		# Avoid to run the browser as user root
		if os.getuid() == 0 and os.environ.has_key('SUDO_USER'):
			command = ['sudo', '-u', os.environ['SUDO_USER']] + command
		subprocess.Popen(command)

	def on_window_tab_added(self, window, tab):
		self.connect_view(tab.get_view())

	def on_window_tab_removed(self, window, tab):
		pass

	def on_view_populate_popup(self, view, menu):
		doc = view.get_buffer()
		win = view.get_window(Gtk.TextWindowType.TEXT);
		ptr_window, x, y, mod = win.get_pointer()
		x, y = view.window_to_buffer_coords(Gtk.TextWindowType.TEXT, x, y);

		# First try at pointer location
		# Starting with GTK 3.20, get_iter_at_location returns a tuple of type
		# (gboolean, GtkTextIter).  Earlier versions return only the GtkTextIter.
		insert = view.get_iter_at_location(x, y);
		if isinstance(insert, tuple):
			insert = insert[1] if insert[0] else None

		# Second try at cursor
		if insert == None:
			insert = doc.get_iter_at_mark(doc.get_insert())
			if isinstance(insert, tuple):
				insert = insert[1] if insert[0] else None

		while insert.forward_char():
			if not RE_DELIM.match(insert.get_char()):
				break

		start = insert.copy()
		while start.backward_char():
			if not RE_DELIM.match(start.get_char()):
				start.forward_char();
				break

		word = doc.get_text(start, insert, False)
		if len(word) == 0:
			return True

		word = self.validate_uri(word)
		if not word:
			return True

		displayed_word = word
		if len(displayed_word) > 50:
			displayed_word = displayed_word[:50] + u"\u2026"

		browse_to = False
		if word.startswith("http://") or word.startswith("https://"):
			browse_to = True

		if browse_to:
			browse_uri_item = Gtk.MenuItem(_("Open '%s'") % (displayed_word))
			browse_uri_item.connect('activate', self.browse_url, word);
			browse_uri_item.show();

			separator = Gtk.SeparatorMenuItem()
			separator.show();
			menu.prepend(separator)
			menu.prepend(browse_uri_item)
		return True

	def validate_uri(self, uri):
		m = RE_URI_RFC2396.search(uri);
		if not m:
			return False

		target = m.group()

		if m.group(4) == None or m.group(4) == "/":
			return False

		if m.group(2) != None:
			if m.group(3) in ACCEPTED_SCHEMES:
				return target
			else:
				return False
		else:
			if m.group(4).startswith("www."):
				return 'http://' + target

		target = os.path.expanduser(target)
		if os.path.isfile(target):
			if os.path.isabs(target):
				return 'file://' + target

		doc_dir = self.window.get_active_document().get_uri_for_display()
		if doc_dir != None:
			if doc_dir.startswith('file://'):
				f = os.path.join(os.path.dirname(doc_dir), target)
				if os.path.isfile(f.replace('file://', '', 1)):
					return f
			else:
				return os.path.join(os.path.dirname(doc_dir), target)

		paths = string.split(os.environ["PATH"], os.pathsep)
		for dirname in paths:
			f = os.path.join(os.path.dirname(dirname), 'include', target)
			if os.path.isfile(f):
				return 'file://' + f

		return False
