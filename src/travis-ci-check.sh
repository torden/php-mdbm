#!/bin/bash
CNT=$(find ./tests/ -name "*.mem" | wc -l)
if [ $CNT -gt 0 ]; then
    echo "FAIL : $CNT";
    ls -la ./tests/
    cat ./tests/*.mem
    cat ./tests/*.log

gdb -c ./tests/08lock.mem.core.* php -ex "bt" -ex "set pagination 0" -batch
gdb -c ./tests/14cache.mem.core.* php -ex "bt" -ex "set pagination 0" -batch
gdb -c ./tests/16replace.mem.core.* php -ex "bt" -ex "set pagination 0" -batch

    exit 1;
else
    echo "OK";
    exit;
fi
