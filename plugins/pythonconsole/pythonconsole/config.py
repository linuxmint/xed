# -*- coding: utf-8 -*-

# config.py -- Config dialog
#
# Copyright (C) 2008 - B. Clausius
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.

# Parts from "Interactive Python-GTK Console" (stolen from epiphany's console.py)
#     Copyright (C), 1998 James Henstridge <james@daa.com.au>
#     Copyright (C), 2005 Adam Hooper <adamh@densi.com>
# Bits from pluma Python Console Plugin
#     Copyrignt (C), 2005 Raphaël Slinckx

import os
import gtk

__all__ = ('PythonConsoleConfig', 'PythonConsoleConfigDialog')

MATECONF_KEY_BASE = '/apps/pluma/plugins/pythonconsole'
MATECONF_KEY_COMMAND_COLOR = MATECONF_KEY_BASE + '/command-color'
MATECONF_KEY_ERROR_COLOR = MATECONF_KEY_BASE + '/error-color'

DEFAULT_COMMAND_COLOR = '#314e6c' # Blue Shadow
DEFAULT_ERROR_COLOR = '#990000' # Accent Red Dark

class PythonConsoleConfig(object):
    try:
        import mateconf
    except ImportError:
        mateconf = None

    def __init__(self):
        pass

    @staticmethod
    def enabled():
        return PythonConsoleConfig.mateconf != None

    @staticmethod
    def add_handler(handler):
        if PythonConsoleConfig.mateconf:
            PythonConsoleConfig.mateconf.client_get_default().notify_add(MATECONF_KEY_BASE, handler)

    color_command = property(
        lambda self: self.mateconf_get_str(MATECONF_KEY_COMMAND_COLOR, DEFAULT_COMMAND_COLOR),
        lambda self, value: self.mateconf_set_str(MATECONF_KEY_COMMAND_COLOR, value))

    color_error = property(
        lambda self: self.mateconf_get_str(MATECONF_KEY_ERROR_COLOR, DEFAULT_ERROR_COLOR),
        lambda self, value: self.mateconf_set_str(MATECONF_KEY_ERROR_COLOR, value))

    @staticmethod
    def mateconf_get_str(key, default=''):
        if not PythonConsoleConfig.mateconf:
            return default

        val = PythonConsoleConfig.mateconf.client_get_default().get(key)
        if val is not None and val.type == mateconf.VALUE_STRING:
            return val.get_string()
        else:
            return default

    @staticmethod
    def mateconf_set_str(key, value):
        if not PythonConsoleConfig.mateconf:
            return

        v = PythonConsoleConfig.mateconf.Value(mateconf.VALUE_STRING)
        v.set_string(value)
        PythonConsoleConfig.mateconf.client_get_default().set(key, v)

class PythonConsoleConfigDialog(object):

    def __init__(self, datadir):
        object.__init__(self)
        self._dialog = None
        self._ui_path = os.path.join(datadir, 'ui', 'config.ui')
        self.config = PythonConsoleConfig()

    def dialog(self):
        if self._dialog is None:
            self._ui = gtk.Builder()
            self._ui.add_from_file(self._ui_path)

            self.set_colorbutton_color(self._ui.get_object('colorbutton-command'),
                                        self.config.color_command)
            self.set_colorbutton_color(self._ui.get_object('colorbutton-error'),
                                        self.config.color_error)

            self._ui.connect_signals(self)

            self._dialog = self._ui.get_object('dialog-config')
            self._dialog.show_all()
        else:
            self._dialog.present()

        return self._dialog

    @staticmethod
    def set_colorbutton_color(colorbutton, value):
        try:
            color = gtk.gdk.color_parse(value)
        except ValueError:
            pass    # Default color in config.ui used
        else:
            colorbutton.set_color(color)

    def on_dialog_config_response(self, dialog, response_id):
        self._dialog.destroy()

    def on_dialog_config_destroy(self, dialog):
        self._dialog = None
        self._ui = None

    def on_colorbutton_command_color_set(self, colorbutton):
        self.config.color_command = colorbutton.get_color().to_string()

    def on_colorbutton_error_color_set(self, colorbutton):
        self.config.color_error = colorbutton.get_color().to_string()

# ex:et:ts=4:
