# Spell checker plugin
plugindir = $(XED_PLUGINS_LIBS_DIR)

AM_CPPFLAGS = 							\
	-I$(top_srcdir)						\
	$(XED_CFLAGS) 					\
	$(GSPELL_CFLAGS)					\
	$(WARN_CFLAGS)						\
	$(DISABLE_DEPRECATED_CFLAGS)

BUILT_SOURCES = 					\
	xed-spell-marshal.c				\
	xed-spell-marshal.h

plugin_LTLIBRARIES = libspell.la

libspell_la_SOURCES = \
	xed-spell-plugin.c 				\
	xed-spell-plugin.h 				\
	$(BUILT_SOURCES)

libspell_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libspell_la_LIBADD  = $(XED_LIBS) $(GSPELL_LIBS)

uidir = $(XED_PLUGINS_DATA_DIR)/spell
ui_DATA = xed-spell-setup-dialog.ui

xed-spell-marshal.h: xed-spell-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) $(GLIB_GENMARSHAL) $< --header --prefix=xed_marshal > $@

xed-spell-marshal.c: xed-spell-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) echo "#include \"xed-spell-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=xed_marshal >> $@

plugin_in_files = spell.plugin.desktop.in

%.plugin: %.plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.plugin.desktop.in=.plugin)

@INTLTOOL_XML_NOMERGE_RULE@
spell_gschema_in = org.x.editor.plugins.spell.gschema.xml.in
gsettings_SCHEMAS = $(spell_gschema_in:.xml.in=.xml)
@GSETTINGS_RULES@


EXTRA_DIST = 					\
	$(ui_DATA)				\
	$(plugin_in_files)			\
	xed-spell-marshal.list    	        \
	$(spell_gschema_in)

CLEANFILES = $(BUILT_SOURCES) $(plugin_DATA) $(gsettings_SCHEMAS)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)

-include $(top_srcdir)/git.mk
