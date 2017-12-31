--TEST--
MDBM iterate
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS|MDBM_O_ASYNC, 0666, 0,0);

for($i=0;$i<10;$i++) {
    $rv = mdbm_store($db, $i, $i);
    if($rv == false) {
        FAIL();
    }
}

$rv = mdbm_sync($db);
if ($rv == false) {
    FAIL();
}

$rv = mdbm_close($db);
if ($rv == false) {
    FAIL();
}


$db2 = mdbm_open(TEST_MDBM, MDBM_O_RDWR, 0666, 0,0);

$rv = mdbm_delete($db2,1);
if ($rv == false) {
    FAIL();
}

$rv = mdbm_delete($db2,9);
if ($rv == false) {
    FAIL();
}

$val = mdbm_fetch($db2,1);
if ($val != false) {
    FAIL();
}

$val = mdbm_fetch($db2,2);
if ($val == false) {
    FAIL();
}

$val = mdbm_fetch($db2,9);
if ($val != false) {
    FAIL();
}
?>
--EXPECT--
