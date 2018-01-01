/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_main.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include <zend_exceptions.h>
#include <mdbm.h>
#include <mdbm_log.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>

#include "php_mdbm.h"

ZEND_DECLARE_MODULE_GLOBALS(mdbm)

typedef struct _php_mdbm_open {
    MDBM *pmdbm;
} php_mdbm_open;

static int le_link, loglevel, dev_null;

#define FETCH_RES(mdbm_link_index, id) {\
    if (mdbm_link_index == NULL) {\
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "not found the mdbm resource");\
        RETURN_FALSE;\
    }\
    ZEND_FETCH_RESOURCE(mdbm_link, php_mdbm_open*, &mdbm_link_index, id, "mdbm link", le_link);\
}

#define CHECK_KEYLEN(key_len) {\
    if (key_len > MDBM_KEYLEN_MAX) {\
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "maximum key length exceeded, (%d > %d)", key_len, MDBM_KEYLEN_MAX);\
        RETURN_FALSE;\
    }\
}

#define CHECK_VALLEN(val_len) {\
    if (val_len > MDBM_VALLEN_MAX) {\
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "maximum value length exceeded, (%d > %d)", val_len, MDBM_VALLEN_MAX);\
        RETURN_FALSE;\
    }\
}

#define CAPTURE_START() {\
    dev_null = open("/dev/null", O_WRONLY);\
    if (loglevel == -1) {\
        dup2(dev_null, STDOUT_FILENO);\
        dup2(dev_null, STDERR_FILENO);\
    }\
}

#define CAPTURE_END() {\
    close(dev_null);\
}

static void _close_mdbm_link(zend_rsrc_list_entry *rsrc TSRMLS_DC) {

    php_mdbm_open *link = (php_mdbm_open *)rsrc->ptr;
    void (*handler) (int);

    handler = signal(SIGPIPE, SIG_IGN);
    mdbm_close(link->pmdbm);
    signal(SIGPIPE, handler);
    efree(link);
}

static int php_info_print(const char *str) {
    TSRMLS_FETCH();
    return php_output_write(str, strlen(str) TSRMLS_CC);
}

//FIX : "Warning: String is not zero-terminated" issue aftre ran mdbm_preload
static inline char* copy_strptr(char *dptr, int dsize) {

    char *pretval = NULL;

    TSRMLS_FETCH();

    pretval = (char *) safe_emalloc(sizeof(char *), dsize+2, 0);
    if (pretval == NULL) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Out of memory while allocating memory");
        return NULL;
    }

    memset(pretval, 0x00, dsize+2);

    //copy
    strncpy(pretval, dptr, dsize);
    if (pretval == NULL) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "failed to strncpy");
        return NULL;
    }

    return pretval;
}


ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_log_minlevel, 0, 0, 1)
    ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_open, 0, 0, 3)
    ZEND_ARG_INFO(0, filepath)
    ZEND_ARG_INFO(0, flags)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, psize)
    ZEND_ARG_INFO(0, presize)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_pmdbm, 0, 0, 1)
    ZEND_ARG_INFO(0, pmdbm)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_pmdbm_flags, 0, 0, 2)
    ZEND_ARG_INFO(0, pmdbm)
    ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_lock_key_flags, 0, 0, 3)
    ZEND_ARG_INFO(0, pmdbm)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_store, 0, 0, 3)
    ZEND_ARG_INFO(0, pmdbm)
    ZEND_ARG_INFO(0, pkey)
    ZEND_ARG_INFO(0, pval)
    ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_key, 0, 0, 2)
    ZEND_ARG_INFO(0, pmdbm)
    ZEND_ARG_INFO(0, pkey)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_flag, 0, 0, 1)
    ZEND_ARG_INFO(0, flag)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_level_verbose, 0, 0, 2)
    ZEND_ARG_INFO(0, pmdbm)
    ZEND_ARG_INFO(0, level)
    ZEND_ARG_INFO(0, verbose)
ZEND_END_ARG_INFO()


