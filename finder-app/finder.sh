#!/bin/sh

#accepts 2 arguments filesdir, searchstr
#exits with error 1 if arguments are not correct
#exits with error 1 if filesdir is not a valid directory
#prints the number of files with the number of 
Xval=0
Yval=0
#if the number of argument is less thatn 2
if  [ $# -ne 2 ] 
then
	echo "missing arguments format: finder.sh arg1 and arg2"
	exit 1
else
	if  [ -d $1 ]
	then 
			Xval=$( ls -Rpc $1 | grep -v / | wc -l )
			Yval=$( grep -rc $2 $1 | wc -l )
			echo "The number of files are ${Xval} and the number of matching lines are ${Yval}"
	else
		echo "Invalid directory"
		exit 1
	fi
fi
