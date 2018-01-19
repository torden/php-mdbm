<?php
require_once('./vendor/autoload.php');
use Performance\Performance;


if(!extension_loaded('mdbm')) {
	dl('mdbm.' . PHP_SHLIB_SUFFIX);
}

$module = 'mdbm';
$functions = get_extension_funcs($module);


$limit = 100000;
Performance::point("mdbm::preload random fetch(number) :: 100,000");

	$db = mdbm_open("/tmp/test_benchmark_1.mdbm", MDBM_O_RDONLY, 0666, 0,0);
    mdbm_preload($db);

	for($i=0; $i < $limit; $i++) {

		$v = rand(0, $limit-1);
		$rv = mdbm_fetch($db, $v);
        if ($rv === false) {
            Performance::message("Error : mdbm_fetch($v)");
        }
	}

	mdbm_close($db);

Performance::finish();

Performance::point("mdbm::preload random fetch(string) :: 100,000");

	$db = mdbm_open("/tmp/test_benchmark_str_1.mdbm", MDBM_O_RDONLY, 0666, 0,0);
    mdbm_preload($db);

	for($i=0; $i < $limit; $i++) {

		$v = rand(0, $limit-1);
		$rv = mdbm_fetch($db, "hello world($v)");
        if ($rv === false) {
            Performance::message("Error : mdbm_fetch($v)");
        }
	}

	mdbm_close($db);

Performance::finish();


$limit = 1000000;
Performance::point("mdbm::preload random fetch(number) :: 1,000,000");


	$db = mdbm_open("/tmp/test_benchmark_2.mdbm", MDBM_O_RDONLY, 0666, 0,0);
    mdbm_preload($db);

	for($i=0; $i < $limit; $i++) {

		$v = rand(0, $limit-1);
		$rv = mdbm_fetch($db, $v);
        if ($rv === false) {
            Performance::message("Error : mdbm_fetch($v)");
        }
	}

	mdbm_close($db);

Performance::finish();

Performance::point("mdbm::preload random fetch(string) :: 1,000,000");

	$db = mdbm_open("/tmp/test_benchmark_str_2.mdbm", MDBM_O_RDONLY, 0666, 0,0);
    mdbm_preload($db);

	for($i=0; $i < $limit; $i++) {

		$v = rand(0, $limit-1);
		$rv = mdbm_fetch($db, "hello world($i)");
        if ($rv === false) {
            Performance::message("Error : mdbm_fetch($v)");
        }
	}

	mdbm_close($db);

Performance::finish();

$limit = 10000000;
Performance::point("mdbm::preload random fetch(number):: 10,000,000");

	$db = mdbm_open("/tmp/test_benchmark_3.mdbm", MDBM_O_RDONLY, 0666, 0,0);
    mdbm_preload($db);

	for($i=0; $i < $limit; $i++) {

		$v = rand(0, $limit-1);
		$rv = mdbm_fetch($db, $v);
        if ($rv === false) {
            Performance::message("Error : mdbm_fetch($v)");
        }
	}

	mdbm_close($db);

Performance::finish();

Performance::point("mdbm::preload random fetch(number) :: 10,000,000");

	$db = mdbm_open("/tmp/test_benchmark_str_3.mdbm", MDBM_O_RDONLY, 0666, 0,0);
    mdbm_preload($db);

	for($i=0; $i < $limit; $i++) {

		$v = rand(0, $limit-1);
		$rv = mdbm_fetch($db, "hello world($i)");
        if ($rv === false) {
            Performance::message("Error : mdbm_fetch($v)");
        }
	}

	mdbm_close($db);

Performance::finish();


Performance::results();
?>
