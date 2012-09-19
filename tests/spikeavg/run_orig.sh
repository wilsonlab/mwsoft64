#Test Extraction

INPUT='orig.pxyabw';

ARC="orig";
EXT='.cl';

BIN='sa_'$ARC;

OUTPUT=$ARC$EXT;

CMD='./'$BIN' orig.tt -if cbfile'

echo $CMD

$CMD > $OUTPUT



