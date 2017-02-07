dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.63.2)

m4_define(xed_major_version, 1)
m4_define(xed_minor_version, 2)
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

PKG_CHECK_MODULES(GMODULE,gmodule-2.0,[GMODULE_ADD="gmodule-2.0"],[GMODULE_ADD=""])
PKG_CHECK_MODULES(XED, [
	libxml-2.0 >= 2.5.0
	glib-2.0 >= 2.40.0
	$GMODULE_ADD
	gthread-2.0 >= 2.13.0
	gio-2.0 >= 2.40.0
	gtk+-3.0 >= 3.14.0
	gtksourceview-3.0 >= 3.14.3
	libpeas-1.0 >= 1.12.0
	libpeas-gtk-1.0 >= 1.12.0
])

PKG_CHECK_MODULES(X11, [x11])

XED_CFLAGS="$XED_CFLAGS $X11_CFLAGS"
XED_LIBS="$XED_LIBS $X11_LIBS"

AC_SUBST(XED_CFLAGS)
AC_SUBST(XED_LIBS)

# Introspection
GOBJECT_INTROSPECTION_CHECK([0.9.3])

if test "$found_introspection" = "yes"; then
	enable_introspection=yes
	AC_DEFINE([ENABLE_INTROSPECTION], [1], [Define to enable GObject Introspection])
else
	enable_introspection=no
fi

dnl ================================================================
dnl GSettings related settings
dnl ================================================================

GLIB_GSETTINGS

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

PLUGIN_LIBTOOL_FLAGS="-module -avoid-version"

AC_SUBST(PLUGIN_LIBTOOL_FLAGS)

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
xed/Makefile
help/Makefile
pixmaps/Makefile
plugins/Makefile
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
])

AC_OUTPUT

echo "

Configuration:

	Source code location:	${srcdir}
	Compiler:				${CC}
	Spell Plugin enabled:	$enable_enchant
	Gvfs metadata enabled:	$enable_gvfs_metadata
	GObject Introspection:	${enable_introspection}
"