static const zend_function_entry mdbm_functions[] = {
    PHP_FE(mdbm_log_minlevel,           arginfo_mdbm_log_minlevel)
    PHP_FE(mdbm_open,                   arginfo_mdbm_open)
    PHP_FE(mdbm_close,                  arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_truncate,               arginfo_mdbm_pmdbm)

    PHP_FE(mdbm_sync,                   arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_fsync,                  arginfo_mdbm_pmdbm)

    PHP_FE(mdbm_get_lockmode,           arginfo_mdbm_pmdbm)

    PHP_FE(mdbm_lock,                   arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_trylock,                arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_plock,                  arginfo_mdbm_lock_key_flags)
    PHP_FE(mdbm_tryplock,               arginfo_mdbm_lock_key_flags)
    PHP_FE(mdbm_lock_shared,            arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_trylock_shared,         arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_lock_smart,             arginfo_mdbm_lock_key_flags)
    PHP_FE(mdbm_trylock_smart,          arginfo_mdbm_lock_key_flags)
    

    PHP_FE(mdbm_unlock,                 arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_punlock,                arginfo_mdbm_lock_key_flags)
    PHP_FE(mdbm_unlock_smart,           arginfo_mdbm_lock_key_flags)

    PHP_FE(mdbm_islocked,               arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_isowned,                arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_lock_reset,             arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_delete_lockfiles,       arginfo_mdbm_pmdbm)


    PHP_FE(mdbm_preload,                arginfo_mdbm_pmdbm)

    PHP_FE(mdbm_get_errno,              arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_get_version,            arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_get_size,               arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_get_page_size,          arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_set_hash,               arginfo_mdbm_pmdbm_flags)
    PHP_FE(mdbm_get_hash,               arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_get_limit_size,         arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_compress_tree,          arginfo_mdbm_pmdbm)

    PHP_FE(mdbm_store,                  arginfo_mdbm_store)
    PHP_FE(mdbm_fetch,                  arginfo_mdbm_key)
    PHP_FE(mdbm_delete,                 arginfo_mdbm_key)

    PHP_FE(mdbm_first,                  arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_next,                   arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_firstkey,               arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_nextkey,                arginfo_mdbm_pmdbm)

    PHP_FE(mdbm_count_records,          arginfo_mdbm_pmdbm)

    PHP_FE(mdbm_set_cachemode,          arginfo_mdbm_pmdbm_flags)
    PHP_FE(mdbm_get_cachemode,          arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_get_cachemode_name,     arginfo_mdbm_flag)
    
    PHP_FE(mdbm_check,                  arginfo_mdbm_level_verbose)
    PHP_FE(mdbm_chk_all_page,           arginfo_mdbm_pmdbm)

    PHP_FE(mdbm_protect,                arginfo_mdbm_pmdbm_flags)
    
    PHP_FE(mdbm_lock_pages,             arginfo_mdbm_pmdbm_flags)
    PHP_FE(mdbm_unlock_pages,           arginfo_mdbm_pmdbm_flags)

    PHP_FE_END
};
 
zend_module_entry mdbm_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "mdbm",
    mdbm_functions,
    PHP_MINIT(mdbm),
    PHP_MSHUTDOWN(mdbm),
    PHP_RINIT(mdbm),
    PHP_RSHUTDOWN(mdbm),  
    PHP_MINFO(mdbm),
#if ZEND_MODULE_API_NO >= 20010901
    PHP_MDBM_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_MDBM
