#!/bin/bash

if [ $# -ne 2 ]; then
    echo "usage: run_suite.sh PROFILE SUITENAME" >&2
    exit 255
fi

if [ ! -r $2 ]; then
    echo "'$2' not found" >&2
    exit 255
fi

PROFILE=$1
. $2

[ -e "./env.sh" ] && . ./env.sh

OK=0
FAILURE=0
SKIP=0
EXPFAILURE=0

#MINNODE=0
#MAXNODE=$(axiom-info -N -q)
#MAXNODE=$((MAXNODE-1))

MINNODE=1
MAXNODE=$(axiom-info -N -q)

make_test() {
    NUMTEST=$1
    NUMARG=$2
    NUMARGS=$3
    EXIT=$4
    EXITALT=$5
    MODE=$6
    EXEC=$7

    echo "@@"
    echo "@@"
    echo "@@ TEST: $NUMTEST/$NUMTESTS params $NUMARG/$NUMARGS"
    echo "@@" 
    echo "@@"
    echo
    echo $EXEC $8 $9 $10 $11 $12 $13 $14
    echo

    # flush pending 
    axiom-run -n $((MINNODE+1))-$MAXNODE ./axiom-utility -f
    axiom-utility -f
    
    echo "########################################"

    if [ "$MODE" = "skip" ]; then
	echo "skip test (does not terminate!)..."
        echo "########################################"
        echo
	echo "TEST: SKIPPED!!!!!!"
	SKIP=$((SKIP+1))
	echo
        echo TESTS=$((OK+FAILURE+SKIP)) OK=$OK BAD=$FAILURE SKIP=$SKIP
        echo	
	return 0
    fi
            
    # NB: the 'eval' is needed because $EXEC, ecc.. can contain arguments
    # that are escaped, surronded by apici, ecc...

    if [ -x ./run_test_${PROFILE}.sh ]; then
	eval ./run_test_${PROFILE}.sh ./$EXEC $8 $9 $10 $11 $12 $13 $14
	RES=$?
    else
	eval axiom-run -P $PROFILE ./$EXEC $8 $9 $10 $11 $12 $13 $14
	RES=$?
    fi
        
    echo "########################################"
    echo
    if [ "$EXITALT" != "none" ]; then
	echo "exit code $RES expected $EXIT or $EXITALT"
	if [ "$RES" -eq "$EXIT" -o "$RES" -eq "$EXITALT" ]; then
	    echo "TEST: SUCCESS"
	    OK=$((OK+1))
	else
	    echo "TEST: FAILURE!"
            if [ "$MODE" = "error" ]; then
		echo "      BUT EXPECTED!"
		EXPFAILURE=$((EXPFAILURE+1))
	    fi
	    FAILURE=$((FAILURE+1))
	fi
    else	
	echo "exit code $RES expected $EXIT"
	if [ "$RES" -eq "$EXIT" ]; then
	    echo "TEST: SUCCESS"
	    OK=$((OK+1))
	else
	    echo "TEST: FAILURE!!!!!!"
            if [ "$MODE" = "error" ]; then
		echo "      BUT EXPECTED!"
		EXPFAILURE=$((EXPFAILURE+1))
	    fi
	    FAILURE=$((FAILURE+1))
	fi
    fi
    echo
    echo TESTS=$((OK+FAILURE+SKIP)) OK=$OK BAD=$FAILURE SKIP=$SKIP
    echo

    # :-)
    axiom-run -n $((MINNODE+1))-$MAXNODE killall axiom-run
    killall axiom-run
    sleep 1
}

if [ -z "$START" ]; then
    START=1
fi
if [ -z "$STARTPAR" ]; then
    STARTPAR=1
fi

for N in $(seq $START $NUMTESTS); do
    TEST=$(eval echo `echo \$\{TEST${N}E\}`)
    NT=$(eval echo `echo \$\{TEST${N}N\}`)
    for A in $(seq $STARTPAR $((NT))); do
	ARG=$(eval echo `echo \$\{TEST${N}A${A}\}`)
	EXIT=$(eval echo `echo \$\{TEST${N}E${A}\}`)
	EXITALT=$(eval echo `echo \$\{TEST${N}EA${A}\}`)
	MODE=$(eval echo `echo \$\{TEST${N}B${A}\}`)
	if [ -z "$EXIT" ]; then
	    EXIT=0
	fi
	if [ -z "$EXITALT" ]; then
	    EXITALT="none"
	fi
	if [ -z "$MODE" ]; then
	    MODE="normal"
	fi
	make_test "$N" "$A" "$NT" "$EXIT" "$EXITALT" "$MODE" "$TEST" "$ARG"	
    done
    STARTPAR=1
done

echo
echo "====================================="
echo
echo "FINAL REPORT"
echo
echo "tests done         : $((OK+FAILURE+SKIP))"
echo "tests ok           : $OK"
echo "tests bad          : $FAILURE"
echo "tests expected bad : $EXPFAILURE"
echo "tests skipped      : $SKIP"
echo
if [ "$FAILURE" -eq "$EXPFAILURE" ]; then
    echo "suite              : SUCCESS"
    exit 0
else
    echo "suite              : FAILURE"
    exit 1
fi
