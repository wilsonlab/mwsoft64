#Test Extraction

INPUT='raw.spk';

NAME='extracted';
ARC='original';
EXT='.tt';

BIN='adextract_'$ARC;

OUTPUT=$NAME'_'$ARC$EXT;

./$BIN -eslen80 -t -probe 0 $INPUT -o $OUTPUT
