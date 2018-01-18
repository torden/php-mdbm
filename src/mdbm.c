/*
  +----------------------------------------------------------------------+
  | PHP Version 5,7                                                      |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | http://www.php.net/license/3_01.txt                                  |
  +----------------------------------------------------------------------+
  | Author: torden <https://github.com/torden/>                          |
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
#include "php_version.h"
#include <Zend/zend_API.h>
#include <Zend/zend_hash.h>
#include <zend_exceptions.h>
#include <mdbm.h>
#include <mdbm_log.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>

#include "php_mdbm.h"


#define HASHKEY_KEY                     "key"
#define HASHKEY_VAL                     "value"
#define HASHKEY_PAGENO                  "__pageno"
#define HASHKEY_NEXT                    "__next"
#define HASHKEY_FLAGS                   "flags"
#define HASHKEY_CACHE_NUM_ACCESSES      "cache_num_accesses"
#define HASHKEY_CACHE_ACCESS_TIME       "cache_access_time"

//ZEND_DECLARE_MODULE_GLOBALS(mdbm)

typedef struct _php_mdbm_open {
    MDBM *pmdbm;
    MDBM_ITER iter;
    int enable_stat; //fix : reset_stat_op before enable_stat_op
} php_mdbm_open;

static int le_link, loglevel, dev_null, org_stdout, org_stderr;

#define LE_MDBM_NAME "PHP-MDBM"

#if PHP_VERSION_ID < 70000

typedef long int _ZEND_LONG;
typedef int _ZEND_STR_LEN;

    #define _FETCH_RES(mdbm_link_index, id) {\
        if (mdbm_link_index == NULL) {\
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "not found the mdbm resource");\
            RETURN_FALSE;\
        }\
        ZEND_FETCH_RESOURCE(mdbm_link, php_mdbm_open*, &mdbm_link_index, id, LE_MDBM_NAME, le_link);\
    }

    #define _R_STRINGL(pval, size, duplicate) {\
        RETURN_STRINGL(pval, size, duplicate);\
    }

    #define _ADD_ASSOC_STRINGL(arg, key, str, length, duplicate) {\
        add_assoc_stringl(arg, key, str, length, duplicate); \
    }
#else // PHP7

typedef zend_long _ZEND_LONG;
typedef size_t _ZEND_STR_LEN;

    #define _FETCH_RES(mdbm_link_index, id) {\
        mdbm_link = (php_mdbm_open *)zend_fetch_resource(Z_RES_P(mdbm_link_index), LE_MDBM_NAME, le_link);\
        if (mdbm_link == NULL) {\
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "not found the mdbm resource");\
            RETURN_FALSE;\
        }\
    }

    #define _R_STRINGL(pval, size, duplicate) {\
        RETURN_STRINGL(pval, size);\
    }

    #define _ADD_ASSOC_STRINGL(arg, key, str, length, duplicate) {\
        add_assoc_stringl(arg, key, str, length); \
    }
#endif



#define _CHECK_MDBM_STR_MAXLEN(key_len) {\
    if (key_len > MDBM_KEYLEN_MAX) {\
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "maximum key length exceeded, (%ld > %ld)", (long)key_len, (long)MDBM_KEYLEN_MAX);\
        RETURN_FALSE;\
    }\
}

#define _CHECK_VALLEN(val_len) {\
    if (val_len > MDBM_VALLEN_MAX) {\
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "maximum value length exceeded, (%ld > %ld)", (long)val_len, (long)MDBM_VALLEN_MAX);\
        RETURN_FALSE;\
    }\
}

// several mdbm api use the fprintf
/*
#define _CAPTURE_START() {\
    dev_null = open("/dev/null", O_WRONLY);\
    org_stdout = dup(STDOUT_FILENO);\
    org_stderr = dup(STDERR_FILENO);\
    if (loglevel == -1) {\
        dup2(dev_null, STDOUT_FILENO);\
        dup2(dev_null, STDERR_FILENO);\
    }\
}

#define _CAPTURE_END() {\
    close(dev_null);\
    if (loglevel == -1) {\
        dup2(org_stdout, STDOUT_FILENO);\
        dup2(org_stderr, STDERR_FILENO);\
    }\
}
*/
#define _CAPTURE_START()
#define _CAPTURE_END() 

#define CHECK_OVERFLOW(org, limit_min, limit_max) {\
    if (limit_min == 0 && org < 0) {\
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "The parameter value of `%s` was less than the minimum allowed value of 0", #org);\
        RETURN_FALSE;\
    }\
    if (org > limit_max) {\
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "The parameter value of `%s` was greater than the maxium allowed value of %lld", #org, (long long)limit_max);\
        RETURN_FALSE;\
    }\
    if (org < limit_min) {\
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "The parameter value of `%s` was less than the minimum allowed value of %lld", #org, (long long)limit_min);\
        RETURN_FALSE;\
    }\
}

#define CHECK_EMPTY_STR(val, len) {\
    if (len < 1) {\
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "The parameter value of `%s` was empty", #val);\
        RETURN_FALSE;\
    }\
}

#define CHECK_EMPTY_STR_NAME(name, len) {\
    if (len < 1) {\
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "The parameter value of `%s` was empty", name);\
        RETURN_FALSE;\
    }\
}



#if PHP_VERSION_ID < 70000
static void _close_mdbm_link(zend_rsrc_list_entry *rsrc TSRMLS_DC) {

    php_mdbm_open *link = (php_mdbm_open *)rsrc->ptr;
    if (link == NULL) {
        return;
    }

    if (link->pmdbm == NULL) {
        return;
    }
    void (*handler) (int);
    handler = signal(SIGPIPE, SIG_IGN);
    mdbm_close(link->pmdbm);
    signal(SIGPIPE, handler);
    efree(link);
}

#else

static void _close_mdbm_link(zend_resource *rsrc TSRMLS_DC) {

    php_mdbm_open *link = (php_mdbm_open *)rsrc->ptr;
    if (link == NULL) {
        return;
    }

    if (link->pmdbm != NULL) {
        mdbm_close(link->pmdbm);
        efree(link);
    }
}
#endif

static int php_info_print(const char *str) {
    TSRMLS_FETCH();
    return php_output_write(str, strlen(str) TSRMLS_CC);
}

//FIX : "Warning: String is not zero-terminated" issue aftre ran mdbm_preload
static inline char* copy_strptr(char *dptr, int dsize) {

    char *pretval = NULL;

    TSRMLS_FETCH();

    pretval = ecalloc(dsize+2, sizeof(char));
    if (pretval == NULL) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Out of memory while allocating memory");
        return NULL;
    }

    memset(pretval, 0x00, dsize+1);

    //copy
    strncpy(pretval, dptr, dsize);
    if (pretval == NULL) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "failed to strncpy");
        return NULL;
    }

    return pretval;
}

