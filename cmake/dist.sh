#!/bin/bash

# $1 is source directory
# $2 is binary directory

DIST=$1/dist/wg-monitor/
mkdir -p $DIST
cp $2/wg-monitor $DIST
cp -r $1/public $DIST
cd $DIST/..
zip -r wg-monitor.zip wg-monitor