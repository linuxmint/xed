# -*- coding: utf-8 -*-
#
#  json_formatter_window_activatable.py - JSON Formatter plugin for Xed
#
#  Copyright (C) 2026 Elnaril <elnaril@gmx.com>
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

import gi
gi.require_version("Xed", "1.0")
from gi.repository import GObject, Gtk, Xed

import gettext
gettext.install("xed")

from .formatter import format_json, minify_json, validate_json

MENU_PATH = "/MenuBar/ToolsMenu/ToolsOps_1"


class JsonFormatterWindowActivatable(GObject.Object, Xed.WindowActivatable):
    __gtype_name__ = "JsonFormatterWindowActivatable"
    window = GObject.Property(type=Xed.Window)

    def __init__(self):
        GObject.Object.__init__(self)
        self._action_group = None
        self._ui_id = 0
        self._statusbar = None
        self._context_id = 0
        self._document = None
        self._document_changed_id = 0

    def do_activate(self):
        self._statusbar = self.window.get_statusbar()
        self._context_id = self._statusbar.get_context_id("json-formatter")
        self._insert_menu()

    def do_deactivate(self):
        self._remove_menu()
        self._track_document(None)
        if self._statusbar is not None:
            self._statusbar.remove_all(self._context_id)

    def do_update_state(self):
        if self._action_group is None:
            return
        doc = self.window.get_active_document()
        if doc is not self._document:
            self._clear_status()
            self._track_document(doc)
        has_doc = doc is not None
        writable = has_doc and not doc.get_readonly()

        self._action_group.get_action("JsonVerifyAction").set_sensitive(has_doc)
        self._action_group.get_action("JsonFormatAction").set_sensitive(writable)
        self._action_group.get_action("JsonMinifyAction").set_sensitive(writable)

    def _insert_menu(self):
        manager = self.window.get_ui_manager()

        self._action_group = Gtk.ActionGroup(name="XedJsonFormatterPluginActions")
        self._action_group.add_actions([
            ("JsonVerifyAction", None, _("_Verify JSON"),
             "<Primary><Alt>v", None, self.on_verify),
            ("JsonFormatAction", None, _("_Format JSON"),
             "<Primary><Alt>f", None, self.on_format),
            ("JsonMinifyAction", None, _("_Minify JSON"),
             "<Primary><Alt>m", None, self.on_minify),
        ])
        manager.insert_action_group(self._action_group)

        self._ui_id = manager.new_merge_id()
        for name in ("JsonVerifyAction", "JsonFormatAction", "JsonMinifyAction"):
            manager.add_ui(self._ui_id, MENU_PATH, name, name,
                           Gtk.UIManagerItemType.MENUITEM, False)

    def _remove_menu(self):
        if self._action_group is None:
            return
        manager = self.window.get_ui_manager()
        manager.remove_ui(self._ui_id)
        manager.remove_action_group(self._action_group)
        manager.ensure_update()
        self._action_group = None

    def on_verify(self, action):
        valid, error = validate_json(self.get_current_text())
        if valid:
            self._notify(_("✅ Valid JSON"))
        else:
            self._notify(_("⚠️ Invalid JSON: {}").format(error))

    def on_format(self, action):
        self._transform(format_json, _("✅ Formatted JSON"))

    def on_minify(self, action):
        self._transform(minify_json, _("✅ Minified JSON"))

    def _transform(self, func, success_message):
        text = self.get_current_text()
        try:
            new_text = func(text)
        except ValueError as error:
            self._notify(_("⚠️ Invalid JSON: {}").format(error))
            return

        # Preserve the document's trailing newline convention.
        if text.endswith("\n") and not new_text.endswith("\n"):
            new_text += "\n"

        self.set_new_text(new_text)
        self._notify(success_message)

    def get_current_text(self) -> str:
        doc = self.window.get_active_document()
        if not doc:
            return ""
        return doc.get_text(
            start=doc.get_start_iter(),
            end=doc.get_end_iter(),
            include_hidden_chars=False,
        )

    def set_new_text(self, text: str) -> None:
        doc = self.window.get_active_document()
        if not doc:
            return

        # Keep the cursor on the same line when possible.
        line = doc.get_iter_at_mark(doc.get_insert()).get_line()

        doc.begin_user_action()
        doc.set_text(text)
        doc.end_user_action()

        line = min(line, doc.get_line_count() - 1)
        doc.place_cursor(doc.get_iter_at_line(line))

    def _track_document(self, doc) -> None:
        if self._document is not None:
            self._document.disconnect(self._document_changed_id)
            self._document_changed_id = 0
        self._document = doc
        if doc is not None:
            self._document_changed_id = doc.connect("changed", self._on_document_changed)

    def _on_document_changed(self, doc) -> None:
        self._clear_status()

    def _clear_status(self) -> None:
        self._statusbar.remove_all(self._context_id)

    def _notify(self, message: str) -> None:
        self._clear_status()
        self._statusbar.push(self._context_id, message)