static inline int iter_handler(php_mdbm_open *mdbm_link, MDBM_ITER **piter,  zval *arr) {

    HashTable *hash_arr = NULL;
#if PHP_VERSION_ID < 70000
    zval **item;
#else
    zval *item = NULL;
#endif
    long in_pageno = 0;
    long in_next = 0;

    //the outside-iter
    if (arr != NULL) {
        TSRMLS_FETCH();
        hash_arr = HASH_OF(arr);
        if ( hash_arr == NULL || zend_hash_num_elements(hash_arr) < 2 ) {
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a not valid parameter: iter");
            return -1;
        }

#if PHP_VERSION_ID < 70000
        if (zend_hash_find(hash_arr, HASHKEY_PAGENO, strlen(HASHKEY_PAGENO) + 1, (void **) &item) == FAILURE) {
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter: iter must have a %s field", HASHKEY_PAGENO);
            return -2;
        } else {
            convert_to_long_ex(item);
            in_pageno = Z_LVAL_PP(item);
        }

        if (zend_hash_find(hash_arr, HASHKEY_NEXT, strlen(HASHKEY_NEXT) + 1, (void **) &item) == FAILURE) {
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter: iter must have a %s field", HASHKEY_NEXT);
            return -2;
        } else {
            convert_to_long_ex(item);
            in_next = Z_LVAL_PP(item);
        }
#else
        item = zend_hash_str_find(hash_arr, HASHKEY_PAGENO, sizeof(HASHKEY_PAGENO)-1);
        if (item != NULL) {
            convert_to_long(item);
            in_pageno = Z_LVAL_P(item);

            if (in_pageno < 0) {
                php_error_docref(NULL TSRMLS_CC, E_ERROR, "The parameter value of `%s` was greater than the maxium allowed value of %lld", HASHKEY_PAGENO, UINT32_MAX);
                return -2;
            }

            if (in_pageno > UINT32_MAX) {
                php_error_docref(NULL TSRMLS_CC, E_ERROR, "The parameter value of `%s` was less than the minimum allowed value of 0", HASHKEY_PAGENO);
                return -2;
            }    

        } else {
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter: iter must have a %s field", HASHKEY_PAGENO);
            return -2;
        }

        item = zend_hash_str_find(hash_arr, HASHKEY_NEXT, sizeof(HASHKEY_NEXT)-1);
        if (item != NULL) {
            convert_to_long(item);
            in_next = Z_LVAL_P(item);

            if (in_next < 0) {
                php_error_docref(NULL TSRMLS_CC, E_ERROR, "The parameter value of `%s` was greater than the maxium allowed value of %ld", HASHKEY_NEXT, INT_MAX);
                return -2;
            }

            if (in_next > INT_MAX) {
                php_error_docref(NULL TSRMLS_CC, E_ERROR, "The parameter value of `%s` was less than the minimum allowed value of %ld", HASHKEY_NEXT, INT_MIN);
                return -2;
            }    

        } else {
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter: iter must have a %s field", HASHKEY_NEXT);
            return -2;
        }
#endif

        (*piter)->m_pageno = (mdbm_ubig_t)in_pageno;
        (*piter)->m_next = (int)in_next;
        return 1;

    } else { //global-iter
        (*piter) = &(*mdbm_link).iter;
        return 2;
    }

    return 0;
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_pmdbm_optional_flags, 0, 0, 2)
    ZEND_ARG_INFO(0, pmdbm)
    ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_pmdbm_pagenum, 0, 0, 2)
    ZEND_ARG_INFO(0, pmdbm)
    ZEND_ARG_INFO(0, pagenum)
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_newfile, 0, 0, 2)
    ZEND_ARG_INFO(0, pmdbm)
    ZEND_ARG_INFO(0, newfile)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_oldfile_newfile, 0, 0, 2)
    ZEND_ARG_INFO(0, oldfile)
    ZEND_ARG_INFO(0, newfile)
ZEND_END_ARG_INFO()
  
ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_pmdbm_iter, 0, 0, 1)
    ZEND_ARG_INFO(0, pmdbm)
    ZEND_ARG_INFO(0, iter)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_pmdbm_n, 0, 0, 2)
    ZEND_ARG_INFO(0, pmdbm)
    ZEND_ARG_INFO(0, pages)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_pageno_next, 0, 0, 0)
    ZEND_ARG_INFO(0, pageno)
    ZEND_ARG_INFO(0, next)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_pmdbm_mode_flags, 0, 0, 3)
    ZEND_ARG_INFO(0, pmdbm)
    ZEND_ARG_INFO(0, newfile)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_pmdbm_align, 0, 0, 2)
    ZEND_ARG_INFO(0, pmdbm)
    ZEND_ARG_INFO(0, align)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_pmdbm_type, 0, 0, 2)
    ZEND_ARG_INFO(0, pmdbm)
    ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_pmdbm_size, 0, 0, 2)
    ZEND_ARG_INFO(0, pmdbm)
    ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_pmdbm_wsize, 0, 0, 2)
    ZEND_ARG_INFO(0, pmdbm)
    ZEND_ARG_INFO(0, wsize)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_pmdbm_pagtenum_flags, 0, 0, 2)
    ZEND_ARG_INFO(0, pmdbm)
    ZEND_ARG_INFO(0, pagenum)
    //ZEND_ARG_INFO(0, flags) //flags Ignored
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdbm_pmdbm_pno, 0, 0, 2)
    ZEND_ARG_INFO(0, pmdbm)
    ZEND_ARG_INFO(0, pno)
ZEND_END_ARG_INFO()

const zend_function_entry mdbm_functions[] = {
    PHP_FE(mdbm_log_minlevel,           arginfo_mdbm_log_minlevel)
    PHP_FE(mdbm_open,                   arginfo_mdbm_open)
    PHP_FE(mdbm_dup_handle,             arginfo_mdbm_pmdbm_optional_flags)
    PHP_FE(mdbm_close,                  arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_close_fd,               arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_truncate,               arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_purge,                  arginfo_mdbm_pmdbm)

    PHP_FE(mdbm_replace_db,             arginfo_mdbm_newfile)
    PHP_FE(mdbm_replace_file,           arginfo_mdbm_oldfile_newfile)
    PHP_FE(mdbm_pre_split,              arginfo_mdbm_pmdbm_n)
    PHP_FE(mdbm_fcopy,                  arginfo_mdbm_pmdbm_mode_flags)

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
    PHP_FE(mdbm_limit_dir_size,         arginfo_mdbm_pmdbm_n)
    PHP_FE(mdbm_get_version,            arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_get_size,               arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_get_page_size,          arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_set_hash,               arginfo_mdbm_pmdbm_flags)
    PHP_FE(mdbm_get_hash,               arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_get_limit_size,         arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_setspillsize,             arginfo_mdbm_pmdbm_size)
    PHP_FE(mdbm_get_alignment,          arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_set_alignment,          arginfo_mdbm_pmdbm_align)
    PHP_FE(mdbm_compress_tree,          arginfo_mdbm_pmdbm)

    PHP_FE(mdbm_store,                  arginfo_mdbm_store)
    PHP_FE(mdbm_store_r,                arginfo_mdbm_store)
    PHP_FE(mdbm_fetch,                  arginfo_mdbm_key)
    PHP_FE(mdbm_fetch_r,                arginfo_mdbm_key)
    PHP_FE(mdbm_fetch_dup_r,            arginfo_mdbm_key)
    PHP_FE(mdbm_fetch_info,             arginfo_mdbm_key)
    PHP_FE(mdbm_delete,                 arginfo_mdbm_key)
    PHP_FE(mdbm_delete_r,               arginfo_mdbm_pmdbm_iter)

    PHP_FE(mdbm_get_global_iter,        arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_get_iter,               arginfo_mdbm_pageno_next)
    PHP_FE(mdbm_reset_global_iter,      arginfo_mdbm_pmdbm)

    PHP_FE(mdbm_first,                  arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_next,                   arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_firstkey,               arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_nextkey,                arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_first_r,                arginfo_mdbm_pmdbm_iter)
    PHP_FE(mdbm_next_r,                 arginfo_mdbm_pmdbm_iter)
    PHP_FE(mdbm_firstkey_r,             arginfo_mdbm_pmdbm_iter)
    PHP_FE(mdbm_nextkey_r,              arginfo_mdbm_pmdbm_iter)

    PHP_FE(mdbm_count_records,          arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_count_pages,            arginfo_mdbm_pmdbm)

    PHP_FE(mdbm_set_cachemode,          arginfo_mdbm_pmdbm_flags)
    PHP_FE(mdbm_get_cachemode,          arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_get_cachemode_name,     arginfo_mdbm_flag)
    
    PHP_FE(mdbm_clean,                  arginfo_mdbm_pmdbm_pagtenum_flags)
    PHP_FE(mdbm_check,                  arginfo_mdbm_level_verbose)
    PHP_FE(mdbm_chk_all_page,           arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_chk_page,               arginfo_mdbm_pmdbm_pagenum)

    PHP_FE(mdbm_protect,                arginfo_mdbm_pmdbm_flags)
    
    PHP_FE(mdbm_lock_pages,             arginfo_mdbm_pmdbm_flags)
    PHP_FE(mdbm_unlock_pages,           arginfo_mdbm_pmdbm_flags)

    PHP_FE(mdbm_get_hash_value,         arginfo_mdbm_pmdbm_flags)
    PHP_FE(mdbm_get_page,               arginfo_mdbm_key)
    PHP_FE(mdbm_get_magic_number,       arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_set_window_size,        arginfo_mdbm_pmdbm_wsize)

    PHP_FE(mdbm_enable_stat_operations, arginfo_mdbm_pmdbm_flags)
    PHP_FE(mdbm_reset_stat_operations,  arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_set_stat_time_func,     arginfo_mdbm_pmdbm_flags)
    PHP_FE(mdbm_get_stat_time,          arginfo_mdbm_pmdbm_type)
    PHP_FE(mdbm_get_stat_counter,       arginfo_mdbm_pmdbm_type)

    PHP_FE(mdbm_stat_all_page,          arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_dump_all_page,          arginfo_mdbm_pmdbm)
    PHP_FE(mdbm_dump_page,              arginfo_mdbm_pmdbm_pno)    

    PHP_FE(mdbm_get_stats,              arginfo_mdbm_pmdbm)
    PHP_FE_END
};
 
