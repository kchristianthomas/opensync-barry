#!/bin/bash

if [ -z "$1" -o -z "$2" -o -z "$3" -o -z "$BARRYTARBALL" ] ; then
	echo
	echo "Do not call deb.sh directly.  Call release.sh instead."
	echo
	exit 1
fi

CHROOTTARGET="$1"
TAG="$2"
DEBTARGETS="$3"

if [ "$THEMODE" = "release" ] ; then
	./make-deb.sh "$BARRYTARBALL" "$CHROOTTARGET" "$TAG" "$DEBTARGETS"
elif [ "$THEMODE" = "test" ] ; then
	./test-build.sh "$BARRYTARBALL" "$CHROOTTARGET"
else
	echo
	echo "Mode not set.  Call release.sh or test.sh instead."
	echo
	exit 1
fi

