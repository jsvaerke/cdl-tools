#!/bin/bash
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2022 Western Digital Corporation or its affiliates.
#

. "${scriptdir}/test_lib"

testname="CDL inactive time (0x0 complete-earliest policy)"
T2A_file="${scriptdir}/cdl/T2A-inactive-time.cdl"
T2B_file="${scriptdir}/cdl/T2B-empty.cdl"
cdl_dld=1
expect_error=0
compare_latencies=0

if [ $# == 0 ]; then
	echo $testname
	exit 0
fi

filename=$0
dev=$1

execute_test "$testname" $T2A_file $T2B_file $cdl_dld $expect_error $compare_latencies $filename $dev || \
	exit_failed " --> FAILED (error executing test)"

exit 0
