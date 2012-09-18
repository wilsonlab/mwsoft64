clear;
DATE=`/bin/date`;
ARCH=`uname -m`;

CMD='gcc ../src/extract/adextract.c ../src/iolib.c -o ../build/adextract_'$ARCH' -I ../src/ -DDATE="date" -lm';

echo $CMD
$CMD
