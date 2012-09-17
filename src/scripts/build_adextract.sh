clear;
DATE=`/bin/date`;
ARCH=`uname -m`;

CMD='gcc adextract.c iolib.c -o ../bin/adextract/adextract_'$ARCH' -I . -DDATE="date" -lm';
echo $CMD
$CMD
