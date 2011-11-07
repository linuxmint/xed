## Process this file with automake to produce Makefile.in

# see /gtk-doc/examples/Makefile.am

# We require automake 1.6 at least.
AUTOMAKE_OPTIONS = 1.6

# The name of the module, e.g. 'glib'.
DOC_MODULE=pluma

# The top-level SGML file. You can change this if you want to.
DOC_MAIN_SGML_FILE=$(DOC_MODULE)-docs.sgml

# The directory containing the source code. Relative to $(srcdir).
# gtk-doc will search all .c & .h files beneath here for inline comments
# documenting the functions and macros.
DOC_SOURCE_DIR=../../pluma

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
HFILE_GLOB=$(top_srcdir)/pluma/*.h
CFILE_GLOB=$(top_srcdir)/pluma/*.c

# Header files to ignore when scanning (These are internal to pluma).
IGNORE_HFILES=		\
	pluma-commands.h			\
	pluma-document-loader.h			\
	pluma-document-saver.h			\
	pluma-documents-panel.h			\
	pluma-io-error-message-area.h		\
	pluma-languages-manager.h		\
	pluma-message-area.h			\
	pluma-object-module.h			\
	pluma-plugin-manager.h			\
	pluma-plugins-engine.h			\
	pluma-prefs-manager-private.h		\
	pluma-session.h				\
	pluma-spinner.h				\
	pluma-ui.h				\
	pluma-window-private.h			\
	pluma-output-window.h			\
	pluma-close-confirmation-dialog.h	\
	pluma-encodings-dialog.h		\
	pluma-open-location-dialog.h		\
	pluma-page-setup-dialog.h		\
	pluma-preferences-dialog.h		\
	pluma-search-dialog.h			\
	pluma-marshal.h				\
	pluma-enum-types.h

# These files are not part of pluma but were copied into it for some reason.
# Do not parse them to make the docs.
IGNORE_HFILES +=	\
	bacon-message-connection.h		\
	plumatextregion.h			\
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
	-I$(top_srcdir)/pluma	\
	-I$(top_builddir)/pluma \
	-I$(top_srcdir)		\
	-I$(top_builddir)	\
	$(PLUMA_CFLAGS)

GTKDOC_LIBS=	\
	$(top_builddir)/pluma/libpluma.la	\
	$(PLUMA_LIBS)

if ENABLE_PYTHON
GTKDOC_CFLAGS += \
        $(NO_STRICT_ALIASING_CFLAGS)    \
        $(PYGTK_CFLAGS)                 \
        $(PYTHON_CFLAGS)                \
        $(AM_CFLAGS)
GTKDOC_LIBS += \
	$(top_builddir)/plugin-loaders/python/bindings/pluma.la
endif

# This includes the standard gtk-doc make rules, copied by gtkdocize.
include $(top_srcdir)/gtk-doc.make

# Other files to distribute
# e.g. EXTRA_DIST += version.xml.in

regenerate-types:
	grep _get_type $(DOC_SOURCE_DIR)/pluma-*.h | grep -oR '^.*\.h' | sort | uniq - | sed -r 's/^.*\.h$/#include "\0"/' > pluma.types
	grep _get_type $(DOC_SOURCE_DIR)/pluma-*.h | grep -oR '^.*\.h' | sort | uniq - | sed -re 'y/-/_/' -e 's/^(.*)\.h$/\1_get_type/' >> pluma.types.new

-include $(top_srcdir)/git.mk
