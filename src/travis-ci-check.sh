#!/bin/bash
CMD_PHP=`which php`
CNT=$(find ./tests/ -name "*.log" -o -name "*.mem" -o -name "*.core*" | wc -l)
if [ $CNT -gt 0 ]; then
    echo "[*] FAIL : $CNT";
    ls -al ./tests/
    echo "[*] mem"
    cat ./tests/*.mem
    echo "[*] log"
    cat ./tests/*.log
    gdb $CMD_PHP tests/*.mem.core.* -ex "thread apply all bt" -ex "set pagination 0" -batch 
    exit 1;
else
    echo "OK";
    exit;
fi
