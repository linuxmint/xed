import os
import tempfile
import sys
import shutil

from snippets.Library import *

class Importer:
        def __init__(self, filename):
                self.filename = filename

        def import_destination(self, filename):
                userdir = Library().userdir
                
                filename = os.path.basename(filename)
                (root, ext) = os.path.splitext(filename)

                filename = os.path.join(userdir, root + ext)
                i = 1
                
                while os.path.exists(filename):
                        filename = os.path.join(userdir, root + '_' + str(i) + ext)
                        i += 1
                
                return filename
                
        def import_file(self, filename):
                if not os.path.exists(filename):
                        return _('File "%s" does not exist') % filename
                
                if not os.path.isfile(filename):
                        return _('File "%s" is not a valid snippets file') % filename
                
                # Find destination for file to copy to
                dest = self.import_destination(filename)

                # Copy file
                shutil.copy(filename, dest)
                
                # Add library
                if not Library().add_user_library(dest):
                        return _('Imported file "%s" is not a valid snippets file') % os.path.basename(dest)
        
        def import_xml(self):
                return self.import_file(self.filename)

        def import_archive(self, cmd):
                dirname = tempfile.mkdtemp()
                status = os.system('cd %s; %s "%s"' % (dirname, cmd, self.filename))
                
                if status != 0:
                        return _('The archive "%s" could not be extracted' % self.filename)
                
                errors = []

                # Now import all the files from the archive
                for f in os.listdir(dirname):
                        f = os.path.join(dirname, f)

                        if os.path.isfile(f):
                                if self.import_file(f):
                                        errors.append(os.path.basename(f))
                        else:
                                sys.stderr.write('Skipping %s, not a valid snippets file' % os.path.basename(f))
                
                # Remove the temporary directory
                shutil.rmtree(dirname)

                if len(errors) > 0:
                        return _('The following files could not be imported: %s') % ', '.join(errors)
                        
        def import_targz(self):
                self.import_archive('tar -x --gzip -f')
        
        def import_tarbz2(self):
                self.import_archive('tar -x --bzip2 -f')

        def import_tar(self):
                self.import_archive('tar -xf')
        
        def run(self):
                if not os.path.exists(self.filename):
                        return _('File "%s" does not exist') % self.filename
                
                if not os.path.isfile(self.filename):
                        return _('File "%s" is not a valid snippets archive') % self.filename
                
                (root, ext) = os.path.splitext(self.filename)
                
                actions = {'.tar.gz': self.import_targz,
                           '.tar.bz2': self.import_tarbz2,
                           '.xml': self.import_xml,
                           '.tar': self.import_tar}

                for k, v in actions.items():
                        if self.filename.endswith(k):
                                return v()
                        
                return _('File "%s" is not a valid snippets archive') % self.filename
# ex:ts=8:et:
