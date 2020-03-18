#!/usr/bin/env bash

# help debug
set -x
set -e
set -o

# Environment setup
LANG=C
LC_ALL=C
export LANG LC_ALL

if [ -d "./tmp" ]; then
    rm -rf ./tmp
fi

mkdir -p tmp
mkdir -p tmp/etc

# Build and local install repositories
BUILD_PREFIX=$PWD/tmp
SCRIPTS_PREFIX=$PWD/tmp/etc

# CCache setup
PATH="`echo "$PATH" | sed -e 's,^/usr/lib/ccache/?:,,' -e 's,:/usr/lib/ccache/?:,,' -e 's,:/usr/lib/ccache/?$,,' -e 's,^/usr/lib/ccache/?$,,'2`"
CCACHE_PATH="$PATH"
CCACHE_DIR="${HOME}/.ccache"
export CCACHE_PATH CCACHE_DIR PATH
HAVE_CCACHE=no

if which ccache && ls -la /usr/lib/ccache ; then
    HAVE_CCACHE=yes
fi

# Get CCache statistics
if [ "$HAVE_CCACHE" = yes ] && [ -d "$CCACHE_DIR" ]; then
    echo "CCache stats before build:"
    ccache -s || true
fi

mkdir -p "${HOME}/.ccache"

# Compiler detection
is_gnucc() {
    if [ -n "$1" ] && "$1" --version 2>&1 | grep 'Free Software Foundation' > /dev/null ; then true ; else false ; fi
}

COMPILER_FAMILY=""
if [ -n "$CC" -a -n "$CXX" ]; then
    if is_gnucc "$CC" && is_gnucc "$CXX" ; then
        COMPILER_FAMILY="GCC"
        export CC CXX
    fi
else
    if is_gnucc "gcc" && is_gnucc "g++" ; then
        # Autoconf would pick this by default
        COMPILER_FAMILY="GCC"
        [ -n "$CC" ] || CC=gcc
        [ -n "$CXX" ] || CXX=g++
        export CC CXX
    elif is_gnucc "cc" && is_gnucc "c++" ; then
        COMPILER_FAMILY="GCC"
        [ -n "$CC" ] || CC=cc
        [ -n "$CXX" ] || CXX=c++
        export CC CXX
    fi
fi

if [ -n "$CPP" ] ; then
    [ -x "$CPP" ] && export CPP
else
    if is_gnucc "cpp" ; then
        CPP=cpp && export CPP
    fi
fi

LIBSODIUM_VER=1.0.8
LIBZMQ_VER=v4.2.5
LIBCZMQ_VER=v4.0.2
MALAMUTE_VER=v1.6.1

CONFIG_FLAGS=()
CONFIG_FLAGS+=("CFLAGS=-I${BUILD_PREFIX}/include")
CONFIG_FLAGS+=("CPPFLAGS=-I${BUILD_PREFIX}/include")
CONFIG_FLAGS+=("CXXFLAGS=-I${BUILD_PREFIX}/include")
CONFIG_FLAGS+=("LDFLAGS=-L${BUILD_PREFIX}/lib")

KERNEL_FLAGS=()
KERNEL_FLAGS+=("INSTALLDIR=${BUILD_PREFIX}/lib/modules/$(uname -r)/extra")
KERNEL_FLAGS+=("INSTALLHDRDIR=${BUILD_PREFIX}/include/pciDriver/driver")

HALCS_OPTS=()
HALCS_OPTS+=(${CONFIG_FLAGS[@]})
HALCS_OPTS+=(${KERNEL_FLAGS[@]})
HALCS_OPTS+=("PREFIX=${BUILD_PREFIX}")
HALCS_OPTS+=("SCRIPTS_PREFIX=${SCRIPTS_PREFIX}")

CONFIG_OPTS=()
CONFIG_OPTS+=(${CONFIG_FLAGS[@]})
CONFIG_OPTS+=("PKG_CONFIG_PATH=${BUILD_PREFIX}/lib/pkgconfig")
CONFIG_OPTS+=("--prefix=${BUILD_PREFIX}")
CONFIG_OPTS+=("--with-docs=no")
CONFIG_OPTS+=("CFLAGS=-Wno-format-truncation")
CONFIG_OPTS+=("CPPFLAGS=-Wno-format-truncation")

ZMQ_CONFIG_OPTS=()
ZMQ_CONFIG_OPTS+=("--with-libsodium")

