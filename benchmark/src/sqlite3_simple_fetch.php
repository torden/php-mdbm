<?php
require_once('./vendor/autoload.php');
use Performance\Performance;

define("TEST1", "/tmp/test_benchmark_1.db");
define("TEST_STR1", "/tmp/test_benchmark_str_1.db");

define("TEST2", "/tmp/test_benchmark_2.db");
define("TEST_STR2", "/tmp/test_benchmark_str_2.db");

define("TEST3", "/tmp/test_benchmark_3.db");
define("TEST_STR3", "/tmp/test_benchmark_str_3.db");


$limit = 100000;
Performance::point("sqlite3 random fetch(number) :: 100,000");
{
    $db = new SQLite3(TEST1);
    $db->busyTimeout(5000);
    $db->exec('PRAGMA journal_mode = wal;');
    $db->exec('PRAGMA synchronous=OFF;');
    $stmt = $db->prepare('SELECT var FROM benchmark WHERE key = :key');

    for($i=0; $i < $limit; $i++) {

        $v = rand(0, $limit-1);
        $stmt->bindValue(':key', $v, SQLITE3_INTEGER);
        $result = $stmt->execute();
        if ($result === false) {
            Performance::message("Error : select($v)");
        }
        $row = $result->fetchArray();
        if ($row === false) {
            Performance::message("Error : select($v)");
        }
        $result->finalize(); 
    }
    $db->close();
}
Performance::finish();

Performance::point("sqlite3 random fetch(string) :: 100,000");
{
    $db = new SQLite3(TEST_STR1);
    $db->busyTimeout(5000);
    $db->exec('PRAGMA journal_mode = wal;');
    $db->exec('PRAGMA synchronous=OFF;');
    $stmt = $db->prepare('SELECT var FROM benchmark WHERE key = :key');

    for($i=0; $i < $limit; $i++) {

        $v = rand(0, $limit-1);
        $stmt->bindValue(':key', "hello word($v)", SQLITE3_TEXT);
        $result = $stmt->execute();
        if ($result === false) {
            Performance::message("Error : select($v)");
        }
        $row = $result->fetchArray();
        if ($row === false) {
            Performance::message("Error : select($v)");
        }
        $result->finalize(); 
    }
    $db->close();
}
Performance::finish();

$limit = 1000000;
Performance::point("sqlite3 random fetch(number) :: 1,000,000");
{
    $db = new SQLite3(TEST2);
    $db->busyTimeout(5000);
    $db->exec('PRAGMA journal_mode = wal;');
    $db->exec('PRAGMA synchronous=OFF;');
    $stmt = $db->prepare('SELECT var FROM benchmark WHERE key = :key');

    for($i=0; $i < $limit; $i++) {

        $v = rand(0, $limit-1);
        $stmt->bindValue(':key', $v, SQLITE3_INTEGER);
        $result = $stmt->execute();
        if ($result === false) {
            Performance::message("Error : select($v)");
        }
        $row = $result->fetchArray();
        if ($row === false) {
            Performance::message("Error : select($v)");
        }
        $result->finalize(); 

    }
    $db->close();
}
Performance::finish();

Performance::point("sqlite3 random fetch(string) :: 1,000,000");
{
    $db = new SQLite3(TEST_STR2);
    $db->busyTimeout(5000);
    $db->exec('PRAGMA journal_mode = wal;');
    $db->exec('PRAGMA synchronous=OFF;');
    $stmt = $db->prepare('SELECT var FROM benchmark WHERE key = :key');

    for($i=0; $i < $limit; $i++) {

        $v = rand(0, $limit-1);
        $stmt->bindValue(':key', "hello word($v)", SQLITE3_TEXT);
        $result = $stmt->execute();
        if ($result === false) {
            Performance::message("Error : select($v)");
        }
        $row = $result->fetchArray();
        if ($row === false) {
            Performance::message("Error : select($v)");
        }
        $result->finalize(); 
    }
    $db->close();
}
Performance::finish();

$limit = 10000000;
Performance::point("sqlite3 random fetch(number):: 10,000,000");
{
    $db = new SQLite3(TEST3);
    $db->busyTimeout(5000);
    $db->exec('PRAGMA journal_mode = wal;');
    $db->exec('PRAGMA synchronous=OFF;');
    $stmt = $db->prepare('SELECT var FROM benchmark WHERE key = :key');

    for($i=0; $i < $limit; $i++) {

        $v = rand(0, $limit-1);
        $stmt->bindValue(':key', $v, SQLITE3_INTEGER);
        $result = $stmt->execute();
        if ($result === false) {
            Performance::message("Error : select($v)");
        }
        $row = $result->fetchArray();
        if ($row === false) {
            Performance::message("Error : select($v)");
        }
        $result->finalize(); 
    }
    $db->close();
}
Performance::finish();

Performance::point("sqlite3 random fetch(string) :: 10,000,000");
{
    $db = new SQLite3(TEST_STR3);
    $db->busyTimeout(5000);
    $db->exec('PRAGMA journal_mode = wal;');
    $db->exec('PRAGMA synchronous=OFF;');
    $stmt = $db->prepare('SELECT var FROM benchmark WHERE key = :key');

    for($i=0; $i < $limit; $i++) {

        $v = rand(0, $limit-1);
        $stmt->bindValue(':key', "hello word($v)", SQLITE3_TEXT);
        $result = $stmt->execute();
        if ($result === false) {
            Performance::message("Error : select($v)");
        }
        $row = $result->fetchArray();
        if ($row === false) {
            Performance::message("Error : select($v)");
        }
        $result->finalize(); 
    }
    $db->close();
}
Performance::finish();


Performance::results();
?>
