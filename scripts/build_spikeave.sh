clear;
DATE=`/bin/date`;
ARCH=`uname -m`;

CMD='gcc ../src/extract/spikeavgn.c ../src/iolib.c -o ../tests/spikeavg/sa_'$ARCH' -I ../src/ -DDATE="date" -lm';

echo $CMD
$CMD
