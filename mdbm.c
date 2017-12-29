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

#include "php_mdbm.h"

//ZEND_DECLARE_MODULE_GLOBALS(mdbm)

typedef struct _php_mdbm_open {
	MDBM *pmdbm;
} php_mdbm_open;

static int le_link;

static void _close_mdbm_link(zend_rsrc_list_entry *rsrc TSRMLS_DC) {

	php_mdbm_open *link = (php_mdbm_open *)rsrc->ptr;
	void (*handler) (int);

	handler = signal(SIGPIPE, SIG_IGN);
	mdbm_close(link->pmdbm);
	signal(SIGPIPE, handler);
	free(link);
}

static int php_info_print(const char *str) {
    TSRMLS_FETCH();
    return php_output_write(str, strlen(str) TSRMLS_CC);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_open, 0, 0, 3)
    ZEND_ARG_INFO(0, filepath)
    ZEND_ARG_INFO(0, flags)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, psize)
    ZEND_ARG_INFO(0, presize)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_close, 0, 0, 0)
    ZEND_ARG_INFO(0, pmdbm)
ZEND_END_ARG_INFO()

static const zend_function_entry mdbm_functions[] = {
	PHP_FE(mdbm_open,	arginfo_mdbm_open)
	PHP_FE(mdbm_close,	arginfo_mdbm_close)
	PHP_FE_END	/* Must be the last line in mdbm_functions[] */
};
 
zend_module_entry mdbm_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"mdbm",
	mdbm_functions,
	PHP_MINIT(mdbm),
	PHP_MSHUTDOWN(mdbm),
	PHP_RINIT(mdbm),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(mdbm),	/* Replace with NULL if there's nothing to do at request end */
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
    STD_PHP_INI_ENTRY("mdbm.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_mdbm_globals, mdbm_globals)
    STD_PHP_INI_ENTRY("mdbm.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_mdbm_globals, mdbm_globals)
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

	le_link = zend_register_list_destructors_ex(_close_mdbm_link, NULL, "mdbm link", module_number);

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
    REGISTER_MDBM_CONSTANT(MDBM_O_ASYNC);
    REGISTER_MDBM_CONSTANT(MDBM_O_FSYNC);
    REGISTER_MDBM_CONSTANT(MDBM_O_CREAT);
    REGISTER_MDBM_CONSTANT(MDBM_O_TRUNC);
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
    REGISTER_MDBM_CONSTANT(MDBM_LOCKMODE_UNKNOWN);
    REGISTER_MDBM_CONSTANT(MDBM_SAVE_COMPRESS_TREE);
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
    php_info_print_table_row(2, "Development", "torden <https://github.com/torden/php-mdbm>");
    php_info_print_table_row(2, "Version (php-mdbm)", PHP_MDBM_VERSION);
    php_info_print_table_row(2, "Version (mdbm)", ver);
	php_info_print_table_end();
/*
    php_info_print_box_start(0);
    php_info_print("' style='float:none'>");
    php_info_print_box_end();
*/

	DISPLAY_INI_ENTRIES();
}
/* }}} */


PHP_FUNCTION(mdbm_open) {

    unsigned char *pfilepath = NULL;
    int path_len = 0;
    long flags = 0;
    long mode = 0;
    long size = 0;
    long resize = 0;
    MDBM *pmdbm = NULL;
    php_mdbm_open *mdbm_link = NULL;

    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sll|ll", &pfilepath,&path_len, &flags, &mode, &size, &resize)) {
        RETURN_FALSE;
    }

    // create the link
    mdbm_link = (php_mdbm_open *) malloc(sizeof(php_mdbm_open));
    if (!mdbm_link) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Out of memory while allocating memory for a MDBM Resource link");
    }

    // disable the log to stderr
    mdbm_log_minlevel(0);
    setenv("MDBM_LOG_LEVEL","0",1);      

    // open the mdbm
    mdbm_log_minlevel(0);
    pmdbm = mdbm_open(pfilepath, flags, mode, size, resize);
    if (!pmdbm) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "failed to open the mdbm");
        RETURN_FALSE;
    }

    mdbm_link->pmdbm = pmdbm;

    ZEND_REGISTER_RESOURCE(return_value, mdbm_link, le_link);
}


PHP_FUNCTION(mdbm_close) {

    zval *mdbm_link_index = NULL;
    MDBM *pmdbm = NULL;
    int id = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

    if (mdbm_link_index == NULL) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required the mdbm resource");
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(pmdbm, MDBM*, &mdbm_link_index, id, "the mdbm link", le_link);

    if (mdbm_link_index) {
        zend_list_delete(Z_RESVAL_P(mdbm_link_index));
    } else {
        zend_list_delete(id);
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
