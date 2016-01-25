# filebrowser

plugindir = $(XEDIT_PLUGINS_LIBS_DIR)

AM_CPPFLAGS = \
	-I$(top_srcdir) 						\
	$(XEDIT_CFLAGS) 						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)

BUILT_SOURCES = \
	xedit-file-browser-enum-types.h		\
	xedit-file-browser-enum-types.c		\
	xedit-file-browser-marshal.h 		\
	xedit-file-browser-marshal.c

plugin_LTLIBRARIES = libfilebrowser.la

NOINST_H_FILES = \
	xedit-file-bookmarks-store.h 		\
	xedit-file-browser-store.h 		\
	xedit-file-browser-view.h 		\
	xedit-file-browser-widget.h 		\
	xedit-file-browser-error.h		\
	xedit-file-browser-utils.h		\
	xedit-file-browser-plugin.h		\
	xedit-file-browser-messages.h

libfilebrowser_la_SOURCES = \
	$(BUILT_SOURCES) 			\
	xedit-file-bookmarks-store.c 		\
	xedit-file-browser-store.c 		\
	xedit-file-browser-view.c 		\
	xedit-file-browser-widget.c 		\
	xedit-file-browser-utils.c 		\
	xedit-file-browser-plugin.c		\
	xedit-file-browser-messages.c		\
	$(NOINST_H_FILES)

libfilebrowser_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libfilebrowser_la_LIBADD = $(XEDIT_LIBS)

# UI files (if you use ui for your plugin, list those files here)
uidir = $(XEDIT_PLUGINS_DATA_DIR)/filebrowser
ui_DATA = xedit-file-browser-widget-ui.xml

plugin_in_files = filebrowser.xedit-plugin.desktop.in

%.xedit-plugin: %.xedit-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

xedit-file-browser-enum-types.h: xedit-file-browser-enum-types.h.template $(NOINST_H_FILES) $(GLIB_MKENUMS)
	(cd $(srcdir) && $(GLIB_MKENUMS) --template xedit-file-browser-enum-types.h.template $(NOINST_H_FILES)) > $@

xedit-file-browser-enum-types.c: xedit-file-browser-enum-types.c.template xedit-file-browser-enum-register.c.template $(NOINST_H_FILES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && \
	$(GLIB_MKENUMS) --template xedit-file-browser-enum-types.c.template $(NOINST_H_FILES) && \
	$(GLIB_MKENUMS) --template xedit-file-browser-enum-register.c.template $(NOINST_H_FILES)) > $@

xedit-file-browser-marshal.h: xedit-file-browser-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) $(GLIB_GENMARSHAL) $< --header --prefix=xedit_file_browser_marshal > $@

xedit-file-browser-marshal.c: xedit-file-browser-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) echo "#include \"xedit-file-browser-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=xedit_file_browser_marshal >> $@

plugin_DATA = $(plugin_in_files:.xedit-plugin.desktop.in=.xedit-plugin)

@INTLTOOL_XML_NOMERGE_RULE@
filebrowser_gschema_in = org.x.editor.plugins.filebrowser.gschema.xml.in
gsettings_SCHEMAS = $(filebrowser_gschema_in:.xml.in=.xml)
@GSETTINGS_RULES@

EXTRA_DIST = \
	$(ui_DATA)					\
	$(plugin_in_files)				\
	$(filebrowser_gschema_in)			\
	xedit-file-browser-enum-types.h.template	\
	xedit-file-browser-enum-types.c.template	\
	xedit-file-browser-enum-register.c.template	\
	xedit-file-browser-marshal.list

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
