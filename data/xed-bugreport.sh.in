#!/bin/sh

PKG_CONFIG_MODULES="glib-2.0 gtk+-3.0 gtksourceview-3.0 enchant iso-codes"

echo_padded ()
{
	echo -n "  - $1 "
	N=$(echo -n $1 | wc -m)
	while test $N -le 20
	do
		echo -n " "
		N=`expr $N + 1`
	done
}

if (which gsettings >/dev/null)
then
	echo "GSettings configuration dump:"
	gsettings list-recursively org.x.editor
	echo
fi

echo "Active plugins:"
gsettings get org.x.editor active-plugins 				\
	| sed -r -e 's/^\[(.*)\]$/\1/' -e 's/, /\n/g'			\
	| sed -e 's/^.*$/  - \0/'
echo

# Manually installed plugins (in $HOME)
if [ -d $HOME/.config/xed/plugins ]
then
	echo "Plugins in \$HOME:"
	ls $HOME/.config/xed/plugins/*.xed-plugin			\
		| sed -r -e 's#.*/([^/]*)\.xed-plugin$#  - \1#'
else
	echo "No plugin installed in \$HOME."
fi
echo

echo "Module versions:"
if (which pkg-config > /dev/null)
then
	for i in $PKG_CONFIG_MODULES
	do
		echo_padded "`echo -n $i | sed -r -e 's/^(.*)-[0-9]\.[0-9]$/\1/'`"
		pkg-config --modversion $i 2>/dev/null || echo
	done
else
	echo "  pkg-config unavailable"
fi
echo
