## Process this file with automake to produce Makefile.in
SUBDIRS = dialogs smclient

bin_PROGRAMS = xed

noinst_LTLIBRARIES = libxed.la

AM_CPPFLAGS =								\
	-I$(top_srcdir)							\
	-I$(srcdir)							\
	-I$(srcdir)/smclient						\
	$(XED_CFLAGS)							\
	$(MATE_DESKTOP_CFLAGS)						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)					\
	-DDATADIR=\""$(datadir)"\"					\
	-DLIBDIR=\""$(libdir)"\"

xed_SOURCES = \
	xed.c

xed_LDADD = libxed.la $(XED_LIBS) $(EGG_SMCLIENT_LIBS)

xed_LDFLAGS = -export-dynamic -no-undefined -export-symbols-regex "^[[^_]].*"

libxed_la_LDFLAGS = -export-dynamic -no-undefined -export-symbols-regex "^[[^_]].*"

libxed_la_LIBADD = \
	dialogs/libdialogs.la		\
	smclient/libeggsmclient.la	\
	$(MATE_DESKTOP_LIBS)

# XED_LIBS must be the last to ensure correct order on some platforms
libxed_la_LIBADD += $(XED_LIBS) -lICE

BUILT_SOURCES = 			\
	xed-enum-types.c		\
	xed-enum-types.h		\
	xed-marshal.c			\
	xed-marshal.h


NOINST_H_FILES =			\
	xed-close-button.h		\
	xed-dirs.h			\
	xed-document-input-stream.h	\
	xed-document-loader.h		\
	xed-document-output-stream.h	\
	xed-document-saver.h		\
	xed-documents-panel.h		\
	xed-gio-document-loader.h	\
	xed-gio-document-saver.h	\
	xed-history-entry.h		\
	xed-io-error-message-area.h	\
	xed-language-manager.h	\
	xed-object-module.h		\
	xed-plugin-info.h		\
	xed-plugin-info-priv.h	\
	xed-plugin-loader.h		\
	xed-plugin-manager.h		\
	xed-plugins-engine.h		\
	xed-prefs-manager-private.h	\
	xed-print-job.h		\
	xed-print-preview.h		\
	xed-session.h			\
	xed-smart-charset-converter.h	\
	xed-style-scheme-manager.h	\
	xed-tab-label.h		\
	xedtextregion.h		\
	xed-ui.h			\
	xed-window-private.h

INST_H_FILES =				\
	xed-app.h			\
	xed-commands.h		\
	xed-debug.h			\
	xed-document.h 		\
	xed-encodings.h		\
	xed-encodings-combo-box.h	\
	xed-file-chooser-dialog.h	\
	xed-help.h 			\
	xed-message-bus.h		\
	xed-message-type.h		\
	xed-message.h			\
	xed-notebook.h		\
	xed-panel.h			\
	xed-plugin.h			\
	xed-prefs-manager-app.h	\
	xed-prefs-manager.h		\
	xed-progress-message-area.h	\
	xed-statusbar.h		\
	xed-status-combo-box.h	\
	xed-tab.h 			\
	xed-utils.h 			\
	xed-view.h 			\
	xed-window.h

if !ENABLE_GVFS_METADATA
INST_H_FILES += xed-metadata-manager.h
endif

headerdir = $(prefix)/include/xed

header_DATA = 				\
	$(INST_H_FILES)


libxed_la_SOURCES = 			\
	$(BUILT_SOURCES)		\
	$(BACON_FILES)			\
	$(POSIXIO_FILES)		\
	xed-app.c			\
	xed-close-button.c		\
	xed-commands-documents.c	\
	xed-commands-edit.c		\
	xed-commands-file.c		\
	xed-commands-file-print.c	\
	xed-commands-help.c		\
	xed-commands-search.c		\
	xed-commands-view.c		\
	xed-debug.c			\
	xed-dirs.c			\
	xed-document.c 		\
	xed-document-input-stream.c	\
	xed-document-loader.c		\
	xed-document-output-stream.c	\
	xed-gio-document-loader.c	\
	xed-document-saver.c		\
	xed-gio-document-saver.c	\
	xed-documents-panel.c		\
	xed-encodings.c		\
	xed-encodings-combo-box.c	\
	xed-file-chooser-dialog.c	\
	xed-help.c			\
	xed-history-entry.c		\
	xed-io-error-message-area.c	\
	xed-language-manager.c	\
	xed-message-bus.c		\
	xed-message-type.c		\
	xed-message.c			\
	xed-object-module.c		\
	xed-notebook.c		\
	xed-panel.c			\
	xed-plugin-info.c		\
	xed-plugin.c			\
	xed-plugin-loader.c		\
	xed-plugin-manager.c		\
	xed-plugins-engine.c		\
	xed-prefs-manager-app.c	\
	xed-prefs-manager.c		\
	xed-prefs-manager-private.h	\
	xed-print-job.c		\
	xed-print-preview.c		\
	xed-progress-message-area.c	\
	xed-session.c			\
	xed-smart-charset-converter.c	\
	xed-statusbar.c		\
	xed-status-combo-box.c	\
	xed-style-scheme-manager.c	\
	xed-tab.c 			\
	xed-tab-label.c		\
	xed-utils.c 			\
	xed-view.c 			\
	xed-window.c			\
	xedtextregion.c		\
	$(NOINST_H_FILES)		\
	$(INST_H_FILES)

if !ENABLE_GVFS_METADATA
libxed_la_SOURCES += xed-metadata-manager.c
endif

xed-enum-types.h: xed-enum-types.h.template $(INST_H_FILES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && $(GLIB_MKENUMS) --template xed-enum-types.h.template $(INST_H_FILES)) > $@

xed-enum-types.c: xed-enum-types.c.template $(INST_H_FILES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && $(GLIB_MKENUMS) --template xed-enum-types.c.template $(INST_H_FILES)) > $@

xed-marshal.h: xed-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) $(GLIB_GENMARSHAL) $< --header --prefix=xed_marshal > $@

xed-marshal.c: xed-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) echo "#include \"xed-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=xed_marshal >> $@

uidir = $(datadir)/xed/ui/
ui_DATA = 				\
	xed-ui.xml			\
	xed-print-preferences.ui

EXTRA_DIST = 				\
	$(ui_DATA)			\
	xed-enum-types.h.template	\
	xed-enum-types.c.template	\
	xed-marshal.list		\
	xed.rc

CLEANFILES = $(BUILT_SOURCES)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)

BACON_DIR=$(srcdir)/../../libbacon/src/
BACON_FILES=bacon-message-connection.h bacon-message-connection.c

regenerate-built-sources:
	BACONFILES="$(BACON_FILES)" BACONDIR="$(BACON_DIR)" $(top_srcdir)/xed/update-from-bacon.sh

-include $(top_srcdir)/git.mk
