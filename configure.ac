dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.63.2)

m4_define(gedit_major_version,  2011)
m4_define(gedit_minor_version, 10)
m4_define(gedit_micro_version,  26)
m4_define(gedit_version, gedit_major_version.gedit_minor_version.gedit_micro_version)

AC_INIT([gedit],[gedit_version],[http://bugzilla.mate.org/enter_bug.cgi?product=gedit],[gedit],[http://projects.mate.org/gedit/])

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_SRCDIR(gedit/gedit.c)
AC_CONFIG_MACRO_DIR([m4])

AC_DEFINE(GEDIT_MAJOR_VERSION, gedit_major_version, [Gedit major version])
AC_SUBST(GEDIT_MAJOR_VERSION, gedit_major_version)
AC_DEFINE(GEDIT_MINOR_VERSION, gedit_minor_version, [Gedit minor version])
AC_SUBST(GEDIT_MINOR_VERSION, gedit_minor_version)
AC_DEFINE(GEDIT_MICRO_VERSION, gedit_micro_version, [Gedit micro version])
AC_SUBST(GEDIT_MICRO_VERSION, gedit_micro_version)

AM_INIT_AUTOMAKE([1.10 dist-bzip2 no-dist-gzip -Wno-portability])
AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Checks for programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_SYS_LARGEFILE
PKG_PROG_PKG_CONFIG
# needed on osx
AC_PROG_OBJC

# Initialize libtool
LT_PREREQ([2.2.6])
LT_INIT(disable-static)

GEDIT_API_VERSION=2.20
AC_SUBST(GEDIT_API_VERSION)

AC_PATH_PROG(MATECONFTOOL, mateconftool-2)

GTK_DOC_CHECK([1.0],[--flavour=no-tmpl])

AC_CHECK_FUNC(sigaction)
AC_CHECK_LIB(m, floor)

dnl make sure we keep ACLOCAL_FLAGS around for maintainer builds to work
AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS -I m4")

dnl active plugins for gedit.schemas
ACTIVE_PLUGINS="docinfo,modelines,filebrowser,spell,time"
AC_SUBST(ACTIVE_PLUGINS)

dnl toolbar style for gedit schemas
TOOLBAR_STYLE="GEDIT_TOOLBAR_SYSTEM"
AC_SUBST(TOOLBAR_STYLE)

dnl check for win32 platform
AC_MSG_CHECKING([for Win32 platform])
case "$host" in
  *-*-mingw*|*-*-cygwin*)
    platform_win32=yes
    TOOLBAR_STYLE="GEDIT_TOOLBAR_ICONS"
    ;;
  *)
    platform_win32=no
    ;;
esac
AC_MSG_RESULT([$platform_win32])
AM_CONDITIONAL(PLATFORM_WIN32, test "$platform_win32" = "yes")

AC_MSG_CHECKING([for native Win32])
case "$host" in
  *-*-mingw*)
    os_win32=yes
    AC_CHECK_TOOL(WINDRES, windres)
    ACTIVE_PLUGINS="${ACTIVE_PLUGINS},checkupdate"
    ;;
  *)
    os_win32=no
    ;;
esac
AC_MSG_RESULT([$os_win32])
AM_CONDITIONAL(OS_WIN32, test "$os_win32" = "yes")

dnl check for osx platform
AC_MSG_CHECKING([for Mac OS X platform])
case "$host" in
  *-*-darwin*)
    platform_osx=yes
    ;;
  *)
    platform_osx=no
    ;;
esac
AC_MSG_RESULT([$platform_osx])
AM_CONDITIONAL(PLATFORM_OSX, test "$platform_osx" = "yes")

dnl check for native osx
gdk_windowing=`$PKG_CONFIG --variable=target gdk-2.0`

AC_MSG_CHECKING([for native Mac OS X])
if test "$gdk_windowing" = "quartz"; then
	os_osx=yes
	ACTIVE_PLUGINS="${ACTIVE_PLUGINS},checkupdate"
