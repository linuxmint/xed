# Spell checker plugin
plugindir = $(XED_PLUGINS_LIBS_DIR)

AM_CPPFLAGS = 							\
	-I$(top_srcdir)						\
	$(XED_CFLAGS) 					\
	$(ENCHANT_CFLAGS)					\
	$(WARN_CFLAGS)						\
	$(DISABLE_DEPRECATED_CFLAGS)	

BUILT_SOURCES = 					\
	xed-spell-marshal.c				\
	xed-spell-marshal.h

plugin_LTLIBRARIES = libspell.la

libspell_la_SOURCES = \
	xed-spell-plugin.c 				\
	xed-spell-plugin.h 				\
	xed-spell-checker.c 				\
	xed-spell-checker.h				\
	xed-spell-checker-dialog.c			\
	xed-spell-checker-dialog.h			\
	xed-spell-checker-language.c			\
	xed-spell-checker-language.h			\
	xed-spell-language-dialog.c			\
	xed-spell-language-dialog.h			\
	xed-automatic-spell-checker.c			\
	xed-automatic-spell-checker.h			\
	xed-spell-utils.c				\
	xed-spell-utils.h				\
	$(BUILT_SOURCES)			

libspell_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS) 
libspell_la_LIBADD  = $(XED_LIBS) $(ENCHANT_LIBS)

uidir = $(XED_PLUGINS_DATA_DIR)/spell
ui_DATA = spell-checker.ui languages-dialog.ui xed-spell-setup-dialog.ui

xed-spell-marshal.h: xed-spell-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) $(GLIB_GENMARSHAL) $< --header --prefix=xed_marshal > $@

xed-spell-marshal.c: xed-spell-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) echo "#include \"xed-spell-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=xed_marshal >> $@

plugin_in_files = spell.xed-plugin.desktop.in

%.xed-plugin: %.xed-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.xed-plugin.desktop.in=.xed-plugin)

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
