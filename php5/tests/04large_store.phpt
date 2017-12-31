--TEST--
MDBM generate large-object
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS|MDBM_O_ASYNC, 0666, 0,0);

for($i=0;$i<=65535;$i++) {

    $v =  rand(0,65535);
    $rv = mdbm_store($db, $i, $v);
    if($rv == false) {
        FAIL();
    }
}

$rv = mdbm_sync($db);
if ($rv == false) {
    FAIL();
}

$rv = mdbm_close($db);
if ($rv == true) {
    OK();
}

?>
--EXPECT--
OK
