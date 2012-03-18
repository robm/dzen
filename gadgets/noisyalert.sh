#!/bin/sh
#
# (c) 2007 by Robert Manea
#
# A noisy alert for dzen
# 
# Syntax noisyalert.sh [Message] [TIMEOUT in seconds] | dzen2

ALERTMSG=${1:-"Alert"}
ALERTSEC=${2:-10}

RECTW=10
RECTH=10


while [ $ALERTSEC -ne 0 ]; do

	if [ `expr $ALERTSEC % 2` -eq 0 ]; then
		echo "^r(${RECTW}x${RECTH}) $ALERTMSG"
	else
		echo "^ro(${RECTW}x${RECTH}) $ALERTMSG"
	fi

	ALERTSEC=`expr $ALERTSEC - 1`
	sleep 1
done

