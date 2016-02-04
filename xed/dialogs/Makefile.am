uidir = $(datadir)/xed/ui/

AM_CPPFLAGS = 							\
	-I$(top_srcdir)						\
	-I$(top_builddir)					\
	-I$(top_srcdir)/xed 					\
	-I$(top_builddir)/xed					\
	$(XED_CFLAGS) 					\
	$(WARN_CFLAGS)						\
	$(DISABLE_DEPRECATED_CFLAGS)

noinst_LTLIBRARIES = libdialogs.la

libdialogs_la_SOURCES = 			\
	xed-preferences-dialog.h		\
	xed-preferences-dialog.c		\
	xed-close-confirmation-dialog.c 	\
	xed-close-confirmation-dialog.h 	\
	xed-encodings-dialog.c		\
	xed-encodings-dialog.h		\
	xed-search-dialog.h			\
	xed-search-dialog.c

ui_DATA =					\
	xed-encodings-dialog.ui		\
	xed-preferences-dialog.ui		\
	xed-search-dialog.ui

EXTRA_DIST = $(ui_DATA)

-include $(top_srcdir)/git.mk
