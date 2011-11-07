# filebrowser

plugindir = $(GEDIT_PLUGINS_LIBS_DIR)

INCLUDES = \
	-I$(top_srcdir) 						\
	$(GEDIT_CFLAGS) 						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)

BUILT_SOURCES = \
	gedit-file-browser-enum-types.h		\
	gedit-file-browser-enum-types.c		\
	gedit-file-browser-marshal.h 		\
	gedit-file-browser-marshal.c

plugin_LTLIBRARIES = libfilebrowser.la

NOINST_H_FILES = \
	gedit-file-bookmarks-store.h 		\
	gedit-file-browser-store.h 		\
	gedit-file-browser-view.h 		\
	gedit-file-browser-widget.h 		\
	gedit-file-browser-error.h		\
	gedit-file-browser-utils.h		\
	gedit-file-browser-plugin.h		\
	gedit-file-browser-messages.h

libfilebrowser_la_SOURCES = \
	$(BUILT_SOURCES) 			\
	gedit-file-bookmarks-store.c 		\
	gedit-file-browser-store.c 		\
	gedit-file-browser-view.c 		\
	gedit-file-browser-widget.c 		\
	gedit-file-browser-utils.c 		\
	gedit-file-browser-plugin.c		\
	gedit-file-browser-messages.c		\
	$(NOINST_H_FILES)

libfilebrowser_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libfilebrowser_la_LIBADD = $(GEDIT_LIBS)

# UI files (if you use ui for your plugin, list those files here)
uidir = $(GEDIT_PLUGINS_DATA_DIR)/filebrowser
ui_DATA = gedit-file-browser-widget-ui.xml

plugin_in_files = filebrowser.gedit-plugin.desktop.in

%.gedit-plugin: %.gedit-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

gedit-file-browser-enum-types.h: gedit-file-browser-enum-types.h.template $(NOINST_H_FILES) $(GLIB_MKENUMS)
	(cd $(srcdir) && $(GLIB_MKENUMS) --template gedit-file-browser-enum-types.h.template $(NOINST_H_FILES)) > $@

gedit-file-browser-enum-types.c: gedit-file-browser-enum-types.c.template gedit-file-browser-enum-register.c.template $(NOINST_H_FILES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && \
	$(GLIB_MKENUMS) --template gedit-file-browser-enum-types.c.template $(NOINST_H_FILES) && \
	$(GLIB_MKENUMS) --template gedit-file-browser-enum-register.c.template $(NOINST_H_FILES)) > $@

gedit-file-browser-marshal.h: gedit-file-browser-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) $(GLIB_GENMARSHAL) $< --header --prefix=gedit_file_browser_marshal > $@

gedit-file-browser-marshal.c: gedit-file-browser-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) echo "#include \"gedit-file-browser-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=gedit_file_browser_marshal >> $@

plugin_DATA = $(plugin_in_files:.gedit-plugin.desktop.in=.gedit-plugin)

schemasdir	 = $(MATECONF_SCHEMA_FILE_DIR)
schemas_in_files = gedit-file-browser.schemas.in
schemas_DATA 	 = $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

if MATECONF_SCHEMAS_INSTALL
install-data-local:
	        if test -z "$(DESTDIR)" ; then \
	                for p in $(schemas_DATA) ; do \
	                        MATECONF_CONFIG_SOURCE=$(MATECONF_SCHEMA_CONFIG_SOURCE) $(MATECONFTOOL) --makefile-install-rule $(top_builddir)/plugins/filebrowser/$$p ; \
	                done \
	        fi
else
install-data-local:
endif


EXTRA_DIST = \
	$(ui_DATA)					\
	$(plugin_in_files)				\
	$(schemas_in_files)				\
	gedit-file-browser-enum-types.h.template	\
	gedit-file-browser-enum-types.c.template	\
	gedit-file-browser-enum-register.c.template	\
	gedit-file-browser-marshal.list

CLEANFILES = \
	$(plugin_DATA)		\
	$(schemas_DATA)		\
	$(BUILT_SOURCES)

DISTCLEANFILES = \
	$(plugin_DATA)		\
	$(schemas_DATA)		\
	$(BUILT_SOURCES)

-include $(top_srcdir)/git.mk
