## Process this file with automake to produce Makefile.in

# see /gtk-doc/examples/Makefile.am

# We require automake 1.6 at least.
AUTOMAKE_OPTIONS = 1.6

# The name of the module, e.g. 'glib'.
DOC_MODULE=xedit

# The top-level SGML file. You can change this if you want to.
DOC_MAIN_SGML_FILE=$(DOC_MODULE)-docs.sgml

# The directory containing the source code. Relative to $(srcdir).
# gtk-doc will search all .c & .h files beneath here for inline comments
# documenting the functions and macros.
DOC_SOURCE_DIR=../../xedit

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
HFILE_GLOB=$(top_srcdir)/xedit/*.h
CFILE_GLOB=$(top_srcdir)/xedit/*.c

# Header files to ignore when scanning (These are internal to xedit).
IGNORE_HFILES=		\
	xedit-commands.h			\
	xedit-document-loader.h			\
	xedit-document-saver.h			\
	xedit-documents-panel.h			\
	xedit-io-error-message-area.h		\
	xedit-languages-manager.h		\
	xedit-object-module.h			\
	xedit-plugin-manager.h			\
	xedit-plugins-engine.h			\
	xedit-prefs-manager-private.h		\
	xedit-session.h				\
	xedit-ui.h				\
	xedit-window-private.h			\
	xedit-output-window.h			\
	xedit-close-confirmation-dialog.h	\
	xedit-encodings-dialog.h		\
	xedit-open-location-dialog.h		\
	xedit-page-setup-dialog.h		\
	xedit-preferences-dialog.h		\
	xedit-search-dialog.h			\
	xedit-marshal.h				\
	xedit-enum-types.h

# These files are not part of xedit but were copied into it for some reason.
# Do not parse them to make the docs.
IGNORE_HFILES +=	\
	bacon-message-connection.h		\
	xedittextregion.h			\
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
	-I$(top_srcdir)/xedit	\
	-I$(top_builddir)/xedit \
	-I$(top_srcdir)		\
	-I$(top_builddir)	\
	$(XEDIT_CFLAGS)

GTKDOC_LIBS=	\
	$(top_builddir)/xedit/libxedit.la	\
	$(XEDIT_LIBS)

if ENABLE_PYTHON
GTKDOC_CFLAGS += \
        $(NO_STRICT_ALIASING_CFLAGS)    \
        $(PYGTK_CFLAGS)                 \
        $(PYTHON_CFLAGS)                \
        $(AM_CFLAGS)
GTKDOC_LIBS += \
	$(top_builddir)/plugin-loaders/python/bindings/xedit.la
endif

# This includes the standard gtk-doc make rules, copied by gtkdocize.
include $(top_srcdir)/gtk-doc.make

# Other files to distribute
# e.g. EXTRA_DIST += version.xml.in

regenerate-types:
	grep _get_type $(DOC_SOURCE_DIR)/xedit-*.h | grep -oR '^.*\.h' | sort | uniq - | sed -r 's/^.*\.h$/#include "\0"/' > xedit.types
	grep _get_type $(DOC_SOURCE_DIR)/xedit-*.h | grep -oR '^.*\.h' | sort | uniq - | sed -re 'y/-/_/' -e 's/^(.*)\.h$/\1_get_type/' >> xedit.types.new

-include $(top_srcdir)/git.mk
