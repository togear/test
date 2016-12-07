#!/bin/bash
#make a num of small files
#author

MAXNUM=200000000000
FIRST_DIR=2000
SECOND_DIR=100
THIRD_DIR=100
PWD=`pwd`
TARGET_OBJ="$PWD/5K"

#echo $TARGET_OBJ

#copy link 
function cplink {
 FILENAME=$PWD/$1
 echo "$1 ===================$2============$FILENAME"
 #ln -s $FILE_NAME $TARGET_OBJ
 ln -s $TARGET_OBJ $FILENAME

}

dd if=/dev/zero of=$TARGET_OBJ bs=1024 count=5
echo "Creat Target file $TARGET_OBJ"

i=0
REPEATS=30
echo
echo "And now the fun really begins."
echo
sleep $JUST_A_SECOND # Hey, wait a second!
while [ $i -lt $REPEATS ]
do
    echo "----------FUNCTIONS---------->"
    echo "<------------ARE-------------"
    echo "<------------FUN------------>"
    echo
    cplink $i
    let "i+=1"
done
