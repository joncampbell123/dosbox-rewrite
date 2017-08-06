#!/bin/bash
aclocal || exit 1
autoconf || exit 1
autoheader || exit 1
automake --add-missing --foreign --copy --force || exit 1

