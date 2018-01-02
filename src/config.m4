PHP_ARG_WITH(mdbm, for mdbm support,
Make sure that the comment is aligned:
[  --with-mdbm             Include mdbm support])


if test "$PHP_MDBM" != "no"; then
  SEARCH_PATH="/usr/local/mdbm /usr/local /usr"   
  SEARCH_FOR="/include/mdbm.h"  
  if test -r $PHP_MDBM/$SEARCH_FOR; then 
    MDBM_DIR=$PHP_MDBM
  else 
    AC_MSG_CHECKING([for mdbm files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        MDBM_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
  
  if test -z "$MDBM_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the mdbm distribution])
  fi

  PHP_ADD_INCLUDE($MDBM_DIR/include)

  LIBNAME=mdbm 
  LIBSYMBOL=mdbm_open

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $MDBM_DIR/lib64/, MDBM_SHARED_LIBADD)
    AC_DEFINE(HAVE_MDBMLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong mdbm lib version or lib not found])
  ],[
    -L$MDBM_DIR/lib64/ -lmdbm
  ])
  
  PHP_SUBST(MDBM_SHARED_LIBADD)

  PHP_NEW_EXTENSION(mdbm, mdbm.c, $ext_shared)
fi

