# PHP-mdbm

*Unfortunately, Not ready for use. It won't be much longer...*

- MDBM is a super-fast memory-mapped key/value store.
- MDBM is an ndbm work-alike hashed database library based on sdbm which is based on Per-Aake Larson’s Dynamic Hashing algorithms.
- MDBM is a high-performance, memory-mapped hash database similar to the homegrown libhash.
- The records stored in a mdbm database may have keys and values of arbitrary and variable lengths.

## API (Support)

the following is list of support api on now.

|*API*|
|---|
|mdbm_log_minlevel|
|mdbm_open|
|mdbm_close|
|mdbm_truncate|
|mdbm_sync|
|mdbm_fsync|
|mdbm_lock|
|mdbm_unlock|
|mdbm_islocked|
|mdbm_isowned|
|mdbm_lock_reset|
|mdbm_delete_lockfiles|
|mdbm_preload|
|mdbm_get_errno|
|mdbm_get_version|
|mdbm_get_size|
|mdbm_get_page_size|
|mdbm_set_hash|
|mdbm_get_hash|
|mdbm_get_limit_size|
|mdbm_store|
|mdbm_fetch|
|mdbm_delete|
|mdbm_first|
|mdbm_next|
|mdbm_firstkey|
|mdbm_nextkey|
|mdbm_count_records|

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

Compile

```shell
cd php-mdbm
$PHP_INSTALLED_PATH/bin/phpize
./configure --with-mdbm=/usr/local/mdbm/
make
make install
```

## Link

- [Yahoo! MDBM](https://github.com/yahoo/mdbm)
- [MDBM::Concept](http://yahoo.github.io/mdbm/guide/concepts.html)
- [MDBM::Build](https://github.com/yahoo/mdbm/blob/master/README.build)
- [MDBM::Document](http://yahoo.github.io/mdbm/)
- [MDBM::FAQ](http://yahoo.github.io/mdbm/guide/faq.html)
- [DBM](https://en.wikipedia.org/wiki/Dbm)
- [Go-mdbm](https://github.com/torden/go-mdbm)


---
Please feel free. I hope it is helpful for you
