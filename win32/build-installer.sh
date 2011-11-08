#!/bin/sh
echo "You need to execute this on a Windows machine within msys (http://www.mingw.org)"
echo "You also need InnoSetup (http://www.innosetup.org) with iscc in your PATH"
echo "You need to have python, pygobject, pycairo and pygtk installed into C:\\Python26"
echo "Make sure pluma and all its dependencies have been installed correctly to /local"
echo "You can specify the paths by yourself:"
echo "./build-installer.sh VERSION GTK_PREFIX PLUMA_PREFIX GTKSOURCEVIEW_PREFIX PYTHON_PREFIX MISC_PREFIX ASPELL_PREFIX WINDOWS_PREFIX"

# we assume glib, gtk etc were installed in the root while pluma and gtksourceview
# in /local
#FIXME we need to figure out a way for autodetecting this
if test "$#" = 7; then
  _gtk_prefix="$2"
  _gtksourceview_prefix="$3"
  _pluma_prefix="$4"
  _python_prefix="$5"
  _misc_prefix="$6"
  _aspell_prefix="$7"
  _windows_prefix="$8"
else
  _gtk_prefix="/c/gtk"
  _gtksourceview_prefix="/usr/local"
  _pluma_prefix="/usr/local"
  _python_prefix="/c/Python26"
  _misc_prefix="/usr"
  _aspell_prefix="/c/Aspell"
  _windows_prefix="/c/WINDOWS/system32"
fi

if test "$1" = '--help'; then
  echo "VERSION: The version of the installer"
  echo "GTK_PREFIX: The path for gtk, by default /c/gtk"
  echo "PLUMA_PREFIX: The path for pluma, by default /usr/local"
  echo "GTKSOURCEVIEW_PREFIX: The path for gtksourceview, by default /usr/local"
  echo "PYTHON_PREFIX: The path for python, by default /c/Python25"
  echo "MISC_PREFIX: The path for the rest of dependencies: i.e: enchant: by default /usr"
  echo "ASPELL_PREFIX: The path for Aspell: by default /c/Aspell"
  exit
fi

revision=$1
if test "$revision" = ''; then
  echo "Installer revision not provided, assuming 1"
  revision=1
fi

echo "Cleanup..."
if test -e installer; then
  rm installer -Rf || exit;
fi

mkdir -p installer || exit

echo "Copying the docs..."
mkdir -p installer/pluma/share/doc || exit
cp ../COPYING installer/pluma/share/doc || exit
cp ../AUTHORS installer/pluma/share/doc || exit
cp ../README installer/pluma/share/doc || exit

echo "Copying gtk DLL files..."

#----------------------------- gtk ------------------------------------
mkdir -p installer/gtk/bin

cp "${_gtk_prefix}/bin/libglib-2.0-0.dll" installer/gtk/bin || exit
cp "${_gtk_prefix}/bin/libgio-2.0-0.dll" installer/gtk/bin || exit
cp "${_gtk_prefix}/bin/libgmodule-2.0-0.dll" installer/gtk/bin || exit
cp "${_gtk_prefix}/bin/libgobject-2.0-0.dll" installer/gtk/bin || exit
cp "${_gtk_prefix}/bin/libgthread-2.0-0.dll" installer/gtk/bin || exit

# TODO: We can probably omit these, as we do not use g_spawn on Windows anymore
cp "${_gtk_prefix}/bin/gspawn-win32-helper.exe" installer/gtk/bin || exit
cp "${_gtk_prefix}/bin/gspawn-win32-helper-console.exe" installer/gtk/bin || exit

cp "${_gtk_prefix}/bin/libatk-1.0-0.dll" installer/gtk/bin || exit
cp "${_gtk_prefix}/bin/libcairo-2.dll" installer/gtk/bin || exit
cp "${_gtk_prefix}/bin/libpng12-0.dll" installer/gtk/bin || exit
cp "${_gtk_prefix}/bin/libjpeg-7.dll" installer/gtk/bin || exit
cp "${_gtk_prefix}/bin/libtiff-3.dll" installer/gtk/bin || exit

cp "${_gtk_prefix}/bin/libpango-1.0-0.dll" installer/gtk/bin || exit
cp "${_gtk_prefix}/bin/libpangocairo-1.0-0.dll" installer/gtk/bin || exit
cp "${_gtk_prefix}/bin/libpangowin32-1.0-0.dll" installer/gtk/bin || exit
cp "${_gtk_prefix}/bin/libpangoft2-1.0-0.dll" installer/gtk/bin || exit

cp "${_gtk_prefix}/bin/libgdk-win32-2.0-0.dll" installer/gtk/bin || exit
cp "${_gtk_prefix}/bin/libgdk_pixbuf-2.0-0.dll" installer/gtk/bin || exit

cp "${_gtk_prefix}/bin/libgtk-win32-2.0-0.dll" installer/gtk/bin || exit

