# Spell checker plugin
plugindir = $(PLUMA_PLUGINS_LIBS_DIR)

AM_CPPFLAGS = 							\
	-I$(top_srcdir)						\
	$(PLUMA_CFLAGS) 					\
	$(ENCHANT_CFLAGS)					\
	$(WARN_CFLAGS)						\
	$(DISABLE_DEPRECATED_CFLAGS)	

BUILT_SOURCES = 					\
	pluma-spell-marshal.c				\
	pluma-spell-marshal.h

plugin_LTLIBRARIES = libspell.la

libspell_la_SOURCES = \
	pluma-spell-plugin.c 				\
	pluma-spell-plugin.h 				\
	pluma-spell-checker.c 				\
	pluma-spell-checker.h				\
	pluma-spell-checker-dialog.c			\
	pluma-spell-checker-dialog.h			\
	pluma-spell-checker-language.c			\
	pluma-spell-checker-language.h			\
	pluma-spell-language-dialog.c			\
	pluma-spell-language-dialog.h			\
	pluma-automatic-spell-checker.c			\
	pluma-automatic-spell-checker.h			\
	pluma-spell-utils.c				\
	pluma-spell-utils.h				\
	$(BUILT_SOURCES)			

libspell_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS) 
libspell_la_LIBADD  = $(PLUMA_LIBS) $(ENCHANT_LIBS)

uidir = $(PLUMA_PLUGINS_DATA_DIR)/spell
ui_DATA = spell-checker.ui languages-dialog.ui

pluma-spell-marshal.h: pluma-spell-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) $(GLIB_GENMARSHAL) $< --header --prefix=pluma_marshal > $@

pluma-spell-marshal.c: pluma-spell-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) echo "#include \"pluma-spell-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=pluma_marshal >> $@

plugin_in_files = spell.pluma-plugin.desktop.in

%.pluma-plugin: %.pluma-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.pluma-plugin.desktop.in=.pluma-plugin)

EXTRA_DIST = 					\
	$(ui_DATA)				\
	$(plugin_in_files)			\
	pluma-spell-marshal.list 

CLEANFILES = $(BUILT_SOURCES) $(plugin_DATA)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)

-include $(top_srcdir)/git.mk