zend_module_entry mdbm_module_entry = {
    STANDARD_MODULE_HEADER,
    "mdbm",
    mdbm_functions,
    PHP_MINIT(mdbm),
    PHP_MSHUTDOWN(mdbm),
    PHP_RINIT(mdbm),
    PHP_RSHUTDOWN(mdbm),  
    PHP_MINFO(mdbm),
    PHP_MDBM_VERSION,
    STANDARD_MODULE_PROPERTIES
};
/* }}} */


#if PHP_VERSION_ID < 70000

#ifdef COMPILE_DL_MDBM
ZEND_GET_MODULE(mdbm)
#endif

#else

#ifdef COMPILE_DL_MDBM
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(mdbm)
#endif

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

#define REGISTER_MDBM_LONG_CONSTANT(__c) REGISTER_LONG_CONSTANT(#__c, __c, CONST_CS | CONST_PERSISTENT)
#define REGISTER_MDBM_STRING_CONSTANT(__c) REGISTER_STRING_CONSTANT(#__c, __c, CONST_CS | CONST_PERSISTENT)

ZEND_MODULE_STARTUP_D(mdbm)
{

    //REGISTER_INI_ENTRIES();
    le_link = zend_register_list_destructors_ex(_close_mdbm_link, NULL, LE_MDBM_NAME, module_number);

    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOG_OFF);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOG_EMERGENCY);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOG_ALERT);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOG_CRITICAL);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOG_ERROR);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOG_WARNING);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOG_NOTICE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOG_INFO);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOG_DEBUG);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOG_DEBUG2);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOG_DEBUG3);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOG_MAXLEVEL);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOG_ABORT);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOG_FATAL);


    REGISTER_MDBM_LONG_CONSTANT(MDBM_KEYLEN_MAX);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_VALLEN_MAX);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOC_NORMAL);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOC_ARENA);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_O_RDONLY);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_O_WRONLY);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_O_RDWR);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_O_ACCMODE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_O_CREAT);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_O_TRUNC);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_O_FSYNC);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_O_ASYNC);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_O_DIRECT);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_NO_DIRTY);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_SINGLE_ARCH);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_OPEN_WINDOWED);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_PROTECT);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_DBSIZE_MB);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_OPERATIONS);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_LARGE_OBJECTS);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_PARTITIONED_LOCKS);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_RW_LOCKS);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_ANY_LOCKS);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_CREATE_V3);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_OPEN_NOLOCK);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_DEMAND_PAGING);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_DBSIZE_MB_OLD);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_COPY_LOCK_ALL);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_SAVE_COMPRESS_TREE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_ALIGN_8_BITS);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_ALIGN_16_BITS);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_ALIGN_32_BITS);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_ALIGN_64_BITS);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_MAGIC);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_FETCH_FLAG_DIRTY);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_INSERT);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_REPLACE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_INSERT_DUP);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_MODIFY);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STORE_MASK);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_RESERVE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_CLEAN);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_CACHE_ONLY);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_CACHE_REPLACE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_CACHE_MODIFY);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STORE_SUCCESS);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STORE_ENTRY_EXISTS);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_ENTRY_DELETED);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_ENTRY_LARGE_OBJECT);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_ITERATE_ENTRIES);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_ITERATE_NOLOCK);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOCKMODE_UNKNOWN);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_CHECK_HEADER);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_CHECK_CHUNKS);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_CHECK_DIRECTORY);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_CHECK_ALL);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_PROT_NONE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_PROT_READ);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_PROT_WRITE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_PROT_NOACCESS);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_PROT_ACCESS);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_CLOCK_STANDARD);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_CLOCK_TSC);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STATS_BASIC);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STATS_TIMED);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_CB_INC);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_CB_SET);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_CB_ELAPSED);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_CB_TIME);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_FETCH);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_STORE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_DELETE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_LOCK);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_FETCH_UNCACHED);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_GETPAGE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_GETPAGE_UNCACHED);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_CACHE_EVICT);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_CACHE_STORE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_PAGE_STORE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_PAGE_DELETE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_SYNC);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_FETCH_NOT_FOUND);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_FETCH_ERROR);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_STORE_ERROR);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_DELETE_FAILED);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_FETCH_LATENCY);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_STORE_LATENCY);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_DELETE_LATENCY);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_FETCH_TIME);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_STORE_TIME);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_DELETE_TIME);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_FETCH_UNCACHED_LATENCY);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_GETPAGE_LATENCY);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_GETPAGE_UNCACHED_LATENCY);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_CACHE_EVICT_LATENCY);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_CACHE_STORE_LATENCY);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_PAGE_STORE_VALUE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_PAGE_DELETE_VALUE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TAG_SYNC_LATENCY);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_DELETED);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_KEYS);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_VALUES);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_PAGES_ONLY);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_NOLOCK);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_BUCKETS);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_CACHEMODE_NONE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_CACHEMODE_LFU);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_CACHEMODE_LRU);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_CACHEMODE_GDSF);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_CACHEMODE_MAX);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_CACHEMODE_EVICT_CLEAN_FIRST);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_CACHEMODE_BITS);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_MINPAGE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_PAGE_ALIGN);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_MAXPAGE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_PAGESIZ);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_MIN_PSHIFT);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_MAX_SHIFT);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_HASH_CRC32);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_HASH_EJB);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_HASH_PHONG);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_HASH_OZ);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_HASH_TOREK);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_HASH_FNV);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_HASH_STL);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_HASH_MD5);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_HASH_SHA_1);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_HASH_JENKINS);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_HASH_HSIEH);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_MAX_HASH);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_CONFIG_DEFAULT_HASH);

    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TYPE_FETCH);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TYPE_STORE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TYPE_DELETE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_STAT_TYPE_MAX);

    REGISTER_MDBM_LONG_CONSTANT(MDBM_PTYPE_FREE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_PTYPE_DATA);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_PTYPE_DIR);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_PTYPE_LOB);

    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOG_TO_STDERR);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOG_TO_FILE);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_LOG_TO_SYSLOG);

    REGISTER_MDBM_STRING_CONSTANT(HASHKEY_FLAGS);
    REGISTER_MDBM_STRING_CONSTANT(HASHKEY_CACHE_NUM_ACCESSES);
    REGISTER_MDBM_STRING_CONSTANT(HASHKEY_CACHE_ACCESS_TIME);

    REGISTER_MDBM_STRING_CONSTANT(PHP_MDBM_VERSION);
    REGISTER_MDBM_LONG_CONSTANT(MDBM_API_VERSION);

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

#if PHP_VERSION_ID >= 70000
    #if defined(COMPILE_DL_MDBM) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
    #endif
#endif
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

    _ZEND_LONG flag = 0;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &flag)) {
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(flag, SHRT_MIN, SHRT_MAX);

    mdbm_log_minlevel((int)flag);
    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_open) {

    unsigned char *pfilepath = NULL;
    _ZEND_STR_LEN path_len = 0;
    _ZEND_LONG flags = 0;
    _ZEND_LONG mode = 0;
    _ZEND_LONG psize = 0;
    _ZEND_LONG presize = 0;
    MDBM *pmdbm = NULL;
    php_mdbm_open *mdbm_link = NULL;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sll|ll", &pfilepath,&path_len, &flags, &mode, &psize, &presize)) {
        RETURN_FALSE;
    }

    //protect : sigfault
    if (flags == (flags | MDBM_O_CREAT) && flags == (flags | MDBM_PROTECT)) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "failed to open the MDBM, not support create flags with MDBM_PROTECT");
        RETURN_FALSE;
    }

    if (flags == (flags | MDBM_O_ASYNC) && flags == (flags | MDBM_O_FSYNC)) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "failed to open the MDBM, not support mixed sync flags (MDBM_O_FSYNC, MDBM_O_ASYNC)");
        RETURN_FALSE;
    }

    if (flags == (flags | MDBM_O_RDONLY) && flags == (flags | MDBM_O_WRONLY)) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "failed to open the MDBM, not support mixed access flags (MDBM_O_RDONLY, MDBM_O_WRONLY, MDBM_O_RDWR)");
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(flags, INT_MIN, INT_MAX);
    CHECK_OVERFLOW(mode, INT_MIN, INT_MAX);
    CHECK_OVERFLOW(psize, INT_MIN, INT_MAX);
    CHECK_OVERFLOW(presize, INT_MIN, INT_MAX);

    //create the link
    mdbm_link = (php_mdbm_open *) ecalloc(1, sizeof(php_mdbm_open));
    if (!mdbm_link) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Out of memory while allocating memory for a MDBM Resource link");
        RETURN_FALSE;
    }

    //disable the log to stderr
    setenv("MDBM_LOG_LEVEL","-1",1);      
    mdbm_log_minlevel(-1);

    //open the mdbm
    _CAPTURE_START();
    pmdbm = mdbm_open(pfilepath, (int)flags, (int)mode, (int)psize, (int)presize);
    _CAPTURE_END();
    if (!pmdbm) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "failed to open the mdbm");
        RETURN_FALSE;
    }

    mdbm_link->pmdbm = pmdbm;

    MDBM_ITER_INIT(&(*mdbm_link).iter);
    mdbm_link->enable_stat = 0; 
    
