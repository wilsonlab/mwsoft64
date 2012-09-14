clear;
DATE=`/bin/date`;
CMD='gcc adextract64.c iolib.c -o ../bin/adextract64 -I . -DDATE="date"';
echo $CMD
$CMD