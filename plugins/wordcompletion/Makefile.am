# Word completion plugin
plugindir = $(XED_PLUGINS_LIBS_DIR)

AM_CPPFLAGS = 							\
	-I$(top_srcdir)						\
	$(XED_CFLAGS) 					\
	$(ENCHANT_CFLAGS)					\
	$(WARN_CFLAGS)						\
	$(DISABLE_DEPRECATED_CFLAGS)

plugin_LTLIBRARIES = libwordcompletion.la

libwordcompletion_la_SOURCES = \
	xed-wordcompletion-plugin.h	\
	xed-wordcompletion-plugin.c	

libwordcompletion_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libwordcompletion_la_LIBADD  = $(XED_LIBS) $(ENCHANT_LIBS)

plugin_in_files = wordcompletion.plugin.desktop.in

%.plugin: %.plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.plugin.desktop.in=.plugin)

uidir = $(XED_PLUGINS_DATA_DIR)/wordcompletion
ui_DATA = xed-wordcompletion-configure.ui

@INTLTOOL_XML_NOMERGE_RULE@
wordcompletion_gschema_in = org.x.editor.plugins.wordcompletion.gschema.xml.in
gsettings_SCHEMAS = $(wordcompletion_gschema_in:.xml.in=.xml)
@GSETTINGS_RULES@

EXTRA_DIST = 					\
	$(plugin_in_files)			\
	$(wordcompletion_gschema_in)
	
	
CLEANFILES = \
	$(plugin_DATA)		\
	$(gsettings_SCHEMAS)		

DISTCLEANFILES = \
	$(plugin_DATA)		

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
	
-include $(top_srcdir)/git.mk