ZEND_GET_MODULE(mdbm)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("mdbm.extention_dir",      "/usr/local/mdbm/lib64", PHP_INI_ALL, OnUpdateLong, global_value, zend_mdbm_globals, mdbm_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_mdbm_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_mdbm_init_globals(zend_mdbm_globals *mdbm_globals)
{
    mdbm_globals->global_value = 0;
    mdbm_globals->global_string = NULL;
}
*/
/* }}} */

#define REGISTER_MDBM_CONSTANT(__c) REGISTER_LONG_CONSTANT(#__c, __c, CONST_CS | CONST_PERSISTENT)

ZEND_MODULE_STARTUP_D(mdbm)
{

    //REGISTER_INI_ENTRIES();

    le_link = zend_register_list_destructors_ex(_close_mdbm_link, NULL, "mdbm-link", module_number);


    REGISTER_MDBM_CONSTANT(MDBM_LOG_OFF);
    REGISTER_MDBM_CONSTANT(MDBM_LOG_EMERGENCY);
    REGISTER_MDBM_CONSTANT(MDBM_LOG_ALERT);
    REGISTER_MDBM_CONSTANT(MDBM_LOG_CRITICAL);
    REGISTER_MDBM_CONSTANT(MDBM_LOG_ERROR);
    REGISTER_MDBM_CONSTANT(MDBM_LOG_WARNING);
    REGISTER_MDBM_CONSTANT(MDBM_LOG_NOTICE);
    REGISTER_MDBM_CONSTANT(MDBM_LOG_INFO);
    REGISTER_MDBM_CONSTANT(MDBM_LOG_DEBUG);
    REGISTER_MDBM_CONSTANT(MDBM_LOG_DEBUG2);
    REGISTER_MDBM_CONSTANT(MDBM_LOG_DEBUG3);
    REGISTER_MDBM_CONSTANT(MDBM_LOG_MAXLEVEL);
    REGISTER_MDBM_CONSTANT(MDBM_LOG_ABORT);
    REGISTER_MDBM_CONSTANT(MDBM_LOG_FATAL);


    REGISTER_MDBM_CONSTANT(MDBM_KEYLEN_MAX);
    REGISTER_MDBM_CONSTANT(MDBM_VALLEN_MAX);
    REGISTER_MDBM_CONSTANT(MDBM_LOC_NORMAL);
    REGISTER_MDBM_CONSTANT(MDBM_LOC_ARENA);
    REGISTER_MDBM_CONSTANT(MDBM_O_RDONLY);
    REGISTER_MDBM_CONSTANT(MDBM_O_WRONLY);
    REGISTER_MDBM_CONSTANT(MDBM_O_RDWR);
    REGISTER_MDBM_CONSTANT(MDBM_O_ACCMODE);
    REGISTER_MDBM_CONSTANT(MDBM_O_CREAT);
    REGISTER_MDBM_CONSTANT(MDBM_O_TRUNC);
    REGISTER_MDBM_CONSTANT(MDBM_O_FSYNC);
    REGISTER_MDBM_CONSTANT(MDBM_O_ASYNC);
    REGISTER_MDBM_CONSTANT(MDBM_O_DIRECT);
    REGISTER_MDBM_CONSTANT(MDBM_NO_DIRTY);
    REGISTER_MDBM_CONSTANT(MDBM_SINGLE_ARCH);
    REGISTER_MDBM_CONSTANT(MDBM_OPEN_WINDOWED);
    REGISTER_MDBM_CONSTANT(MDBM_PROTECT);
    REGISTER_MDBM_CONSTANT(MDBM_DBSIZE_MB);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_OPERATIONS);
    REGISTER_MDBM_CONSTANT(MDBM_LARGE_OBJECTS);
    REGISTER_MDBM_CONSTANT(MDBM_PARTITIONED_LOCKS);
    REGISTER_MDBM_CONSTANT(MDBM_RW_LOCKS);
    REGISTER_MDBM_CONSTANT(MDBM_ANY_LOCKS);
    REGISTER_MDBM_CONSTANT(MDBM_CREATE_V3);
    REGISTER_MDBM_CONSTANT(MDBM_OPEN_NOLOCK);
    REGISTER_MDBM_CONSTANT(MDBM_DEMAND_PAGING);
    REGISTER_MDBM_CONSTANT(MDBM_DBSIZE_MB_OLD);
    REGISTER_MDBM_CONSTANT(MDBM_COPY_LOCK_ALL);
    REGISTER_MDBM_CONSTANT(MDBM_SAVE_COMPRESS_TREE);
    REGISTER_MDBM_CONSTANT(MDBM_ALIGN_8_BITS);
    REGISTER_MDBM_CONSTANT(MDBM_ALIGN_16_BITS);
    REGISTER_MDBM_CONSTANT(MDBM_ALIGN_32_BITS);
    REGISTER_MDBM_CONSTANT(MDBM_ALIGN_64_BITS);
    REGISTER_MDBM_CONSTANT(MDBM_MAGIC);
    REGISTER_MDBM_CONSTANT(MDBM_FETCH_FLAG_DIRTY);
    REGISTER_MDBM_CONSTANT(MDBM_INSERT);
    REGISTER_MDBM_CONSTANT(MDBM_REPLACE);
    REGISTER_MDBM_CONSTANT(MDBM_INSERT_DUP);
    REGISTER_MDBM_CONSTANT(MDBM_MODIFY);
    REGISTER_MDBM_CONSTANT(MDBM_STORE_MASK);
    REGISTER_MDBM_CONSTANT(MDBM_RESERVE);
    REGISTER_MDBM_CONSTANT(MDBM_CLEAN);
    REGISTER_MDBM_CONSTANT(MDBM_CACHE_ONLY);
    REGISTER_MDBM_CONSTANT(MDBM_CACHE_REPLACE);
    REGISTER_MDBM_CONSTANT(MDBM_CACHE_MODIFY);
    REGISTER_MDBM_CONSTANT(MDBM_STORE_SUCCESS);
    REGISTER_MDBM_CONSTANT(MDBM_STORE_ENTRY_EXISTS);
    REGISTER_MDBM_CONSTANT(MDBM_ENTRY_DELETED);
    REGISTER_MDBM_CONSTANT(MDBM_ENTRY_LARGE_OBJECT);
    REGISTER_MDBM_CONSTANT(MDBM_ITERATE_ENTRIES);
    REGISTER_MDBM_CONSTANT(MDBM_ITERATE_NOLOCK);
    REGISTER_MDBM_CONSTANT(MDBM_LOCKMODE_UNKNOWN);
    REGISTER_MDBM_CONSTANT(MDBM_CHECK_HEADER);
    REGISTER_MDBM_CONSTANT(MDBM_CHECK_CHUNKS);
    REGISTER_MDBM_CONSTANT(MDBM_CHECK_DIRECTORY);
    REGISTER_MDBM_CONSTANT(MDBM_CHECK_ALL);
    REGISTER_MDBM_CONSTANT(MDBM_PROT_NONE);
    REGISTER_MDBM_CONSTANT(MDBM_PROT_READ);
    REGISTER_MDBM_CONSTANT(MDBM_PROT_WRITE);
    REGISTER_MDBM_CONSTANT(MDBM_PROT_NOACCESS);
    REGISTER_MDBM_CONSTANT(MDBM_PROT_ACCESS);
    REGISTER_MDBM_CONSTANT(MDBM_CLOCK_STANDARD);
    REGISTER_MDBM_CONSTANT(MDBM_CLOCK_TSC);
    REGISTER_MDBM_CONSTANT(MDBM_STATS_BASIC);
    REGISTER_MDBM_CONSTANT(MDBM_STATS_TIMED);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_CB_INC);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_CB_SET);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_CB_ELAPSED);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_CB_TIME);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_FETCH);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_STORE);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_DELETE);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_LOCK);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_FETCH_UNCACHED);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_GETPAGE);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_GETPAGE_UNCACHED);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_CACHE_EVICT);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_CACHE_STORE);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_PAGE_STORE);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_PAGE_DELETE);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_SYNC);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_FETCH_NOT_FOUND);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_FETCH_ERROR);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_STORE_ERROR);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_DELETE_FAILED);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_FETCH_LATENCY);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_STORE_LATENCY);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_DELETE_LATENCY);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_FETCH_TIME);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_STORE_TIME);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_DELETE_TIME);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_FETCH_UNCACHED_LATENCY);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_GETPAGE_LATENCY);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_GETPAGE_UNCACHED_LATENCY);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_CACHE_EVICT_LATENCY);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_CACHE_STORE_LATENCY);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_PAGE_STORE_VALUE);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_PAGE_DELETE_VALUE);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_TAG_SYNC_LATENCY);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_DELETED);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_KEYS);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_VALUES);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_PAGES_ONLY);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_NOLOCK);
    REGISTER_MDBM_CONSTANT(MDBM_STAT_BUCKETS);
    REGISTER_MDBM_CONSTANT(MDBM_CACHEMODE_NONE);
    REGISTER_MDBM_CONSTANT(MDBM_CACHEMODE_LFU);
    REGISTER_MDBM_CONSTANT(MDBM_CACHEMODE_LRU);
    REGISTER_MDBM_CONSTANT(MDBM_CACHEMODE_GDSF);
    REGISTER_MDBM_CONSTANT(MDBM_CACHEMODE_MAX);
    REGISTER_MDBM_CONSTANT(MDBM_CACHEMODE_EVICT_CLEAN_FIRST);
    REGISTER_MDBM_CONSTANT(MDBM_CACHEMODE_BITS);
    REGISTER_MDBM_CONSTANT(MDBM_MINPAGE);
    REGISTER_MDBM_CONSTANT(MDBM_PAGE_ALIGN);
    REGISTER_MDBM_CONSTANT(MDBM_MAXPAGE);
    REGISTER_MDBM_CONSTANT(MDBM_PAGESIZ);
    REGISTER_MDBM_CONSTANT(MDBM_MIN_PSHIFT);
    REGISTER_MDBM_CONSTANT(MDBM_MAX_SHIFT);
    REGISTER_MDBM_CONSTANT(MDBM_HASH_CRC32);
    REGISTER_MDBM_CONSTANT(MDBM_HASH_EJB);
    REGISTER_MDBM_CONSTANT(MDBM_HASH_PHONG);
    REGISTER_MDBM_CONSTANT(MDBM_HASH_OZ);
    REGISTER_MDBM_CONSTANT(MDBM_HASH_TOREK);
    REGISTER_MDBM_CONSTANT(MDBM_HASH_FNV);
    REGISTER_MDBM_CONSTANT(MDBM_HASH_STL);
    REGISTER_MDBM_CONSTANT(MDBM_HASH_MD5);
    REGISTER_MDBM_CONSTANT(MDBM_HASH_SHA_1);
    REGISTER_MDBM_CONSTANT(MDBM_HASH_JENKINS);
    REGISTER_MDBM_CONSTANT(MDBM_HASH_HSIEH);
    REGISTER_MDBM_CONSTANT(MDBM_MAX_HASH);
    REGISTER_MDBM_CONSTANT(MDBM_CONFIG_DEFAULT_HASH);

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(mdbm)
{
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(mdbm)
{
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(mdbm)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(mdbm)
{

    char ver[12] = {0x00,};
    snprintf(ver, sizeof(ver), "%d", MDBM_API_VERSION);

    php_info_print_table_start();
    php_info_print_table_header(2, "mdbm support", "enable");
    php_info_print_table_row(2, "Development", "Torden <https://github.com/torden/php-mdbm>");
    php_info_print_table_row(2, "Version (php-mdbm)", PHP_MDBM_VERSION);
    php_info_print_table_row(2, "Version (mdbm)", ver);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
/* }}} */

PHP_FUNCTION(mdbm_log_minlevel) {

    long flags = 0;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &flags)) {
        RETURN_FALSE;
    }

    loglevel = (int)flags;

    mdbm_log_minlevel((int)flags);
    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_open) {

    unsigned char *pfilepath = NULL;
    int path_len = 0;
    long flags = 0;
    long mode = 0;
    long size = 0;
    long resize = 0;
    MDBM *pmdbm = NULL;
    php_mdbm_open *mdbm_link = NULL;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sll|ll", &pfilepath,&path_len, &flags, &mode, &size, &resize)) {
        RETURN_FALSE;
    }

    //create the link
    mdbm_link = (php_mdbm_open *) safe_emalloc(sizeof(php_mdbm_open), 1, 0);
    if (!mdbm_link) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Out of memory while allocating memory for a MDBM Resource link");
    }

    //disable the log to stderr
    setenv("MDBM_LOG_LEVEL","-1",1);      
    mdbm_log_minlevel(-1);


    //open the mdbm
    pmdbm = mdbm_open(pfilepath, (int)flags, (int)mode, (int)size, (int)resize);
    if (!pmdbm) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "failed to open the mdbm");
        RETURN_FALSE;
    }

    mdbm_link->pmdbm = pmdbm;

    ZEND_REGISTER_RESOURCE(return_value, mdbm_link, le_link);
}


