#!/bin/sh
GIT_SHA1=`(git show-ref --head --hash=8 2> /dev/null || echo 00000000) | head -n1`
GIT_DIRTY=`git diff --no-ext-diff 2> /dev/null | wc -l`
BUILD_ID=`uname -n`"-"`date +%s`
VERSION="Beta 1.0.0.0"

if [ -n "$SOURCE_DATE_EPOCH" ]; then
	BUILD_ID=$(date -u -d "@$SOURCE_DATE_EPOCH" +%s 2>/dev/null || date -u -r "$SOURCE_DATE_EPOCH" +%s 2>/dev/null || date -u %s)
fi

test -f ./src/release.h || touch ./src/release.h
(cat ./src/release.h | grep SHA1  | grep $GIT_SHA1) && \
(cat ./src/release.h | grep DIRTY | grep $GIT_DIRTY) && exit 0 # Already up-to-date
echo "#define LOG_VERSION   \"$VERSION\""   >> ./src/release.h
echo "#define LOG_GIT_SHA1  \"$GIT_SHA1\""  >> ./src/release.h
echo "#define LOG_GIT_DIRTY \"$GIT_DIRTY\"" >> ./src/release.h
echo "#define LOG_BUILD_ID  \"$BUILD_ID\""  >> ./src/release.h

mkdir -p ./bin/
mkdir -p ./obj/
