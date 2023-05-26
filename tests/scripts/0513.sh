#!/bin/bash
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2022 Western Digital Corporation or its affiliates.
#

. "${scriptdir}/test_lib"

testname="CDL active time (0xf abort policy) reads ncq=off"

if [ $# == 0 ]; then
	echo $testname
	exit 0
fi

filename=$0
dev=$1

read_limits="active-time"
write_limits=""
cdl_dld=4
expect_error=1
compare_latencies=0
ncq=0
rw="randread"

execute_test "${testname}" \
	"${read_limits}" "${write_limits}" \
	"${cdl_dld}" "${expect_error}" \
	"${compare_latencies}" "${filename}" \
	"${dev}" "${ncq}" "${rw}" || \
	exit_failed "error executing fio test"

exit 0
