dnl Process this file with autoconf to produce configure.
AC_INIT(spinnaker-adapters, 1.0.0)
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])
AM_MAINTAINER_MODE

if test "$MPI_CXX" != ""; then
  CXX="$MPI_CXX"
fi

AC_PROG_CXX([mpiCC mpixlcxx mpixlcxx_r mpicxx CC])
MPI_CXX="$CXX"

AC_MSG_CHECKING([which MPI system we think we are using])
case "$MPI_CXX" in
  mpiCC)
    changequote({, })
    MPI_C=mpicc
    if $MPI_CXX -compile_info >/dev/null 2>/dev/null; then
      SYSGUESS=mpich
      CXX="`$MPI_CXX -compile_info | sed -e 's/^\([^ ]*\).*/\1/'`"
    else
      #	mpiCC can be a C compiler under OpenMPI
      if mpicxx -showme >/dev/null 2>/dev/null; then
        SYSGUESS=openmpi
        CXX="`mpicxx -showme | sed -e 's/^\([^ ]*\).*/\1/'`"
      else
        changequote([, ])      
        AC_MSG_ERROR([
Confused when trying to determine MPI_CXX, MPI_CXXFLAGS etc.
Please set them manually.])
        changequote({, })
      fi
    fi
    LAUNCHSTYLE=_mpirun
    TESTDIR=testsuite
    EXTRAS=extras
    changequote([, ])
    ;;
  mpixlcxx_r)
    MPI_C=mpixlc_r
    SYSGUESS=bgp
    CXX="$MPI_CXX"
    #CC="`echo $MPI_CXX | sed -e 's/xlcxx$/xlc/;s/cxx$/cc/'`"
    CC="$MPI_C"
    ;;
  mpixlcxx)
    MPI_C=mpixlc
    SYSGUESS=bgq
    CXX="$MPI_CXX"
    #CC="`echo $MPI_CXX | sed -e 's/xlcxx$/xlc/;s/cxx$/cc/'`"
    CC="$MPI_C"
    ;;
  mpicxx)
    MPI_C=mpicc
    CC="$MPI_C"
    CXX="$MPI_CXX"
    if mpich2version >/dev/null 2>/dev/null; then
       SYSGUESS=mpich2
       AC_DEFINE(HAVE_MPICH2, 1, [Define to 1 if you have MPICH2.])
    else
       SYSGUESS=bgl
    fi
    ;;
  CC)
    # We should be checking MPI version but for the Cray we do it differently
    if uname -r | grep -q cray_gem_s; then
      SYSGUESS=cray_xe6
      AC_DEFINE(CRAY_XE6, 1, [Define if this is a Cray XE6 system.])
      CC=cc
      CXX=CC
    else
      SYSGUESS=unknown
    fi
    ;;
  xlC)
    MPI_C=xlc
    SYSGUESS=bgq_seq
    CXX="$MPI_CXX"
    CC="$MPI_C"
    ;;

  *)
    SYSGUESS=unknown
    ;;
esac
echo "SYSGUESS=$SYSGUESS"

AC_MSG_CHECKING(MPI_CXXFLAGS)
if test "$MPI_CXXFLAGS" != ""; then
  echo "$MPI_CXXFLAGS"
else
  case "$SYSGUESS" in
    mpich|mpich2)
      changequote(<, >)
      MPI_CXXFLAGS="`$MPI_CXX -compile_info | sed -e 's/^[^ ]* //;s/ -c / /'`"
      changequote([, ])
      ;;
    openmpi)
      MPI_CXXFLAGS="`mpicxx -showme:compile`"
      ;;
    bgp)
      MPI_CXXFLAGS="-qarch=450 -qtune=450 -qhot -qnostrict"
      enable_shared=no
      ;;
    bgq)
      MPI_CXXFLAGS="-qarch=qp -qtune=qp -qhot -qnostrict"
      enable_shared=no
      ;;
    cray_xe6)
      MPI_CXXFLAGS=""
      ;;
    bgq_seq)
      MPI_CXXFLAGS=""
      enable_shared=no
      ;;
    *)
      AC_MSG_ERROR([
Could not determine proper value for MPI_CXXFLAGS.  Please see README.])
      ;;
  esac
  echo "$MPI_CXXFLAGS"
fi

AC_MSG_CHECKING(MPI_LDFLAGS)
if test "$MPI_LDFLAGS" != ""; then
  echo "$MPI_LDFLAGS"
else
  case "$SYSGUESS" in
    mpich|mpich2)
      changequote(<, >)
      MPI_LDFLAGS="`$MPI_CXX -link_info | sed -e 's/^[^ ]* //;s/ -c / /'`"
      changequote([, ])
      ;;
    openmpi)
      MPI_LDFLAGS="`mpicxx -showme:link`"
      ;;
    bgp|bgq|cray_xe6|bgq_seq)
      MPI_LDFLAGS=""
      ;;
    *)
      AC_MSG_ERROR([
Could not determine proper value for MPI_LDFLAGS.  Please see README.])
      ;;
  esac
  echo "$MPI_LDFLAGS"
fi

CXXFLAGS="-g -O3"

if test $CXX = g++; then
	CXXFLAGS="-pedantic -Wall -Wno-long-long $CXXFLAGS"
fi

#
# Use libspynnaker_external_device_lib?
#
AC_CHECK_LIB(spynnaker_external_device_lib, libspynnaker_extdev_version, HAVE_LIBSPYNNAKER_EXTDEV="auto", HAVE_LIBSPYNNAKER_EXTDEV="no")
if test "x$HAVE_LIBSPYNNAKER_EXTDEV" = xauto; then
   LIBSPYNNAKER_EXTDEV_DIR="."
fi
AC_ARG_WITH(extdevlib, [  --with-extdevlib[[=directory]]       Request the use of libspynnaker_external_device_lib. Optionally give the directory, where libspynnaker_external_device_lib is installed],
  [
    if test "$withval" != "no"; then
      if test "$withval" != "yes"; then
	LIBSPYNNAKER_EXTDEV_DIR="${withval}"
      fi
      HAVE_LIBSPYNNAKER_EXTDEV="yes"
    else
      HAVE_LIBSPYNNAKER_EXTDEV="no"
    fi
  ])

if test "x$HAVE_LIBSPYNNAKER_EXTDEV" != xyes; then
  AC_MSG_ERROR([
Could not find libspynnaker_external_device_lib.  Please see README.])
fi


AC_SUBST(MPI_CXXFLAGS)
AC_SUBST(MPI_LDFLAGS)
AC_SUBST(LIBSPYNNAKER_EXTDEV_DIR)

AC_CONFIG_FILES([
  Makefile
  src/Makefile
])

AC_OUTPUT
