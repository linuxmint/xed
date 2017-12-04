## Process this file with automake to produce Makefile.in

bin_PROGRAMS = xed

pkglib_LTLIBRARIES = libxed.la

AM_CPPFLAGS =							\
	-I$(top_srcdir)						\
	-I$(srcdir)							\
	$(XED_CFLAGS)						\
	$(WARN_CFLAGS)						\
	$(INTROSPECTION_CFLAGS)				\
	$(DISABLE_DEPRECATED_CFLAGS)		\
	-DDATADIR=\""$(datadir)"\"			\
	-DLIBDIR=\""$(libdir)"\"

xed_SOURCES = \
	xed.c

xed_LDADD = 				\
	libxed.la 				\
	$(XED_LIBS) 			\
	$(INTROSPECTION_LIBS)

xed_LDFLAGS = -export-dynamic -no-undefined -export-symbols-regex "^[[^_]].*"

libxed_la_LDFLAGS = -avoid-version -export-dynamic -no-undefined -export-symbols-regex "^[^_].*"

# XED_LIBS must be the last to ensure correct order on some platforms
libxed_la_LIBADD = $(XED_LIBS)

BUILT_SOURCES = 			\
	xed-enum-types.c		\
	xed-enum-types.h		\
	xed-marshal.c			\
	xed-marshal.h


NOINST_H_FILES =						\
	xed-close-button.h					\
	xed-close-confirmation-dialog.h 	\
	xed-dirs.h							\
	xed-document-private.h 				\
	xed-documents-panel.h				\
	xed-encodings-dialog.h 				\
	xed-history-entry.h					\
	xed-io-error-info-bar.h				\
	xed-metadata-manager.h 				\
	xed-paned.h 						\
	xed-plugins-engine.h				\
	xed-preferences-dialog.h 			\
	xed-print-job.h						\
	xed-print-preview.h					\
	xed-settings.h 						\
	xed-status-combo-box.h 				\
	xed-tab-label.h						\
	xed-ui.h							\
	xed-utils.h 						\
	xed-view-frame.h 					\
	xed-view-gutter-renderer.h 			\
	xed-window-private.h

INST_H_FILES =							\
	xed-app.h							\
	xed-app-activatable.h 				\
	xed-commands.h						\
	xed-debug.h							\
	xed-document.h 						\
	xed-encodings-combo-box.h			\
	xed-file-chooser-dialog.h			\
	xed-message-bus.h					\
	xed-message-type.h					\
	xed-message.h						\
	xed-notebook.h						\
	xed-panel.h							\
	xed-progress-info-bar.h				\
	xed-searchbar.h						\
	xed-statusbar.h						\
	xed-tab.h 							\
	xed-view.h 							\
	xed-view-activatable.h 				\
	xed-window.h 						\
	xed-window-activatable.h

headerdir = $(prefix)/include/xed

header_DATA = 							\
	$(INST_H_FILES)

BUILT_SOURCES_PRIVATE = 				\
	xed-resources.c

libxed_c_files = 						\
	xed-app.c 							\
	xed-app-activatable.c 				\
	xed-view-activatable.c 				\
	xed-window-activatable.c 			\
	xed-resources.c 					\
	xed-close-button.c					\
	xed-close-confirmation-dialog.c 	\
	xed-commands-documents.c			\
	xed-commands-edit.c					\
	xed-commands-file.c					\
	xed-commands-file-print.c			\
	xed-commands-help.c					\
	xed-commands-search.c				\
	xed-commands-view.c					\
	xed-debug.c							\
	xed-dirs.c							\
	xed-document.c 						\
	xed-documents-panel.c				\
	xed-encodings-combo-box.c			\
	xed-encodings-dialog.c 				\
	xed-file-chooser-dialog.c			\
	xed-history-entry.c					\
	xed-io-error-info-bar.c				\
	xed-message-bus.c					\
	xed-message-type.c					\
	xed-message.c						\
	xed-metadata-manager.c 				\
	xed-notebook.c						\
	xed-paned.c 						\
	xed-panel.c							\
	xed-plugins-engine.c				\
	xed-preferences-dialog.c 			\
	xed-print-job.c						\
	xed-print-preview.c					\
	xed-progress-info-bar.c				\
	xed-settings.c 						\
	xed-searchbar.c						\
	xed-statusbar.c						\
	xed-status-combo-box.c				\
	xed-tab.c 							\
	xed-tab-label.c						\
	xed-utils.c 						\
	xed-view.c 							\
	xed-view-frame.c 					\
	xed-view-gutter-renderer.c 			\
	xed-window.c

libxed_la_SOURCES = 					\
	$(BUILT_SOURCES)					\
	$(libxed_c_files)					\
	$(POSIXIO_FILES)					\
	$(NOINST_H_FILES)					\
	$(INST_H_FILES)

xed-enum-types.h: xed-enum-types.h.template $(INST_H_FILES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && $(GLIB_MKENUMS) --template xed-enum-types.h.template $(INST_H_FILES)) > $@

xed-enum-types.c: xed-enum-types.c.template $(INST_H_FILES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && $(GLIB_MKENUMS) --template xed-enum-types.c.template $(INST_H_FILES)) > $@

xed-marshal.h: xed-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) $(GLIB_GENMARSHAL) $< --header --prefix=xed_marshal > $@

xed-marshal.c: xed-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) echo "#include \"xed-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=xed_marshal >> $@

xed-resources.c: resources/xed.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir)/resources --generate-dependencies $(srcdir)/resources/xed.gresource.xml)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir)/resources --generate-source $(srcdir)/resources/xed.gresource.xml

EXTRA_DIST = 								\
	xed-enum-types.h.template				\
	xed-enum-types.c.template				\
	xed-marshal.list						\
	xed.rc 									\
	resources/xed.gresource.xml 			\
	resources/ui/xed-ui.xml 				\
	resources/ui/xed-encodings-dialog.ui 	\
	resources/ui/xed-preferences-dialog.ui 	\
	resources/ui/xed-print-preferences.ui 	\
	resources/ui/xed-searchbar.ui 			\
	resources/ui/xed-view-frame.ui

CLEANFILES = $(BUILT_SOURCES) $(BUILT_SOURCES_PRIVATE)

if HAVE_INTROSPECTION
-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = Xed-1.0.gir

Xed-1.0.gir: xed
INTROSPECTION_SCANNER_ARGS = -I$(top_srcdir) --warn-all
Xed_1_0_gir_NAMESPACE = Xed
Xed_1_0_gir_VERSION = 1.0
Xed_1_0_gir_PROGRAM = $(builddir)/xed
Xed_1_0_gir_FILES = $(INST_H_FILES) $(libxed_c_files) $(BUILT_SOURCES)
Xed_1_0_gir_INCLUDES = Gtk-3.0 GtkSource-3.0

girdir = $(datadir)/xed/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/xed/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += 		\
	$(gir_DATA)		\
	$(typelib_DATA)
endif

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES) $(BUILT_SOURCES_PRIVATE)

-include $(top_srcdir)/git.mk
