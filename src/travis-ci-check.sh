#!/bin/bash
CNT=$(find ./tests/ -name "*.mem" | wc -l)
if [ $CNT -gt 0 ]; then
    echo "[*] FAIL : $CNT";
    ls -al ./tests/
    echo "[*] mem"
    cat ./tests/*.mem
    echo "[*] log"
    cat ./tests/*.log
    gdb `which php` tests/08lock.mem.core.* -ex "thread apply all bt" -ex "set pagination 0" -batch --args
    exit 1;
else
    echo "OK";
    exit;
fi
