#Test Extraction
infile='raw.spk';
out32='out32.tt'
out64='out64.tt'

adextract -eslen80 -t -probe 0 $infile -o $out32
./adextract64 -eslen80 -t -probe 0 $infile -o $out64
