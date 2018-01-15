#!/bin/bash
CNT=$(find ./tests/ -name "*.mem" | wc -l)
if [ $CNT -gt 0 ]; then
    echo "[*] FAIL : $CNT";
    ls -al ./tests/
    echo "[*] mem"
    cat ./tests/*.mem
    echo "[*] log"
    cat ./tests/*.log
    gdb /home/travis/.phpenv/versions/7.0/bin/php tests/08lock.mem.core.* -ex "thread apply all bt" -ex "set pagination 0" -batch 
    exit 1;
else
    echo "OK";
    exit;
fi