else
	os_osx=no
fi
AC_MSG_RESULT([$os_osx])
AM_CONDITIONAL(OS_OSX, test "$os_osx" = "yes")

if test "$platform_osx" = "yes"; then
	AC_DEFINE([PLATFORM_OSX],[1],[Defined if platform is Mac OSX])
fi

AC_SUBST(DEFAULT_FONT)
AC_SUBST(DEFAULT_PRINT_FONT)

if test "$os_osx" = "yes"; then
	AC_DEFINE([OS_OSX],[1],[Defined if os is Mac OSX])

	PKG_CHECK_MODULES(IGE_MAC, ige-mac-integration)

	AC_SUBST(IGE_MAC_CFLAGS)
	AC_SUBST(IGE_MAC_LIBS)
fi

dnl ===============================================================
dnl Expanded dirs
dnl ===============================================================

# This macro expands DIR and assigns it to RET.
# If DIR is NONE, then it's replaced by DEFAULT.
# Based on AC_DEFINE_DIR
AC_DEFUN([GEDIT_FULLPATH], [
  test "x$prefix" = xNONE && prefix="$ac_default_prefix"
  test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'
  ac_expand=[$]$1
  test "x$ac_expand" = xNONE && ac_expand="[$]$2"
  ac_expand=`eval echo [$]ac_expand`
  $3=`eval echo [$]ac_expand`
])

dnl FULL_LIBEXECDIR is used for X-MATE-Bugzilla-ExtraInfoScript expansion
dnl in data/gedit.desktop.in.in
GEDIT_FULLPATH(libexecdir, NONE, FULL_LIBEXECDIR)
AC_SUBST(FULL_LIBEXECDIR)

dnl ================================================================
dnl Gettext stuff.
dnl ================================================================

IT_PROG_INTLTOOL([0.40.0])
AC_SUBST([GETTEXT_PACKAGE], [gedit])
AM_GNU_GETTEXT_VERSION([0.17])
AM_GLIB_GNU_GETTEXT
AC_DEFINE([GETTEXT_PACKAGE],[PACKAGE_TARNAME], [Define to the gettext package name.])

dnl ================================================================
dnl Use gvfs metadata or the old xml file store.
dnl ================================================================

if test "x$os_osx" != "xno" || test "x$os_win32" != "xno"; then
	gvfs_metadata_default="no"
else
	gvfs_metadata_default="yes"
fi

AC_ARG_ENABLE([gvfs-metadata],
	AS_HELP_STRING([--enable-gvfs-metadata], [Enable using gvfs to store metadata]),
	[enable_gvfs_metadata=$enableval],
	[enable_gvfs_metadata=$gvfs_metadata_default])

AM_CONDITIONAL(ENABLE_GVFS_METADATA, test "x$enable_gvfs_metadata" != "xno")

if test "x$enable_gvfs_metadata" != "xno"; then
	AC_DEFINE([ENABLE_GVFS_METADATA], [1], [Define to enable using gvfs metadata])
fi

dnl ================================================================
dnl check update plugin
dnl ================================================================

AC_ARG_ENABLE([updater],
	AS_HELP_STRING([--disable-updater],[Disable check update plugin (default: enabled)]),
	[enable_updater=$enableval],
	[enable_updater=yes])

if test "x$enable_updater" != "xno"; then
	PKG_CHECK_MODULES(LIBSOUP, \
		libsoup-2.4,
		have_libsoup=yes,
		have_libsoup=no)

	if test "x$have_libsoup" = "xyes"; then
		if test "x$os_win32" = "xyes"; then
			enable_updater=yes
		elif test "x$os_osx" = "xyes"; then
			enable_updater=yes
		else
			enable_updater=no
		fi
	else
		enable_updater=no
	fi
fi

AM_CONDITIONAL(ENABLE_UPDATER, test x"$enable_updater" = "xyes")

