# PHP-mdbm

*Unfortunately, Not ready for use. It won't be much longer...*

- MDBM is a super-fast memory-mapped key/value store.
- MDBM is an ndbm work-alike hashed database library based on sdbm which is based on Per-Aake Larson’s Dynamic Hashing algorithms.
- MDBM is a high-performance, memory-mapped hash database similar to the homegrown libhash.
- The records stored in a mdbm database may have keys and values of arbitrary and variable lengths.

[![Build Status](https://travis-ci.org/torden/php-mdbm.svg?branch=master)](https://travis-ci.org/torden/php-mdbm)

## API

### Support APIs

the following is list of support api on now.

|Group|API|
|---|---|
|[File Management](http://yahoo.github.io/mdbm/api/group__FileManagementGroup.html)|mdbm_open , mdbm_close , mdbm_sync , mdbm_fsync , *~~mdbm_close_fd~~ , ~~mdbm_dup_handle~~ , ~~mdbm_replace_db~~ , ~~mdbm_replace_file~~ , ~~mdbm_pre_split~~ , ~~mdbm_fcopy~~ , ~~mdbm_save~~ , ~~mdbm_restore~~*|
|[Configuration](http://yahoo.github.io/mdbm/api/group__ConfigurationGroup.html)|mdbm_get_version , mdbm_get_size , mdbm_get_page_size , mdbm_get_hash , mdbm_get_limit_size , *~~mdbm_stat_header~~ , ~~mdbm_sethash~~ , ~~mdbm_setspillsize~~ , ~~mdbm_get_alignment~~ , ~~mdbm_set_alignment~~ , ~~mdbm_limit_size_v3~~ , ~~mdbm_limit_dir_size~~ , ~~mdbm_get_magic_number~~ , ~~mdbm_set_window_size~~*|
|[Record Access](http://yahoo.github.io/mdbm/api/group__RecordAccessGroup.html)|mdbm_fetch , mdbm_delete , mdbm_store , *~~mdbm_fetch_r~~ , ~~mdbm_fetch_buf~~ , ~~mdbm_fetch_dup_r~~ , ~~mdbm_fetch_str~~ , ~~mdbm_fetch_info~~ , ~~mdbm_delete_r~~ , ~~mdbm_delete_str~~ , ~~mdbm_store_r~~ , ~~mdbm_store_str~~*|
|[Record Iteration](http://yahoo.github.io/mdbm/api/group__RecordIterationGroup.html)|mdbm_first , mdbm_next , mdbm_firstkey , mdbm_nextkey , *~~mdbm_first_r~~ , ~~mdbm_next_r~~ , ~~mdbm_firstkey_r~~ , ~~mdbm_nextkey_r~~ , ~~mdbm_iterate~~*|
|[Locking](http://yahoo.github.io/mdbm/api/group__LockingGroup.html)|mdbm_islocked , mdbm_isowned , mdbm_lock , mdbm_unlock , mdbm_lock_reset , mdbm_delete_lockfiles , mdbm_get_lockmode , *~~mdbm_trylock~~ , ~~mdbm_plock~~ , ~~mdbm_punlock~~ , ~~mdbm_tryplock~~ , ~~mdbm_lock_shared~~ , ~~mdbm_trylock_shared~~ , ~~mdbm_lock_smart~~ , ~~mdbm_trylock_smart~~ , ~~mdbm_unlock_smart~~*|
|[Data Management](http://yahoo.github.io/mdbm/api/group__DataManagementGroup.html)|mdbm_compress_tree , mdbm_truncate , *~~mdbm_prune~~ , ~~mdbm_purge~~ , ~~mdbm_set_cleanfunc~~ , ~~mdbm_clean~~*|
|[Data Integrity](http://yahoo.github.io/mdbm/api/group__DataIntegrityGroup.html)|*~~mdbm_check~~ , ~~mdbm_chk_error~~ , ~~mdbm_chk_page~~ , ~~mdbm_chk_all_page~~ , ~~mdbm_protect~~*|
|[Data Display](http://yahoo.github.io/mdbm/api/group__DataDisplayGroup.html)|*~~mdbm_dump_all_page~~ , ~~mdbm_dump_page~~*|
|[Statistics](http://yahoo.github.io/mdbm/api/group__StatisticsGroup.html)|mdbm_count_records , *~~mdbm_get_stat_counter~~ , ~~mdbm_get_stat_time~~ , ~~mdbm_reset_stat_operations~~ , ~~mdbm_enable_stat_operations~~ , ~~mdbm_set_stat_time_func~~ , ~~mdbm_get_stat_name~~ , ~~mdbm_set_stats_func~~ , ~~mdbm_stat_all_page~~ , ~~mdbm_get_stats~~ , ~~mdbm_get_db_info~~ , ~~mdbm_chunk_iterate~~ , ~~mdbm_get_db_stats~~ , ~~mdbm_get_window_stats~~ , ~~mdbm_count_pages~~*|
|[Cache and Backing Store](http://yahoo.github.io/mdbm/api/group__CacheAndBackingStoreGroup.html)|mdbm_set_cachemode, mdbm_get_cachemode, mdbm_get_cachemode_name, *~~mdbm_set_backingstore~~*|
|[Import and Export](http://yahoo.github.io/mdbm/api/group__ImportExportGroup.html)|*~~mdbm_cdbdump_to_file~~ , ~~mdbm_cdbdump_trailer_and_close~~ , ~~mdbm_cdbdump_add_record~~ , ~~mdbm_dbdump_to_file~~ , ~~mdbm_dbdump_trailer_and_close~~ , ~~mdbm_dbdump_add_record~~ , ~~mdbm_dbdump_export_header~~ , ~~mdbm_dbdump_import_header~~ , ~~mdbm_dbdump_import~~ , ~~mdbm_cdbdump_import~~*|
|[Miscellaneous](http://yahoo.github.io/mdbm/api/group__MiscellaneousGroup.html)|mdbm_preload , mdbm_get_errno , *~~mdbm_get_hash_value~~ , ~~mdbm_get_page~~ , ~~mdbm_lock_pages~~ , ~~mdbm_unlock_pages~~*|

### Deprecated APIs

|*API*|*STATUS*|*COMMENT*|
|---|---|---|
|mdbm_save|DEPRECATED|mdbm_save is only supported for V2 MDBMs.|
|mdbm_restore|DEPRECATED|mdbm_restore is only supported for V2 MDBMs.|
|mdbm_sethash|DEPRECATED|Legacy version of mdbm_set_hash() This function has inconsistent naming, an error return value. It will be removed in a future version.|

### Only a V2 implementation

|*API*|*STATUS*|*COMMENT*|
|---|---|---|
|mdbm_stat_all_page|V3 not supported|There is only a V2 implementation. V3 not currently supported.|
|mdbm_stat_header|V3 not supported|There is only a V2 implementation. V3 not currently supported.|



## Support two compatibility version

### PHP

|*Version*|*Support*|*Test*|*Develop*|
|---|---|---|---|
|5.x|yes|always|doing|
|7.x|yes(as soon)|as soon|as soon|

### MDBM

|*branch or release ver.*|*Support*|*Test*|*Comment*|
|---|---|---|---|
|master|yes|always|
|4.x|yes|always|

## Install

Download from github

```shell
git clone https://github.com/torden/php-mdbm
```

Compile (for PHP5.x)

```shell
cd php-mdbm/php5/
$PHP_INSTALLED_PATH/bin/phpize
./configure --with-mdbm=/usr/local/mdbm/
make
make test
make install
```

## Example

### Creating and populating a database

```php
<?php
$db = mdbm_open("/tmp/test1.mdbm", MDBM_O_RDWR|MDBM_O_CREAT|MDBM_LARGE_OBJECTS|MDBM_O_TRUNC|MDBM_ANY_LOCKS, 0666, 0,0);
if($db === false) {
    echo "Unable to create database";
    exit(2);
}

mdbm_lock($db);

for($i=0;$i<65535;$i++) {

    $v = rand(1,65535);
    //$rv = mdbm_store($db, $i, $v, MDBM_INSERT); same below
    $rv = mdbm_store($db, $i, $v);
    if($rv === false) {
        echo "failed to store";
        break;
    }
}

mdbm_unlock($db);

mdbm_sync(); //optional flush to disk
mdbm_close($db);
?>
```

### Fetching and updating records in-place

```php
<?php
$db = mdbm_open("/tmp/test1.mdbm", MDBM_O_RDWR, 0666, 0,0);
if($db === false) {
    echo "Unable to open database";
    exit(2);
}

for($i=0;$i<65535;$i++) {

    $v = rand(1,65535);
    $val = mdbm_fetch($db, $v);
    if($val === false) {
        echo "failed to store";
        break;
    }

    printf("store : val=%s\n", $val);
}

mdbm_close($db);
?>
```

### Replacing(Updating) value of records in-place

```php
<?php
$db = mdbm_open("/tmp/test1.mdbm", MDBM_O_RDWR, 0666, 0,0);
if($db === false) {
    echo "Unable to open database";
    exit(2);
}

for($i=0;$i<10;$i++) {

    $val = mdbm_fetch($db, $i);
    if($val === false) {
        echo "failed to fetch";
        break;
    }

    $v = rand(1,65535);

    printf("replace : val=%s to %s\n", $val, $v);

    $rv = mdbm_store($db, $v, MDBM_REPLACE);
    if($rv === false) {
        echo "failed to store(replace)";
        break;
    }
}

mdbm_close($db);
?>
```

### Deleting records in-place

```php
<?php
$db = mdbm_open("/tmp/test1.mdbm", MDBM_O_RDWR, 0666, 0,0);
if($db === false) {
    echo "Unable to open database";
    exit(2);
}

for($i=0;$i<10;$i++) {

    $v = rand(1,65535);
    $val = mdbm_fetch($db, $v);
    if($val == false) {
        echo "failed to fetch";
        break;
    }

    printf("delete : val=%s\n", $val);

    $rv = mdbm_delete($db, $v);
    if($rv === false) {
        echo "failed to delete";
        break;
    }
}

mdbm_sync(); //optional flush to disk
mdbm_close($db);
?>
```


### Adding/replacing records

```php
<?php
$db = mdbm_open("/tmp/test1.mdbm", MDBM_O_RDWR, 0666, 0,0);
if($db === false) {
    echo "Unable to open database";
    exit(2);
}

for($i=65536;$i<655360;$i++) {

    mdbm_lock($db);
    $rv = mdbm_store($db, $v, MDBM_REPLACE);
    mdbm_unlock($db);

    if($rv === false) {
        echo "failed to store(replace)";
        break;
    }
}

mdbm_close($db);
?>
```


### Iterating over all records

```php
<?php
$db = mdbm_open("/tmp/test1.mdbm", MDBM_O_RDWR, 0666, 0,0);
if($db === false) {
    echo "Unable to open database";
    exit(2);
}

$kv = mdbm_first($db);
if($kv === false) {
    echo "failed to get a first record";
}

while($kv) {

    print_r($kv);
    $kv = mdbm_next($db);
}

$rv = mdbm_close($db);
if ($rv === false) {
    FAIL();
}
?>
```


### Iterating over all keys

```php
<?php
$db = mdbm_open("/tmp/test1.mdbm", MDBM_O_RDWR, 0666, 0,0);
if($db === false) {
    echo "Unable to open database";
    exit(2);
}

$key = mdbm_firstkey($db);
if($key === false) {
    echo "failed to get a first record";
}

while($key != false) {

    echo "$kv\n";
    $key = mdbm_nextkey($db);
}

$rv = mdbm_close($db);
if ($rv === false) {
    FAIL();
}
?>
```


## Link

- [Yahoo! MDBM](https://github.com/yahoo/mdbm)
- [MDBM::Concept](http://yahoo.github.io/mdbm/guide/concepts.html)
- [MDBM::Build](https://github.com/yahoo/mdbm/blob/master/README.build)
- [MDBM::Document](http://yahoo.github.io/mdbm/)
- [MDBM::FAQ](http://yahoo.github.io/mdbm/guide/faq.html)
- [DBM](https://en.wikipedia.org/wiki/Dbm)
- [MDBM::Macro(const)](http://yahoo.github.io/mdbm/api/mdbm_8h.html)
- [Go-mdbm](https://github.com/torden/go-mdbm)


---
Please feel free. I hope it is helpful for you
