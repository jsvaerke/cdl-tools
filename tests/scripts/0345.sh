#!/bin/bash
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2023 Western Digital Corporation or its affiliates.
#

. "${scriptdir}/test_lib"

testname="CDL active (0xf policy) + CDL inactive (0xf policy) reads"

if [ $# == 0 ]; then
	echo $testname
	exit 0
fi

filename=$0
dev=$1

read_limits="active-inactive-time"
write_limits=""
cdl_dld="3+6"
expect_error=1
compare_latencies=0
ncq=1
rw="randread"

execute_test "${testname}" \
	"${read_limits}" "${write_limits}" \
	"${cdl_dld}" "${expect_error}" \
	"${compare_latencies}" "${filename}" \
	"${dev}" "${ncq}" "${rw}" || \
	exit_failed "error executing fio test"

exit 0
