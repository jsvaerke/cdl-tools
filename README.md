Copyright (C) 2021, Western Digital Corporation or its affiliates.

# <p align="center">CDL tools</p>

This project provides the *cdladm* command line utility which allows
inspecting and configuring command duration limits for block devices
supporting this feature.

## License

The *cdl-tools* project source code is distributed under the terms of the
GNU General Public License v2.0 or later
([GPL-v2](https://opensource.org/licenses/GPL-2.0)).
A copy of this license with *cdl-tools* copyright can be found in the files
[LICENSES/GPL-2.0-or-later.txt](LICENSES/GPL-2.0-or-later.txt) and
[COPYING.GPL](COPYING.GPL).

All source files in *cdl-tools* contain the SPDX short identifier for the
GPL-2.0-or-later license in place of the full license text.

```
SPDX-License-Identifier: GPL-2.0-or-later
```

Some files such as the `Makefile.am` files and the `.gitignore` file are public
domain specified by the [CC0 1.0 Universal (CC0 1.0) Public Domain
Dedication](https://creativecommons.org/publicdomain/zero/1.0/).
These files are identified with the following SPDX short identifier header.

```
SPDX-License-Identifier: CC0-1.0
```

See [LICENSES/CC0-1.0.txt](LICENSES/CC0-1.0.txt) for the full text of this
license.

## Requirements

The following packages must be installed prior to compiling *cdladm*.

* autoconf
* autoconf-archive
* automake
* libtool

## Compilation and Installation

The following commands will compile the *cdladm* utility.

```
$ sh ./autogen.sh
$ ./configure
$ make
```

To install the compiled executable file and the man page for the *cdladm*
utility, the following command can be used.

```
$ sudo make install
```

The default installation directory is /usr/bin. This default location can be
changed using the configure script. Executing the following command displays
the options used to control the installation path.

```
$ ./configure --help
```

## Building RPM Packages

The *rpm* and *rpmbuild* utilities are necessary to build *cdl-tools* RPM
packages. Once these utilities are installed, the RPM packages can be built
using the following command.

```
$ sh ./autogen.sh
$ ./configure
$ make rpm
```

Four RPM packages are built: a binary package providing *cdladm* executable
and its documentation and license files, a source RPM package, a *debuginfo*
RPM package and a *debugsource* RPM package.

The source RPM package can be used to build the binary and debug RPM packages
outside of *cdl-tools* source tree using the following command.

```
$ rpmbuild --rebuild cdl-tools-<version>.src.rpm
```

## Contributing

Read the [CONTRIBUTING](CONTRIBUTING) file and send patches to:

	Damien Le Moal <damien.lemoal@wdc.com>
	Niklas Cassel <niklas.cassel@wdc.com>

# Using Command Duration Limits

## *cdladm* utility

The *cdladm* utility allows manipulating command duration limits configuration
of SAS and SATA hard-disks supporting this feature.

*cdladm* provide many functions. The usage is as follows:

```
$ cdladm --help
Usage:
  cdladm --help | -h
  cdladm --version
  cdladm <command> [options] <device>
Options common to all commands:
  --verbose | -v       : Verbose output
  --force-ata | -a     : Force the use of ATA passthrough commands
Commands:
  info    : Show device and system support information
  list    : List supported pages
  show    : Display one or all supported pages
  save    : Save one or all pages to a file
  upload  : Upload a page to the device
  enable  : Enable command duration limits
  disable : Disable command duration limits
Command options:
  --count
	Apply to the show command.
	Omit the descriptor details and only print the number of
	valid descriptors in a page
  --page <name>
	Apply to the show and save commands.
	Specify the name of the page to show or save. The page name
	can be: "A", "B", "T2A" or "T2B".
  --file <path>
	Apply to the save and upload commands.
	Specify the path of the page file to use.
	Using this option is mandatory with the upload command.
	If this option is not specified with the save command,
	the default file name <dev name>-<page name>.cdl is
	used.
  --permanent
	Apply to the upload command.
	Specify that the device should save the page in
	non-volatile memory in addition to updating the current
	page value.
  --raw
	Apply to the show command.
	Show the raw values of the CDL pages fields
See "man cdladm" for more information
```

### Checking for command duration limits support

To check if a disk supports command duration limits, the "info" command can be
used:

```
$ cdladm info /dev/sda
Device: /dev/sdg
    Vendor: ATA
    Product: xxx  xxxxxxxxxxx
    Revision: xxxx
    42970644480 512-byte sectors (22.000 TB)
    Device interface: ATA
      SAT Vendor: linux
      SAT Product: libata
      SAT revision: 3.00
    Command duration limits: supported, disabled
    Command duration guidelines: supported
    High priority enhancement: supported, disabled
    Duration minimum limit: 20000000 ns
    Duration maximum limit: 4294967295000 ns
System:
    Node name: washi1.fujisawa.hgst.com
    Kernel: Linux 6.4.0-rc5+ #99 SMP PREEMPT_DYNAMIC Tue Jun  6 09:40:12 JST 2023
    Architecture: x86_64
    Command duration limits: supported, disabled
    Device sdg command timeout: 30 s
```

In the above example, the SATA drive tested supports command duration limits.
The warning displayed indicates that a feature-incomplete firmware is being used
(namely in this case, a firmware that does not yet support the set features
command to enable/disable command duration limits).

When applied to a disk that does not support command duration limits, *cdladm*
displays the following output.

```
$ cdladm info /dev/sde
Device: /dev/sdh
    Vendor: ATA
    Product: xxx  xxxxxxxxxxx
    Revision: xxxx
    39063650304 512-byte sectors (20.000 TB)
    Device interface: ATA
      SAT Vendor: linux
      SAT Product: libata
      SAT revision: 3.00
    Command duration limits: not supported, disabled
```

When using a kernel patched to add support for command duration limits, the
sysfs attribute files cdl_supported and cdl_enable will be present for any
 scsi device.

```
$ tree -L 1 /sys/block/sda/device/
/sys/block/sda/device/
|-- access_state
|-- blacklist
|-- block
|-- bsg
|-- cdl_enable
|-- cdl_supported
|-- delete
|-- device_blocked
|-- device_busy
|-- dh_state
|-- driver -> ../../../../../../../bus/scsi/drivers/sd
...
```

The cdl_supported attribute file indicates with a value of "1" if a device
supports cdl. A value of "0" indicates that the device does not implement the
CDL feature.

The cdl_enable attribute files allows enabling and disabling the CDL feature set
for a device supporting it.

### Checking Duration Limits Descriptors

As explained above, the device sysfs duration limits attribute files expose all
read and write limit descriptors values.

Using the *cdladm* show command, the drive can also be checked directly.

```
$ cdladm show /dev/sdg
Device: /dev/sdg
    Vendor: ATA
    Product: xxx  xxxxxxxxxxx
    Revision: xxxx
    42970644480 512-byte sectors (22.000 TB)
    Device interface: ATA
      SAT Vendor: linux
      SAT Product: libata
      SAT revision: 3.00
    Command duration limits: supported, disabled
    Command duration guidelines: supported
    High priority enhancement: supported, disabled
    Duration minimum limit: 20000000 ns
    Duration maximum limit: 4294967295000 ns
System:
    Node name: washi1.fujisawa.hgst.com
    Kernel: Linux 6.4.0-rc5+ #99 SMP PREEMPT_DYNAMIC Tue Jun  6 09:40:12 JST 2023
    Architecture: x86_64
    Command duration limits: supported, disabled
    Device sdg command timeout: 30 s
Page T2A:
  perf_vs_duration_guideline : 20%
  Descriptor 1:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : 30 ms
    duration guideline policy: complete-earliest
  Descriptor 2:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : 50 ms
    duration guideline policy: complete-earliest
  Descriptor 3:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : 100 ms
    duration guideline policy: complete-earliest
  Descriptor 4:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : 500 ms
    duration guideline policy: complete-earliest
  Descriptor 5:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : no limit
  Descriptor 6:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : no limit
  Descriptor 7:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : no limit
Page T2B:
  Descriptor 1:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : no limit
  Descriptor 2:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : no limit
  Descriptor 3:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : no limit
  Descriptor 4:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : no limit
  Descriptor 5:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : no limit
  Descriptor 6:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : no limit
  Descriptor 7:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : no limit
```

In the above example, the disk ```/dev/sdg``` has the read descriptors 1 to 4
configured with a duration guideline limit of 30ms, 50ms, 100ms and 500ms.

### Modifying Duration Limits Descriptors

To modify the duration limit descriptors of a device, *cdladm* "save" and
"upload" commands can be used. These commands operate on either the read
descriptors or the write descriptors. The read and write descriptors cannot be
changed together in a single operation.

The "save" command will save the current duration limit descriptors of the disk
to a file.

```
$ cdladm save --page T2A /dev/sdg
Device: /dev/sdg
    Vendor: ATA
    Product: xxx  xxxxxxxxxxx
    Revision: xxxx
    42970644480 512-byte sectors (22.000 TB)
    Device interface: ATA
      SAT Vendor: linux
      SAT Product: libata
      SAT revision: 3.00
    Command duration limits: supported, disabled
    Command duration guidelines: supported
    High priority enhancement: supported, disabled
    Duration minimum limit: 20000000 ns
    Duration maximum limit: 4294967295000 ns
System:
    Node name: washi1.fujisawa.hgst.com
    Kernel: Linux 6.4.0-rc5+ #99 SMP PREEMPT_DYNAMIC Tue Jun  6 09:40:12 JST 2023
    Architecture: x86_64
    Command duration limits: supported, disabled
    Device sdg command timeout: 30 s
Saving page T2A to file sdg-T2A.cdl
```

The file can then be edited to modify the duration limit descriptors fields.

```
$ less sdg-T2A.cdl
# T2A page format:
# perf-vs-duration-guideline can be one of:
#   - 0%    : 0x0
#   - 0.5%  : 0x1
#   - 1.0%  : 0x2
#   - 1.5%  : 0x3
#   - 2.0%  : 0x4
#   - 2.5%  : 0x5
#   - 3%    : 0x6
#   - 4%    : 0x7
#   - 5%    : 0x8
#   - 8%    : 0x9
#   - 10%   : 0xa
#   - 15%   : 0xb
#   - 20%   : 0xc
# t2cdlunits can be one of:
#   - none   : 0x0
#   - 500ns  : 0x6
#   - 1us    : 0x8
#   - 10ms   : 0xa
#   - 500ms  : 0xe
# max-inactive-time-policy can be one of:
#   - complete-earliest    : 0x0
#   - complete-unavailable : 0xd
#   - abort                : 0xf
# max-active-time-policy can be one of:
#   - complete-earliest    : 0x0
#   - complete-unavailable : 0xd
#   - abort-recovery       : 0xe
#   - abort                : 0xf
# duration-guideline-policy can be one of:
#   - complete-earliest    : 0x0
#   - continue-next-limit  : 0x1
#   - continue-no-limit    : 0x2
#   - complete-unavailable : 0xd
#   - abort                : 0xf

cdlp: T2A

perf-vs-duration-guideline: 0xc

== descriptor: 1
t2cdlunits: 0xa
max-inactive-time: 0
max-inactive-time-policy: 0x0
max-active-time: 0
max-active-time-policy: 0x0
duration-guideline: 3
duration-guideline-policy: 0x0

== descriptor: 2
t2cdlunits: 0xa
max-inactive-time: 0
max-inactive-time-policy: 0x0
max-active-time: 0
max-active-time-policy: 0x0
duration-guideline: 5
duration-guideline-policy: 0x0

== descriptor: 3
t2cdlunits: 0xa
max-inactive-time: 0
max-inactive-time-policy: 0x0
max-active-time: 0
max-active-time-policy: 0x0
duration-guideline: 10
duration-guideline-policy: 0x0

== descriptor: 4
t2cdlunits: 0xa
max-inactive-time: 0
max-inactive-time-policy: 0x0
max-active-time: 0
max-active-time-policy: 0x0
duration-guideline: 50
duration-guideline-policy: 0x0

== descriptor: 5
t2cdlunits: 0x0
max-inactive-time: 0
max-inactive-time-policy: 0x0
max-active-time: 0
max-active-time-policy: 0x0
duration-guideline: 0
duration-guideline-policy: 0x0

== descriptor: 6
t2cdlunits: 0x0
max-inactive-time: 0
max-inactive-time-policy: 0x0
max-active-time: 0
max-active-time-policy: 0x0
duration-guideline: 0
duration-guideline-policy: 0x0

== descriptor: 7
t2cdlunits: 0x0
max-inactive-time: 0
max-inactive-time-policy: 0x0
max-active-time: 0
max-active-time-policy: 0x0
duration-guideline: 0
duration-guideline-policy: 0x0
```

The modified file can then be used to upload to the disk the modified
descriptors. In the example below, descriptor 5 is enabled to define a 1s
duration limit.

```
$ cdladm upload --file sdg-T2A.cdl /dev/sdg
Device: /dev/sdg
    Vendor: ATA
    Product: xxx  xxxxxxxxxxx
    Revision: xxxx
    42970644480 512-byte sectors (22.000 TB)
    Device interface: ATA
      SAT Vendor: linux
      SAT Product: libata
      SAT revision: 3.00
    Command duration limits: supported, disabled
    Command duration guidelines: supported
    High priority enhancement: supported, disabled
    Duration minimum limit: 20000000 ns
    Duration maximum limit: 4294967295000 ns
System:
    Node name: washi1.fujisawa.hgst.com
    Kernel: Linux 6.4.0-rc5+ #99 SMP PREEMPT_DYNAMIC Tue Jun  6 09:40:12 JST 2023
    Architecture: x86_64
    Command duration limits: supported, disabled
    Device sdg command timeout: 30 s
Parsing file sdg-T2A.cdl...
Uploading page T2A:
  perf_vs_duration_guideline : 20%
  Descriptor 1:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : 50 ms
    duration guideline policy: complete-earliest
  Descriptor 2:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : 20 ms
    duration guideline policy: continue-next-limit
  Descriptor 3:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : 50 ms
    duration guideline policy: complete-earliest
  Descriptor 4:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : 20 ms
    duration guideline policy: continue-no-limit
  Descriptor 5:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : 20 ms
    duration guideline policy: complete-unavailable
  Descriptor 6:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : 20 ms
    duration guideline policy: abort
  Descriptor 7:
    max inactive time        : no limit
    max active time          : no limit
    duration guideline       : no limit
```

## Using Command Duration Limits

The Linux kernel support for command duration limits disable the feature by
default. For command duration limits to be effective, CDL support must first be
enabled.

This can be done using the "enable" sysfs attribute file:

```
$ echo 1 > /sys/block/sdg/device/cdl_enable
```

For convenience, *cdladm* can also be used:

```
$ cdladm enable /dev/sdg
Command duration limits is enabled
Device: /dev/sdg
    Vendor: ATA
    Product: xxx  xxxxxxxxxxx
    Revision: xxxx
    42970644480 512-byte sectors (22.000 TB)
    Device interface: ATA
      SAT Vendor: linux
      SAT Product: libata
      SAT revision: 3.00
    Command duration limits: supported, enabled
    Command duration guidelines: supported
    High priority enhancement: supported, disabled
    Duration minimum limit: 20000000 ns
    Duration maximum limit: 4294967295000 ns
System:
    Node name: washi1.fujisawa.hgst.com
    Kernel: Linux 6.4.0-rc5+ #99 SMP PREEMPT_DYNAMIC Tue Jun  6 09:40:12 JST 2023
    Architecture: x86_64
    Command duration limits: supported, enabled
    Device sdg command timeout: 30 s

$ cat /sys/block/sdg/device/cdl_enable
1
```

Conversely, CDL can be disabled either using sysfs:

```
$ echo 0 > /sys/block/sdg/device/cdl_enable
```

Or using *cdladm*:

```
$ cdladm disable /dev/sdg
Command duration limits is disabled
Device: /dev/sdg
    Vendor: ATA
    Product: xxx  xxxxxxxxxxx
    Revision: xxxx
    42970644480 512-byte sectors (22.000 TB)
    Device interface: ATA
      SAT Vendor: linux
      SAT Product: libata
      SAT revision: 3.00
    Command duration limits: supported, disabled
    Command duration guidelines: supported
    High priority enhancement: supported, disabled
    Duration minimum limit: 20000000 ns
    Duration maximum limit: 4294967295000 ns
System:
    Node name: washi1.fujisawa.hgst.com
    Kernel: Linux 6.4.0-rc5+ #99 SMP PREEMPT_DYNAMIC Tue Jun  6 09:40:12 JST 2023
    Architecture: x86_64
    Command duration limits: supported, disabled
    Device sdg command timeout: 30 s
```

*fio* can be used to exercise a drive with a command duration limits enabled
workload. The standard fio options *cmdprio_class*, *cmdprio_percentage* and
*cmdprio_hint* allow a job using the *libaio* IO engine to specify a command
duration limit enabled workload.

For instance, the following fio script:

```
[global]
filename=/dev/sdg
random_generator=tausworthe64
continue_on_error=none
ioscheduler=none
direct=1
write_lat_log=randread.log
per_job_logs=0
log_prio=1

[randread]
rw=randread
bs=128k
ioengine=libaio
iodepth=32
cmdprio_class=2
cmdprio_hint=2
cmdprio_percentage=20
```

Will issue random 128KB read commands at a queue depth of 32 with 20% of the
commands using the best effort priority class with duration limits descriptor 2.

Using the *cmdprio_bssplit* option, different duration limits can be combined
in the same workload for different percentage of commands. For instance, the
following fio job definition:

```
[global]
filename=/dev/sdg
random_generator=tausworthe64
continue_on_error=none
ioscheduler=none
direct=1
write_lat_log=randread.log
per_job_logs=0
log_prio=1

[randread]
rw=randread
bs=128k
ioengine=libaio
iodepth=32
cmdprio_bssplit=128k/10/2/0/1:128k/20/2/0/2
```

will result in the IO job executing 10% of all IOs using the best effor priority
class with CDL descriptor 1 and 20% of all IOs using again the best effort
priority class but with CDL descriptor 2.

## Testing a system Command Duration Limits Support

The *cdl-tools* project includes a test suite to exercise a device supporting
command duration limits. Executing the test suite also allows testing the
host-bus-adapter and kernel being used on the test system.

> **Warning**: cdl-tools test suite is destructive. This means that it will
> overwrite the CDL descriptors in the T2A and T2B pages.
>
> If you want to keep your original limit descriptor settings set by your
> system administrator or HDD vendor, you must back up the T2A and T2B page
> manually, before running the test suite using:
>
> ```
> $ cdladm save --page T2A --file original_T2A.cdl /dev/sda
> $ cdladm save --page T2B --file original_T2B.cdl /dev/sda
> ```

*cdl-tools* test suite is written as a collection of bash scripts and can be
installed as follows as root.

```
$ make install-tests
```

The default installation path is ```/usr/local/cdl-tests```. This installation
path can be changed using the ```--prefix``` option of the configure script.

The top script to use for executing tests is ```cdl-tests.sh```

```
$ cd /usr/local/cdl-tests
$ ./cdl-tests.sh --help
Usage: cdl-tests.sh [Options] <block device file>
Options:
  --help | -h             : This help message
  --list | -l             : List all tests
  --logdir | -g <log dir> : Use this directory to store test log files.
                            default: logs/<bdev name>
  --test | -t <test num>  : Execute only the specified test case. Can be
                            specified multiple times.
  --force | -f            : Run all tests, even the ones skipped due to
                            an inadequate device fw being detected.
  --quick | -q            : Run quick tests with shorter fio runs.
                            This can result in less reliable test results.
  --repeat | -r <num>     : Repeat the execution of the selected test cases
                            <num> times (default: tests are executed once).
```

The test cases can be listed using the option "--list".

```
$ cd /usr/local/cdl-tests
$ ./cdl-tests.sh --list
  Test 0001: cdladm (get device information)
  Test 0002: cdladm (unsupported devices)
  Test 0100: cdladm (list CDL descriptors)
  Test 0101: cdladm (show CDL descriptors)
  Test 0102: cdladm (save CDL descriptors)
  Test 0103: cdladm (upload CDL descriptors)
  Test 0200: CDL sysfs (all attributes present)
  Test 0201: CDL (enable/disable)
  Test 0300: CDL dur. guideline (0x0 complete-earliest policy) reads
  Test 0301: CDL dur. guideline (0x1 continue-next-limit policy) reads
  Test 0302: CDL dur. guideline (0x2 continue-no-limit policy) reads
  Test 0303: CDL dur. guideline (0xd complete-unavailable policy) reads
  Test 0304: CDL dur. guideline (0xf abort policy) reads
  Test 0310: CDL active time (0x0 complete-earliest policy) reads
  Test 0311: CDL active time (0xd complete-unavailable policy) reads
  Test 0312: CDL active time (0xe abort-recovery policy) reads
  Test 0313: CDL active time (0xf abort policy) reads
  Test 0320: CDL inactive time (0x0 complete-earliest policy) reads
  Test 0321: CDL inactive time (0xd complete-unavailable policy) reads
  Test 0322: CDL inactive time (0xf abort policy) reads
  Test 0330: CDL active (0x0 policy) + inactive (0x0 policy) reads
  Test 0331: CDL active (0xd policy) + inactive (0xd policy) reads
  Test 0332: CDL active (0xd policy) + inactive (0xf policy) reads
  Test 0333: CDL active (0xf policy) + inactive (0xd policy) reads
  Test 0334: CDL active (0xf policy) + inactive (0xf policy) reads
  Test 0340: CDL active (0x0 policy) + CDL inactive (0x0 policy) reads
  Test 0341: CDL active (0xd policy) + CDL active (0xf policy) reads
  Test 0342: CDL active (0xd policy) + CDL inactive (0xd policy) reads
  Test 0343: CDL active (0xd policy) + CDL inactive (0xf policy) reads
  Test 0344: CDL active (0xf policy) + CDL inactive (0xd policy) reads
  Test 0345: CDL active (0xf policy) + CDL inactive (0xf policy) reads
  Test 0346: CDL inactive (0xd policy) + CDL inactive (0xf policy) reads
  Test 0400: CDL dur. guideline (0x0 complete-earliest policy) writes
  Test 0401: CDL dur. guideline (0x1 continue-next-limit policy) writes
  Test 0402: CDL dur. guideline (0x2 continue-no-limit policy) writes
  Test 0403: CDL dur. guideline (0xd complete-unavailable policy) writes
  Test 0404: CDL dur. guideline (0xf abort policy) writes
  Test 0410: CDL active time (0x0 complete-earliest policy) writes
  Test 0411: CDL active time (0xd complete-unavailable policy) writes
  Test 0412: CDL active time (0xe abort-recovery policy) writes
  Test 0413: CDL active time (0xf abort policy) writes
  Test 0420: CDL inactive time (0x0 complete-earliest policy) writes
  Test 0421: CDL inactive time (0xd complete-unavailable policy) writes
  Test 0422: CDL inactive time (0xf abort policy) writes
  Test 0500: CDL dur. guideline (0x0 complete-earliest policy) reads ncq=off
  Test 0501: CDL dur. guideline (0x1 continue-next-limit policy) reads ncq=off
  Test 0502: CDL dur. guideline (0x2 continue-no-limit policy) reads ncq=off
  Test 0503: CDL dur. guideline (0xd complete-unavailable policy) reads ncq=off
  Test 0504: CDL dur. guideline (0xf abort policy) reads ncq=off
  Test 0510: CDL active time (0x0 complete-earliest policy) reads ncq=off
  Test 0511: CDL active time (0xd complete-unavailable policy) reads ncq=off
  Test 0512: CDL active time (0xe abort-recovery policy) reads ncq=off
  Test 0513: CDL active time (0xf abort policy) reads ncq=off
  Test 0520: CDL inactive time (0x0 complete-earliest policy) reads ncq=off
  Test 0521: CDL inactive time (0xd complete-unavailable policy) reads ncq=off
  Test 0522: CDL inactive time (0xf abort policy) reads ncq=off
  Test 0600: CDL dur. guideline (0x0 complete-earliest policy) writes ncq=off
  Test 0601: CDL dur. guideline (0x1 continue-next-limit policy) writes ncq=off
  Test 0602: CDL dur. guideline (0x2 continue-no-limit policy) writes ncq=off
  Test 0603: CDL dur. guideline (0xd complete-unavailable policy) writes ncq=off
  Test 0604: CDL dur. guideline (0xf abort policy) writes ncq=off
  Test 0610: CDL active time (0x0 complete-earliest policy) writes ncq=off
  Test 0611: CDL active time (0xd complete-unavailable policy) writes ncq=off
  Test 0612: CDL active time (0xe abort-recovery policy) writes ncq=off
  Test 0613: CDL active time (0xf abort policy) writes ncq=off
  Test 0620: CDL inactive time (0x0 complete-earliest policy) writes ncq=off
  Test 0621: CDL inactive time (0xd complete-unavailable policy) writes ncq=off
  Test 0622: CDL inactive time (0xf abort policy) writes ncq=off
```

Executing the test suite requires root access rights.

```
$ cd /usr/local/cdl-tests
$ sudo ./cdl-tests.sh /dev/sdg
Running CDL tests on cmr /dev/sdg:
    Product: xxx  xxxxxxxxxxx
    Revision: WXYZ
    Using cdl-tools version 0.4.0
    Force all tests: disabled, quick tests: disabled
  Test 0001:  cdladm (get device information)                                      ... PASS
  Test 0002:  cdladm (unsupported devices)                                         ... PASS
  Test 0100:  cdladm (list CDL descriptors)                                        ... PASS
  Test 0101:  cdladm (show CDL descriptors)                                        ... PASS
  Test 0102:  cdladm (save CDL descriptors)                                        ... PASS
  Test 0103:  cdladm (upload CDL descriptors)                                      ... PASS
  Test 0200:  CDL sysfs (all attributes present)                                   ... PASS
  Test 0201:  CDL (enable/disable)                                                 ... PASS
  Test 0300:  CDL dur. guideline (0x0 complete-earliest policy) reads              ...
  ...
```

Log files for each test case are written by default in the "logs"
directory in the current working directory.
