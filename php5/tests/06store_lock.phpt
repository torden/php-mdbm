--TEST--
MDBM generate data on locking
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS|MDBM_O_ASYNC, 0666, 0,0);
CHECK_FALSE($db);

$rv = mdbm_lock($db);
CHECK_FALSE($rv);

$rv = mdbm_islocked($db);
CHECK_FALSE($rv);

$rv = mdbm_isowned($db);
CHECK_FALSE($rv);

//INSERT
for($i=0;$i<10;$i++) {

    $rv = mdbm_store($db, $i, $i);
    //$rv = mdbm_store($db, $i, $i, MDBM_INSERT); 
    CHECK_FALSE($rv);
}

//UPDATE
for($i=0;$i<10;$i++) {

    $v =  rand(1,65535);
    $rv = mdbm_store($db, $i, $v, MDBM_REPLACE);
    CHECK_FALSE($rv);
}


//DUPLICATE
for($i=0;$i<10;$i++) {

    $v =  rand(1,65535);
    $rv = mdbm_store($db, $i, $v, MDBM_INSERT_DUP);
    CHECK_FALSE($rv);
}

$rv = mdbm_islocked($db);
CHECK_FALSE($rv);

$rv = mdbm_unlock($db);
CHECK_FALSE($rv);

$rv = mdbm_sync($db);
CHECK_FALSE($rv);

$rv = mdbm_close($db);
CHECK_FALSE($rv);

?>
--EXPECT--
