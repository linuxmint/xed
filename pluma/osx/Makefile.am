AM_CPPFLAGS = 							\
	-I$(top_srcdir)						\
	-I$(top_builddir)					\
	-I$(top_srcdir)/pluma 					\
	-I$(top_builddir)/pluma					\
	$(PLUMA_CFLAGS) 					\
	$(IGE_MAC_CFLAGS)					\
	$(WARN_CFLAGS)						\
	$(DISABLE_DEPRECATED_CFLAGS)

noinst_LTLIBRARIES = libosx.la

libosx_la_LDFLAGS = -framework Carbon -framework ApplicationServices -framework Cocoa
libosx_la_LIBADD = -lobjc
libosx_la_CFLAGS = -xobjective-c

libosx_la_SOURCES = 		\
	pluma-osx.c 		\
	pluma-osx.h		\
	pluma-osx-delegate.m 	\
	pluma-osx-delegate.h

-include $(top_srcdir)/git.mk
