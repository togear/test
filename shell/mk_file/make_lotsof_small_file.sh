#!/bin/bash
#make a num of small files
#author:yong.huang

MAXNUM=200000000000
FIRST_DIR=2000
SECOND_DIR=100
THIRD_DIR=100
PWD=`pwd`
TARGET_OBJ="$PWD/5K"

#echo $TARGET_OBJ

#copy link 
function cplink {
 FILENAME=$1
 #echo "$1 ===================$2============$FILENAME"
 #ln -s $FILE_NAME $TARGET_OBJ
 ln -s $TARGET_OBJ $FILENAME

}

function null_function {
:
:
#usleep 10000
usleep 1000
}

dd if=/dev/zero of=$TARGET_OBJ bs=1024 count=5
echo "Creat Target file $TARGET_OBJ"

i=0
j=0
k=0
echo
echo "And now the fun really begins."
echo
#sleep $JUST_A_SECOND # Hey, wait a second!

for ((i=0 ;i< $FIRST_DIR ; i++ ))
do # The comma concatenates operations.
    PERCENTAGE1=$(printf "%d%%" $(($i*100/$FIRST_DIR)))
    echo $PERCENTAGE1 $PERCENTAGE2 $PERCENTAGE3
    for ((j=0;j<$SECOND_DIR; j++))
    do
        PERCENTAGE2=$(printf "%d%%" $(($j*100/$SECOND_DIR)))
        echo $PERCENTAGE1 $PERCENTAGE2 $PERCENTAGE3
        for ((k=0 ; k<$THIRD_DIR ;k++))
        do
            PERCENTAGE3=$(printf "%d%%" $(($k*100/$THIRD_DIR)))
            echo $PERCENTAGE1 $PERCENTAGE2 $PERCENTAGE3
            SMFILE="$i-$j-$k" 
            NAME=$PWD/$SMFILE
#           echo $NAME
#           cplink $NAME
            null_function
        done
    done
done
