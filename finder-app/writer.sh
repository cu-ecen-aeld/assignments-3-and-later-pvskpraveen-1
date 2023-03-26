#!/bin/bash
#author: pvskpraveen

if [ $# -ne 2 ]
then
	exit 1
else
	abspath=$1
	mkdir -p "${abspath%/*}/"
	` touch $1 `
	echo $2 >> abspath
fi
