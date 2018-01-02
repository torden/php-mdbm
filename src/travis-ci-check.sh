#!/bin/bash
CNT=$(find ./tests/ -name "*.mem" | wc -l)
if [ $CNT -gt 0 ]; then
    echo "FAIL : $CNT";
    cat ./tests/*.mem
    exit 1;
else
    echo "OK";
    exit;
fi
