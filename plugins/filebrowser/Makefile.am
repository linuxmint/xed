# filebrowser

plugindir = $(XED_PLUGINS_LIBS_DIR)

AM_CPPFLAGS = \
	-I$(top_srcdir) 						\
	$(XED_CFLAGS) 						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)

BUILT_SOURCES = \
	xed-file-browser-enum-types.h		\
	xed-file-browser-enum-types.c		\
	xed-file-browser-marshal.h 		\
	xed-file-browser-marshal.c

plugin_LTLIBRARIES = libfilebrowser.la

NOINST_H_FILES = \
	xed-file-bookmarks-store.h 		\
	xed-file-browser-store.h 		\
	xed-file-browser-view.h 		\
	xed-file-browser-widget.h 		\
	xed-file-browser-error.h		\
	xed-file-browser-utils.h		\
	xed-file-browser-plugin.h		\
	xed-file-browser-messages.h

libfilebrowser_la_SOURCES = \
	$(BUILT_SOURCES) 			\
	xed-file-bookmarks-store.c 		\
	xed-file-browser-store.c 		\
	xed-file-browser-view.c 		\
	xed-file-browser-widget.c 		\
	xed-file-browser-utils.c 		\
	xed-file-browser-plugin.c		\
	xed-file-browser-messages.c		\
	$(NOINST_H_FILES)

libfilebrowser_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libfilebrowser_la_LIBADD = $(XED_LIBS)

# UI files (if you use ui for your plugin, list those files here)
uidir = $(XED_PLUGINS_DATA_DIR)/filebrowser
ui_DATA = xed-file-browser-widget-ui.xml

plugin_in_files = filebrowser.xed-plugin.desktop.in

%.xed-plugin: %.xed-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

xed-file-browser-enum-types.h: xed-file-browser-enum-types.h.template $(NOINST_H_FILES) $(GLIB_MKENUMS)
	(cd $(srcdir) && $(GLIB_MKENUMS) --template xed-file-browser-enum-types.h.template $(NOINST_H_FILES)) > $@

xed-file-browser-enum-types.c: xed-file-browser-enum-types.c.template xed-file-browser-enum-register.c.template $(NOINST_H_FILES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && \
	$(GLIB_MKENUMS) --template xed-file-browser-enum-types.c.template $(NOINST_H_FILES) && \
	$(GLIB_MKENUMS) --template xed-file-browser-enum-register.c.template $(NOINST_H_FILES)) > $@

xed-file-browser-marshal.h: xed-file-browser-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) $(GLIB_GENMARSHAL) $< --header --prefix=xed_file_browser_marshal > $@

xed-file-browser-marshal.c: xed-file-browser-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) echo "#include \"xed-file-browser-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=xed_file_browser_marshal >> $@

plugin_DATA = $(plugin_in_files:.xed-plugin.desktop.in=.xed-plugin)

@INTLTOOL_XML_NOMERGE_RULE@
filebrowser_gschema_in = org.x.editor.plugins.filebrowser.gschema.xml.in
gsettings_SCHEMAS = $(filebrowser_gschema_in:.xml.in=.xml)
@GSETTINGS_RULES@

EXTRA_DIST = \
	$(ui_DATA)					\
	$(plugin_in_files)				\
	$(filebrowser_gschema_in)			\
	xed-file-browser-enum-types.h.template	\
	xed-file-browser-enum-types.c.template	\
	xed-file-browser-enum-register.c.template	\
	xed-file-browser-marshal.list

CLEANFILES = \
	$(plugin_DATA)		\
	$(gsettings_SCHEMAS_in)		\
	$(gsettings_SCHEMAS)		\
	$(BUILT_SOURCES)

DISTCLEANFILES = \
	$(plugin_DATA)		\
	$(gsettings_SCHEMAS_in)		\
	$(gsettings_SCHEMAS)		\
	$(BUILT_SOURCES)

-include $(top_srcdir)/git.mk
