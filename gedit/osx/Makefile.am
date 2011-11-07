INCLUDES = 							\
	-I$(top_srcdir)						\
	-I$(top_builddir)					\
	-I$(top_srcdir)/gedit 					\
	-I$(top_builddir)/gedit					\
	$(GEDIT_CFLAGS) 					\
	$(IGE_MAC_CFLAGS)					\
	$(WARN_CFLAGS)						\
	$(DISABLE_DEPRECATED_CFLAGS)

noinst_LTLIBRARIES = libosx.la

libosx_la_LDFLAGS = -framework Carbon -framework ApplicationServices -framework Cocoa
libosx_la_LIBADD = -lobjc
libosx_la_CFLAGS = -xobjective-c

libosx_la_SOURCES = 		\
	gedit-osx.c 		\
	gedit-osx.h		\
	gedit-osx-delegate.m 	\
	gedit-osx-delegate.h

-include $(top_srcdir)/git.mk