#if PHP_VERSION_ID < 70000
    ZEND_REGISTER_RESOURCE(return_value, mdbm_link, le_link);
#else
    RETURN_RES(zend_register_resource(mdbm_link, le_link));
#endif
}

PHP_FUNCTION(mdbm_dup_handle) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    php_mdbm_open *mdbm_new_link = NULL;
    int id = -1;
    int rv = -1;

    MDBM *pnew_mdbm = NULL;
    _ZEND_LONG flags = 0; // flags Reserved for future use

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    pnew_mdbm = mdbm_dup_handle(mdbm_link->pmdbm, flags); //flags Reserved for future use
    _CAPTURE_END();
    if (pnew_mdbm == NULL) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "failed to duplicate an existing database handle");
        RETURN_FALSE;
    }

    //create the link
    mdbm_new_link = (php_mdbm_open *) safe_emalloc(sizeof(php_mdbm_open), 1, 0);
    if (!mdbm_link) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Out of memory while allocating memory for a MDBM Resource link");
    }


    mdbm_new_link->pmdbm = pnew_mdbm;
    MDBM_ITER_INIT(&(*mdbm_new_link).iter);

#if PHP_VERSION_ID < 70000
    ZEND_REGISTER_RESOURCE(return_value, mdbm_new_link, le_link);
#else
    RETURN_RES(zend_register_resource(mdbm_new_link, le_link));
#endif
}

PHP_FUNCTION(mdbm_close) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;

    int id = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    if (mdbm_link_index) {

#if PHP_VERSION_ID < 70000
        zend_list_delete(Z_RESVAL_P(mdbm_link_index));
    } else {
        zend_list_delete(id);
#else
        zend_list_close(Z_RES_P(mdbm_link_index));
#endif
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_close_fd) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;

    int id = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    mdbm_close_fd(mdbm_link->pmdbm);

    if (mdbm_link_index) {

#if PHP_VERSION_ID < 70000
        zend_list_delete(Z_RESVAL_P(mdbm_link_index));
    } else {
        zend_list_delete(id);
#else
        zend_list_close(Z_RES_P(mdbm_link_index));
#endif
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_truncate) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    mdbm_truncate(mdbm_link->pmdbm);
    RETURN_NULL();
}

PHP_FUNCTION(mdbm_purge) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    mdbm_purge(mdbm_link->pmdbm);
    RETURN_NULL();
}

PHP_FUNCTION(mdbm_replace_db) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    char *pnewfile = NULL;
    _ZEND_STR_LEN newfile_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &mdbm_link_index, &pnewfile, &newfile_len) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("file", newfile_len);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_replace_db(mdbm_link->pmdbm, pnewfile);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_replace_file) {

    int rv = -1;

    char *poldfile = NULL;
    _ZEND_STR_LEN oldfile_len = 0;

    char *pnewfile = NULL;
    _ZEND_STR_LEN newfile_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &poldfile, &oldfile_len, &pnewfile, &newfile_len) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("oldfile", oldfile_len);
    CHECK_EMPTY_STR_NAME("newfile", newfile_len);

    _CAPTURE_START();
    rv = mdbm_replace_file((const char*)poldfile, (const char *)pnewfile);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_pre_split) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int rv = -1;
    int id = -1;
    _ZEND_LONG pages = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &mdbm_link_index, &pages) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(pages, 0, UINT32_MAX);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_pre_split(mdbm_link->pmdbm, (mdbm_ubig_t)pages);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_fcopy) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;
    _ZEND_LONG mode = 0644;
    _ZEND_LONG flags = 0;

    char *pnewfile = NULL;
    _ZEND_STR_LEN newfile_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsl|l", &mdbm_link_index, &pnewfile, &newfile_len, &mode, &flags) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("newfile", newfile_len);

    //check the overlow
    CHECK_OVERFLOW(mode, 0, UINT_MAX);
    CHECK_OVERFLOW(flags, INT_MIN, INT_MAX);

    //check the flags
    if (flags & ~MDBM_COPY_LOCK_ALL) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - there was a not support parameter value : flags(=%ld)", flags);
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    int fd = open(pnewfile, O_RDWR | O_CREAT | O_TRUNC, (mode_t)mode);
    if(fd == -1) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error : does not create a tempfile(=%s), err=%s", pnewfile, strerror(errno));
        RETURN_FALSE;
    } 

    _CAPTURE_START();
    rv = mdbm_fcopy(mdbm_link->pmdbm, fd, flags);
    close(fd);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_sync) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_sync(mdbm_link->pmdbm);
    _CAPTURE_END();
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
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_fsync(mdbm_link->pmdbm);
    _CAPTURE_END();
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
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_get_lockmode(mdbm_link->pmdbm);
    _CAPTURE_END();
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
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_lock(mdbm_link->pmdbm);
    _CAPTURE_END();
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
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_trylock(mdbm_link->pmdbm);
    _CAPTURE_END();
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
    _ZEND_STR_LEN key_len = 0;
    _ZEND_LONG flags = -1;
    datum datum_key = {0x00,};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsl", &mdbm_link_index, &pkey, &key_len, &flags) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("key", key_len);

    //check the overlow
    CHECK_OVERFLOW(flags, INT_MIN, INT_MAX);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    //make a datum
    datum_key.dptr = pkey;
    datum_key.dsize = (int)key_len;

    _CAPTURE_START();
    rv = mdbm_plock(mdbm_link->pmdbm, &datum_key, (int)flags);
    _CAPTURE_END();
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
    _ZEND_STR_LEN key_len = 0;
    _ZEND_LONG flags = -1;
    datum datum_key = {0x00,};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsl", &mdbm_link_index, &pkey, &key_len, &flags) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("key", key_len);

    //check the overlow
    CHECK_OVERFLOW(flags, INT_MIN, INT_MAX);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    //make a datum
    datum_key.dptr = pkey;
    datum_key.dsize = (int)key_len;

    _CAPTURE_START();
    rv = mdbm_tryplock(mdbm_link->pmdbm, &datum_key, (int)flags);
    _CAPTURE_END();
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
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_lock_shared(mdbm_link->pmdbm);
    _CAPTURE_END();
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
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_trylock_shared(mdbm_link->pmdbm);
    _CAPTURE_END();
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
    _ZEND_STR_LEN key_len = 0;
    _ZEND_LONG flags = -1;
    datum datum_key = {0x00,};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsl", &mdbm_link_index, &pkey, &key_len, &flags) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("key", key_len);

    //check the overlow
    CHECK_OVERFLOW(flags, INT_MIN, INT_MAX);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    //make a datum
    datum_key.dptr = pkey;
    datum_key.dsize = (int)key_len;

    _CAPTURE_START();
    rv = mdbm_lock_smart(mdbm_link->pmdbm, &datum_key, (int)flags);
    _CAPTURE_END();
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
    _ZEND_STR_LEN key_len = 0;
    _ZEND_LONG flags = -1;
    datum datum_key = {0x00,};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsl", &mdbm_link_index, &pkey, &key_len, &flags) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("key", key_len);

    //check the overlow
    CHECK_OVERFLOW(flags, INT_MIN, INT_MAX);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    //make a datum
    datum_key.dptr = pkey;
    datum_key.dsize = (int)key_len;

    _CAPTURE_START();
    rv = mdbm_trylock_smart(mdbm_link->pmdbm, &datum_key, (int)flags);
    _CAPTURE_END();
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
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_unlock(mdbm_link->pmdbm);
    _CAPTURE_END();
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
    _ZEND_STR_LEN key_len = 0;
    _ZEND_LONG flags = -1;
    datum datum_key = {0x00,};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsl", &mdbm_link_index, &pkey, &key_len, &flags) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("key", key_len);

    //check the overlow
    CHECK_OVERFLOW(flags, INT_MIN, INT_MAX);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    //make a datum
    datum_key.dptr = pkey;
    datum_key.dsize = (int)key_len;

    _CAPTURE_START();
    rv = mdbm_punlock(mdbm_link->pmdbm, &datum_key, (int)flags);
    _CAPTURE_END();
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
    _ZEND_STR_LEN key_len = 0;
    _ZEND_LONG flags = -1;
    datum datum_key = {0x00,};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsl", &mdbm_link_index, &pkey, &key_len, &flags) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("key", key_len);

    //check the overlow
    CHECK_OVERFLOW(flags, INT_MIN, INT_MAX);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    //make a datum
    datum_key.dptr = pkey;
    datum_key.dsize = (int)key_len;

    _CAPTURE_START();
    rv = mdbm_unlock_smart(mdbm_link->pmdbm, &datum_key, (int)flags);
    _CAPTURE_END();
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
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_islocked(mdbm_link->pmdbm);
    _CAPTURE_END();
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
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_isowned(mdbm_link->pmdbm);
    _CAPTURE_END();
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

    char *pdbfn = NULL;
    int dbfn_len = 0;

    char fn[PATH_MAX] = {0x00};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &pdbfn, &dbfn_len) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("mdbm file path", dbfn_len);

    strncpy(fn, pdbfn, dbfn_len);
    if (fn == NULL) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Out of memory while allocating memory");
        RETURN_TRUE;
    }


    //flags Reserved for future use, and must be 0.
    _CAPTURE_START();
    rv = mdbm_lock_reset((const char *)&fn, 0);
    _CAPTURE_END();
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

    char *pdbfn = NULL;
    int dbfn_len = 0;

    char fn[PATH_MAX] = {0x00};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &pdbfn, &dbfn_len) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("mdbm file path", dbfn_len);

    strncpy(fn, pdbfn, dbfn_len);
    if (fn == NULL) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Out of memory while allocating memory");
        RETURN_TRUE;
    }

    _CAPTURE_START();
    rv = mdbm_delete_lockfiles((const char *)&fn);
    _CAPTURE_END();
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
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_preload(mdbm_link->pmdbm);
    _CAPTURE_END();
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
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_get_errno(mdbm_link->pmdbm);
    _CAPTURE_END();
    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_limit_dir_size) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int rv = -1;
    int id = -1;
    _ZEND_LONG pages = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &mdbm_link_index, &pages) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(pages, INT_MIN, INT_MAX);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_limit_dir_size(mdbm_link->pmdbm, (int)pages);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_get_version) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    uint32_t rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_get_version(mdbm_link->pmdbm);
    _CAPTURE_END();
    RETURN_LONG(rv);
}