PHP_FUNCTION(mdbm_close) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    if (mdbm_link_index) {
        zend_list_delete(Z_RESVAL_P(mdbm_link_index));
    } else {
        zend_list_delete(id);
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_truncate) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    mdbm_truncate(mdbm_link->pmdbm);
    RETURN_NULL();
}



PHP_FUNCTION(mdbm_sync) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    rv = mdbm_sync(mdbm_link->pmdbm);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_fsync) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    rv = mdbm_fsync(mdbm_link->pmdbm);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_get_lockmode) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    uint32_t rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    rv = mdbm_get_lockmode(mdbm_link->pmdbm);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_lock) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    rv = mdbm_lock(mdbm_link->pmdbm);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_trylock) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    rv = mdbm_trylock(mdbm_link->pmdbm);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_plock) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    char *pkey = NULL;
    long key_len = 0;
    long flags = -1;
    datum datum_key;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsl", &mdbm_link_index, &pkey, &key_len, &flags) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    //make a datum
    datum_key.dptr = pkey;
    datum_key.dsize = key_len;

    rv = mdbm_plock(mdbm_link->pmdbm, &datum_key, (int)flags);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_tryplock) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    char *pkey = NULL;
    long key_len = 0;
    long flags = -1;
    datum datum_key;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsl", &mdbm_link_index, &pkey, &key_len, &flags) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    //make a datum
    datum_key.dptr = pkey;
    datum_key.dsize = key_len;

    rv = mdbm_tryplock(mdbm_link->pmdbm, &datum_key, (int)flags);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}



