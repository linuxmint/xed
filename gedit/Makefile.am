## Process this file with automake to produce Makefile.in
SUBDIRS = dialogs smclient

if OS_OSX
SUBDIRS += osx
endif

bin_PROGRAMS = gedit

noinst_LTLIBRARIES = libgedit.la

INCLUDES =								\
	-I$(top_srcdir)							\
	-I$(srcdir)							\
	-I$(srcdir)/smclient						\
	$(GEDIT_CFLAGS)							\
	$(IGE_MAC_CFLAGS)						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)					\
	-DDATADIR=\""$(datadir)"\"					\
	-DLIBDIR=\""$(libdir)"\"					

gedit_SOURCES = \
	gedit.c

gedit_LDADD = libgedit.la $(GEDIT_LIBS) $(IGE_MAC_LIBS) $(EGG_SMCLIENT_LIBS)
 
if PLATFORM_WIN32
gedit_LDFLAGS = -Wl,--export-all-symbols -Wl,--out-implib,libgedit-$(GEDIT_API_VERSION).a
if OS_WIN32
gedit_LDFLAGS += -mwindows
endif
else
gedit_LDFLAGS = -export-dynamic -no-undefined -export-symbols-regex "^[[^_]].*"
endif

libgedit_la_LDFLAGS = -export-dynamic -no-undefined -export-symbols-regex "^[[^_]].*"

libgedit_la_LIBADD = \
	dialogs/libdialogs.la		\
	smclient/libeggsmclient.la

# GEDIT_LIBS must be the last to ensure correct order on some platforms
libgedit_la_LIBADD += $(GEDIT_LIBS)

if OS_OSX
gedit_LDFLAGS += -framework Carbon

libgedit_la_LIBADD += osx/libosx.la
endif

BUILT_SOURCES = 			\
	gedit-enum-types.c		\
	gedit-enum-types.h		\
	gedit-marshal.c			\
	gedit-marshal.h

if OS_WIN32
gedit-res.o: gedit.rc
	$(WINDRES) -i gedit.rc --input-format=rc -o gedit-res.o -O coff

gedit_LDADD += gedit-res.o
endif

NOINST_H_FILES =			\
	gedit-close-button.h		\
	gedit-dirs.h			\
	gedit-document-input-stream.h	\
	gedit-document-loader.h		\
	gedit-document-output-stream.h	\
	gedit-document-saver.h		\
	gedit-documents-panel.h		\
	gedit-gio-document-loader.h	\
	gedit-gio-document-saver.h	\
	gedit-history-entry.h		\
	gedit-io-error-message-area.h	\
	gedit-language-manager.h	\
	gedit-object-module.h		\
	gedit-plugin-info.h		\
	gedit-plugin-info-priv.h	\
	gedit-plugin-loader.h		\
	gedit-plugin-manager.h		\
	gedit-plugins-engine.h		\
	gedit-prefs-manager-private.h	\
	gedit-print-job.h		\
	gedit-print-preview.h		\
	gedit-session.h			\
	gedit-smart-charset-converter.h	\
	gedit-style-scheme-manager.h	\
	gedit-tab-label.h		\
	gedittextregion.h		\
	gedit-ui.h			\
	gedit-window-private.h

INST_H_FILES =				\
	gedit-app.h			\
	gedit-commands.h		\
	gedit-debug.h			\
	gedit-document.h 		\
	gedit-encodings.h		\
	gedit-encodings-combo-box.h	\
	gedit-file-chooser-dialog.h	\
	gedit-help.h 			\
	gedit-message-bus.h		\
	gedit-message-type.h		\
	gedit-message.h			\
	gedit-notebook.h		\
	gedit-panel.h			\
	gedit-plugin.h			\
	gedit-prefs-manager-app.h	\
	gedit-prefs-manager.h		\
	gedit-progress-message-area.h	\
	gedit-statusbar.h		\
	gedit-status-combo-box.h	\
	gedit-tab.h 			\
	gedit-utils.h 			\
	gedit-view.h 			\
	gedit-window.h

if !ENABLE_GVFS_METADATA
INST_H_FILES += gedit-metadata-manager.h
endif

headerdir = $(prefix)/include/gedit-@GEDIT_API_VERSION@/gedit

header_DATA = 				\
	$(INST_H_FILES)


