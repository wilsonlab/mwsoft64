clear;
DATE=`/bin/date`;
ARCH=`uname -m`;

CMD='gcc spikeparms2.c iolib.c -o ../bin/spikeparms/sp2_'$ARCH' -I . -DDATE="date" -lm';
echo $CMD
$CMD
