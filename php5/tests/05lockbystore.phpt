--TEST--
MDBM generate data on locking
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php
include("config.inc");

$db = mdbm_open(TEST_MDBM, MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS|MDBM_O_ASYNC, 0666, 0,0);

//INSERT
for($i=0;$i<10;$i++) {

    $rv = mdbm_lock($db);
    if($rv == false) {
        FAIL();
    }

    
    $rv = mdbm_store($db, $i, $i);
    //$rv = mdbm_store($db, $i, $i, MDBM_INSERT); 
    if($rv == false) {
        FAIL();
    }

    $rv = mdbm_unlock($db);
    if($rv == false) {
        FAIL();
    }
}

//UPDATE
for($i=0;$i<10;$i++) {
    $v =  rand(1,65535);

    $rv = mdbm_lock($db);
    if($rv == false) {
        FAIL();
    }

    
    $rv = mdbm_store($db, $i, $v, MDBM_REPLACE);
    if($rv == false) {
        FAIL();
    }

    $rv = mdbm_unlock($db);
    if($rv == false) {
        FAIL();
    }
}


//DUPLICATE
for($i=0;$i<10;$i++) {
    $v =  rand(1,65535);

    $rv = mdbm_lock($db);
    if($rv == false) {
        FAIL();
    }

    
    $rv = mdbm_store($db, $i, $v, MDBM_INSERT_DUP);
    if($rv == false) {
        FAIL();
    }

    $rv = mdbm_unlock($db);
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
