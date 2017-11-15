# -*- coding: utf-8 -*-
#
#  __init__.py - Text size plugin
#
#  Copyright (C) 2008 - Konstantin Mikhaylov <jtraub.devel@gmail.com>
#  Copyright (C) 2009 - Wouter Bolsterlee <wbolster@gnome.org>
#  Copyright (C) 2010 - Ignacio Casal Quinteiro <icq@gnome.org>
#  Copyright (C) 2010 - Jesse van den Kieboom <jessevdk@gnome.org>
#  Copyright (C) 2017 - Linux Mint team <https://github.com/linuxmint>
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
#  Foundation, Inc., 51 Franklin Street, Fifth Floor,
#  Boston, MA 02110-1301, USA.

from gi.repository import GObject, Gio, Gtk, Gdk, Xed
from .documenthelper import DocumentHelper

import gettext
gettext.install("xed")

MENU_PATH = "/MenuBar/ViewMenu/ViewOps_1"

class TextSizePlugin(GObject.Object, Xed.WindowActivatable):
    __gtype_name__ = "TextSizePlugin"

    window = GObject.property(type=Xed.Window)

    def __init__(self):
        GObject.Object.__init__(self)

    def do_activate(self):
        self._views  = {}

        # Insert menu items
        self._insert_menu()

        # Insert document helpers
        for view in self.window.get_views():
            self.add_document_helper(view)

        self.window.connect('tab-added', self.on_tab_added)
        self.window.connect('tab-removed', self.on_tab_removed)

        self._accel_group = Gtk.AccelGroup()
        self.window.add_accel_group(self._accel_group)

        self._proxy_callback_map = {
            'LargerTextAction': self.on_larger_text_accel,
            'SmallerTextAction': self.on_smaller_text_accel,
            'NormalSizeAction': self.on_normal_size_accel
        }

        self._proxy_mapping = {}
        self._init_proxy_accels()
        self._accel_map_handler_id = Gtk.AccelMap.get().connect('changed', self.on_accel_map_changed)

    def _install_proxy(self, action):
        if not isinstance(action, Gtk.Action):
            action = self._action_group.get_action(str(action))

        if not action:
            return

        entry = Gtk.AccelMap.lookup_entry(action.get_accel_path())

        if not entry:
            return

        mapping = {
            Gdk.KEY_equal: Gdk.KEY_KP_Equal,
            Gdk.KEY_KP_Equal: Gdk.KEY_equal,
            Gdk.KEY_minus: Gdk.KEY_KP_Subtract,
            Gdk.KEY_KP_Subtract: Gdk.KEY_minus,
            Gdk.KEY_0: Gdk.KEY_KP_0,
            Gdk.KEY_KP_0: Gdk.KEY_0
        }

        if entry[0] in mapping:
            key = mapping[entry[0]]
            mod = entry[1]

            callback = self._proxy_callback_map[action.get_name()]

            self._accel_group.connect_group(key, mod, Gtk.ACCEL_LOCKED, callback)
            self._proxy_mapping[action] = (key, mod)

    def _init_proxy_accels(self):
        self._install_proxy('LargerTextAction')
        self._install_proxy('SmallerTextAction')
        self._install_proxy('NormalSizeAction')

    def do_deactivate(self):
        # Remove any installed menu items
        self._remove_menu()

        for view in self.window.get_views():
            self.remove_document_helper(view)

        self.window.remove_accel_group(self._accel_group)

        Gtk.AccelMap.get().disconnect(self._accel_map_handler_id)

        self._accel_group = None
        self._action_group = None

    def _insert_menu(self):
        # Get the GtkUIManager
        manager = self.window.get_ui_manager()

        # Create a new action group
        self._action_group = Gtk.ActionGroup("XedTextSizePluginActions")
        self._action_group.add_actions([("LargerTextAction", None, _("_Larger Text"),
                                         "<Ctrl>equal", None,
                                         self.on_larger_text_activate),
                                         ("SmallerTextAction", None, _("S_maller Text"),
                                         "<Ctrl>minus", None,
                                         self.on_smaller_text_activate),
                                         ("NormalSizeAction", None, _("_Normal size"),
                                         "<Ctrl>0", None,
                                         self.on_normal_size_activate)])

        # Insert the action group
        manager.insert_action_group(self._action_group)

        self._ui_id = manager.new_merge_id();

        manager.add_ui(self._ui_id,
                       MENU_PATH,
                       "LargerTextAction",
                       "LargerTextAction",
                       Gtk.UIManagerItemType.MENUITEM,
                       False)

        manager.add_ui(self._ui_id,
                       MENU_PATH,
                       "SmallerTextAction",
                       "SmallerTextAction",
                       Gtk.UIManagerItemType.MENUITEM,
                       False)

        manager.add_ui(self._ui_id,
                       MENU_PATH,
                       "NormalSizeAction",
                       "NormalSizeAction",
                       Gtk.UIManagerItemType.MENUITEM,
                       False)

    def _remove_menu(self):
        # Get the GtkUIManager
        manager = self.window.get_ui_manager()

        # Remove the ui
        manager.remove_ui(self._ui_id)

        # Remove the action group
        manager.remove_action_group(self._action_group)

        # Make sure the manager updates
        manager.ensure_update()

    def do_update_state(self):
        self._action_group.set_sensitive(self.window.get_active_document() != None)

    def get_helper(self, view):
        if not hasattr(view, "textsize_document_helper"):
            return None
        return view.textsize_document_helper

    def add_document_helper(self, view):
        if self.get_helper(view) != None:
            return

        DocumentHelper(view)

    def remove_document_helper(self, view):
        helper = self.get_helper(view)

        if helper != None:
            helper.stop()

    def call_helper(self, cb):
        view = self.window.get_active_view()

        if view:
            cb(self.get_helper(view))

    # Menu activate handlers
    def on_larger_text_activate(self, action, user_data=None):
        self.call_helper(lambda helper: helper.larger_text())

    def on_smaller_text_activate(self, action, user_data=None):
        self.call_helper(lambda helper: helper.smaller_text())

    def on_normal_size_activate(self, action, user_data=None):
        self.call_helper(lambda helper: helper.normal_size())

    def on_larger_text_accel(self, group, accel, key, mod):
        self.call_helper(lambda helper: helper.larger_text())

    def on_smaller_text_accel(self, group, accel, key, mod):
        self.call_helper(lambda helper: helper.smaller_text())

    def on_normal_size_accel(self, group, accel, key, mod):
        self.call_helper(lambda helper: helper.normal_size())

    def on_tab_added(self, window, tab):
        self.add_document_helper(tab.get_view())

    def on_tab_removed(self, window, tab):
        self.remove_document_helper(tab.get_view())

    def _remap_proxy(self, action):
        # Remove previous proxy

        if action in self._proxy_mapping:
            item = self._proxy_mapping[action]
            self._accel_group.disconnect_key(item[0], item[1])

        self._install_proxy(action)

    def on_accel_map_changed(self, accelmap, path, key, mod):
        for action in self._action_group.list_actions():
            if action.get_accel_path() == path:
                self._remap_proxy(action)
                return