cp "${_gtk_prefix}/bin/libgailutil-18.dll" installer/gtk/bin || exit
cp "${_gtk_prefix}/bin/libfontconfig-1.dll" installer/gtk/bin || exit
cp "${_gtk_prefix}/bin/libexpat-1.dll" installer/gtk/bin || exit
cp "${_gtk_prefix}/bin/freetype6.dll" installer/gtk/bin || exit

echo "Stripping DLL files..."
strip installer/gtk/bin/*.dll || exit
strip installer/gtk/bin/*.exe || exit

#Copy zlib1 after stripping, as we strip this library it makes crash pluma
cp "${_gtk_prefix}/bin/zlib1.dll" installer/gtk/bin || exit

#-------------------------------- pluma ------------------------------------
echo "Copying misc DLL files..."
mkdir -p installer/pluma/bin

cp "${_misc_prefix}/bin/libgettextpo-0.dll" installer/pluma/bin || exit

cp "${_misc_prefix}/bin/libMateCORBA-2-0.dll" installer/pluma/bin || exit
cp "${_misc_prefix}/bin/libMateCORBACosNaming-2-0.dll" installer/pluma/bin || exit
cp "${_misc_prefix}/bin/libMateCORBA-imodule-2-0.dll" installer/pluma/bin || exit

cp "${_misc_prefix}/bin/libmateconf-2-4.dll" installer/pluma/bin || exit

cp "${_misc_prefix}/bin/libenchant.dll" installer/pluma/bin || exit
cp "${_misc_prefix}/bin/libsoup-2.4-1.dll" installer/pluma/bin || exit

cp "${_gtksourceview_prefix}/bin/libgtksourceview-2.0-0.dll" installer/pluma/bin || exit

echo "Stripping DLL files..."
strip installer/pluma/bin/*.dll || exit


# stripping libxml2.dll renders it unusable (although not changing it in size).
# We therefore copy it after having stripped the rest. Same with the other DLLs
# here. Perhaps those were built with MSVC.
cp "${_misc_prefix}/bin/libxml2-2.dll" installer/pluma/bin || exit
cp "${_misc_prefix}/bin/intl.dll" installer/pluma/bin || exit
cp "${_misc_prefix}/bin/iconv.dll" installer/pluma/bin || exit


echo "Copying Python..."

# TODO: Find out Windows directory somehow, we should use WINDIR substuting c:\?
cp ${_windows_prefix}/python26.dll installer/pluma/bin || exit

# We through all python modules into python/. pluma sets PYTHONPATH accordingly.
mkdir -p installer/python || exit

# Copy the dlls needed to run python
cp -R ${_python_prefix}/DLLs installer/python || exit

# TODO: Perhaps some scripts need more python modules.
mkdir -p installer/python/Lib || exit
cp ${_python_prefix}/Lib/*.py installer/python/Lib || exit

mkdir -p installer/python/Lib/encodings || exit
cp ${_python_prefix}/Lib/encodings/*.py installer/python/Lib/encodings || exit

cp -R ${_python_prefix}/Lib/site-packages installer/python/Lib || exit

cp -R ${_python_prefix}/Lib/xml installer/python/Lib || exit

mkdir -p installer/python/Lib/sqlite3 || exit
cp ${_python_prefix}/Lib/sqlite3/*.py installer/python/Lib/sqlite3 || exit

echo "Copying modules..."

cp "${_gtk_prefix}/bin/gtk-query-immodules-2.0.exe" installer/gtk/bin || exit

mkdir -p installer/gtk/lib/gtk-2.0/2.10.0/engines || exit
cp "${_gtk_prefix}/lib/gtk-2.0/2.10.0/engines/libwimp.dll" installer/gtk/lib/gtk-2.0/2.10.0/engines || exit
strip installer/gtk/lib/gtk-2.0/2.10.0/engines/libwimp.dll || exit

mkdir -p installer/gtk/lib/gtk-2.0/2.10.0/loaders || exit
cp "${_gtk_prefix}/lib/gtk-2.0/2.10.0/loaders/"*.dll installer/gtk/lib/gtk-2.0/2.10.0/loaders || exit
strip installer/gtk/lib/gtk-2.0/2.10.0/loaders/*.dll || exit
cp "${_gtk_prefix}/bin/gdk-pixbuf-query-loaders.exe" installer/gtk/bin || exit

# Gail
mkdir -p installer/gtk/lib/gtk-2.0/modules || exit
cp "${_gtk_prefix}/lib/gtk-2.0/modules/libgail.dll" installer/gtk/lib/gtk-2.0/modules || exit
strip installer/gtk/lib/gtk-2.0/modules/libgail.dll

# TODO: Can we omit this?
mkdir -p installer/gtk/etc/gtk-2.0
#cp "${_gtk_prefix}/etc/gtk-2.0/gtk.immodules" installer/etc/gtk-2.0 || exit -1
cp "${_gtk_prefix}/etc/gtk-2.0/gdk-pixbuf.loaders" installer/gtk/etc/gtk-2.0 || exit -1

mkdir -p installer/gtk/share/themes || exit
cp -R "${_gtk_prefix}/share/themes/MS-Windows" installer/gtk/share/themes || exit
mkdir -p installer/gtk/etc/gtk-2.0 || exit
echo "gtk-theme-name = \"MS-Windows\"" > installer/gtk/etc/gtk-2.0/gtkrc || exit

# Enchant
mkdir -p installer/pluma/lib/enchant || exit
cp "${_misc_prefix}/lib/enchant/"* installer/pluma/lib/enchant || exit
strip installer/pluma/lib/enchant/*.dll || exit
mkdir -p installer/pluma/share/enchant || exit
cp "${_misc_prefix}/share/enchant/"* installer/pluma/share/enchant || exit

# Iso codes
mkdir -p installer/pluma/share/iso-codes || exit
cp "${_misc_prefix}/share/iso-codes/"* installer/pluma/share/iso-codes || exit
mkdir -p installer/pluma/share/xml/iso-codes || exit
cp "${_misc_prefix}/share/xml/iso-codes/"* installer/pluma/share/xml/iso-codes || exit

echo "Copying locales..."

# We need to keep the locale files from share/locale in share/locale and those
# from lib/locale in lib/locale:
mkdir -p installer/locale/share/ || exit
cp "${_gtk_prefix}/share/locale" installer/locale/share/ -R || exit
cp "${_pluma_prefix}/share/locale" installer/locale/share/ -R || exit
cp "${_misc_prefix}/share/locale" installer/locale/share/ -R || exit

find installer/locale/share/locale/ -type f | grep -v atk10.mo | grep -v gtk20.mo | grep -v MateConf2.mo | grep -v glib20.mo | grep -v pluma.mo | grep -v gtk20.mo | grep -v gtk20-properties.mo | grep -v gtksourceview-2.0.mo | grep -v iso_*.mo | xargs rm
find installer/locale/share/locale -type d | xargs rmdir -p --ignore-fail-on-non-empty

echo "Copying executable..."
cp "${_pluma_prefix}/bin/pluma.exe" installer/pluma/bin || exit
strip installer/pluma/bin/pluma.exe || exit


echo "Copying shared data (ui files, icons, etc.)..."

mkdir -p installer/pluma/share/gtksourceview-2.0 || exit
cp -R "${_gtksourceview_prefix}/share/gtksourceview-2.0/language-specs" installer/pluma/share/gtksourceview-2.0 || exit
cp -R "${_gtksourceview_prefix}/share/gtksourceview-2.0/styles" installer/pluma/share/gtksourceview-2.0 || exit

#GtkBuilder files and xml files
mkdir -p installer/pluma/share/pluma/ui || exit
cp "${_pluma_prefix}/share/pluma/ui/"* installer/pluma/share/pluma/ui || exit

#Icons & logo
mkdir -p installer/pluma/share/pluma/icons || exit
cp "${_pluma_prefix}/share/pluma/icons/pluma-plugin.png" installer/pluma/share/pluma/icons || exit

#Plugins
mkdir -p installer/pluma/share/pluma/plugins || exit
cp -R "${_pluma_prefix}/share/pluma/plugins/"* installer/pluma/share/pluma/plugins || exit
mkdir -p installer/pluma/lib/pluma/plugins || exit
cp -R "${_pluma_prefix}/lib/pluma/plugins/"* installer/pluma/lib/pluma/plugins || exit
mkdir -p installer/pluma/lib/pluma/plugin-loaders || exit
cp -R "${_pluma_prefix}/lib/pluma/plugin-loaders/"* installer/pluma/lib/pluma/plugin-loaders || exit

#MateConf
mkdir -p installer/pluma/etc/mateconf/schemas || exit
cp "${_pluma_prefix}/etc/mateconf/schemas/"* installer/pluma/etc/mateconf/schemas || exit
cp -R "${_misc_prefix}/etc/mateconf/"* installer/pluma/etc/mateconf/ || exit
mkdir -p installer/pluma/lib/MateConf/2
cp "${_misc_prefix}/lib/MateConf/2/"* installer/pluma/lib/MateConf/2 || exit
strip installer/pluma/lib/MateConf/2/*.dll || exit
mkdir -p installer/pluma/libexec || exit
cp "${_misc_prefix}/libexec/mateconfd-2.exe" installer/pluma/libexec || exit

#Aspell
mkdir -p installer/pluma/data || exit
cp "${_aspell_prefix}/data/"* installer/pluma/data || exit
cp "${_aspell_prefix}/bin/aspell-15.dll" installer/pluma/bin/libaspell-15.dll || exit

echo "Creating installer..."

perl -pe "s/INSTALLERREVISION/$revision/" pluma.iss > installer/pluma.iss || exit
#cp installer || exit
iscc installer/pluma.iss || exit

echo "Done"
