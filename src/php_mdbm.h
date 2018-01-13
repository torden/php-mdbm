/*
  +----------------------------------------------------------------------+
  | PHP Version 5,7                                                      |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2017 The PHP Group                                |
  +----------------------------------------------------------------------+
  | http://www.php.net/license/3_01.txt                                  |
  +----------------------------------------------------------------------+
  | Author: torden <https://github.com/torden/>                          |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_MDBM_H
#define PHP_MDBM_H

extern zend_module_entry mdbm_module_entry;
#define phpext_mdbm_ptr &mdbm_module_entry

#define PHP_MDBM_VERSION "0.0.1" 
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


#define MDBM_LOG_OFF            -1
#define MDBM_LOG_EMERGENCY      LOG_EMERG
#define MDBM_LOG_ALERT          LOG_ALERT
#define MDBM_LOG_CRITICAL       LOG_CRIT
#define MDBM_LOG_ERROR          LOG_ERR
#define MDBM_LOG_WARNING        LOG_WARNING
#define MDBM_LOG_NOTICE         LOG_NOTICE
#define MDBM_LOG_INFO           LOG_INFO
#define MDBM_LOG_DEBUG          LOG_DEBUG
#define MDBM_LOG_DEBUG2         LOG_DEBUG+1
#define MDBM_LOG_DEBUG3         LOG_DEBUG+2
#define MDBM_LOG_MAXLEVEL       LOG_DEBUG+3
#define MDBM_LOG_ABORT          LOG_EMERG
#define MDBM_LOG_FATAL          LOG_ALERT


#ifdef PHP_WIN32
#	define PHP_MDBM_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_MDBM_API __attribute__ ((visibility("default")))
#else
#	define PHP_MDBM_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(mdbm);
PHP_MSHUTDOWN_FUNCTION(mdbm);
PHP_RINIT_FUNCTION(mdbm);
PHP_RSHUTDOWN_FUNCTION(mdbm);
PHP_MINFO_FUNCTION(mdbm);

PHP_FUNCTION(mdbm_log_minlevel);
PHP_FUNCTION(mdbm_open);
PHP_FUNCTION(mdbm_dup_handle);
PHP_FUNCTION(mdbm_close);
PHP_FUNCTION(mdbm_truncate);
PHP_FUNCTION(mdbm_sync);
PHP_FUNCTION(mdbm_fsync);
PHP_FUNCTION(mdbm_replace_db);
PHP_FUNCTION(mdbm_replace_file);

PHP_FUNCTION(mdbm_get_lockmode);

PHP_FUNCTION(mdbm_lock);
PHP_FUNCTION(mdbm_trylock);
PHP_FUNCTION(mdbm_plock);
PHP_FUNCTION(mdbm_tryplock);
PHP_FUNCTION(mdbm_lock_shared);
PHP_FUNCTION(mdbm_trylock_shared);
PHP_FUNCTION(mdbm_lock_smart);
PHP_FUNCTION(mdbm_trylock_smart);
    
PHP_FUNCTION(mdbm_unlock);
PHP_FUNCTION(mdbm_punlock);
PHP_FUNCTION(mdbm_unlock_smart);

PHP_FUNCTION(mdbm_islocked);
PHP_FUNCTION(mdbm_isowned);
PHP_FUNCTION(mdbm_lock_reset);
PHP_FUNCTION(mdbm_delete_lockfiles);

PHP_FUNCTION(mdbm_preload);
PHP_FUNCTION(mdbm_get_errno);
PHP_FUNCTION(mdbm_get_version);
PHP_FUNCTION(mdbm_get_size);
PHP_FUNCTION(mdbm_get_page_size);
PHP_FUNCTION(mdbm_set_hash);
PHP_FUNCTION(mdbm_get_hash);
PHP_FUNCTION(mdbm_get_limit_size);
PHP_FUNCTION(mdbm_compress_tree);

PHP_FUNCTION(mdbm_store);
PHP_FUNCTION(mdbm_fetch);
PHP_FUNCTION(mdbm_delete);
PHP_FUNCTION(mdbm_first);
PHP_FUNCTION(mdbm_next);
PHP_FUNCTION(mdbm_firstkey);
PHP_FUNCTION(mdbm_nextkey);

PHP_FUNCTION(mdbm_first_r);
PHP_FUNCTION(mdbm_next_r);

PHP_FUNCTION(mdbm_count_records);
PHP_FUNCTION(mdbm_count_pages);

PHP_FUNCTION(mdbm_set_cachemode);
PHP_FUNCTION(mdbm_get_cachemode);
PHP_FUNCTION(mdbm_get_cachemode_name);

PHP_FUNCTION(mdbm_check);
PHP_FUNCTION(mdbm_chk_all_page);
PHP_FUNCTION(mdbm_chk_page);


PHP_FUNCTION(mdbm_protect);

PHP_FUNCTION(mdbm_lock_pages);
PHP_FUNCTION(mdbm_unlock_pages);

PHP_FUNCTION(mdbm_get_hash_value);
PHP_FUNCTION(mdbm_get_page);
PHP_FUNCTION(mdbm_get_magic_number);


/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(mdbm)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(mdbm)
*/

#if PHP_VERSION_ID < 70000 // PHP5 

#ifdef ZTS
#define MDBM_G(v) TSRMG(mdbm_globals_id, zend_mdbm_globals *, v)
#else
#define MDBM_G(v) (mdbm_globals.v)
#endif

#else // PHP7

#define MDBM_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(mdbm, v)
#if defined(ZTS) && defined(COMPILE_DL_MDBM)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif

#endif	/* PHP_MDBM_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
