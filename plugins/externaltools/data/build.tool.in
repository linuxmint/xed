#!/bin/sh

EHOME=`echo $HOME | sed "s/#/\#/"`
DIR=$GEDIT_CURRENT_DOCUMENT_DIR
while test "$DIR" != "/"; do
    for m in GNUmakefile makefile Makefile; do
        if [ -f "${DIR}/${m}" ]; then
            echo "Using ${m} from ${DIR}" | sed "s#$EHOME#~#" > /dev/stderr
            make -C "${DIR}"
            exit
        fi
    done
    DIR=`dirname "${DIR}"`
done
echo "No Makefile found!" > /dev/stderr
