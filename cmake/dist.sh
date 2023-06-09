#!/bin/bash

# $1 is source directory
# $2 is binary directory

DIST=$1/dist/wg-monitor/
mkdir -p $DIST

cp $2/wg-monitor $DIST
cp $1/wg-monitor.service $DIST
cp -r $1/www $DIST

cd $DIST/..
zip -r wg-monitor.zip wg-monitor