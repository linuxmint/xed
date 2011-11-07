import gtksourceview2 as gsv
import gobject
import gedit
import gtk

from Library import Library
from LanguageManager import get_language_manager
from Snippet import Snippet

class Proposal(gobject.GObject, gsv.CompletionProposal):
        def __init__(self, snippet):
                gobject.GObject.__init__(self)
                self._snippet = Snippet(snippet)
        
        def snippet(self):
                return self._snippet.data
        
        # Interface implementation
        def do_get_markup(self):
                return self._snippet.display()
        
        def do_get_info(self):
                return self._snippet.data['text']

class Provider(gobject.GObject, gsv.CompletionProvider):
        def __init__(self, name, language_id, handler):
                gobject.GObject.__init__(self)
                
                self.name = name
                self.info_widget = None
                self.proposals = []
                self.language_id = language_id
                self.handler = handler
                self.info_widget = None
                self.mark = None
                
                theme = gtk.icon_theme_get_default()
                w, h = gtk.icon_size_lookup(gtk.ICON_SIZE_MENU)

                self.icon = theme.load_icon(gtk.STOCK_JUSTIFY_LEFT, w, 0)
        
        def __del__(self):
                if self.mark:
                        self.mark.get_buffer().delete_mark(self.mark)
        
        def set_proposals(self, proposals):
                self.proposals = proposals

        def mark_position(self, it):
                if not self.mark:
                        self.mark = it.get_buffer().create_mark(None, it, True)
                else:
                        self.mark.get_buffer().move_mark(self.mark, it)
        
        def get_word(self, context):
                it = context.get_iter()
                
                if it.starts_word() or it.starts_line() or not it.ends_word():
                        return None
                
                start = it.copy()
                
                if start.backward_word_start():
                        self.mark_position(start)
                        return start.get_text(it)
                else:
                        return None
        
        def do_get_start_iter(self, context, proposal):
                if not self.mark or self.mark.get_deleted():
                        return None
                
                return self.mark.get_buffer().get_iter_at_mark(self.mark)
                
        def do_match(self, context):
                return True

        def get_proposals(self, word):
                if self.proposals:
                        proposals = self.proposals
                else:
                        proposals = Library().get_snippets(None)
                        
                        if self.language_id:
                                proposals += Library().get_snippets(self.language_id)

                # Filter based on the current word
                if word:
                        proposals = filter(lambda x: x['tag'].startswith(word), proposals)

                return map(lambda x: Proposal(x), proposals)

        def do_populate(self, context):
                proposals = self.get_proposals(self.get_word(context))
                context.add_proposals(self, proposals, True)

        def do_get_name(self):
                return self.name

        def do_activate_proposal(self, proposal, piter):
                return self.handler(proposal, piter)
        
        def do_get_info_widget(self, proposal):
                if not self.info_widget:
                        view = gedit.View(gedit.Document())
                        manager = get_language_manager()

                        lang = manager.get_language('snippets')
                        view.get_buffer().set_language(lang)
                        
                        sw = gtk.ScrolledWindow()
                        sw.add(view)
                        
                        self.info_view = view
                        self.info_widget = sw
                
                return self.info_widget
        
        def do_update_info(self, proposal, info):
                buf = self.info_view.get_buffer()
                
                buf.set_text(proposal.get_info())
                buf.move_mark(buf.get_insert(), buf.get_start_iter())
                buf.move_mark(buf.get_selection_bound(), buf.get_start_iter())
                self.info_view.scroll_to_iter(buf.get_start_iter(), False)

                info.set_sizing(-1, -1, False, False)
                info.process_resize()
        
        def do_get_icon(self):
                return self.icon

        def do_get_activation(self):
                return gsv.COMPLETION_ACTIVATION_USER_REQUESTED

class Defaults(gobject.GObject, gsv.CompletionProvider):
        def __init__(self, handler):
                gobject.GObject.__init__(self)

                self.handler = handler
                self.proposals = []
        
        def set_defaults(self, defaults):
                self.proposals = []
                
                for d in defaults:
                        self.proposals.append(gsv.CompletionItem(d))
                
        def do_get_name(self):
                return ""
        
        def do_activate_proposal(self, proposal, piter):
                return self.handler(proposal, piter)
        
        def do_populate(self, context):
                context.add_proposals(self, self.proposals, True)

        def do_get_activation(self):
                return gsv.COMPLETION_ACTIVATION_NONE

gobject.type_register(Proposal)
gobject.type_register(Provider)
gobject.type_register(Defaults)

# ex:ts=8:et:
