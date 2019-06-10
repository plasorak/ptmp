#!/bin/bash

source setup_while_running.sh
if [ -z "$1" ]
then
    NSEC=10
else
    NSEC=$1
fi

echo "******************************************************"
echo "THIS SCRIPT IS FOR DUMPING FELIX BR HITS"
echo "You should set the addresses as they are in the config you are running."
dumphit "FELIX_BR_501" "tcp://10.73.136.67:15141" $NSEC &
dumphit "FELIX_BR_502" "tcp://10.73.136.67:15142" $NSEC &
dumphit "FELIX_BR_503" "tcp://10.73.136.67:15143" $NSEC &
dumphit "FELIX_BR_504" "tcp://10.73.136.67:15144" $NSEC &
dumphit "FELIX_BR_505" "tcp://10.73.136.67:15145" $NSEC &
dumphit "FELIX_BR_506" "tcp://10.73.136.67:15146" $NSEC &
dumphit "FELIX_BR_507" "tcp://10.73.136.67:15147" $NSEC &
dumphit "FELIX_BR_508" "tcp://10.73.136.67:15148" $NSEC &
dumphit "FELIX_BR_509" "tcp://10.73.136.67:15149" $NSEC &
dumphit "FELIX_BR_510" "tcp://10.73.136.67:15150" $NSEC &
dumphit "FELIX_BR_601" "tcp://10.73.136.60:15151" $NSEC &
dumphit "FELIX_BR_602" "tcp://10.73.136.60:15152" $NSEC &
dumphit "FELIX_BR_603" "tcp://10.73.136.60:15153" $NSEC &
dumphit "FELIX_BR_604" "tcp://10.73.136.60:15154" $NSEC &
dumphit "FELIX_BR_605" "tcp://10.73.136.60:15155" $NSEC &
dumphit "FELIX_BR_606" "tcp://10.73.136.60:15156" $NSEC &
dumphit "FELIX_BR_607" "tcp://10.73.136.60:15157" $NSEC &
dumphit "FELIX_BR_608" "tcp://10.73.136.60:15158" $NSEC &
dumphit "FELIX_BR_609" "tcp://10.73.136.60:15159" $NSEC &
dumphit "FELIX_BR_610" "tcp://10.73.136.60:15160" $NSEC &
echo "\n\n\n\nDone with all the test"
echo "******************************************************"
