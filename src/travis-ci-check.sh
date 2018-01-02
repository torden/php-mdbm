#!/bin/bash
CNT=$(find ./tests/ -name "*.mem" | wc -l)
if [ $CNT -gt 0 ]; then
    echo "FAIL : $CNT";
    ls -la ./tests/
    cat ./tests/*.mem
    cat ./tests/*.log

    gdb `which php` -c ./tests/08lock.mem.core.* -ex "thread apply all bt" -ex "set pagination 0" -batch
    gdb `which php` -c ./tests/14cache.mem.core.* -ex "thread apply all bt" -ex "set pagination 0" -batch
    gdb `which php` -c ./tests/16replace.mem.core.* -ex "thread apply all bt" -ex "set pagination 0" -batch

    exit 1;
else
    echo "OK";
    exit;
fi
