#!/bin/bash
set -u
set -e

# Prefix
PREFIX=/usr/local
# Broker Endpoint
EXPECTED_ARGS=1

if [ "$(id -u)" != "0" ]
then
	echo "This must be run as root"
	exit 1
fi

if [ $# -ne $EXPECTED_ARGS ]
then
	echo "Usage: `basename $0` {board slot number}"
	exit 1;
fi

board_slot=$1

valgrind --leak-check=yes --trace-children=yes \
    --suppressions=valgrind.supp \
    ${PREFIX}/bin/halcsd -f ${PREFIX}/etc/halcs/halcs.cfg \
    -n be -t pcie \
    -i ${board_slot} \
    -b tcp://127.0.0.1:8978 -l stdout > \
    valgrind_report.txt 2>&1

