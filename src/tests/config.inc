<?php
define("TEST_MDBM", "/tmp/test_phpt.mdbm");
define("TEST_MDBM_REPLACE1", "/tmp/test_phpt_replace1.mdbm");
define("TEST_MDBM_REPLACE2", "/tmp/test_phpt_replace2.mdbm");

function OK() { echo "OK\n"; }
function FAIL() { echo "FAIL\n"; print_r(debug_backtrace()); }

function CHECK_FALSE($rv) {
    if ($rv === false) {
        FAIL();
    }
}
?>