dnl ================================================================
dnl spell plugins checks: enchant and iso-codes
dnl ================================================================

ENCHANT_REQUIRED=1.2.0
ISO_CODES_REQUIRED=0.35

AC_ARG_ENABLE([spell],
	AS_HELP_STRING([--disable-spell],[Disable spell plugin (default: enabled)]),
	[enable_enchant=$enableval],
	[enable_enchant=yes])

if test "x$enable_enchant" = "xyes" ; then

	PKG_CHECK_MODULES(ENCHANT, enchant >= $ENCHANT_REQUIRED, \
			  have_enchant=yes, have_enchant=no)

	if test "x$have_enchant" = "xyes"; then

		PKG_CHECK_EXISTS([iso-codes >= $ISO_CODES_REQUIRED],
				 [have_iso_codes=yes],[have_iso_codes=no])

		if test "x$have_iso_codes" = "xyes"; then
			AC_MSG_CHECKING([whether iso-codes has iso-639 and iso-3166 domains])
			if $PKG_CONFIG --variable=domains iso-codes | grep 639 > /dev/null && \
			   $PKG_CONFIG --variable=domains iso-codes | grep 3166 > /dev/null ; then
				result=yes
			else
				result=no
				have_iso_codes=no
			fi
			AC_MSG_RESULT([$result])
		fi

		if test "x$have_iso_codes" = "xyes"; then
			AC_DEFINE_UNQUOTED([ISO_CODES_PREFIX],["`$PKG_CONFIG --variable=prefix iso-codes`"],[ISO codes prefix])
			AC_DEFINE([HAVE_ISO_CODES],[1],[Define if you have the iso-codes package])
		else
			AC_MSG_ERROR([iso-codes is required to build the spell plugin. Use --disable-spell to build without spell plugin.])
		fi
	else
		enable_enchant=no
		AC_MSG_ERROR([Enchant library not found or too old. Use --disable-spell to build without spell plugin.])
	fi
fi

AM_CONDITIONAL(ENABLE_ENCHANT, test x"$enable_enchant" = "xyes")

dnl ================================================================
dnl libsocket checks - needed for bacon-connection on solaris.
dnl ================================================================
AC_SEARCH_LIBS(bind, socket)

dnl ================================================================
dnl Start of pkg-config checks
dnl ================================================================

PKG_CHECK_MODULES(GEDIT, [
	libxml-2.0 >= 2.5.0
	glib-2.0 >= 2.22.0
	gthread-2.0 >= 2.13.0
	gio-2.0 >= 2.23.1
	gtk+-2.0 >= 2.16.0
	gtksourceview-2.0 >= 2.9.7
	mateconf-2.0 >= 1.1.11
])

if test "$os_osx" = "no" &&
   test "$os_win32" = "no"; then
	PKG_CHECK_MODULES(X11, [
		x11
	])
else
	X11_CFLAGS=
	X11_LIBS=
fi

GEDIT_CFLAGS="$GEDIT_CFLAGS $X11_CFLAGS"
GEDIT_LIBS="$GEDIT_LIBS $X11_LIBS"

AC_SUBST(GEDIT_CFLAGS)
AC_SUBST(GEDIT_LIBS)

dnl FIXME: Remove this when removing gedit-message-area

if $PKG_CONFIG --atleast-version 2.17.1 gtk+-2.0; then
       gtkatleast=
else
       gtkatleast=no
fi

AM_CONDITIONAL(BUILD_MESSAGE_AREA, test "$gtkatleast" = "no")

dnl FIXME: Remove this when removing gedit-spinner

if $PKG_CONFIG --atleast-version 2.19.0 gtk+-2.0; then
       gtkatleast=
else
       gtkatleast=no
fi

AM_CONDITIONAL(BUILD_SPINNER, test "$gtkatleast" = "no")
if test "$gtkatleast" = "no"; then
	AC_DEFINE([BUILD_SPINNER],[1],[Whether to use GeditSpinner instead of GtkSpinner])
