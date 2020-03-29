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
              lambda w: self.join_lines(w)),
             ("SplitLinesAction", None, _('_Split Lines'), "<Shift><Ctrl>J",
              _("Split the selected lines"),
              lambda w: self.split_lines(w))])

        manager.insert_action_group(self._action_group)

        self._ui_id = manager.new_merge_id()

        manager.add_ui(self._ui_id,
                       MENU_PATH,
                       "JoinLinesAction",
                       "JoinLinesAction",
                       Gtk.UIManagerItemType.MENUITEM,
                       False)

        manager.add_ui(self._ui_id,
                       MENU_PATH,
                       "SplitLinesAction",
                       "SplitLinesAction",
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

    def split_lines(self, w):
        view = self.window.get_active_view()
        if view is None:
            return

        doc = view.get_buffer()

        width = view.get_right_margin_position()
        tabwidth = view.get_tab_width()

        doc.begin_user_action()

        try:
            # get selection bounds
            start, end = doc.get_selection_bounds()

            # measure indent until selection start
            indent_iter = start.copy()
            indent_iter.set_line_offset(0)
            indent = ''
            while indent_iter.get_offset() != start.get_offset():
                if indent_iter.get_char() == '\t':
                    indent = indent + '\t'
                else:
                    indent = indent + ' '
                indent_iter.forward_char()
        except ValueError:
            # select from start to line end
            start = doc.get_iter_at_mark(doc.get_insert())
            start.set_line_offset(0)
            end = start.copy()
            if not end.ends_line():
                end.forward_to_line_end()

            # measure indent of line
            indent_iter = start.copy()
            indent = ''
            while indent_iter.get_char() in (' ', '\t'):
                indent = indent + indent_iter.get_char()
                indent_iter.forward_char()

        end_mark = doc.create_mark(None, end)

        # ignore first word
        previous_word_end = start.copy()
        self.forward_to_word_start(previous_word_end)
        self.forward_to_word_end(previous_word_end)

        while 1:
            current_word_start = previous_word_end.copy()
            self.forward_to_word_start(current_word_start)

            current_word_end = current_word_start.copy()
            self.forward_to_word_end(current_word_end)

            if current_word_end.get_char() != '' and ord(current_word_end.get_char()) and \
               doc.get_iter_at_mark(end_mark).compare(current_word_end) >= 0:

                word_length = current_word_end.get_offset() - \
                              current_word_start.get_offset()

                doc.delete(previous_word_end, current_word_start)

                line_offset = self.get_line_offset(current_word_start, tabwidth) + word_length
                if line_offset > width - 1:
                    doc.insert(current_word_start, '\n' + indent)
                else:
                    doc.insert(current_word_start, ' ')

                previous_word_end = current_word_start.copy()
                previous_word_end.forward_chars(word_length)
            else:
                break

        doc.delete_mark(end_mark)
        doc.end_user_action()

    def get_line_offset(self, text_iter, tabwidth):
        offset_iter = text_iter.copy()
        offset_iter.set_line_offset(0)

        line_offset = 0
        while offset_iter.get_offset() < text_iter.get_offset():
            char = offset_iter.get_char()
            if char == '\t':
                line_offset += tabwidth
            else:
                line_offset += 1
            offset_iter.forward_char()

        return line_offset

    def forward_to_word_start(self, text_iter):
        char = text_iter.get_char()
        while char != '' and ord(char) and (char in (' ', '\t', '\n', '\r')):
            text_iter.forward_char()
            char = text_iter.get_char()

    def forward_to_word_end(self, text_iter):
        char = text_iter.get_char()
        while char != '' and ord(char) and (not (char in (' ', '\t', '\n', '\r', ''))):
            text_iter.forward_char()
            char = text_iter.get_char()