PHP_FUNCTION(mdbm_lock_shared) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    rv = mdbm_lock_shared(mdbm_link->pmdbm);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_trylock_shared) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    rv = mdbm_trylock_shared(mdbm_link->pmdbm);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_lock_smart) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    char *pkey = NULL;
    long key_len = 0;
    long flags = -1;
    datum datum_key;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsl", &mdbm_link_index, &pkey, &key_len, &flags) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    //make a datum
    datum_key.dptr = pkey;
    datum_key.dsize = key_len;

    rv = mdbm_lock_smart(mdbm_link->pmdbm, &datum_key, (int)flags);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_trylock_smart) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    char *pkey = NULL;
    long key_len = 0;
    long flags = -1;
    datum datum_key;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsl", &mdbm_link_index, &pkey, &key_len, &flags) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    //make a datum
    datum_key.dptr = pkey;
    datum_key.dsize = key_len;

    rv = mdbm_trylock_smart(mdbm_link->pmdbm, &datum_key, (int)flags);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_unlock) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    rv = mdbm_unlock(mdbm_link->pmdbm);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_punlock) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    char *pkey = NULL;
    long key_len = 0;
    long flags = -1;
    datum datum_key;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsl", &mdbm_link_index, &pkey, &key_len, &flags) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    //make a datum
    datum_key.dptr = pkey;
    datum_key.dsize = key_len;

    rv = mdbm_punlock(mdbm_link->pmdbm, &datum_key, (int)flags);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_unlock_smart) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    char *pkey = NULL;
    long key_len = 0;
    long flags = -1;
    datum datum_key;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsl", &mdbm_link_index, &pkey, &key_len, &flags) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    //make a datum
    datum_key.dptr = pkey;
    datum_key.dsize = key_len;

    rv = mdbm_unlock_smart(mdbm_link->pmdbm, &datum_key, (int)flags);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}


