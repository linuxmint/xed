## Process this file with automake to produce Makefile.in
SUBDIRS = dialogs smclient

if OS_OSX
SUBDIRS += osx
endif

bin_PROGRAMS = pluma

noinst_LTLIBRARIES = libpluma.la

AM_CPPFLAGS =								\
	-I$(top_srcdir)							\
	-I$(srcdir)							\
	-I$(srcdir)/smclient						\
	$(PLUMA_CFLAGS)							\
	$(IGE_MAC_CFLAGS)						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)					\
	-DDATADIR=\""$(datadir)"\"					\
	-DLIBDIR=\""$(libdir)"\"

pluma_SOURCES = \
	pluma.c

pluma_LDADD = libpluma.la $(PLUMA_LIBS) $(IGE_MAC_LIBS) $(EGG_SMCLIENT_LIBS)

if PLATFORM_WIN32
pluma_LDFLAGS = -Wl,--export-all-symbols -Wl,--out-implib,libpluma.a
if OS_WIN32
pluma_LDFLAGS += -mwindows
endif
else
pluma_LDFLAGS = -export-dynamic -no-undefined -export-symbols-regex "^[[^_]].*"
endif

libpluma_la_LDFLAGS = -export-dynamic -no-undefined -export-symbols-regex "^[[^_]].*"

libpluma_la_LIBADD = \
	dialogs/libdialogs.la		\
	smclient/libeggsmclient.la

# PLUMA_LIBS must be the last to ensure correct order on some platforms
libpluma_la_LIBADD += $(PLUMA_LIBS) -lICE

if OS_OSX
pluma_LDFLAGS += -framework Carbon

libpluma_la_LIBADD += osx/libosx.la
endif

BUILT_SOURCES = 			\
	pluma-enum-types.c		\
	pluma-enum-types.h		\
	pluma-marshal.c			\
	pluma-marshal.h

if OS_WIN32
pluma-res.o: pluma.rc
	$(WINDRES) -i pluma.rc --input-format=rc -o pluma-res.o -O coff

pluma_LDADD += pluma-res.o
endif

NOINST_H_FILES =			\
	pluma-close-button.h		\
	pluma-dirs.h			\
	pluma-document-input-stream.h	\
	pluma-document-loader.h		\
	pluma-document-output-stream.h	\
	pluma-document-saver.h		\
	pluma-documents-panel.h		\
	pluma-gio-document-loader.h	\
	pluma-gio-document-saver.h	\
	pluma-history-entry.h		\
	pluma-io-error-message-area.h	\
	pluma-language-manager.h	\
	pluma-object-module.h		\
	pluma-plugin-info.h		\
	pluma-plugin-info-priv.h	\
	pluma-plugin-loader.h		\
	pluma-plugin-manager.h		\
	pluma-plugins-engine.h		\
	pluma-prefs-manager-private.h	\
	pluma-print-job.h		\
	pluma-print-preview.h		\
	pluma-session.h			\
	pluma-smart-charset-converter.h	\
	pluma-style-scheme-manager.h	\
	pluma-tab-label.h		\
	plumatextregion.h		\
	pluma-ui.h			\
	pluma-window-private.h

INST_H_FILES =				\
	pluma-app.h			\
	pluma-commands.h		\
	pluma-debug.h			\
	pluma-document.h 		\
	pluma-encodings.h		\
	pluma-encodings-combo-box.h	\
	pluma-file-chooser-dialog.h	\
	pluma-help.h 			\
	pluma-message-bus.h		\
	pluma-message-type.h		\
	pluma-message.h			\
	pluma-notebook.h		\
	pluma-panel.h			\
	pluma-plugin.h			\
	pluma-prefs-manager-app.h	\
	pluma-prefs-manager.h		\
	pluma-progress-message-area.h	\
	pluma-statusbar.h		\
	pluma-status-combo-box.h	\
	pluma-tab.h 			\
	pluma-utils.h 			\
	pluma-view.h 			\
	pluma-window.h

if !ENABLE_GVFS_METADATA
INST_H_FILES += pluma-metadata-manager.h
endif

headerdir = $(prefix)/include/pluma

header_DATA = 				\
	$(INST_H_FILES)


