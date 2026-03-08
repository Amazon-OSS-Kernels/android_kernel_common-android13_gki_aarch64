#!/bin/bash

BIN2HEX=./bin2hex.pl
HEX_OUT=key.txt
PERL=/usr/bin/perl

if [ $# -eq 0 ]; then
	echo "Usage: sh keybin_release.sh <YOUR_KEY_BIN>"
	exit
fi

$PERL $BIN2HEX $1 1 > $HEX_OUT
echo "Release $HEX_OUT"