PHP_FUNCTION(mdbm_islocked) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    rv = mdbm_islocked(mdbm_link->pmdbm);
    if (rv == 0) {
        RETURN_FALSE;
    } else if (rv == 1) {
        RETURN_TRUE;
    }
 
    php_error_docref(NULL TSRMLS_CC, E_ERROR, "failed to check the return value of mdbm_islocked");
    RETURN_FALSE;
}

PHP_FUNCTION(mdbm_isowned) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    rv = mdbm_isowned(mdbm_link->pmdbm);
    if (rv == 0) {
        RETURN_FALSE;
    } else if (rv == 1) {
        RETURN_TRUE;
    }
 
    php_error_docref(NULL TSRMLS_CC, E_ERROR, "failed to check the return value of mdbm_islocked");
    RETURN_FALSE;
}

PHP_FUNCTION(mdbm_lock_reset) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    char *dbfn = NULL;
    int dbfn_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &dbfn, &dbfn_len) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

    //flags Reserved for future use, and must be 0.
    CAPTURE_START();
        rv = mdbm_lock_reset(dbfn, 0);
    CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_delete_lockfiles) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    char *dbfn = NULL;
    int dbfn_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &dbfn, &dbfn_len) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

    rv = mdbm_delete_lockfiles(dbfn);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_preload) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    rv = mdbm_preload(mdbm_link->pmdbm);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_get_errno) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    rv = mdbm_get_errno(mdbm_link->pmdbm);
    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_get_version) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    uint32_t rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    rv = mdbm_get_version(mdbm_link->pmdbm);
    RETURN_LONG(rv);
}


PHP_FUNCTION(mdbm_get_size) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    uint64_t rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    rv = mdbm_get_size(mdbm_link->pmdbm);
    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_get_page_size) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    rv = mdbm_get_page_size(mdbm_link->pmdbm);
    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_get_hash) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    rv = mdbm_get_hash(mdbm_link->pmdbm);

    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_set_hash) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    long hash = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &mdbm_link_index, &hash) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    if (hash < MDBM_HASH_CRC32 || hash > MDBM_MAX_HASH) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "mdbm_set_hash does not support hash(=%ld)", hash);
        RETURN_FALSE;
    }

    rv = mdbm_set_hash(mdbm_link->pmdbm, (int)hash);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_get_limit_size) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    rv = mdbm_get_limit_size(mdbm_link->pmdbm);

    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_compress_tree) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

    if (mdbm_link_index == NULL) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "not found the mdbm resource");
        RETURN_FALSE;
    }

    //get mdbm link
    ZEND_FETCH_RESOURCE(mdbm_link, php_mdbm_open*, &mdbm_link_index, id, "mdbm link", le_link);

    mdbm_compress_tree(mdbm_link->pmdbm);
    RETURN_NULL();
}