# CCache CCACHE_PATH setup
if [ "$HAVE_CCACHE" = yes ] && [ "${COMPILER_FAMILY}" = GCC ]; then
    PATH="/usr/lib/ccache:$PATH"
    export PATH

    if [ -n "$CC" ] && [ -x "/usr/lib/ccache/`basename "$CC"`" ]; then
        case "$CC" in
            *ccache*)
                ;;
            */*)
                DIR_CC="`dirname "$CC"`" && [ -n "$DIR_CC" ] && \
                    DIR_CC="`cd "$DIR_CC" && pwd `" && [ -n "$DIR_CC" ] && \
                    [ -d "$DIR_CC" ] || DIR_CC=""

                [ -z "$CCACHE_PATH" ] && CCACHE_PATH="$DIR_CC" || \
                    if echo "$CCACHE_PATH" | egrep '(^'"$DIR_CC"':.*|^'"$DIR_CC"'$|:'"$DIR_CC"':|:'"$DIR_CC"'$)' ; then
                        CCACHE_PATH="$DIR_CC:$CCACHE_PATH"
                    fi
                ;;
            esac
        CC="/usr/lib/ccache/`basename "$CC"`"
    else
        : # CC="ccache $CC"
    fi
    if [ -n "$CXX" ] && [ -x "/usr/lib/ccache/`basename "$CXX"`" ]; then
        case "$CXX" in
            *ccache*)
                ;;
            */*)
                DIR_CXX="`dirname "$CXX"`" && [ -n "$DIR_CXX" ] && \
                    DIR_CXX="`cd "$DIR_CXX" && pwd `" && [ -n "$DIR_CXX" ] && \
                    [ -d "$DIR_CXX" ] || DIR_CXX=""

                [ -z "$CCACHE_PATH" ] && CCACHE_PATH="$DIR_CXX" || \
                    if echo "$CCACHE_PATH" | egrep '(^'"$DIR_CXX"':.*|^'"$DIR_CXX"'$|:'"$DIR_CXX"':|:'"$DIR_CXX"'$)' ; then
                        CCACHE_PATH="$DIR_CXX:$CCACHE_PATH"
                    fi
                ;;
            esac
        CXX="/usr/lib/ccache/`basename "$CXX"`"
    else
        : # CXX="ccache $CXX"
    fi
    if [ -n "$CPP" ] && [ -x "/usr/lib/ccache/`basename "$CPP"`" ]; then
        case "$CPP" in
            *ccache*)
                ;;
            */*)
                DIR_CPP="`dirname "$CPP"`" && [ -n "$DIR_CPP" ] && \
                    DIR_CPP="`cd "$DIR_CPP" && pwd `" && [ -n "$DIR_CPP" ] && \
                    [ -d "$DIR_CPP" ] || DIR_CPP=""

                [ -z "$CCACHE_PATH" ] && CCACHE_PATH="$DIR_CPP" || \
                    if echo "$CCACHE_PATH" | egrep '(^'"$DIR_CPP"':.*|^'"$DIR_CPP"'$|:'"$DIR_CPP"':|:'"$DIR_CPP"'$)' ; then
                        CCACHE_PATH="$DIR_CPP:$CCACHE_PATH"
                    fi
                ;;
        esac
        CPP="/usr/lib/ccache/`basename "$CPP"`"
    else
        : # CPP="ccache $CPP"
    fi

    CONFIG_OPTS+=("CC=${CC}")
    CONFIG_OPTS+=("CXX=${CXX}")
    CONFIG_OPTS+=("CPP=${CPP}")
fi

###########   libsodium
echo ""
BASE_PWD=${PWD}
echo "`date`: INFO: Building prerequisite 'libsodium' from Git repository..." >&2
git clone --branch=${LIBSODIUM_VER} git://github.com/jedisct1/libsodium.git
cd libsodium

# CCache directory
CCACHE_BASEDIR=${PWD}
export CCACHE_BASEDIR

git --no-pager log --oneline -n1o

# Build/Install
if [ -e autogen.sh ]; then
    ./autogen.sh 2> /dev/null
fi
if [ -e buildconf ]; then
    ./buildconf 2> /dev/null
fi
if [ ! -e autogen.sh ] && [ ! -e buildconf ] && [ ! -e ./configure ] && [ -s ./configure.ac ]; then
    libtoolize --copy --force && \
        aclocal -I . && \
        autoheader && \
        automake --add-missing --copy && \
        autoconf || \
        autoreconf -fiv
fi
./configure "${CONFIG_OPTS[@]}"
make -j4
make install
cd "${BASE_PWD}"

###########   libzmq
echo ""
BASE_PWD=${PWD}
echo "`date`: INFO: Building prerequisite 'libzmq' from Git repository..." >&2
git clone --branch=${LIBZMQ_VER} git://github.com/zeromq/libzmq.git
cd libzmq

# CCache directory
CCACHE_BASEDIR=${PWD}
export CCACHE_BASEDIR

git --no-pager log --oneline -n1o

# Build/Install
if [ -e autogen.sh ]; then
    ./autogen.sh 2> /dev/null
fi
if [ -e buildconf ]; then
    ./buildconf 2> /dev/null
fi
if [ ! -e autogen.sh ] && [ ! -e buildconf ] && [ ! -e ./configure ] && [ -s ./configure.ac ]; then
    libtoolize --copy --force && \
        aclocal -I . && \
        autoheader && \
        automake --add-missing --copy && \
        autoconf || \
        autoreconf -fiv
fi
./configure "${CONFIG_OPTS[@]}" "${ZMQ_CONFIG_OPTS[@]}"
make -j4
make install
cd "${BASE_PWD}"

###########   CZMQ
echo ""
BASE_PWD=${PWD}
echo "`date`: INFO: Building prerequisite 'czmq' from Git repository..." >&2
git clone --branch=${LIBCZMQ_VER} git://github.com/zeromq/czmq.git
cd czmq

# CCache directory
CCACHE_BASEDIR=${PWD}
export CCACHE_BASEDIR

git --no-pager log --oneline -n1o

# Build/Install
if [ -e autogen.sh ]; then
    ./autogen.sh 2> /dev/null
fi
if [ -e buildconf ]; then
    ./buildconf 2> /dev/null
fi
if [ ! -e autogen.sh ] && [ ! -e buildconf ] && [ ! -e ./configure ] && [ -s ./configure.ac ]; then
    libtoolize --copy --force && \
        aclocal -I . && \
        autoheader && \
        automake --add-missing --copy && \
        autoconf || \
        autoreconf -fiv
fi
./configure "${CONFIG_OPTS[@]}"
make -j4
make install
cd "${BASE_PWD}"

###########   Malamute
echo ""
BASE_PWD=${PWD}
echo "`date`: INFO: Building prerequisite 'malamute' from Git repository..." >&2
git clone --branch=${MALAMUTE_VER} git://github.com/lnls-dig/malamute.git
cd malamute

# CCache directory
CCACHE_BASEDIR=${PWD}
export CCACHE_BASEDIR

git --no-pager log --oneline -n1o

# Build/Install
if [ -e autogen.sh ]; then
    ./autogen.sh 2> /dev/null
fi
if [ -e buildconf ]; then
    ./buildconf 2> /dev/null
fi
if [ ! -e autogen.sh ] && [ ! -e buildconf ] && [ ! -e ./configure ] && [ -s ./configure.ac ]; then
    libtoolize --copy --force && \
        aclocal -I . && \
        autoheader && \
        automake --add-missing --copy && \
        autoconf || \
        autoreconf -fiv
fi
./configure "${CONFIG_OPTS[@]}"
make -j4
make install
cd "${BASE_PWD}"

########## HALCS
if [ -n "$GRADLE" ]; then
    export ${CONFIG_FLAGS[@]}
    ./gradlew $GRADLE
elif [ "$PURE_MAKE" = yes ]; then
    build-wrapper-linux-x86-64 --out-dir bw-output \
        make BOARD="$BOARD" APPS="${APP}" ${HALCS_OPTS[*]}
    make ${HALCS_OPTS[*]} install

    if [ "$EXAMPLES" = yes ]; then
        make ${HALCS_OPTS[*]} examples
    fi

    if [ "$SYSTEM_INTEGRATION" = yes ]; then
        make ${HALCS_OPTS[*]} scripts_install
    fi

elif [ "$COMPILE_SCRIPT" = yes ]; then
    build-wrapper-linux-x86-64 --out-dir bw-output ./compile.sh -b $BOARD -a "${APP}" -e $EXAMPLES -l $SYSTEM_INTEGRATION -x "${HALCS_OPTS[*]}"
else
    build-wrapper-linux-x86-64 --out-dir bw-output ./compile.sh -b $BOARD -a "${APP}" -e $EXAMPLES -l $SYSTEM_INTEGRATION -x "${HALCS_OPTS[*]}"
fi

# Get CCache statistics
if [ "$HAVE_CCACHE" = yes ]; then
    echo "CCache stats after build:"
    ccache -s
fi
