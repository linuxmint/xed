# Spell checker plugin
plugindir = $(XEDIT_PLUGINS_LIBS_DIR)

AM_CPPFLAGS = 							\
	-I$(top_srcdir)						\
	$(XEDIT_CFLAGS) 					\
	$(ENCHANT_CFLAGS)					\
	$(WARN_CFLAGS)						\
	$(DISABLE_DEPRECATED_CFLAGS)	

BUILT_SOURCES = 					\
	xedit-spell-marshal.c				\
	xedit-spell-marshal.h

plugin_LTLIBRARIES = libspell.la

libspell_la_SOURCES = \
	xedit-spell-plugin.c 				\
	xedit-spell-plugin.h 				\
	xedit-spell-checker.c 				\
	xedit-spell-checker.h				\
	xedit-spell-checker-dialog.c			\
	xedit-spell-checker-dialog.h			\
	xedit-spell-checker-language.c			\
	xedit-spell-checker-language.h			\
	xedit-spell-language-dialog.c			\
	xedit-spell-language-dialog.h			\
	xedit-automatic-spell-checker.c			\
	xedit-automatic-spell-checker.h			\
	xedit-spell-utils.c				\
	xedit-spell-utils.h				\
	$(BUILT_SOURCES)			

libspell_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS) 
libspell_la_LIBADD  = $(XEDIT_LIBS) $(ENCHANT_LIBS)

uidir = $(XEDIT_PLUGINS_DATA_DIR)/spell
ui_DATA = spell-checker.ui languages-dialog.ui xedit-spell-setup-dialog.ui

xedit-spell-marshal.h: xedit-spell-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) $(GLIB_GENMARSHAL) $< --header --prefix=xedit_marshal > $@

xedit-spell-marshal.c: xedit-spell-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) echo "#include \"xedit-spell-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=xedit_marshal >> $@

plugin_in_files = spell.xedit-plugin.desktop.in

%.xedit-plugin: %.xedit-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.xedit-plugin.desktop.in=.xedit-plugin)

@INTLTOOL_XML_NOMERGE_RULE@
spell_gschema_in = org.x.editor.plugins.spell.gschema.xml.in
gsettings_SCHEMAS = $(spell_gschema_in:.xml.in=.xml)
@GSETTINGS_RULES@


EXTRA_DIST = 					\
	$(ui_DATA)				\
	$(plugin_in_files)			\
	xedit-spell-marshal.list    	        \
	$(spell_gschema_in)

CLEANFILES = $(BUILT_SOURCES) $(plugin_DATA) $(gsettings_SCHEMAS)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)

-include $(top_srcdir)/git.mk
