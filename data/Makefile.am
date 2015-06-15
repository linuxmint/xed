desktopdir 	 = $(datadir)/applications
desktop_in_files = pluma.desktop.in
desktop_DATA 	 = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

@INTLTOOL_XML_RULE@
appdatadir = $(datadir)/appdata
appdata_in_files = pluma.appdata.xml.in
appdata_DATA = $(appdata_in_files:.xml.in=.xml)

gsettings_SCHEMAS = org.mate.pluma.gschema.xml
@GSETTINGS_RULES@

convertdir = $(datadir)/MateConf/gsettings
convert_DATA = pluma.convert

man_MANS = pluma.1

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = pluma.pc

bugreportdir = $(libexecdir)/pluma
bugreport_SCRIPTS = pluma-bugreport.sh

EXTRA_DIST = 			\
	$(appdata_in_files)	\
	$(desktop_in_files)	\
	$(convert_DATA)		\
	$(man_MANS)			\
	pluma.pc.in			\
	pluma-bugreport.sh.in

CLEANFILES =	 			\
	$(desktop_DATA)			\
	$(gsettings_SCHEMAS)	\
	$(pkgconfig_DATA)		\
	$(appdata_DATA)




-include $(top_srcdir)/git.mk