PHP_FUNCTION(mdbm_store) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;

    int rv = -1;
    char *pkey = NULL;
    long key_len = 0;
    char *pval = NULL;
    long val_len = 0;
    long flags = MDBM_INSERT;
    datum key, val;

    char *psetkey = NULL;
    char *psetval = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss|l", &mdbm_link_index, &pkey, &key_len, &pval, &val_len, &flags) == FAILURE) {
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    //check the length of key
    CHECK_KEYLEN(key_len);

    //check the length of value
    CHECK_VALLEN(val_len);


/* FIX: 
mdbm_store_r (db=0x7d44530, key=key@entry=0xffeffb830, val=val@entry=0xffeffb820, flags=1, iter=iter@entry=0x0) at mdbm.c:4867
(esize > maxesize || (db->db_spillsize && MDBM_LOB_ENABLED(db) && vsize >= db->db_spillsize)) {
*/
    psetkey = copy_strptr((char *)pkey, key_len);
    if (psetkey == NULL) {
        RETURN_FALSE;
    }

    psetval = copy_strptr((char *)pval, val_len);
    if (psetkey == NULL) {
        RETURN_FALSE;
    }

    //make a datum
    key.dptr = psetkey;
    key.dsize = (int)key_len;
    val.dptr = psetval;
    val.dsize = (int)val_len;


    rv = mdbm_store(mdbm_link->pmdbm, key, val, (int)flags);

    efree(psetkey);
    efree(psetval);

    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_fetch) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;
    datum key, val;

    char *pkey = NULL;
    long key_len = 0;

    char *pretval = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &mdbm_link_index, &pkey, &key_len) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    //check the length of key
    CHECK_KEYLEN(key_len);

    //make a datum
    key.dptr = pkey;
    key.dsize = (int)key_len;

    val = mdbm_fetch(mdbm_link->pmdbm, key);
    if (val.dptr == NULL) {
        RETURN_FALSE;
    }

    //for fix "Warning: String is not zero-terminated" issue aftre ran mdbm_preload
    pretval = copy_strptr(val.dptr, val.dsize);
    if (pretval == NULL) {
        RETURN_FALSE;
    }

    RETURN_STRINGL(pretval, val.dsize+1, 0);
}

PHP_FUNCTION(mdbm_delete) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;
    datum key;

    char *pkey = NULL;
    long key_len = 0;

    char *pretval = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &mdbm_link_index, &pkey, &key_len) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    //check the length of key
    CHECK_KEYLEN(key_len);

    //make a datum
    key.dptr = pkey;
    key.dsize = (int)key_len;

    rv = mdbm_delete(mdbm_link->pmdbm, key);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}


PHP_FUNCTION(mdbm_first) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    kvpair kv;
    char *pretkey = NULL;
    char *pretval = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    kv = mdbm_first(mdbm_link->pmdbm);
    if (kv.key.dptr == NULL || kv.val.dptr == NULL) {
        RETURN_FALSE;
    }

    pretkey = copy_strptr(kv.key.dptr, kv.key.dsize);
    if (pretkey == NULL) {
        RETURN_FALSE;
    }

    pretval = copy_strptr(kv.val.dptr, kv.val.dsize);
    if (pretval == NULL) {
        RETURN_FALSE;
    }

    array_init(return_value);

    add_assoc_stringl(return_value, "key", pretkey, kv.key.dsize, 0);
    add_assoc_stringl(return_value, "value", pretval, kv.val.dsize, 0);
}

PHP_FUNCTION(mdbm_next) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    kvpair kv;
    char *pretkey = NULL;
    char *pretval = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    kv = mdbm_next(mdbm_link->pmdbm);
    if (kv.key.dptr == NULL || kv.val.dptr == NULL) {
        RETURN_FALSE;
    }

    pretkey = copy_strptr(kv.key.dptr, kv.key.dsize);
    if (pretkey == NULL) {
        RETURN_FALSE;
    }

    pretval = copy_strptr(kv.val.dptr, kv.val.dsize);
    if (pretval == NULL) {
        RETURN_FALSE;
    }

    array_init(return_value);

    add_assoc_stringl(return_value, "key", pretkey, kv.key.dsize, 0);
    add_assoc_stringl(return_value, "value", pretval, kv.val.dsize, 0);
}

