clear;
DATE=`/bin/date`;
ARCH=`uname -m`;

CMD='gcc ../src/extract/spikeparms2.c ../src/iolib.c -o ../test/spikeparms/sp2_'$ARCH' -I ../src/ -DDATE="date" -lm';
echo $CMD
$CMD
