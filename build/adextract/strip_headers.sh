#!/bin/bash

SEDARG='/% Program:/,/mode: SPIKE/d';

for F in *.tt
do

	OUT=$F'.nh';
echo 'sed '$SEDARG' '$F' > '$OUT;
sed "$SEDARG" $F > $OUT

done

	



