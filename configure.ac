dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.63.2)

m4_define(xed_major_version, 1)
m4_define(xed_minor_version, 0)
m4_define(xed_micro_version, 2)
m4_define(xed_version, xed_major_version.xed_minor_version.xed_micro_version)

AC_INIT([xed], [xed_version],
    [https://github.com/linuxmint/xed/issues],
    [xed], [http://github.com/linuxmint/xed])

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_SRCDIR(xed/xed.c)
AC_CONFIG_MACRO_DIR([m4])

AC_DEFINE(XED_MAJOR_VERSION, xed_major_version, [Xed major version])
AC_SUBST(XED_MAJOR_VERSION, xed_major_version)
AC_DEFINE(XED_MINOR_VERSION, xed_minor_version, [Xed minor version])
AC_SUBST(XED_MINOR_VERSION, xed_minor_version)
AC_DEFINE(XED_MICRO_VERSION, xed_micro_version, [Xed micro version])
AC_SUBST(XED_MICRO_VERSION, xed_micro_version)

AM_INIT_AUTOMAKE([1.10 dist-xz no-dist-gzip -Wno-portability check-news])
AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Checks for programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_SYS_LARGEFILE
PKG_PROG_PKG_CONFIG

# Initialize libtool
LT_PREREQ([2.2.6])
LT_INIT(disable-static)

GTK_DOC_CHECK([1.0],[--flavour=no-tmpl])

AC_CHECK_FUNC(sigaction)
AC_CHECK_LIB(m, floor)

dnl make sure we keep ACLOCAL_FLAGS around for maintainer builds to work
AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS -I m4")

dnl toolbar style for GSettings schemas
TOOLBAR_STYLE="XED_TOOLBAR_SYSTEM"
AC_SUBST(TOOLBAR_STYLE)

# GLib min/max required versions
AC_DEFINE([GLIB_VERSION_MAX_ALLOWED], [GLIB_VERSION_2_36],
	[Warn on use of APIs added after GLib 2.36])
AC_DEFINE([GLIB_VERSION_MIN_REQUIRED], [GLIB_VERSION_2_36],
	[Warn on use of APIs deprecated before GLib 2.36])

dnl ===============================================================
dnl Expanded dirs
dnl ===============================================================

# This macro expands DIR and assigns it to RET.
# If DIR is NONE, then it's replaced by DEFAULT.
# Based on AC_DEFINE_DIR
AC_DEFUN([XED_FULLPATH], [
  test "x$prefix" = xNONE && prefix="$ac_default_prefix"
  test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'
  ac_expand=[$]$1
  test "x$ac_expand" = xNONE && ac_expand="[$]$2"
  ac_expand=`eval echo [$]ac_expand`
  $3=`eval echo [$]ac_expand`
])

dnl ================================================================
dnl Gettext stuff.
dnl ================================================================

IT_PROG_INTLTOOL([0.50.1])
GETTEXT_PACKAGE=xed
AC_SUBST([GETTEXT_PACKAGE])
AM_GLIB_GNU_GETTEXT
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Define to the Gettext package name])

dnl ================================================================
dnl Use gvfs metadata or the old xml file store.
dnl ================================================================

AC_ARG_ENABLE([gvfs-metadata],
	AS_HELP_STRING([--enable-gvfs-metadata], [Enable using gvfs to store metadata]),
	[enable_gvfs_metadata=$enableval],
	[enable_gvfs_metadata=yes])

AM_CONDITIONAL(ENABLE_GVFS_METADATA, test "x$enable_gvfs_metadata" != "xno")

if test "x$enable_gvfs_metadata" != "xno"; then
	AC_DEFINE([ENABLE_GVFS_METADATA], [1], [Define to enable using gvfs metadata])
fi

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
dnl Start of pkg-config checks
dnl ================================================================

AC_MSG_CHECKING([which gtk+ version to compile against])
AC_ARG_WITH([gtk],
  [AS_HELP_STRING([--with-gtk=2.0|3.0],[which gtk+ version to compile against (default: 2.0)])],
  [case "$with_gtk" in
     2.0|3.0) ;;
     *) AC_MSG_ERROR([invalid gtk version specified]) ;;
   esac],
  [with_gtk=2.0])
AC_MSG_RESULT([$with_gtk])

case "$with_gtk" in
  2.0) GTK_API_VERSION=2.0
       GTK_REQUIRED=2.24.0
       ;;
  3.0) GTK_API_VERSION=3.0
       GTK_REQUIRED=3.0.0
       ;;
esac
AC_SUBST(GTK_API_VERSION)

