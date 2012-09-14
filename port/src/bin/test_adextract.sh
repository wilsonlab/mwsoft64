#Test Extraction

INPUT='raw.spk';

NAME='extracted';
ARC=`uname -m`;
EXT='.tt';

BIN='adextract_'$ARC;

OUTPUT=$NAME$ARC$EXT;

./$BIN -eslen80 -t -probe 0 $INPUT -o $OUTPUT
