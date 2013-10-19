# filebrowser

plugindir = $(PLUMA_PLUGINS_LIBS_DIR)

AM_CPPFLAGS = \
	-I$(top_srcdir) 						\
	$(PLUMA_CFLAGS) 						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)

BUILT_SOURCES = \
	pluma-file-browser-enum-types.h		\
	pluma-file-browser-enum-types.c		\
	pluma-file-browser-marshal.h 		\
	pluma-file-browser-marshal.c

plugin_LTLIBRARIES = libfilebrowser.la

NOINST_H_FILES = \
	pluma-file-bookmarks-store.h 		\
	pluma-file-browser-store.h 		\
	pluma-file-browser-view.h 		\
	pluma-file-browser-widget.h 		\
	pluma-file-browser-error.h		\
	pluma-file-browser-utils.h		\
	pluma-file-browser-plugin.h		\
	pluma-file-browser-messages.h

libfilebrowser_la_SOURCES = \
	$(BUILT_SOURCES) 			\
	pluma-file-bookmarks-store.c 		\
	pluma-file-browser-store.c 		\
	pluma-file-browser-view.c 		\
	pluma-file-browser-widget.c 		\
	pluma-file-browser-utils.c 		\
	pluma-file-browser-plugin.c		\
	pluma-file-browser-messages.c		\
	$(NOINST_H_FILES)

libfilebrowser_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libfilebrowser_la_LIBADD = $(PLUMA_LIBS)

# UI files (if you use ui for your plugin, list those files here)
uidir = $(PLUMA_PLUGINS_DATA_DIR)/filebrowser
ui_DATA = pluma-file-browser-widget-ui.xml

plugin_in_files = filebrowser.pluma-plugin.desktop.in

%.pluma-plugin: %.pluma-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

pluma-file-browser-enum-types.h: pluma-file-browser-enum-types.h.template $(NOINST_H_FILES) $(GLIB_MKENUMS)
	(cd $(srcdir) && $(GLIB_MKENUMS) --template pluma-file-browser-enum-types.h.template $(NOINST_H_FILES)) > $@

pluma-file-browser-enum-types.c: pluma-file-browser-enum-types.c.template pluma-file-browser-enum-register.c.template $(NOINST_H_FILES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && \
	$(GLIB_MKENUMS) --template pluma-file-browser-enum-types.c.template $(NOINST_H_FILES) && \
	$(GLIB_MKENUMS) --template pluma-file-browser-enum-register.c.template $(NOINST_H_FILES)) > $@

pluma-file-browser-marshal.h: pluma-file-browser-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) $(GLIB_GENMARSHAL) $< --header --prefix=pluma_file_browser_marshal > $@

pluma-file-browser-marshal.c: pluma-file-browser-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) echo "#include \"pluma-file-browser-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=pluma_file_browser_marshal >> $@

plugin_DATA = $(plugin_in_files:.pluma-plugin.desktop.in=.pluma-plugin)

@INTLTOOL_XML_NOMERGE_RULE@
gsettings_SCHEMAS = org.mate.pluma.plugins.filebrowser.gschema.xml
@GSETTINGS_RULES@

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN) $(SED) -e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' < $< > $@


EXTRA_DIST = \
	$(ui_DATA)					\
	$(plugin_in_files)				\
	$(gsettings_SCHEMAS).in.in			\
	pluma-file-browser-enum-types.h.template	\
	pluma-file-browser-enum-types.c.template	\
	pluma-file-browser-enum-register.c.template	\
	pluma-file-browser-marshal.list

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
