#    Gedit snippets plugin
#    Copyright (C) 2005-2006  Jesse van den Kieboom <jesse@icecrew.nl>
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

import re
import os
import gettext

import gtk
from gtk import gdk
import gedit

from Document import Document
from Library import Library

class WindowHelper:
        def __init__(self, plugin):
                self.plugin = plugin
                self.current_controller = None
                self.current_language = None
                self.signal_ids = {}
                
        def run(self, window):
                self.window = window

                self.insert_menu()
                self.register_messages()
                
                self.accel_group = Library().get_accel_group(None)
                
                window.add_accel_group(self.accel_group)
                window.connect('tab-added', self.on_tab_added)
                
                # Add controllers to all the current views
                for view in self.window.get_views():
                        if isinstance(view, gedit.View) and not self.has_controller(view):
                                view._snippet_controller = Document(self, view)
                
                self.update()
        
        def stop(self):
                self.window.remove_accel_group(self.accel_group)
                self.accel_group = None
                
                #self.window.remove_accel_group(accel)
                self.remove_menu()
                self.unregister_messages()

                # Iterate over all the tabs and remove every controller
                for view in self.window.get_views():
                        if isinstance(view, gedit.View) and self.has_controller(view):
                                view._snippet_controller.stop()
                                view._snippet_controller = None
                
                self.window = None
                self.plugin = None
        
        def register_messages(self):
                bus = self.window.get_message_bus()
                
                self.messages = {
                        'activate': bus.register('/plugins/snippets', 'activate', ('view', 'iter'), trigger=str, view=gedit.View, iter=gtk.TextIter),
                        'parse-and-activate': bus.register('/plugins/snippets', 'parse-and-activate', ('view', 'iter'), snippet=str, view=gedit.View, iter=gtk.TextIter)
                }
                
                bus.connect('/plugins/snippets', 'activate', self.on_message_activate)
                bus.connect('/plugins/snippets', 'parse-and-activate', self.on_message_parse_and_activate)
        
        def unregister_messages(self):
                bus = self.window.get_message_bus()
                
                for name in self.messages:
                        bus.unregister(self.messages[name])
                
                self.messages = {}
        
        def on_message_activate(self, bus, message):
                if message.has_key('view'):
                        view = message.view
                else:
                        view = self.window.get_active_view()
                
                if not self.has_controller(view):
                        return
                
                if message.has_key('iter'):
                        iter = message.iter
                else:
                        iter = view.get_buffer().get_iter_at_mark(view.get_buffer().get_insert())
                
                controller = view._snippet_controller
                controller.run_snippet_trigger(message.trigger, (iter, iter))

        def on_message_parse_and_activate(self, bus, message):
                if message.has_key('view'):
                        view = message.view
                else:
                        view = self.window.get_active_view()
                
                if not self.has_controller(view):
                        return
                
                if message.has_key('iter'):
                        iter = message.iter
                else:
                        iter = view.get_buffer().get_iter_at_mark(view.get_buffer().get_insert())
                
                controller = view._snippet_controller
                controller.parse_and_run_snippet(message.snippet, iter)

        def insert_menu(self):
                manager = self.window.get_ui_manager()

                self.action_group = gtk.ActionGroup("GeditSnippetPluginActions")
                self.action_group.set_translation_domain('gedit')
                self.action_group.add_actions([('ManageSnippets', None,
                                _('Manage _Snippets...'), \
                                None, _('Manage snippets'), \
                                self.on_action_snippets_activate)])

                self.merge_id = manager.new_merge_id()
                manager.insert_action_group(self.action_group, -1)
                manager.add_ui(self.merge_id, '/MenuBar/ToolsMenu/ToolsOps_5', \
                                'ManageSnippets', 'ManageSnippets', gtk.UI_MANAGER_MENUITEM, False)

        def remove_menu(self):
                manager = self.window.get_ui_manager()
                manager.remove_ui(self.merge_id)
                manager.remove_action_group(self.action_group)
                self.action_group = None
        
        def find_snippet(self, snippets, tag):
                result = []
                
                for snippet in snippets:
                        if Snippet(snippet)['tag'] == tag:
                                result.append(snippet)
                
                return result

        def has_controller(self, view):
                return hasattr(view, '_snippet_controller') and view._snippet_controller

        def update_language(self):
                if not self.window:
                        return

                if self.current_language:
                        accel_group = Library().get_accel_group( \
                                        self.current_language)
                        self.window.remove_accel_group(accel_group)

                if self.current_controller:
                        self.current_language = self.current_controller.language_id
                        
                        if self.current_language != None:
                                accel_group = Library().get_accel_group( \
                                                self.current_language)
                                self.window.add_accel_group(accel_group)
                else:
                        self.current_language = None
                
        def language_changed(self, controller):
                if controller == self.current_controller:
                        self.update_language()
                
        def update(self):
                view = self.window.get_active_view()
                
                if not view or not self.has_controller(view):
                        return
                
                controller = view._snippet_controller
                
                if controller != self.current_controller:
                        self.current_controller = controller
                        self.update_language()

        # Callbacks
        
        def on_tab_added(self, window, tab):
                # Create a new controller for this tab if it has a standard gedit view
                view = tab.get_view()
                
                if isinstance(view, gedit.View) and not self.has_controller(view):
                        view._snippet_controller = Document(self, view)

                self.update()

        def on_action_snippets_activate(self, item):
                self.plugin.create_configure_dialog()

        def accelerator_activated(self, keyval, mod):
                return self.current_controller.accelerator_activate(keyval, mod)

# ex:ts=8:et:
