#!/bin/bash


# for each .h and .c file
for f in *.{h,c}; do
	# check to see if the $f is a file
	if [ -f $f ]; then
		TMP=${f%.c}.c2
	    cp $f ${f%.c}.c.bkp
	    cp $f $TMP

	    # Replace "unsigned long" with uint32_t
	    SEDARG='s/\bunsigned long\b/uint32_t/g'   
	    sed -e "$SEDARG" $f > $TMP
	    cp $TMP $f

	    # Replace "long" with int32_t
	    SEDARG='s/\blong\b/int32_t/g'   
	    sed -e "$SEDARG" $f > $TMP
	    cp $TMP $f

		# Replace "unsigned int" with int32_t
	    SEDARG='s/\bunsigned int\b/uint32_t/g'   
	    sed -e "$SEDARG" $f > $TMP
	    cp $TMP $f    

	    # Replace "int" with int32_t
	    SEDARG='s/\bint\b/int32_t/g'   
	    sed -e "$SEDARG" $f > $TMP
	    cp $TMP $f


	    # Replace "unsigned short" with int16_t
	    SEDARG='s/\bunsigned short\b/int16_t/g'   
	    sed -e "$SEDARG" $f > $TMP
	    cp $TMP $f


	    # Replace "short" with int16_t
	    SEDARG='s/\bshort\b/int16_t/g'   
	    sed -e "$SEDARG" $f > $TMP
	    cp $TMP $f

	    rm $TMP
	    echo -e "Replaced vars in:"$f"\tOriginal saved as:"${f%.c}.c.bkp
	fi
done
