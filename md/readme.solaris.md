% socklog - on Solaris

[G. Pape](https://smarden.org/pape/)\
[socklog](index.html)

---

# socklog - on Solaris

---

## Logging on Solaris

Solaris uses a STREAMS file, usually `/dev/log` to route locally
generated syslog messages. See the *log*(7d) driver manual page for more
information. Starting with SunOS 5.6 (Solaris 2.5.1) the *syslog*(3)
library contains support for \"doors\", a superfast and multithreaded
IPC mechanism provided by the Solaris Operating Environment. Using a
door file - `/etc/.syslog_door` up to SunOS 5.7, `/var/run/syslog_door`
on SunOS 5.8 and later, the process listening on the syslog_door gets
immediate notification upon receipt of a log message.

## Format of Solaris syslog messages

Starting with Solaris 8, Sun introduced a new log message format
containing so called message IDs (See *syslogd*(1) and *msgid*(1)).

    [ID msgid facility.priority]

    Jul  9 10:13:06 u5 pseudo: [ID 129642 kern.info] pseudo-device: tod0
    Jul  9 16:14:51 u5 login: [ID 644210 auth.notice] ROOT LOGIN /dev/console

All Solaris versions up to Solaris 7 use the well known, good old syslog
messages:

    Jul 10 16:24:21 u5 login: ROOT LOGIN /dev/console
    Jul 10 16:35:19 u5 unix: se0 is /pci@1f,0/pci@1,1/ebus@1/se@14,400000

The format is configurable. The *log*(7d) driver configuration file is
`/kernel/drv/log.conf`. Set the variable \"msgid\" to either 1 or 0, to
en- or disable message ID generation. Note: you must reboot to activate
the changes.

*socklog* does prepend the formated \'`facility.priority: `\' to every
log message, whether message IDs are used or not. This makes it easier
to write svlogd or multilog filters. So this is how a log message
processed by *socklog* will look like:

without message IDs:

    auth.info: Jul 15 21:40:52 sshd[1576]: Accepted publickey for root from 195.71.100.159 port 36621 ssh2

with message IDs enabled (default on Solaris 8 and later):

    auth.info: Jul 11 14:10:53 sshd[382]: [ID 800047 auth.info] Accepted rsa for ROOT from 195.71.100.159 port 36177

I prefer the first method, because it\'s much less visual clutter, and I
don\'t need message IDs anyway.

## Kernel messages

Solaris has no such thing as `/proc/kmsg`, or `/dev/klog`. Kernel log
messages are provided by the same STREAMS device (`/dev/log`) as normal
log messages. All kernel messages are logged with a facility of
\"kern\":

    kern.info: unix: avail mem = 250077184
    kern.info: unix: root nexus = Sun Ultra 5/10 UPA/PCI (UltraSPARC-IIi 360MHz)
    kern.info: unix: pcipsy0 at root: UPA 0x1f 0x0

The kernel messages do not contain timestamps. It seems that syslog does
automatically prepend timestamps to kernel messages, so I might
implement this in *socklog* as well.

*dmesg* will not work any longer, since it is just a sh shell script
which prints out the last 200 lines from `/var/adm/messages`. If you
need *dmesg*, you can easily write your own version :-).

Note that if message IDs are enabled, kernel log messages contain the
name of the loadable module or driver instead of just \"unix:\".

    kern.info: m64: [ID 308573 kern.info] m64#0: 1280x1024, 4M mappable, rev 4750.7c
    kern.info: hme: [ID 517527 kern.info] SUNW,hme0 : PCI IO 2.0 (Rev Id = c1) Found

Be sure to enable message IDs to get these.

## How to compile socklog on Solaris

You\'ll have to decide if you want \"door\" support or not.

1\. With syslog_door support

    echo "gcc -O2 -Wall -DWANT_SUN_DOOR" > src/conf-cc
    echo "gcc -s -ldoor -lthread" > src/conf-ld

2\. Without syslog_door support (default) Just run package/install. Then
follow the standard installation instructions.

## How to configure socklog for the STREAMS log driver

    $ socklog-conf unix acct logacct [/rundir] [/logdir]

This will create the following run script:

    #!/bin/sh
    exec 2>&1
    exec chpst -Uacct socklog sun_stream /dev/log /var/run/syslog_door

It is possible to run multiple *socklog* instances, but only one
*socklog* can listen on the door file. If another process is in charge
of the door file, *socklog* will exit immediately with exit code 100.
Simply remove the last argument (path to door file), to start *socklog*
without door support. This way you can run syslogd and *socklog*
parallel, to test your *socklog* setup.

## Support

If you have problems, questions or suggestions regarding the Solaris
support, email me: [Lars Uffmann
\<lu@cachescrubber.org\>](mailto:lu@cachescrubber.org), or post to the
`<misc@list.smarden.org>` mailing list.

---

[Lars Uffmann, \<lu@cachescrubber.org\>](mailto:lu@cachescrubber.org)
