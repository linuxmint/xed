# -*- coding: utf-8 -*-

# config.py -- Config dialog
#
# Copyright (C) 2008 - B. Clausius
# Copyright (C) 2012-2021 MATE Developers
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
# Bits from xed Python Console Plugin
#     Copyrignt (C), 2005 Raphaël Slinckx

import os
from gi.repository import Gio, Gtk, Gdk

__all__ = ('PythonConsoleConfig', 'PythonConsoleConfigWidget')

class PythonConsoleConfig(object):

    CONSOLE_KEY_BASE = 'org.x.editor.plugins.pythonconsole'
    CONSOLE_KEY_COMMAND_COLOR = 'command-color'
    CONSOLE_KEY_ERROR_COLOR = 'error-color'
    CONSOLE_KEY_USE_SYSTEM_FONT = 'use-system-font'
    CONSOLE_KEY_FONT = 'font'

    INTERFACE_KEY_BASE = 'org.mate.interface'
    INTERFACE_KEY_MONOSPACE_FONT_NAME = 'monospace-font-name'

    color_command = property(
        lambda self: self.console_settings.get_string(self.CONSOLE_KEY_COMMAND_COLOR),
        lambda self, value: self.console_settings.set_string(self.CONSOLE_KEY_COMMAND_COLOR, value)
    )

    color_error = property(
        lambda self: self.console_settings.get_string(self.CONSOLE_KEY_ERROR_COLOR),
        lambda self, value: self.console_settings.set_string(self.CONSOLE_KEY_ERROR_COLOR, value)
    )

    use_system_font = property(
        lambda self: self.console_settings.get_boolean(self.CONSOLE_KEY_USE_SYSTEM_FONT),
        lambda self, value: self.console_settings.set_boolean(self.CONSOLE_KEY_USE_SYSTEM_FONT, value)
    )

    font = property(
        lambda self: self.console_settings.get_string(self.CONSOLE_KEY_FONT),
        lambda self, value: self.console_settings.set_string(self.CONSOLE_KEY_FONT, value)
    )

    monospace_font_name = property(
        lambda self: self.interface_settings.get_string(self.INTERFACE_KEY_MONOSPACE_FONT_NAME)
    )

    console_settings = Gio.Settings.new(CONSOLE_KEY_BASE)
    interface_settings = Gio.Settings.new(INTERFACE_KEY_BASE)

    def __init__(self):
        object.__init__(self)

    @classmethod
    def enabled(self):
        return self.console_settings != None

    @classmethod
    def add_handler(self, handler):
        self.console_settings.connect("changed", handler)
        self.interface_settings.connect("changed", handler)


class PythonConsoleConfigWidget(object):

    CONSOLE_KEY_BASE = 'org.x.editor.plugins.pythonconsole'
    CONSOLE_KEY_COMMAND_COLOR = 'command-color'
    CONSOLE_KEY_ERROR_COLOR = 'error-color'

    def __init__(self, datadir):
        object.__init__(self)
        self._widget = None
        self._ui_path = os.path.join(datadir, 'ui', 'config.ui')
        self._config = PythonConsoleConfig()
        self._ui = Gtk.Builder()

    def configure_widget(self):
        if self._widget is None:
            self._ui.add_from_file(self._ui_path)

            self.set_colorbutton_color(self._ui.get_object('colorbutton-command'),
                                        self._config.color_command)
            self.set_colorbutton_color(self._ui.get_object('colorbutton-error'),
                                        self._config.color_error)
            checkbox = self._ui.get_object('checkbox-system-font')
            checkbox.set_active(self._config.use_system_font)
            self._fontbutton = self._ui.get_object('fontbutton-font')
            self._fontbutton.set_font_name(self._config.font)
            self.on_checkbox_system_font_toggled(checkbox)
            self._ui.connect_signals(self)

            self._widget = self._ui.get_object('widget-config')
            self._widget.show_all()

        return self._widget

    @staticmethod
    def set_colorbutton_color(colorbutton, value):
        rgba = Gdk.RGBA()
        parsed = rgba.parse(value)

        if parsed:
            colorbutton.set_rgba(rgba)

    def on_colorbutton_command_color_set(self, colorbutton):
        self._config.color_command = colorbutton.get_color().to_string()

    def on_colorbutton_error_color_set(self, colorbutton):
        self._config.color_error = colorbutton.get_color().to_string()

    def on_checkbox_system_font_toggled(self, checkbox):
        val = checkbox.get_active()
        self._config.use_system_font = val
        self._fontbutton.set_sensitive(not val)

    def on_fontbutton_font_set(self, fontbutton):
        self._config.font = fontbutton.get_font_name()

    def on_widget_config_parent_set(self, widget, oldparent):
        # Set icon in dialog close button.
        try:
            actionarea = widget.get_toplevel().get_action_area()
            image = Gtk.Image.new_from_icon_name("window-close",
                                                 Gtk.IconSize.BUTTON)
            for button in actionarea.get_children():
                button.set_image(image)
                button.set_property("always-show-image", True)
        except:
            pass

# ex:et:ts=4:
