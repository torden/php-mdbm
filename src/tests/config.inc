<?php
define("TEST_MDBM", "/tmp/test_phpt.mdbm");
define("TEST_MDBM_REPLACE1", "/tmp/test_phpt_replace1.mdbm");
define("TEST_MDBM_REPLACE2", "/tmp/test_phpt_replace2.mdbm");

function OK() { echo "OK\n"; }
function FAIL() { echo "FAIL\n"; (debug_backtrace()); }

function CHECK_FALSE($rv) {
    if ($rv === false) {
        echo "FAIL\n";
        echo "rv : "; echo (int)$rv; echo"\n";
        var_dump(debug_backtrace());
    }
}

function CHECK_NOT_FALSE($rv) {
    if ($rv !== false) {
        echo "FAIL\n";
        echo "rv : "; echo (int)$rv; echo"\n";
        var_dump(debug_backtrace());
    }
}

function CHECK_EQUALS($src, $match) {
    if ($src !== $match) {
        echo "FAIL : not equals\n";
        echo "src(=$src) !== match(=$match)\n";
        var_dump(debug_backtrace());
    }
}

function CHECK_GREATER_THAN_EQUAL_TO($src, $match) {

    if ($src < $match) {
        echo "FAIL : not equals or not greater than\n";
        echo "src(=$src) >== match(=$match)\n";
        var_dump(debug_backtrace());
    }
}

function CHECK_LESS_THAN_EQUALS_TO($src, $match) {

    if ($src > $match) {
        echo "FAIL : not equals or not greater than\n";
        echo "src(=$src) >== match(=$match)\n";
        var_dump(debug_backtrace());
    }
}


?>
