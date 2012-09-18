#Test Extraction

INPUT='raw.spk';

ARC=`uname -m`;
EXT='.tt';

BIN='adextract_'$ARC;

OUTPUT=$ARC$EXT;

./$BIN -eslen80 -t -probe 0 $INPUT -o $OUTPUT
