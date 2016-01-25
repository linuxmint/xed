uidir = $(datadir)/xedit/ui/

AM_CPPFLAGS = 							\
	-I$(top_srcdir)						\
	-I$(top_builddir)					\
	-I$(top_srcdir)/xedit 					\
	-I$(top_builddir)/xedit					\
	$(XEDIT_CFLAGS) 					\
	$(WARN_CFLAGS)						\
	$(DISABLE_DEPRECATED_CFLAGS)

noinst_LTLIBRARIES = libdialogs.la

libdialogs_la_SOURCES = 			\
	xedit-preferences-dialog.h		\
	xedit-preferences-dialog.c		\
	xedit-close-confirmation-dialog.c 	\
	xedit-close-confirmation-dialog.h 	\
	xedit-encodings-dialog.c		\
	xedit-encodings-dialog.h		\
	xedit-search-dialog.h			\
	xedit-search-dialog.c

ui_DATA =					\
	xedit-encodings-dialog.ui		\
	xedit-preferences-dialog.ui		\
	xedit-search-dialog.ui

EXTRA_DIST = $(ui_DATA)

-include $(top_srcdir)/git.mk
