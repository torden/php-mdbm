--TEST--
MDBM iterate
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");
$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS, 0666, 0,0);
for($i=0;$i<10;$i++) {
    $v =  rand(1,65535);
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
if ($rv == false) {
    FAIL();
}

$db2 = mdbm_open(TEST_MDBM, MDBM_O_RDWR, 0666, 0,0);
$key = mdbm_firstkey($db2);
while($key != false) {

    echo "$key\n";
    $key = mdbm_nextkey($db2);
}

$rv = mdbm_close($db2);
if ($rv == false) {
    FAIL();
}

?>
--EXPECT--
