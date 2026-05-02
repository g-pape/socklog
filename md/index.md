% socklog - system and kernel logging services

[G. Pape](https://smarden.org/pape/)

---

# socklog - system and kernel logging services

---

[How to install socklog](install.html)\
[Upgrading from previous versions of socklog](upgrade.html)\
[How to use dietlibc](usedietlibc.html)\
[Using on solaris](readme.solaris.html)

[Benefits](benefits.html)\
[Configuration](configuration.html)\
[Examples](examples.html)\
[Network logging](network.html)\
[Log events notification](notify.html)

[The `socklog-conf` program](socklog-conf.8.html)\
[The `socklog` program](socklog.8.html)\
[The `tryto` program](tryto.1.html)\
[The `uncat` program](uncat.1.html)

---

*socklog*, in cooperation with the [runit](https://smarden.org/runit/)
package, is a small and secure replacement for syslogd. There are three
main features syslogd provides:

-   receiving syslog messages from an unix domain socket
    (\"`/dev/log`\") and writing them to various files on disk depending
    on `facility` and `priority`.
-   receiving syslog messages from an udp socket (\"`0.0.0.0:514`\") and
    writing them to various files on disk depending on `facility` and
    `priority`.
-   writing received syslog messages to an udp socket
    (\"`a.b.c.d:514`\")

*socklog* provides these features with the help of
[runit](https://smarden.org/runit/)\'s
[runsvdir](https://smarden.org/runit/runsvdir.8.html),
[runsv](https://smarden.org/runit/runsv.8.html), and
[svlogd](https://smarden.org/runit/svlogd.8.html), provides a different
[network logging concept](network.html), and additionally does [log
event notification](notify.html).

[svlogd](https://smarden.org/runit/svlogd.8.html) has a built-in [log
file rotation based on file size](benefits.html#logrotation), so there
is no need for any cron jobs or similar to rotate the logs. Log
partitions can be calculated properly.

*socklog* runs on **Linux**, glibc 2.1.0 or higher, or dietlibc,
**Open-BSD**, and **Free-BSD**. *socklog* reportedly runs on **Solaris**
and **Net-BSD**.

*socklog* is small, secure, reliable.

---

Contribute to *socklog* through [GitHub
socklog](https://github.com/g-pape/socklog/).

---

*socklog* is run under [runit](https://smarden.org/runit/)\'s
[runsv](https://smarden.org/runit/runsv.8.html), writing syslog messages
it receives from an unix domain socket `path` (\"`/dev/log`\") or an
inet udp socket `ip:port` (\"`0.0.0.0:514`\") through a pipe provided by
[runsv](https://smarden.org/runit/runsv.8.html) to a
[svlogd](https://smarden.org/runit/svlogd.8.html) process.

*socklog* can be run as an [ucspi](https://cr.yp.to/proto/ucspi.txt)
application to listen to an unix domain stream socket and for
centralized or more flexible distributed logging (see [network
logging](network.html)).

If *socklog* listens to an udp socket, it prepends `ip:`
(\"`a.b.c.d:` \") to each syslog message it receives, where `a.b.c.d` is
the ip address of the connecting system.

If the environment variables `$UID` and/or `$GID` are present, *socklog*
drops permissions to those ids after creating and binding to the socket
(not in [ucspi](https://cr.yp.to/proto/ucspi.txt) mode).

*socklog* converts syslog `facility` and `priority` information to names
(\"`facility.priority`\") as found in `/usr/include/syslog.h` at compile
time if present, you can use this for
[svlogd](https://smarden.org/runit/svlogd.8.html)\'s line selecting by
pattern.

---

A proper `run` file for [runsv](https://smarden.org/runit/runsv.8.html)
is:

    #!/bin/sh
    exec 2>&1
    exec chpst -Unobody socklog unix /dev/log

A proper `log/run` file for
[runsv](https://smarden.org/runit/runsv.8.html) is:

    #!/bin/sh
    exec chpst -ulog svlogd -t main/*

---

See [INSTALL](install.html) on how to install *socklog* and
[Configuration](configuration.html) for setting up *socklog* services.

---

See <https://smarden.org/socklog/> for recent information.

---

[Gerrit Pape \<pape@smarden.org\>](mailto:pape@smarden.org)
