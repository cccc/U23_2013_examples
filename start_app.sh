#!/bin/bash
if [ -z "$1" ]; then
    echo Please provide a new app name
    exit 1
fi
if [ -e "apps/$1" ]; then
    echo An app with this name already exists
    exit 1
fi
cp -r ./.template_app ./apps/$1
sed -i -e "s/^APPNAME/TARGET = $1/" ./apps/$1/target.mak
sed -i -e "s/^\\(SUBDIRS.*\\)/\\1 $1/" ./apps/target.mak
echo Find your new app in ./apps/$1/
