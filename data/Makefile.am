desktopdir 	 = $(datadir)/applications
desktop_in_files = xedit.desktop.in
desktop_DATA 	 = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

@INTLTOOL_XML_RULE@
appdatadir = $(datadir)/appdata
appdata_in_files = xedit.appdata.xml.in
appdata_DATA = $(appdata_in_files:.xml.in=.xml)

gsettings_SCHEMAS = org.x.editor.gschema.xml
@GSETTINGS_RULES@

man_MANS = xedit.1

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = xedit.pc

bugreportdir = $(libexecdir)/xedit
bugreport_SCRIPTS = xedit-bugreport.sh

EXTRA_DIST = 			\
	$(appdata_in_files)	\
	$(desktop_in_files)	\
	$(man_MANS)			\
	xedit.pc.in			\
	xedit-bugreport.sh.in

CLEANFILES =	 			\
	$(desktop_DATA)			\
	$(gsettings_SCHEMAS)	\
	$(pkgconfig_DATA)		\
	$(appdata_DATA)




-include $(top_srcdir)/git.mk