PHP_FUNCTION(mdbm_get_size) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    uint64_t rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_get_size(mdbm_link->pmdbm);
    _CAPTURE_END();
    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_get_page_size) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_get_page_size(mdbm_link->pmdbm);
    _CAPTURE_END();
    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_get_hash) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_get_hash(mdbm_link->pmdbm);
    _CAPTURE_END();

    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_set_hash) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    _ZEND_LONG hash = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &mdbm_link_index, &hash) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(hash, INT_MIN, INT_MAX);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    if (hash < MDBM_HASH_CRC32 || hash > MDBM_MAX_HASH) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - there was a not support parameter value : hash(=%ld)", hash);
        RETURN_FALSE;
    }

    _CAPTURE_START();
    rv = mdbm_set_hash(mdbm_link->pmdbm, (int)hash);
    _CAPTURE_END();
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
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_get_limit_size(mdbm_link->pmdbm);
    _CAPTURE_END();

    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_setspillsize) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    _ZEND_LONG size = -1;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &mdbm_link_index, &size) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(size, INT_MIN, INT_MAX);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_setspillsize(mdbm_link->pmdbm, size);
    _CAPTURE_END();

    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_get_alignment) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_get_alignment(mdbm_link->pmdbm);
    _CAPTURE_END();

    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_set_alignment) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;

    _ZEND_LONG align = -1;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &mdbm_link_index, &align) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(align, INT_MIN, INT_MAX);

    if (align != MDBM_ALIGN_8_BITS 
            && align != MDBM_ALIGN_16_BITS
            && align != MDBM_ALIGN_32_BITS
            && align != MDBM_ALIGN_64_BITS) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "The parameter value of `align` is less than the minimum allowed value of 0");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_set_alignment(mdbm_link->pmdbm, (int)align);
    _CAPTURE_END();

    RETURN_LONG(rv);
}


PHP_FUNCTION(mdbm_compress_tree) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    if (mdbm_link_index == NULL) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "not found the mdbm resource");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    mdbm_compress_tree(mdbm_link->pmdbm);
    RETURN_NULL();
}

PHP_FUNCTION(mdbm_store) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;

    int rv = -1;
    char *pkey = NULL;
    _ZEND_STR_LEN key_len = 0;
    char *pval = NULL;
    _ZEND_STR_LEN val_len = 0;
    _ZEND_LONG  flags = MDBM_INSERT;
    datum key = {0x00,};
    datum val = {0x00,};

    char *psetkey = NULL;
    char *psetval = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss|l", &mdbm_link_index, &pkey, &key_len, &pval, &val_len, &flags) == FAILURE) {
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("key", key_len);
    CHECK_EMPTY_STR_NAME("val", val_len);

    //check the length of key
    _CHECK_MDBM_STR_MAXLEN(key_len);

    //check the length of value
    _CHECK_VALLEN(val_len);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

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


    _CAPTURE_START();
    rv = mdbm_store(mdbm_link->pmdbm, key, val, (int)flags);
    _CAPTURE_END();

    efree(psetkey);
    efree(psetval);

    if (rv == -1) {
        RETURN_FALSE;
    }

    if (rv == 1 && flags == (flags | MDBM_INSERT)) { //the key already exists
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "the key(=%s) already exists", pkey);
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_store_r) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;

    int rv = -1;
    char *pkey = NULL;
    _ZEND_STR_LEN key_len = 0;
    char *pval = NULL;
    _ZEND_STR_LEN val_len = 0;
    _ZEND_LONG  flags = MDBM_INSERT;
    datum key = {0x00,};
    datum val = {0x00,};

    MDBM_ITER arg_iter;
    MDBM_ITER *parg_iter = &arg_iter;

    char *psetkey = NULL;
    char *psetval = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss|l", &mdbm_link_index, &pkey, &key_len, &pval, &val_len, &flags) == FAILURE) {
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("key", key_len);
    CHECK_EMPTY_STR_NAME("val", val_len);

    //check the length of key
    _CHECK_MDBM_STR_MAXLEN(key_len);

    //check the length of value
    _CHECK_VALLEN(val_len);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

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


    _CAPTURE_START();
    rv = mdbm_store_r(mdbm_link->pmdbm, &key, &val, (int)flags, parg_iter);
    _CAPTURE_END();

    efree(psetkey);
    efree(psetval);

    if (rv == -1) {
        RETURN_FALSE;
    }

    if (rv == 1 && flags == (flags | MDBM_INSERT)) { //the key already exists
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "the key(=%s) already exists", pkey);
        RETURN_FALSE;
    }

    array_init(return_value);
    add_assoc_long(return_value, HASHKEY_PAGENO, parg_iter->m_pageno);
    add_assoc_long(return_value, HASHKEY_NEXT, parg_iter->m_next);
}

PHP_FUNCTION(mdbm_fetch) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;
    datum key = {0x00,};
    datum val = {0x00,};

    char *pkey = NULL;
    _ZEND_STR_LEN key_len = 0;

    char *pretval = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &mdbm_link_index, &pkey, &key_len) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("key", key_len);

    //check the length of key
    _CHECK_MDBM_STR_MAXLEN(key_len);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    //make a datum
    key.dptr = pkey;
    key.dsize = (int)key_len;

    _CAPTURE_START();
    val = mdbm_fetch(mdbm_link->pmdbm, key);
    _CAPTURE_END();
    if (val.dptr == NULL) {
        RETURN_FALSE;
    }

    //for fix "Warning: String is not zero-terminated" issue aftre ran mdbm_preload
    pretval = copy_strptr(val.dptr, val.dsize);
    if (pretval == NULL) {
        RETURN_FALSE;
    }

    _R_STRINGL(pretval, val.dsize+1, 0);
}

