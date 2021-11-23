#!/bin/bash

echo autogen
aclocal
autoheader
automake --add-missing
autoreconf -i -f
