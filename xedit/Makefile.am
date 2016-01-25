## Process this file with automake to produce Makefile.in
SUBDIRS = dialogs smclient

bin_PROGRAMS = xedit

noinst_LTLIBRARIES = libxedit.la

AM_CPPFLAGS =								\
	-I$(top_srcdir)							\
	-I$(srcdir)							\
	-I$(srcdir)/smclient						\
	$(XEDIT_CFLAGS)							\
	$(MATE_DESKTOP_CFLAGS)						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)					\
	-DDATADIR=\""$(datadir)"\"					\
	-DLIBDIR=\""$(libdir)"\"

xedit_SOURCES = \
	xedit.c

xedit_LDADD = libxedit.la $(XEDIT_LIBS) $(EGG_SMCLIENT_LIBS)

xedit_LDFLAGS = -export-dynamic -no-undefined -export-symbols-regex "^[[^_]].*"

libxedit_la_LDFLAGS = -export-dynamic -no-undefined -export-symbols-regex "^[[^_]].*"

libxedit_la_LIBADD = \
	dialogs/libdialogs.la		\
	smclient/libeggsmclient.la	\
	$(MATE_DESKTOP_LIBS)

# XEDIT_LIBS must be the last to ensure correct order on some platforms
libxedit_la_LIBADD += $(XEDIT_LIBS) -lICE

BUILT_SOURCES = 			\
	xedit-enum-types.c		\
	xedit-enum-types.h		\
	xedit-marshal.c			\
	xedit-marshal.h


NOINST_H_FILES =			\
	xedit-close-button.h		\
	xedit-dirs.h			\
	xedit-document-input-stream.h	\
	xedit-document-loader.h		\
	xedit-document-output-stream.h	\
	xedit-document-saver.h		\
	xedit-documents-panel.h		\
	xedit-gio-document-loader.h	\
	xedit-gio-document-saver.h	\
	xedit-history-entry.h		\
	xedit-io-error-message-area.h	\
	xedit-language-manager.h	\
	xedit-object-module.h		\
	xedit-plugin-info.h		\
	xedit-plugin-info-priv.h	\
	xedit-plugin-loader.h		\
	xedit-plugin-manager.h		\
	xedit-plugins-engine.h		\
	xedit-prefs-manager-private.h	\
	xedit-print-job.h		\
	xedit-print-preview.h		\
	xedit-session.h			\
	xedit-smart-charset-converter.h	\
	xedit-style-scheme-manager.h	\
	xedit-tab-label.h		\
	xedittextregion.h		\
	xedit-ui.h			\
	xedit-window-private.h

INST_H_FILES =				\
	xedit-app.h			\
	xedit-commands.h		\
	xedit-debug.h			\
	xedit-document.h 		\
	xedit-encodings.h		\
	xedit-encodings-combo-box.h	\
	xedit-file-chooser-dialog.h	\
	xedit-help.h 			\
	xedit-message-bus.h		\
	xedit-message-type.h		\
	xedit-message.h			\
	xedit-notebook.h		\
	xedit-panel.h			\
	xedit-plugin.h			\
	xedit-prefs-manager-app.h	\
	xedit-prefs-manager.h		\
	xedit-progress-message-area.h	\
	xedit-statusbar.h		\
	xedit-status-combo-box.h	\
	xedit-tab.h 			\
	xedit-utils.h 			\
	xedit-view.h 			\
	xedit-window.h

if !ENABLE_GVFS_METADATA
INST_H_FILES += xedit-metadata-manager.h
endif

headerdir = $(prefix)/include/xedit

header_DATA = 				\
	$(INST_H_FILES)


libxedit_la_SOURCES = 			\
	$(BUILT_SOURCES)		\
	$(BACON_FILES)			\
	$(POSIXIO_FILES)		\
	xedit-app.c			\
	xedit-close-button.c		\
	xedit-commands-documents.c	\
	xedit-commands-edit.c		\
	xedit-commands-file.c		\
	xedit-commands-file-print.c	\
	xedit-commands-help.c		\
	xedit-commands-search.c		\
	xedit-commands-view.c		\
	xedit-debug.c			\
	xedit-dirs.c			\
	xedit-document.c 		\
	xedit-document-input-stream.c	\
	xedit-document-loader.c		\
	xedit-document-output-stream.c	\
	xedit-gio-document-loader.c	\
	xedit-document-saver.c		\
	xedit-gio-document-saver.c	\
	xedit-documents-panel.c		\
	xedit-encodings.c		\
	xedit-encodings-combo-box.c	\
	xedit-file-chooser-dialog.c	\
	xedit-help.c			\
	xedit-history-entry.c		\
	xedit-io-error-message-area.c	\
	xedit-language-manager.c	\
	xedit-message-bus.c		\
	xedit-message-type.c		\
	xedit-message.c			\
	xedit-object-module.c		\
	xedit-notebook.c		\
	xedit-panel.c			\
	xedit-plugin-info.c		\
	xedit-plugin.c			\
	xedit-plugin-loader.c		\
	xedit-plugin-manager.c		\
	xedit-plugins-engine.c		\
	xedit-prefs-manager-app.c	\
	xedit-prefs-manager.c		\
	xedit-prefs-manager-private.h	\
	xedit-print-job.c		\
	xedit-print-preview.c		\
	xedit-progress-message-area.c	\
	xedit-session.c			\
	xedit-smart-charset-converter.c	\
	xedit-statusbar.c		\
	xedit-status-combo-box.c	\
	xedit-style-scheme-manager.c	\
	xedit-tab.c 			\
	xedit-tab-label.c		\
	xedit-utils.c 			\
	xedit-view.c 			\
	xedit-window.c			\
	xedittextregion.c		\
	$(NOINST_H_FILES)		\
	$(INST_H_FILES)

if !ENABLE_GVFS_METADATA
libxedit_la_SOURCES += xedit-metadata-manager.c
endif

xedit-enum-types.h: xedit-enum-types.h.template $(INST_H_FILES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && $(GLIB_MKENUMS) --template xedit-enum-types.h.template $(INST_H_FILES)) > $@

xedit-enum-types.c: xedit-enum-types.c.template $(INST_H_FILES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && $(GLIB_MKENUMS) --template xedit-enum-types.c.template $(INST_H_FILES)) > $@

xedit-marshal.h: xedit-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) $(GLIB_GENMARSHAL) $< --header --prefix=xedit_marshal > $@

xedit-marshal.c: xedit-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) echo "#include \"xedit-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=xedit_marshal >> $@

uidir = $(datadir)/xedit/ui/
ui_DATA = 				\
	xedit-ui.xml			\
	xedit-print-preferences.ui

EXTRA_DIST = 				\
	$(ui_DATA)			\
	xedit-enum-types.h.template	\
	xedit-enum-types.c.template	\
	xedit-marshal.list		\
	xedit.rc

CLEANFILES = $(BUILT_SOURCES)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)

BACON_DIR=$(srcdir)/../../libbacon/src/
BACON_FILES=bacon-message-connection.h bacon-message-connection.c

regenerate-built-sources:
	BACONFILES="$(BACON_FILES)" BACONDIR="$(BACON_DIR)" $(top_srcdir)/xedit/update-from-bacon.sh

-include $(top_srcdir)/git.mk