libpluma_la_SOURCES = 			\
	$(BUILT_SOURCES)		\
	$(BACON_FILES)			\
	$(POSIXIO_FILES)		\
	pluma-app.c			\
	pluma-close-button.c		\
	pluma-commands-documents.c	\
	pluma-commands-edit.c		\
	pluma-commands-file.c		\
	pluma-commands-file-print.c	\
	pluma-commands-help.c		\
	pluma-commands-search.c		\
	pluma-commands-view.c		\
	pluma-debug.c			\
	pluma-dirs.c			\
	pluma-document.c 		\
	pluma-document-input-stream.c	\
	pluma-document-loader.c		\
	pluma-document-output-stream.c	\
	pluma-gio-document-loader.c	\
	pluma-document-saver.c		\
	pluma-gio-document-saver.c	\
	pluma-documents-panel.c		\
	pluma-encodings.c		\
	pluma-encodings-combo-box.c	\
	pluma-file-chooser-dialog.c	\
	pluma-help.c			\
	pluma-history-entry.c		\
	pluma-io-error-message-area.c	\
	pluma-language-manager.c	\
	pluma-message-bus.c		\
	pluma-message-type.c		\
	pluma-message.c			\
	pluma-object-module.c		\
	pluma-notebook.c		\
	pluma-panel.c			\
	pluma-plugin-info.c		\
	pluma-plugin.c			\
	pluma-plugin-loader.c		\
	pluma-plugin-manager.c		\
	pluma-plugins-engine.c		\
	pluma-prefs-manager-app.c	\
	pluma-prefs-manager.c		\
	pluma-prefs-manager-private.h	\
	pluma-print-job.c		\
	pluma-print-preview.c		\
	pluma-progress-message-area.c	\
	pluma-session.c			\
	pluma-smart-charset-converter.c	\
	pluma-statusbar.c		\
	pluma-status-combo-box.c	\
	pluma-style-scheme-manager.c	\
	pluma-tab.c 			\
	pluma-tab-label.c		\
	pluma-utils.c 			\
	pluma-view.c 			\
	pluma-window.c			\
	plumatextregion.c		\
	$(NOINST_H_FILES)		\
	$(INST_H_FILES)

if !ENABLE_GVFS_METADATA
libpluma_la_SOURCES += pluma-metadata-manager.c
endif

pluma-enum-types.h: pluma-enum-types.h.template $(INST_H_FILES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && $(GLIB_MKENUMS) --template pluma-enum-types.h.template $(INST_H_FILES)) > $@

pluma-enum-types.c: pluma-enum-types.c.template $(INST_H_FILES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && $(GLIB_MKENUMS) --template pluma-enum-types.c.template $(INST_H_FILES)) > $@

pluma-marshal.h: pluma-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) $(GLIB_GENMARSHAL) $< --header --prefix=pluma_marshal > $@

pluma-marshal.c: pluma-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) echo "#include \"pluma-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=pluma_marshal >> $@

uidir = $(datadir)/pluma/ui/
ui_DATA = 				\
	pluma-ui.xml			\
	pluma-print-preferences.ui

EXTRA_DIST = 				\
	$(ui_DATA)			\
	pluma-enum-types.h.template	\
	pluma-enum-types.c.template	\
	pluma-marshal.list		\
	pluma.rc

CLEANFILES = $(BUILT_SOURCES)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)

install-exec-hook:
if PLATFORM_WIN32
	$(mkinstalldirs) "$(DESTDIR)$(libdir)"
	$(INSTALL_DATA) libpluma.a "$(DESTDIR)$(libdir)"
endif

if !OS_WIN32
BACON_DIR=$(srcdir)/../../libbacon/src/
BACON_FILES=bacon-message-connection.h bacon-message-connection.c

regenerate-built-sources:
	BACONFILES="$(BACON_FILES)" BACONDIR="$(BACON_DIR)" $(top_srcdir)/pluma/update-from-bacon.sh
else
BACON_DIR=
endif

if BUILD_MESSAGE_AREA
libpluma_la_SOURCES += pluma-message-area.c
INST_H_FILES += pluma-message-area.h
endif

if BUILD_SPINNER
libpluma_la_SOURCES += pluma-spinner.c
NOINST_H_FILES += pluma-spinner.h
endif

-include $(top_srcdir)/git.mk
