#!/bin/bash
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2022 Western Digital Corporation or its affiliates.
#

. "${scriptdir}/test_lib"

testname="CDL inactive time (0xd complete-unavailable policy)"
T2A_file="${scriptdir}/cdl/T2A-inactive-time.cdl"
T2B_file="${scriptdir}/cdl/T2B-empty.cdl"

if [ $# == 0 ]; then
	echo $testname
	exit 0
fi

filename=$0
dev=$1

if dev_has_bad_fw "$1"; then
	exit_skip
fi

test_setup $dev $T2A_file $T2B_file || \
	exit_failed " --> FAILED (error during setup)"

# fio command
fiocmd=$(fio_common_cmdline $dev $filename "$testname")
fiocmd+=" --cmdprio_percentage=100 --cmdprio_class=4 --cmdprio=2"

echo "Running fio:"
fiolog="${logdir}/$(test_num $filename)_fio.log"
echo "${fiocmd}"
eval ${fiocmd} | tee "${fiolog}" || exit_failed " --> FAILED"

# We should have IO errors
if ! fio_has_io_error "${fiolog}"; then
	exit_failed " --> FAILED (fio did not see any IO error)"
fi

# IO errors should be 62 (ETIME)
errno="$(fio_get_io_error ${fiolog})"
if [ "${errno}" != "62" ]; then
	exit_failed " --> FAILED (fio saw error ${errno} instead of ETIME=62)"
fi

exit 0
