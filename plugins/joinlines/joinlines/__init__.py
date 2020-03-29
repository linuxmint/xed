from gi.repository import GObject, Gtk, Xed

import gettext
gettext.install("xed")

MENU_PATH = "/MenuBar/ViewMenu/ViewOps_1"

class JoinLinesPlugin(GObject.Object, Xed.WindowActivatable):
    __gtype_name__ = "JoinLinesPlugin"

    window = GObject.property(type=Xed.Window)

    def __init__(self):
        GObject.Object.__init__(self)

    def do_activate(self):
        self._views = {}

        self._insert_menu()

    def _insert_menu(self):
        manager = self.window.get_ui_manager()

        self._action_group = Gtk.ActionGroup(name="XedJoinLinesPluginActions")
        self._action_group.add_actions(
            [("JoinLinesAction", None, _("_Join Lines"), "<Ctrl>J",
              _("Join the selected lines"),
              lambda w: self.join_lines(w))])

        manager.insert_action_group(self._action_group)

        self._ui_id = manager.new_merge_id()

        manager.add_ui(self._ui_id,
                       MENU_PATH,
                       "JoinLinesAction",
                       "JoinLinesAction",
                       Gtk.UIManagerItemType.MENUITEM,
                       False)

    def do_update_state(self):
        self._action_group.set_sensitive(self.window.get_active_document() != None)

    def do_deactivate(self):
        self._remove_menu()


    def _remove_menu(self):
        manager = self.window.get_ui_manager()
        manager.remove_ui(self._ui_id)
        manager.remove_action_group(self._action_group)
        manager.ensure_update()

    def join_lines(self, w):
        doc = self.window.get_active_document()
        if doc is None:
            return

        doc.begin_user_action()

        # If there is a selection use it, otherwise join the
        # next line
        try:
            start, end = doc.get_selection_bounds()
        except ValueError:
            start = doc.get_iter_at_mark(doc.get_insert())
            end = start.copy()
            end.forward_line()

        end_mark = doc.create_mark(None, end)

        if not start.ends_line():
            start.forward_to_line_end()

        # Include trailing spaces in the chunk to be removed
        while start.backward_char() and start.get_char() in ('\t', ' '):
            pass
        start.forward_char()

        while doc.get_iter_at_mark(end_mark).compare(start) == 1:
            end = start.copy()
            while end.get_char() in ('\r', '\n', ' ', '\t'):
                end.forward_char()
            doc.delete(start, end)

            doc.insert(start, ' ')
            start.forward_to_line_end()

        doc.delete_mark(end_mark)
        doc.end_user_action()

