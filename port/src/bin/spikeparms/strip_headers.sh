#!/bin/bash

SEDARG='/% Program:/,/Binary/d';

for F in *.pxyabw
do

	OUT=$F'.nh';
echo 'sed '$SEDARG' '$F' > '$OUT;
sed "$SEDARG" $F > $OUT

done

	



