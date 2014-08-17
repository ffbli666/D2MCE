#!/bin/sh
if [ $# -ne 1 ]; then
	echo "usage: $0 <programe>"
	exit
else
	i=1
	for i in 1 2 3 4;
			do ssh -f cluster$i $HOME/new_d2mce/src/examples/matrix-d2mce/$1;
	done
fi
