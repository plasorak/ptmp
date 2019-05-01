#!/bin/bash

source setup_while_running.sh
if [ -z "$1" ]
then
    NSEC=10
else
    NSEC=$1
fi

echo "******************************************************"
echo "THIS SCRIPT IS FOR DUMPING HIT FINDER BR HITS"
echo "You should set the addresses as they are in the config you are running."
dumphit "HITFINDER_BR_501" "tcp://10.73.136.67:25141" $NSEC 
dumphit "HITFINDER_BR_502" "tcp://10.73.136.67:25142" $NSEC
dumphit "HITFINDER_BR_503" "tcp://10.73.136.67:25143" $NSEC
dumphit "HITFINDER_BR_504" "tcp://10.73.136.67:25144" $NSEC
dumphit "HITFINDER_BR_505" "tcp://10.73.136.67:25145" $NSEC
dumphit "HITFINDER_BR_506" "tcp://10.73.136.67:25146" $NSEC
dumphit "HITFINDER_BR_507" "tcp://10.73.136.67:25147" $NSEC
dumphit "HITFINDER_BR_508" "tcp://10.73.136.67:25148" $NSEC
dumphit "HITFINDER_BR_509" "tcp://10.73.136.67:25149" $NSEC
dumphit "HITFINDER_BR_510" "tcp://10.73.136.67:25150" $NSEC
dumphit "HITFINDER_BR_601" "tcp://10.73.136.60:25151" $NSEC
dumphit "HITFINDER_BR_602" "tcp://10.73.136.60:25152" $NSEC
dumphit "HITFINDER_BR_603" "tcp://10.73.136.60:25153" $NSEC
dumphit "HITFINDER_BR_604" "tcp://10.73.136.60:25154" $NSEC
dumphit "HITFINDER_BR_605" "tcp://10.73.136.60:25155" $NSEC
dumphit "HITFINDER_BR_606" "tcp://10.73.136.60:25156" $NSEC
dumphit "HITFINDER_BR_607" "tcp://10.73.136.60:25157" $NSEC
dumphit "HITFINDER_BR_608" "tcp://10.73.136.60:25158" $NSEC
dumphit "HITFINDER_BR_609" "tcp://10.73.136.60:25159" $NSEC
dumphit "HITFINDER_BR_610" "tcp://10.73.136.60:25160" $NSEC
echo "\n\n\n\nDone with all the dumps"
echo "******************************************************"
