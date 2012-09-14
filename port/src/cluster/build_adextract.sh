clear;
DATE=`/bin/date`;
ARCH=`uname -m`;
CMD='gcc adextract64.c iolib.c -o ../bin/adextract_'$ARCH' -I . -DDATE="date"';
echo $CMD
$CMD
