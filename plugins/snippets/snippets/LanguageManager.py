import gtksourceview2 as gsv
import os

from Library import Library

global manager
manager = None

def get_language_manager():
        global manager
        
        if not manager:
                dirs = []
        
                for d in Library().systemdirs:
                        dirs.append(os.path.join(d, 'lang'))
        
                manager = gsv.LanguageManager()
                manager.set_search_path(dirs + manager.get_search_path())
        
        return manager
