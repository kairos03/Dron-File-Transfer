#!/bin/bash

if [ $# -le 0 ]
then
    echo Plese input time
    echo Useage : ./capture_video [time]
else

    ID=RACK001 
    time=$((1000*60*$1)) #1 min

    filename=./media/$ID`date +%Y%m%d%H%M`

    raspivid -w 1280 -h 720 -fps 25 -b 2000000 -o $filename.h264 -t $time
    MP4Box -add $filename.h264 $filename.mp4
    rm -f $filename.h264
    
    echo $filename.mp4 done

fi
