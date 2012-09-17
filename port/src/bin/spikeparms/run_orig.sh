#Test Extraction

INPUT='orig.tt';

ARC='orig';
EXT='.pxyabw';

BIN='sp2_'$ARC;

OUTPUT=$ARC$EXT;

CMD='./'$BIN' '"$INPUT"' -tetrode -parms t_px,t_py,t_pa,t_pb,t_maxwd,t_maxht,time,t_h1,t_h2,t_h3,t_h4 -binary -o '"$OUTPUT";
echo $CMD
$CMD