PHP_FUNCTION(mdbm_fetch_r) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;
    datum key = {0x00,};
    datum val = {0x00,};

    MDBM_ITER arg_iter;
    MDBM_ITER *parg_iter = &arg_iter;
    char *pkey = NULL;
    _ZEND_STR_LEN key_len = 0;

    char *pretval = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &mdbm_link_index, &pkey, &key_len) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s) with key");
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("key", key_len);

    //check the length of key
    _CHECK_MDBM_STR_MAXLEN(key_len);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    //auto select to iter
    rv = iter_handler(mdbm_link, &parg_iter, NULL);
    if (rv < 0) {
        RETURN_FALSE;
    }

    //make a datum
    key.dptr = pkey;
    key.dsize = (int)key_len;

    _CAPTURE_START();
    rv = mdbm_fetch_r(mdbm_link->pmdbm, &key, &val, parg_iter);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }
    if (val.dptr == NULL) {
        RETURN_FALSE;
    }

    //for fix "Warning: String is not zero-terminated" issue aftre ran mdbm_preload
    pretval = copy_strptr(val.dptr, val.dsize);
    if (pretval == NULL) {
        RETURN_FALSE;
    }

    array_init(return_value);
    _ADD_ASSOC_STRINGL(return_value, HASHKEY_VAL, pretval, val.dsize, 0);
    add_assoc_long(return_value, HASHKEY_PAGENO, parg_iter->m_pageno);
    add_assoc_long(return_value, HASHKEY_NEXT, parg_iter->m_next);
}

PHP_FUNCTION(mdbm_fetch_dup_r) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;
    datum key = {0x00,};
    datum val = {0x00,};

    MDBM_ITER arg_iter = {0x00,};
    MDBM_ITER *parg_iter = &arg_iter;
    char *pkey = NULL;
    _ZEND_STR_LEN key_len = 0;

    char *pretval = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &mdbm_link_index, &pkey, &key_len) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s) with key");
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("key", key_len);

    //check the length of key
    _CHECK_MDBM_STR_MAXLEN(key_len);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    //auto select to iter
    rv = iter_handler(mdbm_link, &parg_iter, NULL);
    if (rv < 0) {
        RETURN_FALSE;
    }

    //make a datum
    key.dptr = pkey;
    key.dsize = (int)key_len;

    _CAPTURE_START();
    rv = mdbm_fetch_dup_r(mdbm_link->pmdbm, &key, &val, parg_iter);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }
    if (val.dptr == NULL) {
        RETURN_FALSE;
    }

    //for fix "Warning: String is not zero-terminated" issue aftre ran mdbm_preload
    pretval = copy_strptr(val.dptr, val.dsize);
    if (pretval == NULL) {
        RETURN_FALSE;
    }

    array_init(return_value);
    _ADD_ASSOC_STRINGL(return_value, HASHKEY_VAL, pretval, val.dsize, 0);
    add_assoc_long(return_value, HASHKEY_PAGENO, parg_iter->m_pageno);
    add_assoc_long(return_value, HASHKEY_NEXT, parg_iter->m_next);
}

PHP_FUNCTION(mdbm_fetch_info) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;
    datum key = {0x00,};
    datum val = {0x00,};
    datum buf = {0x00,};

    MDBM_ITER arg_iter = {0x00,};
    MDBM_ITER *parg_iter = &arg_iter;
    char *pkey = NULL;
    _ZEND_STR_LEN key_len = 0;
    struct mdbm_fetch_info info = {0x00,};

    char *pretval = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &mdbm_link_index, &pkey, &key_len) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s) with key");
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("key", key_len);

    //check the length of key
    _CHECK_MDBM_STR_MAXLEN(key_len);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    //auto select to iter
    rv = iter_handler(mdbm_link, &parg_iter, NULL);
    if (rv < 0) {
        RETURN_FALSE;
    }

    //make a datum
    key.dptr = pkey;
    key.dsize = (int)key_len;

    _CAPTURE_START();
    rv = mdbm_fetch_info(mdbm_link->pmdbm, &key, &val, &buf, &info, parg_iter);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }
    if (val.dptr == NULL) {
        RETURN_FALSE;
    }

    //for fix "Warning: String is not zero-terminated" issue aftre ran mdbm_preload
    pretval = copy_strptr(val.dptr, val.dsize);
    if (pretval == NULL) {
        RETURN_FALSE;
    }

    array_init(return_value);
    _ADD_ASSOC_STRINGL(return_value, HASHKEY_VAL, pretval, val.dsize, 0);
    add_assoc_long(return_value, HASHKEY_PAGENO, parg_iter->m_pageno);
    add_assoc_long(return_value, HASHKEY_NEXT, parg_iter->m_next);
    add_assoc_long(return_value, HASHKEY_FLAGS, info.flags);
    add_assoc_long(return_value, HASHKEY_CACHE_NUM_ACCESSES, info.cache_num_accesses);
    add_assoc_long(return_value, HASHKEY_CACHE_ACCESS_TIME, info.cache_access_time);
}


PHP_FUNCTION(mdbm_delete) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;
    datum key = {0x00,};

    char *pkey = NULL;
    _ZEND_STR_LEN key_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &mdbm_link_index, &pkey, &key_len) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("key", key_len);

    //check the length of key
    _CHECK_MDBM_STR_MAXLEN(key_len);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    //check the length of key
    _CHECK_MDBM_STR_MAXLEN(key_len);

    //make a datum
    key.dptr = pkey;
    key.dsize = (int)key_len;

    _CAPTURE_START();
    rv = mdbm_delete(mdbm_link->pmdbm, key);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_delete_r) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    zval *arr = NULL;
    MDBM_ITER arg_iter;
    MDBM_ITER *parg_iter = &arg_iter;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ra", &mdbm_link_index, &arr) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s) with iter");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    //auto select to iter
    rv = iter_handler(mdbm_link, &parg_iter, arr);
    if (rv < 0) {
        RETURN_FALSE;
    }

    _CAPTURE_START();
    rv = mdbm_delete_r(mdbm_link->pmdbm, parg_iter );
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    array_init(return_value);
    add_assoc_long(return_value, HASHKEY_PAGENO, parg_iter->m_pageno);
    add_assoc_long(return_value, HASHKEY_NEXT, parg_iter->m_next);
}

PHP_FUNCTION(mdbm_first) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    kvpair kv = {0x00,};
    char *pretkey = NULL;
    char *pretval = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    kv = mdbm_first(mdbm_link->pmdbm);
    _CAPTURE_END();
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

    _ADD_ASSOC_STRINGL(return_value, HASHKEY_KEY, pretkey, kv.key.dsize, 0);
    _ADD_ASSOC_STRINGL(return_value, HASHKEY_VAL, pretval, kv.val.dsize, 0);
}

PHP_FUNCTION(mdbm_next) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    kvpair kv = {0x00,};
    char *pretkey = NULL;
    char *pretval = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    kv = mdbm_next(mdbm_link->pmdbm);
    _CAPTURE_END();
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

    _ADD_ASSOC_STRINGL(return_value, HASHKEY_KEY, pretkey, kv.key.dsize, 0);
    _ADD_ASSOC_STRINGL(return_value, HASHKEY_VAL, pretval, kv.val.dsize, 0);
}

PHP_FUNCTION(mdbm_firstkey) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    datum key = {0x00,};
    char *pretkey = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    key = mdbm_firstkey(mdbm_link->pmdbm);
    _CAPTURE_END();
    if (key.dptr == NULL) {
        RETURN_FALSE;
    }

    //for fix "Warning: String is not zero-terminated" issue aftre ran mdbm_preload
    pretkey = copy_strptr(key.dptr, key.dsize);
    if (pretkey == NULL) {
        RETURN_FALSE;
    }

    _R_STRINGL(pretkey, key.dsize, 0);
}

PHP_FUNCTION(mdbm_nextkey) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    datum val = {0x00,};
    char *pretval = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    val = mdbm_nextkey(mdbm_link->pmdbm);
    _CAPTURE_END();
    if (val.dptr == NULL) {
        RETURN_FALSE;
    }

    //for fix "Warning: String is not zero-terminated" issue aftre ran mdbm_preload
    pretval = copy_strptr(val.dptr, val.dsize);
    if (pretval == NULL) {
        RETURN_FALSE;
    }

    _R_STRINGL(pretval, val.dsize, 0);
}

PHP_FUNCTION(mdbm_reset_global_iter) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    MDBM_ITER_INIT(&(*mdbm_link).iter);

    RETURN_NULL();
}

PHP_FUNCTION(mdbm_get_global_iter) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    array_init(return_value);
    add_assoc_long(return_value, HASHKEY_PAGENO, (*mdbm_link).iter.m_pageno);
    add_assoc_long(return_value, HASHKEY_NEXT, (*mdbm_link).iter.m_next);
}