fi

if test "$os_osx" = "no" &&
   test "$os_win32" = "no"; then
	PKG_CHECK_MODULES(EGG_SMCLIENT, [
		sm >= 1.0.0
	])
else
	EGG_SMCLIENT_CFLAGS=
	EGG_SMCLIENT_LIBS=
fi

AC_SUBST(EGG_SMCLIENT_CFLAGS)
AC_SUBST(EGG_SMCLIENT_LIBS)

dnl ================================================================
dnl MateConf related settings
dnl ================================================================

AM_MATECONF_SOURCE_2

dnl ================================================================
dnl Python
dnl ================================================================

AC_MSG_CHECKING([whether Python support is requested])
AC_ARG_ENABLE([python],
	AS_HELP_STRING([--enable-python],[Enable python support]),
	[enable_python=$enableval have_python=$enableval],
	[enable_python=autodetect have_python=yes])
AC_MSG_RESULT([$enable_python])

if test "x$have_python" != "xno"; then
	AM_PATH_PYTHON([2.5],[],[no])
	if test "x$PYTHON" = "x:"; then
		have_python=no
	fi
fi

if test "x$have_python" != "xno"; then
	AM_CHECK_PYTHON_HEADERS([],[have_python=no])
fi

if test "x$have_python" != "xno"; then
	PY_EXEC_PREFIX=`$PYTHON -c 'import sys ; print sys.exec_prefix'`
	PYTHON_LIBS="-lpython$PYTHON_VERSION"

	if test "$os_win32" = yes; then
		PYTHON_LIBS=`echo $PYTHON_LIBS | sed 's/\.//g'`
		PYTHON_LIB_LOC="-I$PY_EXEC_PREFIX/libs"
		PYTHON_CFLAGS="-I$PY_EXEC_PREFIX/include"
		PYTHON_EXTRA_LIBS=
	else
		PY_PREFIX=`$PYTHON -c 'import sys ; print sys.prefix'`
		PYTHON_LIB_LOC="-L$PY_EXEC_PREFIX/lib/python$PYTHON_VERSION/config"
		PYTHON_CFLAGS="-I$PY_PREFIX/include/python$PYTHON_VERSION"
		PYTHON_MAKEFILE="$PY_EXEC_PREFIX/lib/python$PYTHON_VERSION/config/Makefile"
		PYTHON_BASEMODLIBS=`sed -n -e 's/^BASEMODLIBS=\(.*\)/\1/p' $PYTHON_MAKEFILE`
		PYTHON_OTHER_LIBS=`sed -n -e 's/^LIBS=\(.*\)/\1/p' $PYTHON_MAKEFILE`
		PYTHON_EXTRA_LIBS="$PYTHON_BASEMODLIBS $PYTHON_OTHER_LIBS"
	fi
	AC_SUBST([PYTHON_LIBS])
	AC_SUBST([PYTHON_LIB_LOC])
	AC_SUBST([PYTHON_CFLAGS])
	AC_SUBST([PYTHON_EXTRA_LIBS])
fi

if test "x$have_python" != "xyes"; then
	if test "x$enable_python" = "xyes"; then
		AC_MSG_ERROR([Python not found])
	elif test "x$enable_python" = "xautodetect"; then
		enable_python=no
		AC_MSG_WARN([Python not found, disabling python support])
	fi
fi

if test "x$have_python" != "xno"; then
	PYGOBJECT_REQUIRED=2.15.4
	PYGTK_REQUIRED=2.12.0
	PYGTKSOURCEVIEW_REQUIRED=2.9.2

	PKG_CHECK_MODULES([PYGTK], [
		pygobject-2.0 >= $PYGOBJECT_REQUIRED
		pygtk-2.0 >= $PYGTK_REQUIRED
		pygtksourceview-2.0 >= $PYGTKSOURCEVIEW_REQUIRED],
		[],
		[
		have_python=no
		if test "x$enable_python" = "xyes"; then
			AC_MSG_ERROR([$PYGTK_PKG_ERRORS])
		elif test "x$enable_python" = "xautodetect"; then
			enable_python=no
			AC_MSG_WARN([$PYGTK_PKG_ERRORS])
			AC_MSG_WARN([Disabling python support])
		fi
		])