PHP_FUNCTION(mdbm_firstkey) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    datum key;
    char *pretkey = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    key = mdbm_firstkey(mdbm_link->pmdbm);
    if (key.dptr == NULL) {
        RETURN_FALSE;
    }

    //for fix "Warning: String is not zero-terminated" issue aftre ran mdbm_preload
    pretkey = copy_strptr(key.dptr, key.dsize);
    if (pretkey == NULL) {
        RETURN_FALSE;
    }

    RETURN_STRINGL(pretkey, key.dsize, 0);
}

PHP_FUNCTION(mdbm_nextkey) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    datum val;
    char *pretval = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

    //fetch the resource
	FETCH_RES(mdbm_link_index, id);

    val = mdbm_nextkey(mdbm_link->pmdbm);
    if (val.dptr == NULL) {
        RETURN_FALSE;
    }

    //for fix "Warning: String is not zero-terminated" issue aftre ran mdbm_preload
    pretval = copy_strptr(val.dptr, val.dsize);
    if (pretval == NULL) {
        RETURN_FALSE;
    }

    RETURN_STRINGL(pretval, val.dsize, 0);
}

PHP_FUNCTION(mdbm_count_records) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    uint64_t rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

    //fetch the resource
    FETCH_RES(mdbm_link_index, id);

    rv = mdbm_count_records(mdbm_link->pmdbm);
    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_set_cachemode) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    long flag = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &mdbm_link_index, &flag) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

    //fetch the resource
    FETCH_RES(mdbm_link_index, id);

    if (flag < MDBM_CACHEMODE_NONE || flag > MDBM_CACHEMODE_MAX) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "mdbm_set_cachemode does not support hash(=%ld)", flag);
        RETURN_FALSE;
    }

    rv = mdbm_set_cachemode(mdbm_link->pmdbm, (int)flag);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_get_cachemode) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

    //fetch the resource
    FETCH_RES(mdbm_link_index, id);

    rv = mdbm_get_cachemode(mdbm_link->pmdbm);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_get_cachemode_name) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    const char *pcache_name = NULL;
    int cacheno = -1;

    char *pretval = NULL;
    int retval_len = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &cacheno) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

    pcache_name = mdbm_get_cachemode_name(cacheno); //return value from stack
    retval_len = (int)strlen(pcache_name);

    pretval = copy_strptr((char *)pcache_name, retval_len);

    RETURN_STRINGL(pretval, retval_len, 0);
}

PHP_FUNCTION(mdbm_check) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    long level = -1;
    zend_bool verbose = FALSE;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl|b", &mdbm_link_index, &level, &verbose) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

    //fetch the resource
    FETCH_RES(mdbm_link_index, id);

    if (level < MDBM_CHECK_HEADER || level > MDBM_CHECK_ALL) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "mdbm_check does not support level(=%ld)", level);
        RETURN_FALSE;
    }

    if (verbose < 0 || verbose > 1 ) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "mdbm_check does not support verbose(=%ld)", verbose);
        RETURN_FALSE;
    }


    rv = mdbm_check(mdbm_link->pmdbm, (int)level, (int)verbose);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_chk_all_page) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    long level = -1;
    long verbose = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

    //fetch the resource
    FETCH_RES(mdbm_link_index, id);

    rv = mdbm_chk_all_page(mdbm_link->pmdbm);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_protect) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    long protect = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &mdbm_link_index, &protect) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

    //fetch the resource
    FETCH_RES(mdbm_link_index, id);

    if (protect < MDBM_PROT_NONE || protect > MDBM_PROT_ACCESS) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "mdbm_protect does not support mdbm_protect(=%ld)", mdbm_protect);
        RETURN_FALSE;
    }

    rv = mdbm_protect(mdbm_link->pmdbm, (int)protect);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_lock_pages) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    rv = mdbm_lock_pages(mdbm_link->pmdbm);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_unlock_pages) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

	//fetch the resource
	FETCH_RES(mdbm_link_index, id);

    rv = mdbm_unlock_pages(mdbm_link->pmdbm);
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
