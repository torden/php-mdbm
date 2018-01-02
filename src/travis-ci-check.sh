#!/bin/bash
CNT=$(find ./tests/ -name "*.mem" | wc -l)
if [ $CNT -gt 0 ]; then
    echo "FAIL : $CNT";
    cat ./tests/*.mem
    cat ./tests/*.log

OREFILE=$(find . -maxdepth 1 -name "*core*" | head -n 1)
if [[ -f "$COREFILE" ]]; then 
    gdb -c "$COREFILE" php -ex "bt" -ex "set pagination 0" -batch;
fi


    exit 1;
else
    echo "OK";
    exit;
fi