libgedit_la_SOURCES = 			\
	$(BUILT_SOURCES)		\
	$(BACON_FILES)			\
	$(POSIXIO_FILES)		\
	gedit-app.c			\
	gedit-close-button.c		\
	gedit-commands-documents.c	\
	gedit-commands-edit.c		\
	gedit-commands-file.c		\
	gedit-commands-file-print.c	\
	gedit-commands-help.c		\
	gedit-commands-search.c		\
	gedit-commands-view.c		\
	gedit-debug.c			\
	gedit-dirs.c			\
	gedit-document.c 		\
	gedit-document-input-stream.c	\
	gedit-document-loader.c		\
	gedit-document-output-stream.c	\
	gedit-gio-document-loader.c	\
	gedit-document-saver.c		\
	gedit-gio-document-saver.c	\
	gedit-documents-panel.c		\
	gedit-encodings.c		\
	gedit-encodings-combo-box.c	\
	gedit-file-chooser-dialog.c	\
	gedit-help.c			\
	gedit-history-entry.c		\
	gedit-io-error-message-area.c	\
	gedit-language-manager.c	\
	gedit-message-bus.c		\
	gedit-message-type.c		\
	gedit-message.c			\
	gedit-object-module.c		\
	gedit-notebook.c		\
	gedit-panel.c			\
	gedit-plugin-info.c		\
	gedit-plugin.c			\
	gedit-plugin-loader.c		\
	gedit-plugin-manager.c		\
	gedit-plugins-engine.c		\
	gedit-prefs-manager-app.c	\
	gedit-prefs-manager.c		\
	gedit-prefs-manager-private.h	\
	gedit-print-job.c		\
	gedit-print-preview.c		\
	gedit-progress-message-area.c	\
	gedit-session.c			\
	gedit-smart-charset-converter.c	\
	gedit-statusbar.c		\
	gedit-status-combo-box.c	\
	gedit-style-scheme-manager.c	\
	gedit-tab.c 			\
	gedit-tab-label.c		\
	gedit-utils.c 			\
	gedit-view.c 			\
	gedit-window.c			\
	gedittextregion.c		\
	$(NOINST_H_FILES)		\
	$(INST_H_FILES)

if !ENABLE_GVFS_METADATA
libgedit_la_SOURCES += gedit-metadata-manager.c 
endif

gedit-enum-types.h: gedit-enum-types.h.template $(INST_H_FILES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && $(GLIB_MKENUMS) --template gedit-enum-types.h.template $(INST_H_FILES)) > $@

gedit-enum-types.c: gedit-enum-types.c.template $(INST_H_FILES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && $(GLIB_MKENUMS) --template gedit-enum-types.c.template $(INST_H_FILES)) > $@

gedit-marshal.h: gedit-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) $(GLIB_GENMARSHAL) $< --header --prefix=gedit_marshal > $@

gedit-marshal.c: gedit-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) echo "#include \"gedit-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=gedit_marshal >> $@

uidir = $(datadir)/gedit-2/ui/
ui_DATA = 				\
	gedit-ui.xml			\
	gedit-print-preferences.ui

EXTRA_DIST = 				\
	$(ui_DATA)			\
	gedit-enum-types.h.template	\
	gedit-enum-types.c.template	\
	gedit-marshal.list		\
	gedit.rc

CLEANFILES = $(BUILT_SOURCES)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)

install-exec-hook:
if PLATFORM_WIN32
	$(mkinstalldirs) "$(DESTDIR)$(libdir)"
	$(INSTALL_DATA) libgedit-$(GEDIT_API_VERSION).a "$(DESTDIR)$(libdir)"
else
	rm -f $(DESTDIR)$(bindir)/mate-text-editor
	ln -s gedit $(DESTDIR)$(bindir)/mate-text-editor
endif

if !OS_WIN32
BACON_DIR=$(srcdir)/../../libbacon/src/
BACON_FILES=bacon-message-connection.h bacon-message-connection.c

regenerate-built-sources:
	BACONFILES="$(BACON_FILES)" BACONDIR="$(BACON_DIR)" $(top_srcdir)/gedit/update-from-bacon.sh
else
BACON_DIR=
endif

if BUILD_MESSAGE_AREA
libgedit_la_SOURCES += gedit-message-area.c
INST_H_FILES += gedit-message-area.h
endif

if BUILD_SPINNER
libgedit_la_SOURCES += gedit-spinner.c
NOINST_H_FILES += gedit-spinner.h
endif

-include $(top_srcdir)/git.mk
