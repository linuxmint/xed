desktopdir 	 = $(datadir)/applications
desktop_in_files = gedit.desktop.in
desktop_DATA 	 = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

schemasdir	 = $(MATECONF_SCHEMA_FILE_DIR)
schemas_in_files = gedit.schemas.in
schemas_DATA 	 = $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

man_MANS = gedit.1

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gedit-@GEDIT_API_VERSION@.pc

gedit-@GEDIT_API_VERSION@.pc: gedit.pc
	cp gedit.pc gedit-@GEDIT_API_VERSION@.pc

bugreportdir = $(libexecdir)/gedit-2
bugreport_SCRIPTS = gedit-bugreport.sh

if MATECONF_SCHEMAS_INSTALL
install-data-local:
	        if test -z "$(DESTDIR)" ; then \
	                for p in $(schemas_DATA) ; do \
	                        MATECONF_CONFIG_SOURCE=$(MATECONF_SCHEMA_CONFIG_SOURCE) $(MATECONFTOOL) --makefile-install-rule $(top_builddir)/data/$$p ; \
	                done \
	        fi
else
install-data-local:
endif

EXTRA_DIST = 				\
	$(desktop_in_files)		\
	$(schemas_in_files)		\
	gedit.schemas.in.in		\
	$(man_MANS)			\
	gedit.pc.in			\
	gedit-bugreport.sh.in

CLEANFILES =	 			\
	$(desktop_DATA)			\
	$(schemas_DATA)			\
	$(pkgconfig_DATA)




-include $(top_srcdir)/git.mk