PHP_FUNCTION(mdbm_get_iter) {

    MDBM_ITER iter;
    _ZEND_LONG in_pageno = -1;
    _ZEND_LONG in_next = -1;
    int id = -1;
    int rv = -1;
    int argc = ZEND_NUM_ARGS();
    

    if (zend_parse_parameters(argc TSRMLS_CC, "|ll", &in_pageno, &in_next) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Failed to parsing the parameters");
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(in_pageno, 0, UINT32_MAX); //mdbm_usib_t = uint32_t
    CHECK_OVERFLOW(in_next, INT_MIN, INT_MAX);


    if (argc < 1) {
        MDBM_ITER_INIT(&iter);
        in_pageno = (long)iter.m_pageno;
        in_next = (long)iter.m_next;
    }

    array_init(return_value);
    add_assoc_long(return_value, HASHKEY_PAGENO, in_pageno);
    add_assoc_long(return_value, HASHKEY_NEXT, in_next);
}

PHP_FUNCTION(mdbm_first_r) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    zval *arr = NULL;
    MDBM_ITER arg_iter;
    MDBM_ITER *parg_iter = &arg_iter;

    int id = -1;
    int rv = -1;

    kvpair kv = {0x00,};
    char *pretkey = NULL;
    char *pretval = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|a", &mdbm_link_index, &arr) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    //auto select to iter
    rv = iter_handler(mdbm_link, &parg_iter, arr);
    if (rv < 0) {
        RETURN_FALSE;
    }

    _CAPTURE_START();
    kv = mdbm_first_r(mdbm_link->pmdbm, parg_iter);
    _CAPTURE_END();

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

    _ADD_ASSOC_STRINGL(return_value, HASHKEY_KEY, pretkey, kv.key.dsize, 0);
    _ADD_ASSOC_STRINGL(return_value, HASHKEY_VAL, pretval, kv.val.dsize, 0);
    add_assoc_long(return_value, HASHKEY_PAGENO, parg_iter->m_pageno);
    add_assoc_long(return_value, HASHKEY_NEXT, parg_iter->m_next);
}

PHP_FUNCTION(mdbm_next_r) {

    zval *mdbm_link_index = NULL;
    zval *arr = NULL;
    php_mdbm_open *mdbm_link = NULL;
    MDBM_ITER arg_iter;
    MDBM_ITER *parg_iter = &arg_iter;
    int id = -1;
    int rv = -1;

    kvpair kv = {0x00,};
    char *pretkey = NULL;
    char *pretval = NULL;

    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|a", &mdbm_link_index, &arr) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    //auto select to iter
    rv = iter_handler(mdbm_link, &parg_iter, arr);
    if (rv < 0) {
        RETURN_FALSE;
    }

    _CAPTURE_START();
    kv = mdbm_next_r(mdbm_link->pmdbm, parg_iter);
    _CAPTURE_END();

    if (kv.key.dptr == NULL || kv.val.dptr == NULL) {
        RETURN_FALSE;
    }

    pretkey = copy_strptr(kv.key.dptr, kv.key.dsize);
    if (pretkey == NULL) {
        RETURN_FALSE;
    }

    pretval = copy_strptr(kv.val.dptr, kv.val.dsize);
    if (pretval == NULL) {
        efree(pretkey);
        RETURN_FALSE;
    }

    array_init(return_value);

    _ADD_ASSOC_STRINGL(return_value, HASHKEY_KEY, pretkey, kv.key.dsize, 0);
    _ADD_ASSOC_STRINGL(return_value, HASHKEY_VAL, pretval, kv.val.dsize, 0);
    add_assoc_long(return_value, HASHKEY_PAGENO, parg_iter->m_pageno);
    add_assoc_long(return_value, HASHKEY_NEXT, parg_iter->m_next);

}

PHP_FUNCTION(mdbm_firstkey_r) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    zval *arr = NULL;
    MDBM_ITER arg_iter;
    MDBM_ITER *parg_iter = &arg_iter;

    int id = -1;
    int rv = -1;

    datum key = {0x00,};
    char *pretkey = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|a", &mdbm_link_index, &arr) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    //auto select to iter
    rv = iter_handler(mdbm_link, &parg_iter, arr);
    if (rv < 0) {
        RETURN_FALSE;
    }

    _CAPTURE_START();
    key  = mdbm_firstkey_r(mdbm_link->pmdbm, parg_iter);
    _CAPTURE_END();

    if (key.dptr == NULL) {
        RETURN_FALSE;
    }

    pretkey = copy_strptr(key.dptr, key.dsize);
    if (pretkey == NULL) {
        RETURN_FALSE;
    }

    array_init(return_value);

    _ADD_ASSOC_STRINGL(return_value, HASHKEY_KEY, pretkey, key.dsize, 0);
    add_assoc_long(return_value, HASHKEY_PAGENO, parg_iter->m_pageno);
    add_assoc_long(return_value, HASHKEY_NEXT, parg_iter->m_next);
}


PHP_FUNCTION(mdbm_nextkey_r) {

    zval *mdbm_link_index = NULL;
    zval *arr = NULL;
    php_mdbm_open *mdbm_link = NULL;
    MDBM_ITER arg_iter;
    MDBM_ITER *parg_iter = &arg_iter;
    int id = -1;
    int rv = -1;

    datum key = {0x00,};
    char *pretkey = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|a", &mdbm_link_index, &arr) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    //auto select to iter
    rv = iter_handler(mdbm_link, &parg_iter, arr);
    if (rv < 0) {
        RETURN_FALSE;
    }

    _CAPTURE_START();
    key = mdbm_nextkey_r(mdbm_link->pmdbm, parg_iter);
    _CAPTURE_END();

    if (key.dptr == NULL) {
        RETURN_FALSE;
    }

    pretkey = copy_strptr(key.dptr, key.dsize);
    if (pretkey == NULL) {
        RETURN_FALSE;
    }

    array_init(return_value);

    _ADD_ASSOC_STRINGL(return_value, HASHKEY_KEY, pretkey, key.dsize, 0);
    add_assoc_long(return_value, HASHKEY_PAGENO, parg_iter->m_pageno);
    add_assoc_long(return_value, HASHKEY_NEXT, parg_iter->m_next);
}


PHP_FUNCTION(mdbm_count_records) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    uint64_t rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_count_records(mdbm_link->pmdbm);
    _CAPTURE_END();
    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_count_pages) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    uint64_t rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_count_pages(mdbm_link->pmdbm);
    _CAPTURE_END();
    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_set_cachemode) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    _ZEND_LONG flag = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &mdbm_link_index, &flag) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(flag, INT_MIN, INT_MAX);

    if (flag < MDBM_CACHEMODE_NONE || flag > MDBM_CACHEMODE_MAX) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - there was a not support parameter value : flag(=%ld)", flag);
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_set_cachemode(mdbm_link->pmdbm, (int)flag);
    _CAPTURE_END();
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
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_get_cachemode(mdbm_link->pmdbm);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_LONG((long)rv);
}

PHP_FUNCTION(mdbm_get_cachemode_name) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    const char *pcache_name = NULL;
    _ZEND_LONG cacheno = -1;

    char *pretval = NULL;
    int retval_len = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &cacheno) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(cacheno, INT_MIN, INT_MAX);

    _CAPTURE_START();
    pcache_name = mdbm_get_cachemode_name((int)cacheno); //return value from stack
    _CAPTURE_END();
    retval_len = (int)strlen(pcache_name);


#if PHP_VERSION_ID < 70000
    pretval = copy_strptr((char *)pcache_name, retval_len);
    _R_STRINGL(pretval, retval_len, 0);
#else // PHP7
    _R_STRINGL(pcache_name, retval_len, 0);
#endif

}

PHP_FUNCTION(mdbm_clean) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    _ZEND_LONG pagenum = -1;
    //_ZEND_LONG flags = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &mdbm_link_index, &pagenum) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(pagenum, INT_MIN, INT_MAX);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_clean(mdbm_link->pmdbm, (int)pagenum, 0); //flags Ignored
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_check) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    _ZEND_LONG level = -1;
    zend_bool verbose = FALSE;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl|b", &mdbm_link_index, &level, &verbose) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(level, INT_MIN, INT_MAX);

    if (level < MDBM_CHECK_HEADER || level > MDBM_CHECK_ALL) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - there was a not support parameter value : level(=%ld)", level);
        RETURN_FALSE;
    }

    if ((int)verbose < 0 || (int)verbose > 1 ) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - there was a not support parameter value : verbose(=%d)", (int)verbose);
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    
    _CAPTURE_START();
    rv = mdbm_check(mdbm_link->pmdbm, (int)level, (int)verbose);
    _CAPTURE_END();
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

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_chk_all_page(mdbm_link->pmdbm);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_LONG(rv);
}

