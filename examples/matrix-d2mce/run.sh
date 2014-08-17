#!/bin/sh
if [ $# -ne 2 ]; then
	echo "usage: $0 <programe> <times>"
	exit
else
	i=1
	while [ $i -lt $2 ]
		do ./$1 -s 2048
		i=`expr $i  + 1`
	done
fi