fi

if test "x$have_python" != "xno"; then
	AC_MSG_CHECKING([for pygtk defs])
	PYGTK_DEFSDIR=`$PKG_CONFIG --variable=defsdir pygtk-2.0`
	AC_MSG_RESULT([$PYGTK_DEFSDIR])

	AC_MSG_CHECKING([for pygobject defs])
	PYGOBJECT_DEFSDIR=`$PKG_CONFIG --variable=defsdir pygobject-2.0`
	AC_MSG_RESULT([$PYGOBJECT_DEFSDIR])

	AC_MSG_CHECKING([for pygtk codegen])
	PYGTK_CODEGEN="$PYTHON `$PKG_CONFIG --variable=codegendir pygtk-2.0`/codegen.py"
	AC_MSG_RESULT([$PYGTK_CODEGEN])

	AC_MSG_CHECKING([for pygtk h2def])
	PYGTK_H2DEF="$PYTHON `$PKG_CONFIG --variable=codegendir pygtk-2.0`/h2def.py"
	AC_MSG_RESULT([$PYGTK_H2DEF])

	AC_SUBST([PYGTK_DEFSDIR])
	AC_SUBST([PYGOBJECT_DEFSDIR])
	AC_SUBST([PYGTK_CODEGEN])
	AC_SUBST([PYGTK_H2DEF])

	dnl Check for -fno-strict-aliasing
	FLAGS="-fno-strict-aliasing"
	save_CFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS $FLAGS"
	AC_MSG_CHECKING([whether [$]CC understands $FLAGS])
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])],[compiler_has_option=yes],[compiler_has_option=no])
	CFLAGS="$save_CFLAGS"
	AC_MSG_RESULT($compiler_has_option)
	if test $compiler_has_option = yes; then
		NO_STRICT_ALIASING_CFLAGS="$FLAGS"
	fi
	AC_SUBST([NO_STRICT_ALIASING_CFLAGS])
fi

if test "x$have_python" != "xno" -a "x$enable_python" != "xno"; then
	enable_python=yes
	AC_DEFINE([ENABLE_PYTHON],[1],[Define to compile with python support])
fi

AM_CONDITIONAL([ENABLE_PYTHON],[test "x$enable_python" = "xyes"])

dnl This allows the bug-report script to know whether python has been enabled
AC_SUBST(enable_python)

dnl ================================================================
dnl Misc
dnl ================================================================
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)
AC_PATH_PROG(GLIB_MKENUMS, glib-mkenums)

MATE_COMPILE_WARNINGS(yes)

AC_ARG_ENABLE(deprecations,
              [AS_HELP_STRING([--enable-deprecations],
                              [warn about deprecated usages [default=no]])],,
              [enable_deprecations=no])

if test "x$enable_deprecations" = "xyes"; then
   DISABLE_DEPRECATED_CFLAGS="\
-DG_DISABLE_DEPRECATED \
-DGDK_DISABLE_DEPRECATED \
-DGTK_DISABLE_DEPRECATED \
-DGDK_PIXBUF_DISABLE_DEPRECATED \
-DMATE_DISABLE_DEPRECATED"
   AC_SUBST(DISABLE_DEPRECATED_CFLAGS)
fi