PKG_CHECK_MODULES(GMODULE,gmodule-2.0,[GMODULE_ADD="gmodule-2.0"],[GMODULE_ADD=""])
PKG_CHECK_MODULES(XED, [
	libxml-2.0 >= 2.5.0
	glib-2.0 >= 2.36.0
	$GMODULE_ADD
	gthread-2.0 >= 2.13.0
	gio-2.0 >= 2.26.0
	gtk+-$GTK_API_VERSION >= $GTK_REQUIRED
	gtksourceview-$GTK_API_VERSION >= 2.9.7
])

PKG_CHECK_MODULES(X11, [x11])

XED_CFLAGS="$XED_CFLAGS $X11_CFLAGS"
XED_LIBS="$XED_LIBS $X11_LIBS"

AC_SUBST(XED_CFLAGS)
AC_SUBST(XED_LIBS)

PKG_CHECK_MODULES(EGG_SMCLIENT, [sm >= 1.0.0])

AC_SUBST(EGG_SMCLIENT_CFLAGS)
AC_SUBST(EGG_SMCLIENT_LIBS)

dnl ================================================================
dnl GSettings related settings
dnl ================================================================

GLIB_GSETTINGS

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

	PY_PREFIX=`$PYTHON -c 'import sys ; print sys.prefix'`
	PYTHON_LIB_LOC="-L$libdir/python$PYTHON_VERSION/config"
	PYTHON_CFLAGS="-I$PY_PREFIX/include/python$PYTHON_VERSION"
	PYTHON_MAKEFILE="$libdir/python$PYTHON_VERSION/config/Makefile"
	PYTHON_BASEMODLIBS=`sed -n -e 's/^BASEMODLIBS=\(.*\)/\1/p' $PYTHON_MAKEFILE`
	PYTHON_OTHER_LIBS=`sed -n -e 's/^LIBS=\(.*\)/\1/p' $PYTHON_MAKEFILE`
	PYTHON_EXTRA_LIBS="$PYTHON_BASEMODLIBS $PYTHON_OTHER_LIBS"

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

GNOME_COMPILE_WARNINGS(yes)

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
-DGNOME_DISABLE_DEPRECATED"
   AC_SUBST(DISABLE_DEPRECATED_CFLAGS)
fi

AC_ARG_ENABLE([tests],
	AS_HELP_STRING([--enable-tests], [Enable the tests]),
	[enable_tests=$enableval],
	[enable_tests=yes])

AM_CONDITIONAL(ENABLE_TESTS, test x$enable_tests = xyes)

PLUGIN_LIBTOOL_FLAGS="-module -avoid-version"
LOADER_LIBTOOL_FLAGS="-module -avoid-version"

AC_SUBST(PLUGIN_LIBTOOL_FLAGS)
AC_SUBST(LOADER_LIBTOOL_FLAGS)

XED_PLUGINS_DATA_DIR="$datadir/xed/plugins"
AC_SUBST(XED_PLUGINS_DATA_DIR)

XED_PLUGINS_LIBS_DIR="$libdir/xed/plugins"
AC_SUBST(XED_PLUGINS_LIBS_DIR)

YELP_HELP_INIT

AC_CONFIG_FILES([
Makefile
data/xed.desktop.in
data/xed-bugreport.sh
data/xed.pc
data/org.x.editor.gschema.xml
data/Makefile
docs/Makefile
docs/reference/Makefile
xed/dialogs/Makefile
xed/smclient/Makefile
xed/Makefile
help/Makefile
pixmaps/Makefile
plugin-loaders/Makefile
plugin-loaders/c/Makefile
plugin-loaders/python/Makefile
plugin-loaders/python/bindings/Makefile
plugins/Makefile
plugins/changecase/Makefile
plugins/docinfo/Makefile
plugins/filebrowser/Makefile
plugins/filebrowser/org.x.editor.plugins.filebrowser.gschema.xml
plugins/modelines/Makefile
plugins/sort/Makefile
plugins/spell/Makefile
plugins/spell/org.x.editor.plugins.spell.gschema.xml
plugins/taglist/Makefile
plugins/time/Makefile
plugins/time/org.x.editor.plugins.time.gschema.xml
plugins/trailsave/Makefile
po/Makefile.in
tests/Makefile
])

AC_OUTPUT

echo "

Configuration:

	Source code location:	${srcdir}
	Compiler:		${CC}
	GTK API version:	$GTK_API_VERSION
	Python Plugins Support: $enable_python
	Spell Plugin enabled:	$enable_enchant
	Gvfs metadata enabled:	$enable_gvfs_metadata
	Tests enabled:		$enable_tests
"
