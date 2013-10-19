desktopdir 	 = $(datadir)/applications
desktop_in_files = pluma.desktop.in
desktop_DATA 	 = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

@INTLTOOL_XML_NOMERGE_RULE@
gsettings_SCHEMAS = org.mate.pluma.gschema.xml
@GSETTINGS_RULES@

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN) $(SED) \
		-e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' \
		-e 's^\@ACTIVE_PLUGINS\@^$(ACTIVE_PLUGINS)^g' \
		-e 's^\@TOOLBAR_STYLE\@^$(TOOLBAR_STYLE)^g' \
		< $< > $@

convertdir = $(datadir)/MateConf/gsettings
convert_DATA = pluma.convert

man_MANS = pluma.1

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = pluma.pc

bugreportdir = $(libexecdir)/pluma
bugreport_SCRIPTS = pluma-bugreport.sh

EXTRA_DIST = 				\
	$(desktop_in_files)		\
	$(gsettings_SCHEMAS).in.in	\
	$(convert_DATA)			\
	$(man_MANS)			\
	pluma.pc.in			\
	pluma-bugreport.sh.in

CLEANFILES =	 			\
	$(desktop_DATA)			\
	$(gsettings_SCHEMAS_in)	\
	$(gsettings_SCHEMAS)	\
	$(pkgconfig_DATA)




-include $(top_srcdir)/git.mk
