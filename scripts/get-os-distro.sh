#!/usr/bin/env bash
# Detects which OS and if it is Linux then it will detect which Linux Distribution.

OS=`uname -s`
REV=`uname -r`
MACH=`uname -m`

GetVersionFromFile()
{
    VERSION=`cat $1 | tr "\n" ' ' | sed s/.*VERSION.*=\ // `
}

function usage() {
    echo "Usage: $0 [-d <Print distribution name only>] [-r <Print distribution revision only>]"
}

DIST_ONLY=0
REV_ONLY=0

# Get command line options
while getopts ":dr" opt; do
    case $opt in
        d)
            DIST_ONLY=1
            ;;
        r)
            REV_ONLY=1
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            usage
            exit 1
            ;;
        :)
            echo "Option -$OPTARG requires an argument." >&2
            usage
            exit 1
            ;;
    esac
done

# Check some OSes and Distro
if [ "${OS}" = "SunOS" ] ; then
    OS=Solaris
    ARCH=`uname -p`
#    OSSTR="${OS} ${REV}(${ARCH} `uname -v`)"
elif [ "${OS}" = "AIX" ] ; then
    OSSTR="${OS} `oslevel` (`oslevel -r`)"
elif [ "${OS}" = "Linux" ] ; then
    KERNEL=`uname -r`
    if [ -f /etc/redhat-release ] ; then
        DIST='RedHat'
        PSUEDONAME=`cat /etc/redhat-release | sed s/.*\(// | sed s/\)//`
        REV=`cat /etc/redhat-release | sed s/.*release\ // | sed s/\ .*//`
    elif [ -f /etc/fedora-release ] ; then
        DIST='Fedora'
        PSUEDONAME=`cat /etc/fedora-release | sed s/.*\(// | sed s/\)//`
        REV=`cat /etc/fedora-release | sed s/.*release\ // | sed s/\ .*//`
    elif [ -f /etc/SUSE-release ] ; then
        DIST=`cat /etc/SUSE-release | tr "\n" ' '| sed s/VERSION.*//`
        REV=`cat /etc/SUSE-release | tr "\n" ' ' | sed s/.*=\ //`
    elif [ -f /etc/lsb-release ] ; then
        DIST='Ubuntu'
        PSUEDONAME=`cat /etc/lsb-release | grep "DISTRIB_CODENAME=.*" | sed 's/DISTRIB_CODENAME=\(.*\)/\1/'`
        REV=`cat /etc/lsb-release | grep "DISTRIB_RELEASE=.*" | sed 's/DISTRIB_RELEASE=\(.*\)/\1/'`
    elif [ -f /etc/debian_version ] ; then
        DIST="Debian `cat /etc/debian_version`"
        REV=""
    fi

    OSSTR="${OS} ${DIST} ${REV}(${PSUEDONAME} ${KERNEL} ${MACH})"
fi

OUT=
if [ "${DIST_ONLY}" = 0 ] && [ "${REV_ONLY}" = 0 ] ; then
    OUT+=${OSSTR}
else
    if [ "${DIST_ONLY}" = 1 ] ; then
        OUT+="${DIST}"
    fi

    if [ "${REV_ONLY}" = 1 ] ; then
        OUT+=" ${REV}"
    fi
fi

echo ${OUT}
