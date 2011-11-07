# -*- coding: utf-8 -*-

#  Copyright (C) 2009 - Jesse van den Kieboom
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330,
#  Boston, MA 02111-1307, USA.

import gedit
import gtk
from popup import Popup
import os
import gedit.commands
import gio
import glib
from virtualdirs import RecentDocumentsDirectory
from virtualdirs import CurrentDocumentsDirectory

ui_str = """<ui>
  <menubar name="MenuBar">
    <menu name="FileMenu" action="File">
      <placeholder name="FileOps_2">
        <menuitem name="QuickOpen" action="QuickOpen"/>
      </placeholder>
    </menu>
  </menubar>
</ui>
"""

class WindowHelper:
        def __init__(self, window, plugin):
                self._window = window
                self._plugin = plugin

                self._popup = None
                self._install_menu()

        def deactivate(self):
                self._uninstall_menu()
                self._window = None
                self._plugin = None

        def update_ui(self):
                pass

        def _uninstall_menu(self):
                manager = self._window.get_ui_manager()

                manager.remove_ui(self._ui_id)
                manager.remove_action_group(self._action_group)

                manager.ensure_update()

        def _install_menu(self):
                manager = self._window.get_ui_manager()
                self._action_group = gtk.ActionGroup("GeditQuickOpenPluginActions")
                self._action_group.add_actions([
                        ("QuickOpen", gtk.STOCK_OPEN, _("Quick open"),
                         '<Ctrl><Alt>O', _("Quickly open documents"),
                         self.on_quick_open_activate)
                ])

                manager.insert_action_group(self._action_group, -1)
                self._ui_id = manager.add_ui_from_string(ui_str)

        def _create_popup(self):
                paths = []

                # Open documents
                paths.append(CurrentDocumentsDirectory(self._window))

                doc = self._window.get_active_document()

                # Current document directory
                if doc and doc.is_local():
                        gfile = doc.get_location()
                        paths.append(gfile.get_parent())

                # File browser root directory
                if gedit.version[0] > 2 or (gedit.version[0] == 2 and (gedit.version[1] > 26 or (gedit.version[1] == 26 and gedit.version[2] >= 2))):
                        bus = self._window.get_message_bus()

                        try:
                                msg = bus.send_sync('/plugins/filebrowser', 'get_root')

                                if msg:
                                        uri = msg.get_value('uri')

                                        if uri:
                                                gfile = gio.File(uri)

                                                if gfile.is_native():
                                                        paths.append(gfile)

                        except StandardError:
                                pass

                # Recent documents
                paths.append(RecentDocumentsDirectory(screen=self._window.get_screen()))

                # Local bookmarks
                for path in self._local_bookmarks():
                        paths.append(path)

                # Desktop directory
                desktopdir = self._desktop_dir()

                if desktopdir:
                        paths.append(gio.File(desktopdir))

                # Home directory
                paths.append(gio.File(os.path.expanduser('~')))

                self._popup = Popup(self._window, paths, self.on_activated)

                self._popup.set_default_size(*self._plugin.get_popup_size())
                self._popup.set_transient_for(self._window)
                self._popup.set_position(gtk.WIN_POS_CENTER_ON_PARENT)

                self._window.get_group().add_window(self._popup)

                self._popup.connect('destroy', self.on_popup_destroy)

        def _local_bookmarks(self):
                filename = os.path.expanduser('~/.gtk-bookmarks')

                if not os.path.isfile(filename):
                        return []

                paths = []

                for line in file(filename, 'r').xreadlines():
                        uri = line.strip().split(" ")[0]
                        f = gio.File(uri)

                        if f.is_native():
                                try:
                                        info = f.query_info("standard::type")

                                        if info and info.get_file_type() == gio.FILE_TYPE_DIRECTORY:
                                                paths.append(f)
                                except glib.GError:
                                        pass

                return paths

        def _desktop_dir(self):
                config = os.getenv('XDG_CONFIG_HOME')

                if not config:
                        config = os.path.expanduser('~/.config')

                config = os.path.join(config, 'user-dirs.dirs')
                desktopdir = None

                if os.path.isfile(config):
                        for line in file(config, 'r').xreadlines():
                                line = line.strip()

                                if line.startswith('XDG_DESKTOP_DIR'):
                                        parts = line.split('=', 1)
                                        desktopdir = os.path.expandvars(parts[1].strip('"').strip("'"))
                                        break

                if not desktopdir:
                        desktopdir = os.path.expanduser('~/Desktop')

                return desktopdir

        # Callbacks
        def on_quick_open_activate(self, action):
                if not self._popup:
                        self._create_popup()

                self._popup.show()

        def on_popup_destroy(self, popup):
                alloc = popup.get_allocation()
                self._plugin.set_popup_size((alloc.width, alloc.height))

                self._popup = None

        def on_activated(self, gfile):
                gedit.commands.load_uri(self._window, gfile.get_uri(), None, -1)
                return True

# ex:ts=8:et:
