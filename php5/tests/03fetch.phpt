--TEST--
MDBM fetching data
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");
$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS|MDBM_O_ASYNC, 0666, 0,0);

for($i=0;$i<65535;$i++) {
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
if ($rv == true) {
    OK();
}

$db2 = mdbm_open(TEST_MDBM, MDBM_O_RDONLY, 0666, 0,0);
$rv = mdbm_preload($db2);
if ($rv == false) {
    FAIL();
}

for($i=0;$i<10;$i++) {
    $v =  rand(0,10);
    $rv = mdbm_fetch($db2, $v);
    if($rv == false || $rv != $v) {
        printf("rv=%d, key=%s,val=%s\n", $rv,$i,$v);
        FAIL();
    }
}

$rv = mdbm_close($db2);
if ($rv == true) {
    OK();
}

?>
--EXPECT--
OK
OK
