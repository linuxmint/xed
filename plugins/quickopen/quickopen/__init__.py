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
from windowhelper import WindowHelper

class QuickOpenPlugin(gedit.Plugin):
        def __init__(self):
                gedit.Plugin.__init__(self)

                self._popup_size = (450, 300)
                self._helpers = {}

        def activate(self, window):
                self._helpers[window] = WindowHelper(window, self)

        def deactivate(self, window):
                self._helpers[window].deactivate()
                del self._helpers[window]

        def update_ui(self, window):
                self._helpers[window].update_ui()

        def get_popup_size(self):
                return self._popup_size

        def set_popup_size(self, size):
                self._popup_size = size

# ex:ts=8:et:
