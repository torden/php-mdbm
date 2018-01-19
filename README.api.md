# API Document

See the [MDBM API Document](http://yahoo.github.io/mdbm/api/modules.html) for more details

(PHP >= 5.6.x, PHP7)

## Table of Contents

- [Basic](#basic)
	- [mdbm_open](#mdbm_open)
	- [mdbm_close](#mdbm_close)
	- [mdbm_store](#mdbm_store)
	- [mdbm_fetch](#mdbm_fetch)
	- [mdbm_delete](#mdbm_delete)
- [Additional](#additional-api)
 	- [mdbm_get_iter](#mdbm_get_iter)
	- [mdbm_get_global_iter](#mdbm_get_global_iter)
	- [mdbm_reset_global_iter](#mdbm_reset_global_iter)
- [Example](#example)
    - [Creating and populating a database](#creating-and-populating-a-database)
    - [Fetching records in-place](#fetching-records-in-place)
    - [Replacing(Updating) value of records in-place](#replacing(updating)-value-of-records-in-place)
    - [Deleting records in-place](#deleting-records-in-place)
    - [Adding/replacing records](#adding/replacing-records)
    - [Iterating over all records](#iterating-over-all-records)
    - [Iterating over all keys](#iterating-over-all-keys)
- [Constants](#constants)
	- [Global](#global)
	- [For API](#for-api)
- [Link](#link)

## Basic

### mdbm_open

`mdbm_open` - Creates and/or opens a database

#### Description

```php
resource mdbm_open(string $filename, int flags, int $mode [, int psize[, int presize ]]);
```

#### Parameters

- **file** : Name of the backing file for the database.
- **flags** : Specifies the open-mode for the file, usually either (MDBM_O_RDWR|MDBM_O_CREAT) or (MDBM_O_RDONLY).
```
Flag MDBM_LARGE_OBJECTS may be used to enable large object support.
Large object support can only be enabled when the database is first created.
Subsequent mdbm_open calls will ignore the flag.
Flag MDBM_PARTITIONED_LOCKS may be used to enable
partition locking a per mdbm_open basis.
Values for flags mask:
  - MDBM_O_RDONLY          - open for reading only
  - MDBM_O_RDWR            - open for reading and writing
  - MDBM_O_WRONLY          - same as RDWR (deprecated)
  - MDBM_O_CREAT           - create file if it does not exist (requires flag MDBM_O_RDWR)
  - MDBM_O_TRUNC           - truncate size to 0
  - MDBM_O_ASYNC           - enable asynchronous sync'ing by the kernel syncing process.
  - MDBM_O_FSYNC           - sync MDBM upon mdbm_close
  - MDBM_O_DIRECT          - use O_DIRECT when accessing backing-store files
  - MDBM_NO_DIRTY          - do not not track clean/dirty status
  - MDBM_OPEN_WINDOWED     - use windowing to access db, only available with MDBM_O_RDWR
  - MDBM_PROTECT           - protect database except when locked (MDBM V3 only)
  - MDBM_DBSIZE_MB         - dbsize is specific in MB (MDBM V3 only)
  - MDBM_LARGE_OBJECTS     - support large objects
  - MDBM_PARTITIONED_LOCKS - partitioned locks
  - MDBM_RW_LOCKS          - read-write locks
  - MDBM_CREATE_V2         - create a V2 db (obsolete)
  - MDBM_CREATE_V3         - create a V3 db
  - MDBM_HEADER_ONLY       - map header only (internal use)
  - MDBM_OPEN_NOLOCK       - do not lock during open
  - MDBM_ANY_LOCKS         - (V4 only) treat the locking flags as only a suggestion
  - MDBM_SINGLE_ARCH       - (V4 only) user guarantees no mixed (32/64-bit) access in exchange for faster (pthreads) locking
```
- **mode** Used to set the file permissions if the file needs to be created.
- **psize** Specifies the page size for the database and is set when the database is created.
```
The minimum page size is 128.
In v2, the maximum is 64K.
In v3, the maximum is 16M - 64.
The default, if 0 is specified, is 4096.
```
- **presize** Specifies the initial size for the database.
```
The database will dynamically grow as records are added, but specifying an initial size may improve efficiency.
If this is not a multiple of psize, it will be increased to the next psize multiple.
```

#### Return Values

Returns an MDBM Handler, *FALSE* on errors.

#### Examples

```php
$db = mdbm_open("/tmp/test.mdbm", MDBM_O_CREATE | MDBM_O_RDWR, 0644);
```


### mdbm_close

`mdbm_close` - Closes the database.

#### Description

```php
mdbm_close(resource $db);
```

> mdbm_close closes the database specified by the db argument.
> The in-memory pages are not flushed to disk by close.
> They will be written to disk over time as they are paged out,
> but an explicit mdbm_sync call is necessary before closing if on-disk consistency is required.

#### Parameters

**db**
> *The MDBM Handler*

#### Return Values

N/A

#### Examples

```php
$db = mdbm_open("/tmp/test.mdbm", MDBM_O_RDWR, 0644);
mdbm_close($db);
```

### mdbm_store

`mdbm_store` - Stores the record specified by the key and val parameters.

#### Description

```php
bool mdbm_store(resource db, string key, string val, int flags);
```
> This is a wrapper around mdbm_store_r, with a static iterator.

#### Parameters

**db**
> *The MDBM Handler*

**key**
> Stored key

**val**
> Key's value

**flags**
> Store flags
```
Values for flags mask:
  - MDBM_INSERT - Operation will fail if a record with the same key
    already exists in the database.
  - MDBM_REPLACE - A record with the same key will be replaced.
    If the key does not exist, the record will be created.
  - MDBM_INSERT_DUP - allows multiple records with the same key to be inserted.
    Fetching a record with the key will return only one of the duplicate
    records, and which record is returned is not defined.
  - MDBM_MODIFY - Store only if matching entry already exists.
  - MDBM_RESERVE - Reserve space; value not copied (\ref mdbm_store_r only)
  - MDBM_CACHE_ONLY  - Perform store only in the Cache, not in Backing Store.
  - MDBM_CACHE_MODIFY  - Update Cache only if key exists; update the Backing Store
```

#### Return Values

 Returns **TRUE** on success or **FALSE** on failure.

#### Examples

```php
$db = mdbm_open("/tmp/test.mdbm", MDBM_O_CREATE | MDBM_O_RDWR, 0644);
$rv = mdbm_store($db, "ABC", "CBA", MDBM_INSERT);
$rv = mdbm_store($db, "123", "456", MDBM_INSERT_DUP);
$rv = mdbm_store($db, "123", "4567", MDBM_INSERT_DUP);
$rv = mdbm_store($db, "123", "45678", MDBM_INSERT_DUP);
$rv = mdbm_store($db, "ABC", "PHP", MDBM_REPLACE);
mdbm_close($db)
```

### mdbm_fetch

`mdbm_fetch` -  Fetches the record specified by the key argument and returns a string that value of key.


#### Description

```php
string or bool mdbm_fetch(resource db, string key);
```

> Fetches the record specified by the key argument and returns a string that the value of key.
> The contents returned in string has a value in mapped memory.
> If there is any system-wide process that could modify your MDBM, this value must be accessed in a locked context.

#### Parameters

**db**
> *The MDBM Handler*

**key**
> key Lookup key


#### Return Values

Returns an string, *FALSE* on errors.

#### Examples

```php
$db = mdbm_open("/tmp/test.mdbm", MDBM_O_RDONLY, 0644);
$val = mdbm_fetch($db, "ABC");
print_r($val);
mdbm_close($db);
```


### mdbm_delete

`mdbm_delete` - Deletes a specific record.


#### Description

```php
bool mdbm_delete(resource db, string key);
```

#### Parameters

**db**
> *The MDBM Handler*

**key**
> key Lookup key


#### Return Values

Returns an string, *FALSE* on errors.

#### Examples

```php
$db = mdbm_open("/tmp/test.mdbm", MDBM_O_RDONLY, 0644);
$rv = mdbm_delete($db, "ABC");
print_r($rv);
mdbm_close($db);
```

## Additional API

### mdbm_get_iter

`mdbm_get_iter` - Returns an Initialized Iterator

#### Description

```php
array mdbm_get_iter(resource $db);
```

#### Parameters

**db**
> *The MDBM Handler*

#### Return Values

Returns an Initialized Iterator, *FALSE* on errors.

The properties of the array are:

- **___pageno** : fetched last number of page
- **___next** : Index for getnext

#### Examples

```php
$db = mdbm_open("/tmp/test.mdbm", MDBM_O_CREATE | MDBM_O_RDWR, 0644);
$rv = mdbm_store($db, "ABC", "CBA");
$rv = mdbm_store($db, "123", "456");
$rv = mdbm_store($db, "Hello", "PHP");
$iter = mdbm_get_iter($db);
print_r($iter)

$mkv = mdbm_next_r($db, $iter);
print_r($mkv);
```

### mdbm_get_global_iter

`mdbm_get_global_iter` - Returns the Value of Global Iterator

#### Description

```php
array mdbm_get_global_iter(resource $db);
```

#### Parameters

**db**
> *The MDBM Handler*

#### Return Values

Returns the Value of Global Iterator , *FALSE* on errors.

The properties of the array are:

- **___pageno** : fetched last number of page
- **___next** : Index for getnext


#### Examples

```php
$db = mdbm_open("/tmp/test.mdbm", MDBM_O_RDONLY, 0644);

$kv = mdbm_first($db)
while($kv) {
	$iter = mdbm_get_global_iter($db);
	print_r($iter)

	print_r($kv);
	$kv = mdbm_next($db)
}
```

### mdbm_reset_global_iter

`mdbm_reset_global_iter` - Resets the Global Iterator

#### Description

```php
mdbm_reset_global_iter(resource $db);
```

#### Parameters

**db**
> *The MDBM Handler*

#### Return Values

N/A

#### Examples

```php
$db = mdbm_open("/tmp/test.mdbm", MDBM_O_RDONLY, 0644);

$kv = mdbm_first($db)
while($kv) {
	print_r($kv);
	$kv = mdbm_next($db)
}

mdbm_reset_global_iter($db);

$kv = mdbm_next($db)
while($kv) {
	print_r($kv);
	$kv = mdbm_next($db)
}

mdbm_reset_global_iter($db);
$kv = mdbm_next($db);
print_r($kv);
```

## Constants

These constants are defined by the py-mdbm.

### Global

|*Constants*|*Comment*|
|:--|:--|
|PHP_MDBM_VERSION|php-mdbm's version|
|MDBM_API_VERSION|mdbm api version|
|MDBM_KEYLEN_MAX|Maximum key size|
|MDBM_VALLEN_MAX|Maximum key size|
|MDBM_O_RDONLY           |Read-only access |
|MDBM_O_WRONLY           |Write-only access (deprecated in V3) |
|MDBM_O_RDWR             |Read and write access |
|MDBM_O_ACCMODE          |MDBM_O_RDONLY &#124; MDBM_O_WRONLY &#124; MDBM_O_RDW|
|MDBM_O_CREAT            |Create file if it does not exist |
|MDBM_O_TRUNC            |Truncate file |
|MDBM_O_FSYNC            |Sync file on close |
|MDBM_O_ASYNC            |Perform asynchronous writes |
|MDBM_O_DIRECT           |Perform direction I/O |
|MDBM_NO_DIRTY           |Do not not track clean/dirty status |
|MDBM_SINGLE_ARCH        |User *promises* not to mix 32/64-bit access |
|MDBM_OPEN_WINDOWED      |Use windowing to access db |
|MDBM_PROTECT            |Protect database except when locked |
|MDBM_DBSIZE_MB          |Dbsize is specific in MB |
|MDBM_STAT_OPERATIONS    |collect stats for fetch, store, delete |
|MDBM_LARGE_OBJECTS      |Support large objects - obsolete |
|MDBM_PARTITIONED_LOCKS  |Partitioned locks |
|MDBM_RW_LOCKS           |Read-write locks |
|MDBM_ANY_LOCKS          |Open, even if existing locks don't match flags |
|MDBM_OPEN_NOLOCK        |Don't lock during open |
|MDBM_MINPAGE    |Size should be >= header, but this cuts off header stats, which is ok|
|MDBM_MAXPAGE    |Maximum page size is 16MB-64bytes, because any page size above that would be rounded to 16MB, which does not fit in the 24 bits allocated for the in-page offset to an object|
|MDBM_PAGESIZ    |A good default. Size should be >= header|
|MDBM_DEFAULT_HASH|MDBM_HASH_FNV is best|


### For API

|*API*|*Parameter or Return Value*|*Constants*|*Comment*|
|:--|:--|:--|:--|
|mdbm_fcopy|flags|MDBM_COPY_LOCK_ALL|Whether lock for the duration of the copy.For a consistent snapshot of the entire database, this flag must be. Otherwise, consistency will only be on a per-page level.|
|mdbm_get_alignment,<br>mdbm_set_alignment|return value OR align parameter |MDBM_ALIGN_8_BITS       |1-Byte data alignment|
|||MDBM_ALIGN_16_BITS      |2-Byte data alignment|
|||MDBM_ALIGN_32_BITS      |4-Byte data alignment|
|||MDBM_ALIGN_64_BITS      |8-Byte data alignment|
|mdbm_get_magic_number|return value|_MDBM_MAGIC             |V2 file identifier |
|||_MDBM_MAGIC_NEW         |V2 file identifier with large objects|
|||_MDBM_MAGIC_NEW2        |V3 file identifier|
|||MDBM_MAGIC              |=_MDBM_MAGIC_NEW2|
|mdbm_store|flags|MDBM_INSERT     	|Insert if key does not exist; fail if exists |
|||MDBM_REPLACE    	|Update if key exists; insert if does not exist |
|||MDBM_INSERT_DUP 	|Insert new record (creates duplicate if key exists) |
|||MDBM_MODIFY     	|Update if key exists; fail if does not exist |
|||MDBM_STORE_MASK 	|Mask for all store options |
|||MDBM_RESERVE    	|Reserve space; Value not copied |
|||MDBM_CLEAN      	|Mark entry as clean |
|||MDBM_CACHE_ONLY 	|Do not operate on the backing store; use cache only |
|||MDBM_CACHE_REPLACE  |Update cache if key exists; insert if does not exist |
|||MDBM_CACHE_MODIFY   |Update cache if key exists; do not insert if does not |
|mdbm_store<br>mdbm_store_r|flags|MDBM_INSERT     	|Insert if key does not exist; fail if exists |
|||MDBM_REPLACE    	|Update if key exists; insert if does not exist |
|||MDBM_INSERT_DUP 	|Insert new record (creates duplicate if key exists) |
|||MDBM_MODIFY     	|Update if key exists; fail if does not exist |
|||MDBM_STORE_MASK 	|Mask for all store options |
|||MDBM_RESERVE    	|Reserve space; Value not copied |
|||MDBM_CLEAN      	|Mark entry as clean |
|||MDBM_CACHE_ONLY 	|Do not operate on the backing store; use cache only |
|||MDBM_CACHE_REPLACE  |Update cache if key exists; insert if does not exist |
|||MDBM_CACHE_MODIFY   |Update cache if key exists; do not insert if does not |
|mdbm_check|level|MDBM_CHECK_HEADER|Check MDBM header for integrity |
|||MDBM_CHECK_CHUNKS    |Check MDBM header and chunks (page structure) |
|||MDBM_CHECK_DIRECTORY |Check MDBM header, chunks, and directory |
|||MDBM_CHECK_ALL       |Check MDBM header, chunks, directory, and data  |
|mdbm_protect|protect|MDBM_PROT_NONE          |Page no access |
|||MDBM_PROT_READ          |Page read access |
|||MDBM_PROT_WRITE         |Page write access |
|||MDBM_PROT_NOACCESS      |Page no access (=MDBM_PROT_NONE)|
|||MDBM_PROT_ACCESS        |Page protection mask |
|mdbm_enable_stat_operations,mdbm_set_stats_func|falgs|MDBM_STATS_BASIC  |enables gathering only the stats counters.|
|||MDBM_STATS_TIMED  |enables gathering only the stats timestamps.|
|||(MDBM_STATS_BASIC &#124; MDBM_STATS_TIMED) | enables both the stats counters and timestamps.|
|mdbm_set_stat_time_func|flags|MDBM_CLOCK_TSC|Enables use of TSC|
|||MDBM_CLOCK_STANDARD|Disables use of TSC|
|mdbm_set_stats_func|flags|MDBM_STAT_TAG_FETCH             |Successful fetch stats-callback counter |
|||MDBM_STAT_TAG_STORE             |Successful store stats-callback counter |
|||MDBM_STAT_TAG_DELETE            |Successful delete stats-callback counter |
|||MDBM_STAT_TAG_LOCK              |lock stats-callback counter (not implemented) |
|||MDBM_STAT_TAG_FETCH_UNCACHED    |Cache-miss with cache+backingstore |
|||MDBM_STAT_TAG_GETPAGE           |Generic access counter in windowed mode |
|||MDBM_STAT_TAG_GETPAGE_UNCACHED  |Windowed-mode "miss" (load new page into window) |
|||MDBM_STAT_TAG_CACHE_EVICT       |Cache evict stats-callback counter |
|||MDBM_STAT_TAG_CACHE_STORE       |Successful cache store counter (BS only) |
|||MDBM_STAT_TAG_PAGE_STORE        |Successful page-level store indicator |
|||MDBM_STAT_TAG_PAGE_DELETE       |Successful page-level delete indicator |
|||MDBM_STAT_TAG_SYNC              |Counter of mdbm_syncs and fsyncs |
|||MDBM_STAT_TAG_FETCH_NOT_FOUND   |Fetch cannot find a key in MDBM |
|||MDBM_STAT_TAG_FETCH_ERROR       |Error occurred during fetch |
|||MDBM_STAT_TAG_STORE_ERROR       |Error occurred during store (e.g. MODIFY failed) |
|||MDBM_STAT_TAG_DELETE_FAILED     |Delete failed: cannot find a key in MDBM |
|||MDBM_STAT_TAG_FETCH_LATENCY|Fetch latency (expensive to collect)|
|||MDBM_STAT_TAG_STORE_LATENCY|Store latency (expensive to collect)|
|||MDBM_STAT_TAG_DELETE_LATENCY|Delete latency (expensive to collect)|
|||MDBM_STAT_TAG_FETCH_TIME|timestamp of last fetch (not yet implemented)|
|||MDBM_STAT_TAG_STORE_TIME|timestamp of last store (not yet implemented)|
|||MDBM_STAT_TAG_DELETE_TIME|timestamp of last delete (not yet implemented)|
|||MDBM_STAT_TAG_FETCH_UNCACHED_LATENCY|Cache miss latency for cache+Backingstore only (expensive to collect)|
|||MDBM_STAT_TAG_GETPAGE_LATENCY|access latency in windowed mode (expensive to collect)|
|||MDBM_STAT_TAG_GETPAGE_UNCACHED_LATENCY|windowed-mode miss latency (expensive to collect)|
|||MDBM_STAT_TAG_CACHE_EVICT_LATENCY|cache evict latency (expensive to collect)|
|||MDBM_STAT_TAG_CACHE_STORE_LATENCY|Cache store latency in Cache+backingstore mode only (expensive to collect)|
|||MDBM_STAT_TAG_PAGE_STORE_VALUE|Indicates a store occurred on a particular page.  Value returned by callback is the page number. It is up to the callback function to maintain a per-page count||
|||MDBM_STAT_TAG_PAGE_DELETE_VALUE|Indicates a delete occurred on a particular page.  Value returned by callback is the page number.  It is up to the callback function to maintain a per-page count|
|||MDBM_STAT_TAG_SYNC_LATENCY|mdbm_sync/fsync latency (expensive to collect)|
|mdbm_set_cachemode<br>mdbm_get_cachemode| cachemode, return value|MDBM_CACHEMODE_NONE     			|No caching behavior |
|||MDBM_CACHEMODE_LFU      			|Entry with smallest number of accesses is evicted |
|||MDBM_CACHEMODE_LRU      			|Entry with oldest access time is evicted |
|||MDBM_CACHEMODE_GDSF	            |Greedy dual-size frequency (size and frequency) eviction |
|mdbm_set_cachemode| cachemode|MDBM_CACHEMODE_MAX      			|Maximum cache mode value |
|mdbm_set_cachemode| cachemode|MDBM_CACHEMODE_EVICT_CLEAN_FIRST |add to cachemode to evict clean items 1st |
|mdbm_get_hash_value<br>mdbm_set_hash|hashFunctionCode, hashid|MDBM_HASH_CRC32   |Table based 32bit CRC|
|||MDBM_HASH_EJB     |From hsearch|
|||MDBM_HASH_PHONG   |Congruential hash|
|||MDBM_HASH_OZ      |From sdbm|
|||MDBM_HASH_TOREK   |From BerkeleyDB|
|||MDBM_HASH_FNV     |Fowler/Vo/Noll hash (DEFAULT)|
|||MDBM_HASH_STL     |STL string hash|
|||MDBM_HASH_MD5     |MD5|
|||MDBM_HASH_SHA_1   |SHA_1|
|||MDBM_HASH_JENKINS |Jenkins string|
|||MDBM_HASH_HSIEH   |Hsieh SuperFast|
|mdbm_log_minlevel|flag|MDBM_LOG_OFF|Off|
|||MDBM_LOG_EMERGENCY      |EMERG|
|||MDBM_LOG_ALERT          |action must be taken immediately|
|||MDBM_LOG_CRITICAL       |critical conditions|
|||MDBM_LOG_ERROR          |error conditions|
|||MDBM_LOG_WARNING        |warning conditions|
|||MDBM_LOG_NOTICE         |normal but signification condition|
|||MDBM_LOG_INFO           |informational|
|||MDBM_LOG_DEBUG          |debug-level messages|
|||MDBM_LOG_DEBUG2         |DEBUG+1|
|||MDBM_LOG_DEBUG3         |DEBUG+2|
|||MDBM_LOG_MAXLEVEL       |DEBUG+3|
|||MDBM_LOG_ABORT          |=MDBM_LOG_EMERGENCY|
|||MDBM_LOG_FATAL          |=MDBM_LOG_ALERT|



## Link

- [Yahoo! MDBM](https://github.com/yahoo/mdbm)
- [MDBM::Concept](http://yahoo.github.io/mdbm/guide/concepts.html)
- [MDBM::Build](https://github.com/yahoo/mdbm/blob/master/README.build)
- [MDBM::Document](http://yahoo.github.io/mdbm/)
- [MDBM::FAQ](http://yahoo.github.io/mdbm/guide/faq.html)
- [DBM](https://en.wikipedia.org/wiki/Dbm)
- [MDBM::Macro(const)](http://yahoo.github.io/mdbm/api/mdbm_8h.html)
- [Go-mdbm](https://github.com/torden/go-mdbm)
- [Py-mdbm](https://github.com/torden/py-mdbm)

---
*Please feel free. I hope it is helpful for you*

