## Process this file with automake to produce Makefile.in

# see /gtk-doc/examples/Makefile.am

# We require automake 1.6 at least.
AUTOMAKE_OPTIONS = 1.6

# The name of the module, e.g. 'glib'.
DOC_MODULE=gedit

# The top-level SGML file. You can change this if you want to.
DOC_MAIN_SGML_FILE=$(DOC_MODULE)-docs.sgml

# The directory containing the source code. Relative to $(srcdir).
# gtk-doc will search all .c & .h files beneath here for inline comments
# documenting the functions and macros.
DOC_SOURCE_DIR=../../gedit

# Extra options to pass to gtkdoc-scangobj. Not normally needed.
SCANGOBJ_OPTIONS=

# Extra options to supply to gtkdoc-scan.
# e.g. SCAN_OPTIONS=--deprecated-guards="GTK_DISABLE_DEPRECATED" 
SCAN_OPTIONS=

# Extra options to supply to gtkdoc-mkdb.
MKDB_OPTIONS=--sgml-mode --output-format=xml

# Extra options to supply to gtkdoc-mktmpl
# e.g. MKTMPL_OPTIONS=--only-section-tmpl
MKTMPL_OPTIONS=

# Extra options to supply to gtkdoc-fixref. Not normally needed.
# e.g. FIXXREF_OPTIONS=--extra-dir=../gdk-pixbuf/html --extra-dir=../gdk/html
FIXXREF_OPTIONS=

# Used for dependencies. The docs will be rebuilt if any of these change.
HFILE_GLOB=$(top_srcdir)/gedit/*.h
CFILE_GLOB=$(top_srcdir)/gedit/*.c

# Header files to ignore when scanning (These are internal to gedit).
IGNORE_HFILES=		\
	gedit-commands.h			\
	gedit-document-loader.h			\
	gedit-document-saver.h			\
	gedit-documents-panel.h			\
	gedit-io-error-message-area.h		\
	gedit-languages-manager.h		\
	gedit-message-area.h			\
	gedit-object-module.h			\
	gedit-plugin-manager.h			\
	gedit-plugins-engine.h			\
	gedit-prefs-manager-private.h		\
	gedit-session.h				\
	gedit-spinner.h				\
	gedit-ui.h				\
	gedit-window-private.h			\
	gedit-output-window.h			\
	gedit-close-confirmation-dialog.h	\
	gedit-encodings-dialog.h		\
	gedit-open-location-dialog.h		\
	gedit-page-setup-dialog.h		\
	gedit-preferences-dialog.h		\
	gedit-search-dialog.h			\
	gedit-marshal.h				\
	gedit-enum-types.h

# These files are not part of gedit but were copied into it for some reason.
# Do not parse them to make the docs.
IGNORE_HFILES +=	\
	bacon-message-connection.h		\
	gedittextregion.h			\
	sexy-icon-entry.h

# Images to copy into HTML directory.
HTML_IMAGES=

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE).
# e.g. content_files=running.sgml building.sgml changes-2.0.sgml
content_files=

# SGML files where gtk-doc abbrevations (#GtkWidget) are expanded
# These files must be listed here *and* in content_files
# e.g. expand_content_files=running.sgml
expand_content_files=

# CFLAGS and LDFLAGS for compiling gtkdoc-scangobj with your library.
# Only needed if you are using gtkdoc-scangobj to dynamically query widget
# signals and properties.
GTKDOC_CFLAGS=	\
	-I$(top_srcdir)/gedit	\
	-I$(top_builddir)/gedit \
	-I$(top_srcdir)		\
	-I$(top_builddir)	\
	$(GEDIT_CFLAGS)

GTKDOC_LIBS=	\
	$(top_builddir)/gedit/libgedit.la	\
	$(GEDIT_LIBS)

if ENABLE_PYTHON
GTKDOC_CFLAGS += \
        $(NO_STRICT_ALIASING_CFLAGS)    \
        $(PYGTK_CFLAGS)                 \
        $(PYTHON_CFLAGS)                \
        $(AM_CFLAGS)
GTKDOC_LIBS += \
	$(top_builddir)/plugin-loaders/python/bindings/gedit.la
endif

# This includes the standard gtk-doc make rules, copied by gtkdocize.
include $(top_srcdir)/gtk-doc.make

# Other files to distribute
# e.g. EXTRA_DIST += version.xml.in

regenerate-types:
	grep _get_type $(DOC_SOURCE_DIR)/gedit-*.h | grep -oR '^.*\.h' | sort | uniq - | sed -r 's/^.*\.h$/#include "\0"/' > gedit.types
	grep _get_type $(DOC_SOURCE_DIR)/gedit-*.h | grep -oR '^.*\.h' | sort | uniq - | sed -re 'y/-/_/' -e 's/^(.*)\.h$/\1_get_type/' >> gedit.types.new

-include $(top_srcdir)/git.mk
