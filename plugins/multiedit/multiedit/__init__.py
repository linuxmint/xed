# -*- coding: utf-8 -*-
#
#  multiedit.py - Multi Edit
#
#  Copyright (C) 2009 - Jesse van den Kieboom
#  Copyright (C) 2020 - Linux Mint team <https://github.com/linuxmint
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

from gi.repository import GObject, Gtk, Xed
from .signals import Signals
from .documenthelper import DocumentHelper

import gettext
gettext.install("xed")

MENU_PATH = "/MenuBar/ViewMenu/ViewOps_1"

#class MultiEditPlugin(GObject.Object, Xed.WindowActivatable, Signals):
class MultiEditPlugin(GObject.Object, Xed.WindowActivatable):
    __gtype_name__ = "MultiEditPlugin"

    window = GObject.property(type=Xed.Window)

    def __init__(self):
        GObject.Object.__init__(self)
        Signals.__init__(self)

    def do_activate(self):
        self._views  = {}

        # Insert menu items
        self._insert_menu()

        for view in self.window.get_views():
            self.add_document_helper(view)

        #self.connect_signal(self.window, 'tab-added', self.on_tab_added)
        #self.connect_signal(self.window, 'tab-removed', self.on_tab_removed)
        #self.connect_signal(self.window, 'active-tab-changed', self.on_active_tab_changed)

        self.window.connect('tab-added', self.on_tab_added)
        self.window.connect('tab-removed', self.on_tab_removed)
        self.window.connect('active-tab-changed', self.on_active_tab_changed)

    def do_deactivate(self):
        print("do_deactivate")
        #self.disconnect_signals(self.window)

        self._remove_menu()

        for view in self.window.get_views():
            self.remove_document_helper(view)

    def _insert_menu(self):
        print("insert_menu")
        manager = self.window.get_ui_manager()

        self._action_group = Gtk.ActionGroup("XedMultiEditPluginActions")
        self._action_group.add_toggle_actions([('MultiEditModeAction',
                                                None,
                                                _('Multi Edit Mode'),
                                                '<Ctrl><Shift>C',
                                                _('Start multi edit mode'),
                                                self.on_multi_edit_mode)])

        manager.insert_action_group(self._action_group)

        self._ui_id = manager.new_merge_id()

        manager.add_ui(self._ui_id,
                       MENU_PATH,
                       "MultiEditModeAction",
                       "MultiEditModeAction",
                       Gtk.UIManagerItemType.MENUITEM,
                       False)

    def _remove_menu(self):
        print("remove_menu")
        manager = self.window.get_ui_manager()
        manager.remove_ui(self._ui_id)
        manager.remove_action_group(self._action_group)
        manager.ensure_update()

    def do_update_state(self):
        print("update_state")
        self._action_group.set_sensitive(self.window.get_active_document() != None)
        #pass

    def get_helper(self, view):
        print("get_helper")
        if not hasattr(view, "multiedit_document_helper"):
            print("none")
            return None
        print("where is helper?")
        return view.multiedit_document_helper

    def add_document_helper(self, view):
        print("add_document_helper")
        if self.get_helper(view) != None:
            return

        DocumentHelper(view)

        #helper = DocumentHelper(view)
        #helper.set_toggle_callback(self.on_multi_edit_toggled, helper)

    def remove_document_helper(self, view):
        print("remove_document_helper")
        helper = self.get_helper(view)

        if helper != None:
            helper.stop()

    def get_action(self):
        print("get_action")
        return self._action_group.get_action('MultiEditModeAction')

    def on_multi_edit_toggled(self, helper):
        print("multi_edit_toggled")
        if helper.get_view() == self.window.get_active_view():
            self.get_action().set_active(helper.enabled())

    def on_tab_added(self, window, tab):
        print("on_tab_added")
        self.add_document_helper(tab.get_view())

    def on_tab_removed(self, window, tab):
        print("on_tab_removed")
        self.remove_document_helper(tab.get_view())

    def on_active_tab_changed(self, window, tab):
        print("active_tab_changed")
        view = tab.get_view()
        helper = self.get_helper(view)

        self.get_action().set_active(helper != None and helper.enabled())

    def on_multi_edit_mode(self, action):
        print("multi_edit_mode")
        view = self.window.get_active_view()
        helper = self.get_helper(view)

        if helper != None:
            helper.toggle_multi_edit(self.get_action().get_active())

