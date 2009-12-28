#!/bin/bash

#Build version string into version2.h.tmp
echo -e -n "const char* g_version = \"3.0-" > version.h.tmp
git rev-parse --short=8 HEAD >> version.h.tmp
tr -d "\n\\\\" < version.h.tmp > version2.h.tmp
echo -e "\";" >> version2.h.tmp
rm version.h.tmp

#No version.h file? Make one
if [ ! -f version.h ]
then
	cp version2.h.tmp version.h
fi

#See if we need to overwrite the existing version
cmp -s version.h version2.h.tmp > /dev/null
if [ $? -eq 1 ]
then
	rm version.h
	cp version2.h.tmp version.h
fi

#Clean up
rm version2.h.tmp
