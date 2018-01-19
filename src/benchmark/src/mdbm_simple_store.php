<?php
require_once('./vendor/autoload.php');
use Performance\Performance;


if(!extension_loaded('mdbm')) {
    dl('mdbm.' . PHP_SHLIB_SUFFIX);
}

$module = 'mdbm';
$functions = get_extension_funcs($module);


$limit = 100000;
Performance::point("mdbm store(number, number) :: 100,000");

    $db = mdbm_open("/tmp/test_benchmark_1.mdbm", MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC, 0666, 0,0);

    for($i=0; $i < $limit; $i++) {

        $v = rand(0, $limit-1);
        $rv = mdbm_store($db, $i, $v);
        if ($rv === false) {
            Performance::message("Error : mdbm_store($i, $v)");
        }
    }

    mdbm_sync($db);
    mdbm_close($db);

Performance::finish();

Performance::point("mdbm store(string, string) :: 100,000");

    $db = mdbm_open("/tmp/test_benchmark_str_1.mdbm", MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC, 0666, 0,0);

    for($i=0; $i < $limit; $i++) {

        $v = rand(0, $limit-1);
        $rv = mdbm_store($db, "hello world($i)", "hello php-mdbm($v)");
        if ($rv === false) {
            Performance::message("Error : mdbm_store($i, $v)");
        }
    }

    mdbm_sync($db);
    mdbm_close($db);

Performance::finish();


$limit = 1000000;
Performance::point("mdbm store(number, number) :: 1,000,000");

    $db = mdbm_open("/tmp/test_benchmark_2.mdbm", MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC, 0666, 0,0);

    for($i=0; $i < $limit; $i++) {

        $v = rand(0, $limit-1);
        $rv = mdbm_store($db, $i, $v);
        if ($rv === false) {
            Performance::message("Error : mdbm_store($i, $v)");
        }
    }

    mdbm_sync($db);
    mdbm_close($db);

Performance::finish();

Performance::point("mdbm store(string, string) :: 1,000,000");

    $db = mdbm_open("/tmp/test_benchmark_str_2.mdbm", MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC, 0666, 0,0);

    for($i=0; $i < $limit; $i++) {

        $v = rand(0, $limit-1);
        $rv = mdbm_store($db, "hello world($i)", "hello php-mdbm($v)");
        if ($rv === false) {
            Performance::message("Error : mdbm_store($i, $v)");
        }
    }

    mdbm_sync($db);
    mdbm_close($db);

Performance::finish();

$limit = 10000000;
Performance::point("mdbm store(number, number):: 10,000,000");

    $db = mdbm_open("/tmp/test_benchmark_3.mdbm", MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC, 0666, 0,0);

    for($i=0; $i < $limit; $i++) {

        $v = rand(0, $limit-1);
        $rv = mdbm_store($db, $i, $v);
        if ($rv === false) {
            Performance::message("Error : mdbm_store($i, $v)");
        }

    }

    mdbm_sync($db);
    mdbm_close($db);

Performance::finish();

Performance::point("mdbm store(string, string) :: 10,000,000");

    $db = mdbm_open("/tmp/test_benchmark_str_3.mdbm", MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC, 0666, 0,0);

    for($i=0; $i < $limit; $i++) {

        $v = rand(0, $limit-1);
        $rv = mdbm_store($db, "hello world($i)", "hello php-mdbm($v)");
        if ($rv === false) {
            Performance::message("Error : mdbm_store($i, $v)");
        }
    }

    mdbm_sync($db);
    mdbm_close($db);

Performance::finish();


Performance::results();
?>
