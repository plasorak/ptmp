#!/bin/bash

source setup_while_running.sh

#felix501_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.67:15141"
#felix502_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.67:15142"
#felix503_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.67:15143"
#felix504_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.67:15144"
#felix505_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.67:15145"
#felix506_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.67:15146"
#felix507_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.67:15147"
#felix508_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.67:15148"
#felix509_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.67:15149"
#felix510_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.67:15150"
#felix601_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.60:15151"
#felix602_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.60:15152"
#felix603_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.60:15153"
#felix604_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.60:15154"
#felix605_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.60:15155"
#felix606_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.60:15156"
#felix607_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.60:15157"
#felix608_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.60:15158"
#felix609_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.60:15159"
#felix610_hw_cfg.fcl:zmq_hit_send_connection: "tcp://10.73.136.60:15160”

function connect {
    ./build/test/check_recv --ntpsets 100 -a connect --socket-endpoints $2 --socket-pattern SUB --timeout-ms 1000 &>/dev/null
    if [ $? -eq 0 ]
    then
        echo "$1 is sending hits!"
    else
        echo "$1 is NOT sending hits!"
    fi
}

echo "******************************************************"
echo "THIS SCRIPT IS TESTING WHICH FELIX BR ARE SENDING HITS"
echo "You should set the addresses as they are in the config you are running."
connect "FELIX BR 501" "tcp://10.73.136.67:15141" 
connect "FELIX BR 502" "tcp://10.73.136.67:15142"
connect "FELIX BR 503" "tcp://10.73.136.67:15143"
connect "FELIX BR 504" "tcp://10.73.136.67:15144"
connect "FELIX BR 505" "tcp://10.73.136.67:15145"
connect "FELIX BR 506" "tcp://10.73.136.67:15146"
connect "FELIX BR 507" "tcp://10.73.136.67:15147"
connect "FELIX BR 508" "tcp://10.73.136.67:15148"
connect "FELIX BR 509" "tcp://10.73.136.67:15149"
connect "FELIX BR 510" "tcp://10.73.136.67:15150"
connect "FELIX BR 601" "tcp://10.73.136.60:15151"
connect "FELIX BR 602" "tcp://10.73.136.60:15152"
connect "FELIX BR 603" "tcp://10.73.136.60:15153"
connect "FELIX BR 604" "tcp://10.73.136.60:15154"
connect "FELIX BR 605" "tcp://10.73.136.60:15155"
connect "FELIX BR 606" "tcp://10.73.136.60:15156"
connect "FELIX BR 607" "tcp://10.73.136.60:15157"
connect "FELIX BR 608" "tcp://10.73.136.60:15158"
connect "FELIX BR 609" "tcp://10.73.136.60:15159"
connect "FELIX BR 610" "tcp://10.73.136.60:15160"
echo "\n\n\n\nDone with all the test"
echo "******************************************************"
