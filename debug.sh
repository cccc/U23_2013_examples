#!/bin/sh
while true; do
    make upload-fast 2&>1 > /dev/null
    openocd -f interface/stlink-v2.cfg -f target/stm32f4x_stlink.cfg 2&>1 > /dev/null &
    sleep 2.5
    pid=`pidof openocd`
    if [ -n "$pid" ]
    then
        break
    fi
    echo "Couldn't connect to STM32. Replug USB if this doesn't go away. (Wait a few seconds after replugging)"
done
echo "Connection to STM32 established. You can start debugging now"
echo "Ctrl+C or unplug USB to kill Connection "

function ctrl_c() {
    kill `pidof openocd`
    exit
}

trap ctrl_c INT
while true; do
    sleep 1
    pid=`pidof openocd`
    if [ -z "$pid" ]
    then
        exit
    fi
done

