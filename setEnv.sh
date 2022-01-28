#!/bin/bash

SPHENIXADCDIR=/home/cfmcginn/Projects/sPHENIXADC/
SPHENIXADCDIRLIB="$SPHENIXADCDIR"lib

if [[ -d $SPHENIXADCDIR ]]
then
    echo "SPHENIXADCDIR set to '$SPHENIXADCDIR'; if wrong please fix"
    export SPHENIXADCDIR=$SPHENIXADCDIR


    if [[ $LD_LIBRARY_PATH == *"$SPHENIXADCDIRLIB"* ]]
    then
        dummy=0
    else
        export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SPHENIXADCDIRLIB
    fi
else
    echo "SPHENIXADCDIR given, '$SPHENIXADCDIR' not found!!! Please fix"
fi

