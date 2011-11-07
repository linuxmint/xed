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

import gtk
import gio

class VirtualDirectory:
        def __init__(self, name):
                self._name = name
                self._children = []

        def get_uri(self):
                return 'virtual://' + self._name

        def get_parent(self):
                return None

        def enumerate_children(self, attr):
                return self._children

        def append(self, child):
                if not child.is_native():
                        return

                try:
                        info = child.query_info("standard::*")

                        if info:
                                self._children.append((child, info))
                except:
                        pass

class RecentDocumentsDirectory(VirtualDirectory):
        def __init__(self, maxitems=10, screen=None):
                VirtualDirectory.__init__(self, 'recent')

                self._maxitems = maxitems
                self.fill(screen)

        def fill(self, screen):
                if screen:
                        manager = gtk.recent_manager_get_for_screen(screen)
                else:
                        manager = gtk.recent_manager_get_default()

                items = manager.get_items()
                items.sort(lambda a, b: cmp(b.get_visited(), a.get_visited()))

                added = 0

                for item in items:
                        if item.has_group('gedit'):
                                self.append(gio.File(item.get_uri()))
                                added += 1

                                if added >= self._maxitems:
                                        break

class CurrentDocumentsDirectory(VirtualDirectory):
        def __init__(self, window):
                VirtualDirectory.__init__(self, 'documents')

                self.fill(window)

        def fill(self, window):
                for doc in window.get_documents():
                        location = doc.get_location()
                        if location:
                                self.append(location)

# ex:ts=8:et:
