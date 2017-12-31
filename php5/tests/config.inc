<?php
define("TEST_MDBM", "/tmp/test_phpt.mdbm");

function OK() { echo "OK\n"; }
function FAIL() { echo "FAIL\n"; print_r(debug_backtrace()); }

function CHECK_FALSE($rv) {
    if ($rv === false) {
        FAIL();
    }
}
?>