PHP_FUNCTION(mdbm_chk_page) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    _ZEND_LONG pagenum = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &mdbm_link_index, &pagenum) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(pagenum, INT_MIN, INT_MAX);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_chk_page(mdbm_link->pmdbm, (int)pagenum);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_protect) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;

    _ZEND_LONG protect = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &mdbm_link_index, &protect) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(protect, INT_MIN, INT_MAX);

    if (protect < MDBM_PROT_NONE || protect > MDBM_PROT_ACCESS) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - there was a not support parameter value : protect(=%ld)", protect);
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_protect(mdbm_link->pmdbm, (int)protect);
    _CAPTURE_END();
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
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_lock_pages(mdbm_link->pmdbm);
    _CAPTURE_END();
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
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_unlock_pages(mdbm_link->pmdbm);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_get_hash_value) {

    int id = -1;
    int rv = -1;
    uint32_t hashv = -1;
    datum key = {0x00,};

    char *pkey = NULL;
    _ZEND_STR_LEN key_len = -1;
    _ZEND_LONG hfc = -1;
    

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &pkey, &key_len, &hfc) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("key", key_len);

    //check the overlow
    CHECK_OVERFLOW(hfc, INT_MIN, INT_MAX);

    if (hfc < MDBM_HASH_CRC32 || hfc > MDBM_MAX_HASH) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - there was a not support parameter value : hash function code(=%d)", (int)hfc);
        RETURN_FALSE;
    }

    if (key_len < 1) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "required! hashValue(length=%d)", (int)key_len);
        RETURN_FALSE;
    }

    //make a datum
    key.dptr = pkey;
    key.dsize = (int)key_len;

    _CAPTURE_START();
    rv = mdbm_get_hash_value(key, (int)hfc, &hashv);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_LONG((long)hashv);
}

PHP_FUNCTION(mdbm_get_page) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    mdbm_ubig_t rv = -1;
    datum key = {0x00,};

    char *pkey = NULL;
    _ZEND_STR_LEN key_len = 0;


    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &mdbm_link_index, &pkey, &key_len) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the length
    CHECK_EMPTY_STR_NAME("key", key_len);

    //check the length of key
    _CHECK_MDBM_STR_MAXLEN(key_len);

     //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    //make a datum
    key.dptr = pkey;
    key.dsize = (int)key_len;

    _CAPTURE_START();
    rv = mdbm_get_page(mdbm_link->pmdbm, &key);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_LONG((long)rv);
}

PHP_FUNCTION(mdbm_get_magic_number) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    int rv = -1;
    uint32_t magic = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_get_magic_number(mdbm_link->pmdbm, &magic);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_LONG((long)magic);
}

PHP_FUNCTION(mdbm_set_window_size) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int rv = -1;
    int id = -1;
    _ZEND_LONG wsize = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &mdbm_link_index, &wsize) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(wsize, INT_MIN, INT_MAX);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_set_window_size(mdbm_link->pmdbm, (size_t)wsize);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_enable_stat_operations) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int rv = -1;
    int id = -1;
    _ZEND_LONG flags = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &mdbm_link_index, &flags) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(flags, INT_MIN, INT_MAX);

    if (flags > (MDBM_STATS_BASIC | MDBM_STATS_TIMED)) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - there was a not support parameter value : flags(=%ld)", flags);
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_enable_stat_operations(mdbm_link->pmdbm, (int)flags);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    mdbm_link->enable_stat = 1;

    RETURN_TRUE;
}

PHP_FUNCTION(mdbm_reset_stat_operations) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int rv = -1;
    int id = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    if (mdbm_link->enable_stat != 1) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - Required! call to mdbm_reset_stat_operations aftre mdbm_enable_stat_operations.");
        RETURN_FALSE;
    }

    _CAPTURE_START();
    mdbm_reset_stat_operations(mdbm_link->pmdbm);
    _CAPTURE_END();
    RETURN_NULL();
}

PHP_FUNCTION(mdbm_set_stat_time_func) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int rv = -1;
    int id = -1;
    _ZEND_LONG flags = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &mdbm_link_index, &flags) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(flags, INT_MIN, INT_MAX);

    if (flags != MDBM_CLOCK_TSC && flags != MDBM_CLOCK_STANDARD) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - there was a not support parameter value : flags(=%ld)", flags);
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_set_stat_time_func(mdbm_link->pmdbm, flags);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_LONG((long)rv);
}

PHP_FUNCTION(mdbm_get_stat_time) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    time_t value = 0;
    char *ptime = NULL;
    size_t time_len = 0;
    int rv = -1;
    int id = -1;
    _ZEND_LONG type = -1;
    char *pretval = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &mdbm_link_index, &type) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(type, INT_MIN, INT_MAX);

    if (type != MDBM_STAT_TYPE_FETCH && type != MDBM_STAT_TYPE_STORE && type != MDBM_STAT_TYPE_DELETE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - there was a not support parameter value : type(=%ld)", type);
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_get_stat_time(mdbm_link->pmdbm, (mdbm_stat_type)type, &value);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    ptime = ctime(&value);
    time_len = strlen(ptime);

    pretval = copy_strptr((char *)ptime, time_len);
    _R_STRINGL(pretval, time_len, 0);
}

PHP_FUNCTION(mdbm_get_stat_counter) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    mdbm_counter_t value = 0;
    char *ptime = NULL;
    size_t time_len = 0;
    int rv = -1;
    int id = -1;
    _ZEND_LONG type = -1;
    char *pretval = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &mdbm_link_index, &type) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(type, INT_MIN, INT_MAX);

    if (type != MDBM_STAT_TYPE_FETCH && type != MDBM_STAT_TYPE_STORE && type != MDBM_STAT_TYPE_DELETE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - there was a not support parameter value : type(=%ld)", type);
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_get_stat_counter(mdbm_link->pmdbm, (mdbm_stat_type)type, &value);
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    RETURN_LONG((long)value);
}


PHP_FUNCTION(mdbm_stat_all_page) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    mdbm_stat_all_page(mdbm_link->pmdbm);
    _CAPTURE_END();

    RETURN_NULL();
}


PHP_FUNCTION(mdbm_dump_all_page) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    mdbm_dump_all_page(mdbm_link->pmdbm);
    _CAPTURE_END();

    RETURN_NULL();
}

PHP_FUNCTION(mdbm_dump_page) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    int id = -1;
    _ZEND_LONG pno = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &mdbm_link_index, &pno) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //check the overlow
    CHECK_OVERFLOW(pno, INT_MIN, INT_MAX);

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    mdbm_dump_page(mdbm_link->pmdbm, (int)pno);
    _CAPTURE_END();

    RETURN_NULL();
}

PHP_FUNCTION(mdbm_get_stats) {

    zval *mdbm_link_index = NULL;
    php_mdbm_open *mdbm_link = NULL;
    mdbm_stats_t s = {0x00,};
    int id = -1;
    int rv = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mdbm_link_index) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error - There was a missing parameter(s)");
        RETURN_FALSE;
    }

    //fetch the resource
    _FETCH_RES(mdbm_link_index, id);

    _CAPTURE_START();
    rv = mdbm_get_stats(mdbm_link->pmdbm, &s, sizeof(s));
    _CAPTURE_END();
    if (rv == -1) {
        RETURN_FALSE;
    }

    array_init(return_value);
    add_assoc_long(return_value, "s_size", (long)s.s_size);
    add_assoc_long(return_value, "s_page_size", (long)s.s_page_size);
    add_assoc_long(return_value, "s_page_count", (long)s.s_page_count);
    add_assoc_long(return_value, "s_pages_used", (long)s.s_pages_used);
    add_assoc_long(return_value, "s_bytes_used", (long)s.s_bytes_used);
    add_assoc_long(return_value, "s_num_entries", (long)s.s_num_entries);
    add_assoc_long(return_value, "s_min_level", (long)s.s_min_level);
    add_assoc_long(return_value, "s_max_level", (long)s.s_max_level);
    add_assoc_long(return_value, "s_large_page_size", (long)s.s_large_page_size);
    add_assoc_long(return_value, "s_large_page_count", (long)s.s_large_page_count);
    add_assoc_long(return_value, "s_large_threshold", (long)s.s_large_threshold);
    add_assoc_long(return_value, "s_large_pages_used", (long)s.s_large_pages_used);
    add_assoc_long(return_value, "s_large_num_free_entries", (long)s.s_large_num_free_entries);
    add_assoc_long(return_value, "s_large_max_free", (long)s.s_large_max_free);
    add_assoc_long(return_value, "s_large_num_entries", (long)s.s_large_num_entries);
    add_assoc_long(return_value, "s_large_bytes_used", (long)s.s_large_bytes_used);
    add_assoc_long(return_value, "s_large_min_size", (long)s.s_large_min_size);
    add_assoc_long(return_value, "s_large_max_size", (long)s.s_large_max_size);
    add_assoc_long(return_value, "s_cache_mode", (long)s.s_cache_mode);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
