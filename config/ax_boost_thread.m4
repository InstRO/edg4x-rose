##### http://autoconf-archive.cryp.to/ax_boost_thread.html
#
# SYNOPSIS
#
#   AX_BOOST_THREAD
#
# DESCRIPTION
#
#   Test for Thread library from the Boost C++ libraries. The macro
#   requires a preceding call to AX_BOOST_BASE. Further documentation
#   is available at <http://randspringer.de/boost/index.html>.
#
#   This macro calls:
#
#     AC_SUBST(BOOST_THREAD_LIB)
#
#   And sets:
#
#     HAVE_BOOST_THREAD
#
# LAST MODIFICATION
#
#   2007-11-22
#
# COPYLEFT
#
#   Copyright (c) 2007 Thomas Porschberg <thomas@randspringer.de>
#   Copyright (c) 2007 Michael Tindal
#
#   Copying and distribution of this file, with or without
#   modification, are permitted in any medium without royalty provided
#   the copyright notice and this notice are preserved.

AC_DEFUN([AX_BOOST_THREAD],
[
    AC_ARG_WITH([boost-thread],
    AS_HELP_STRING([--with-boost-thread@<:@=special-lib@:>@],
                   [Use the Thread library from Boost.  If the value is 'yes' or a certain library then ROSE will
                        incorporate multi-thread support into some of its algorithms. The default is to not
                        incorporate multi-thread support into any algorithms regardless of what the ROSE documentation
                        says about the thread safety of the algorithm.  If thread support is enabled with this switch
                        then user code should also be compiled and linked with switches appropriate for multi-threading
                        e.g., -pthread for GCC. ]),
        [
        if test "$withval" = "no"; then
            want_boost="no"
        elif test "$withval" = "yes"; then
            want_boost="yes"
            ax_boost_user_thread_lib=""
        else
            want_boost="yes"
            ax_boost_user_thread_lib="$withval"
        fi
        ],
        [want_boost="no"]
    )

    if test "x$want_boost" = "xyes"; then
        AC_REQUIRE([AC_PROG_CC])
        AC_REQUIRE([AC_CANONICAL_BUILD])
        CPPFLAGS_SAVED="$CPPFLAGS"
        CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
        export CPPFLAGS

        LDFLAGS_SAVED="$LDFLAGS"
        LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
        export LDFLAGS

        AC_CACHE_CHECK(whether the Boost::Thread library is available,
                       ax_cv_boost_thread,
        [AC_LANG_PUSH([C++])
             CXXFLAGS_SAVE=$CXXFLAGS

             case "$build_os" in
               solaris ) CXXFLAGS="-pthreads $CXXFLAGS" ;;
               ming32 ) CXXFLAGS="-mthreads $CXXFLAGS" ;;
               darwin* ) CXXFLAGS="$CXXFLAGS" ;;
               * ) CXXFLAGS="-pthread $CXXFLAGS" ;;
             esac
             AC_COMPILE_IFELSE(AC_LANG_PROGRAM([[@%:@include <boost/thread/thread.hpp>]],
                                   [[boost::thread_group thrds;
                                   return 0;]]),
                   ax_cv_boost_thread=yes, ax_cv_boost_thread=no)
             CXXFLAGS=$CXXFLAGS_SAVE
             AC_LANG_POP([C++])
        ])
        if test "x$ax_cv_boost_thread" = "xyes"; then
            case "$build_os" in
              solaris ) BOOST_CPPFLAGS="-pthreads $BOOST_CPPFLAGS" ;;
              ming32 ) BOOST_CPPFLAGS="-mthreads $BOOST_CPPFLAGS" ;;
              darwin* ) BOOST_CPPFLAGS="$BOOST_CPPFLAGS" ;;
              * ) BOOST_CPPFLAGS="-pthread $BOOST_CPPFLAGS" ;;
            esac

            AC_SUBST(BOOST_CPPFLAGS)

            AC_DEFINE(HAVE_BOOST_THREAD,,[define if the Boost::Thread library is available])
            BOOSTLIBDIR=`echo $BOOST_LDFLAGS | sed -e 's/@<:@^\/@:>@*//'`

            LDFLAGS_SAVE=$LDFLAGS
                        case "x$build_os" in
                          *bsd* )
                               LDFLAGS="-pthread $LDFLAGS"
                          break;
                          ;;
                        esac
            if test "x$ax_boost_user_thread_lib" = "x"; then
                for libextension in `ls $BOOSTLIBDIR/libboost_thread*.{so,dylib,a}* 2>/dev/null | sed 's,.*/,,' | sed -e 's;^lib\(boost_thread.*\)\.so.*$;\1;' -e 's;^lib\(boost_thread.*\)\.a*$;\1;' -e 's;^lib\(boost_thread.*\)\.dylib$;\1;'` ; do
                     ax_lib=${libextension}
                    AC_CHECK_LIB($ax_lib, toupper,
                                 [BOOST_THREAD_LIB="-l$ax_lib"; AC_SUBST(BOOST_THREAD_LIB) link_thread="yes"; break],
                                 [link_thread="no"])
                done
                if test "x$link_thread" != "xyes"; then
                for libextension in `ls $BOOSTLIBDIR/boost_thread*.{dll,a}* 2>/dev/null | sed 's,.*/,,' | sed -e 's;^\(boost_thread.*\)\.dll.*$;\1;' -e 's;^\(boost_thread.*\)\.a*$;\1;'` ; do
                     ax_lib=${libextension}
                    AC_CHECK_LIB($ax_lib, toupper,
                                 [BOOST_THREAD_LIB="-l$ax_lib"; AC_SUBST(BOOST_THREAD_LIB) link_thread="yes"; break],
                                 [link_thread="no"])
                done
                fi

            else
               for ax_lib in $ax_boost_user_thread_lib boost_thread-$ax_boost_user_thread_lib; do
                      AC_CHECK_LIB($ax_lib, toupper,
                                   [BOOST_THREAD_LIB="-l$ax_lib"; AC_SUBST(BOOST_THREAD_LIB) link_thread="yes"; break],
                                   [link_thread="no"])
                  done

            fi
            if test "x$link_thread" = "xno"; then
                AC_MSG_ERROR(Could not link against $ax_lib !)
                        else
                           case "x$build_os" in
                              *bsd* )
                    BOOST_LDFLAGS="-pthread $BOOST_LDFLAGS"
                              break;
                              ;;
                           esac

            fi
        fi

        CPPFLAGS="$CPPFLAGS_SAVED"
        LDFLAGS="$LDFLAGS_SAVED"
    fi
])
