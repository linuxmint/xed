## Process this file with automake to produce Makefile.in
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = gedit pixmaps po data plugin-loaders plugins docs tests win32 osx

if !OS_OSX
SUBDIRS += help
endif

distuninstallcheck_listfiles = find . -type f -print | grep -v scrollkeeper

EXTRA_DIST = 			\
	BUGS			\
	ChangeLog-20011116	\
	ChangeLog-20051212	\
	ChangeLog-20090418	\
	HACKING			\
	MAINTAINERS		\
	gedit.doap		\
	xmldocs.make 		\
	omf.make		\
	mate-doc-utils.make    \
	intltool-extract.in 	\
	intltool-merge.in 	\
	intltool-update.in

DISTCLEANFILES = 		\
	mate-doc-utils.make    \
	intltool-extract 	\
	intltool-merge 		\
	intltool-update

MAINTAINERCLEANFILES = 		\
	aclocal.m4		\
	config.guess		\
	config.h.in		\
	config.sub		\
	depcomp			\
	gtk-doc.make		\
	install-sh		\
	ltmain.sh		\
	missing			\
	mkinstalldirs		\
	omf.make		\
	py-compile		\
	xmldocs.make		\
	m4/mate-doc-utils.m4	\
	m4/gtk-doc.m4		\
	m4/intltool.m4		\
	m4/libtool.m4		\
	m4/ltoptions.m4		\
	m4/ltsugar.m4		\
	m4/ltversion.m4		\
	m4/lt~obsolete.m4	\
	`find "$(srcdir)" -type f -name Makefile.in -print`

DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper --enable-gtk-doc

CHANGELOG_START = GEDIT_2_26_1

dist-hook:
	@if test -d "$(srcdir)/.git"; \
	then \
		echo Creating ChangeLog && \
		(GIT_DIR=$(top_srcdir)/.git \
		  ./missing --run git log $(CHANGELOG_START)^^.. --stat -M -C --name-status  --date=short --no-color) | \
		  fmt --split-only > ChangeLog.tmp \
		&& mv -f ChangeLog.tmp $(top_distdir)/ChangeLog \
		|| ( rm -f ChangeLog.tmp ; \
		  echo Failed to generate ChangeLog >&2 ); \
	else \
		echo A git clone is required to generate a ChangeLog >&2; \
	fi

-include $(top_srcdir)/git.mk