# on win32 plugins must be able to resolve all symbols at link time
# so we create a lib
if test "$platform_win32" = yes; then
	GEDIT_IMPLIB="-lgedit-$GEDIT_API_VERSION"
	PLUGIN_LIBTOOL_FLAGS="-module -avoid-version -no-undefined -Wl,\$(top_builddir)/gedit/libgedit-\$(GEDIT_API_VERSION).a"
	LOADER_LIBTOOL_FLAGS="-module -avoid-version -no-undefined -Wl,\$(top_builddir)/gedit/libgedit-\$(GEDIT_API_VERSION).a"
	if test "$os_win32" = yes; then
		PLUGIN_LIBTOOL_FLAGS="${PLUGIN_LIBTOOL_FLAGS} -mwindows"
		LOADER_LIBTOOL_FLAGS="${LOADER_LIBTOOL_FLAGS} -mwindows"
	fi
else
	GEDIT_IMPLIB=""
	PLUGIN_LIBTOOL_FLAGS="-module -avoid-version"
	LOADER_LIBTOOL_FLAGS="-module -avoid-version"
fi
AC_SUBST(GEDIT_IMPLIB)
AC_SUBST(PLUGIN_LIBTOOL_FLAGS)
AC_SUBST(LOADER_LIBTOOL_FLAGS)

GEDIT_PLUGINS_DATA_DIR="$datadir/gedit-2/plugins"
AC_SUBST(GEDIT_PLUGINS_DATA_DIR)

GEDIT_PLUGINS_LIBS_DIR="$libdir/gedit-2/plugins"
AC_SUBST(GEDIT_PLUGINS_LIBS_DIR)

MATE_DOC_INIT([0.9.0],,
  [AC_MSG_WARN([[mate-doc-utils not found: documentation will not be built.]])])

AC_CONFIG_FILES([
Makefile
data/gedit.desktop.in
data/gedit-bugreport.sh
data/gedit.pc
data/gedit.schemas.in
data/Makefile
docs/Makefile
docs/reference/Makefile
gedit/dialogs/Makefile
gedit/smclient/Makefile
gedit/osx/Makefile
gedit/Makefile
help/Makefile
pixmaps/Makefile
plugin-loaders/Makefile
plugin-loaders/c/Makefile
plugin-loaders/python/Makefile
plugin-loaders/python/bindings/Makefile
plugins/Makefile
plugins/changecase/Makefile
plugins/checkupdate/Makefile
plugins/docinfo/Makefile
plugins/externaltools/data/Makefile
plugins/externaltools/Makefile
plugins/externaltools/scripts/Makefile
plugins/externaltools/tools/Makefile
plugins/filebrowser/Makefile
plugins/modelines/Makefile
plugins/pythonconsole/Makefile
plugins/pythonconsole/pythonconsole/Makefile
plugins/quickopen/Makefile
plugins/quickopen/quickopen/Makefile
plugins/snippets/data/lang/Makefile
plugins/snippets/data/Makefile
plugins/snippets/Makefile
plugins/snippets/snippets/Makefile
plugins/sort/Makefile
plugins/spell/Makefile
plugins/taglist/Makefile
plugins/time/Makefile
po/Makefile.in
tests/Makefile
win32/gedit.iss
win32/Makefile
osx/Info.plist
osx/Makefile])

AC_OUTPUT

echo "

Configuration:

	Source code location:	${srcdir}
	Compiler:		${CC}
	Python Plugins Support: $enable_python
	Spell Plugin enabled:	$enable_enchant
	Gvfs metadata enabled:	$enable_gvfs_metadata
"

if expr ${GEDIT_MINOR_VERSION} % 2 > /dev/null; then
	stable_version=`expr ${GEDIT_MINOR_VERSION} - 1`

cat << EOF
 *** IMPORTANT ***

This is an unstable version of gedit.
It is for test purposes only.

Please, DO NOT use it in a production environment.
It will probably crash and you will lose your data.

If you are looking for a stable release of gedit, either download
it from:

  http://ftp.mate.org/pub/mate/sources/gedit/2.${stable_version}/

Or checkout the mate-${stable_version} branch of the gedit module from:

  git://git.mate.org/gedit

Thanks,
The gedit team

 *** END OF WARNING ***
EOF

fi
