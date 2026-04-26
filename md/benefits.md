% socklog - benefits

[G. Pape](https://smarden.org/pape/)\
[socklog](index.html)

---

# socklog - benefits

---

[Log file rotation based on file size](#logrotation)\
[Selecting and de-selecting log entries](#selecting)\
[Modular, reliable network logging](#network)\
[Sortable logs](#sortable)\
[Small code size](#smallcode)

---

Most of the benefits described here are actually benefits of
[svlogd](https://smarden.org/runit/svlogd.8.html) and
[multilog](https://cr.yp.to/daemontools/multilog.html). But since
*socklog* uses [svlogd](https://smarden.org/runit/svlogd.8.html) as its
back-end, the benefits apply to *syslogging* with *socklog*.

---

## [Log file rotation based on file size]{#logrotation}

[svlogd](https://smarden.org/runit/svlogd.8.html) supports automatically
rotated logs, there is no need for any other tool running from cron or
similar to rotate the logs.

[svlogd](https://smarden.org/runit/svlogd.8.html) keeps a specified
number of log files with a maximum file size, so it is possible to
calculate a log file partition properly.

For example, this `log/run` file

      #!/bin/sh
      LOGDIR=/var/log/socklog
      exec setuidgid log svlogd ${LOGDIR}/main ${LOGDIR}/debug

with these log `config` files

      # cat /var/log/socklog/main/config
      s4999999
      n10
      # cat /var/log/socklog/debug/config
      s999999
      n5
      -*
      +*.debug*

causes [svlogd](https://smarden.org/runit/svlogd.8.html) to hold maximal
10 log files, each of maximal size 4999999 bytes in
`/var/log/socklog/main/`, and maximal 5 log files, each of maximal size
999999 bytes in `/var/log/socklog/debug/`.

So the maximal used space for these both logs is less then:

`(10 * 5000000) + (5 * 1000000) = 55000000 bytes < 55 MB`

If there is any service acting up and filling up your logs rapidly, you
will never run into a filled up log partition, causing loss of new log
entries, even of other services. You can lose old log entries from this
service, but will ever have the recent ones.

Please note that above numbers could change if you use
[svlogd](https://smarden.org/runit/svlogd.8.html) with a *processor*.

---

## [Selecting and de-selecting log entries]{#selecting}

[svlogd](https://smarden.org/runit/svlogd.8.html) supports selecting and
de-selecting log lines based on pattern matching.

This is much more flexible than the usual *syslogd* support for
selecting log entries by `facility` and `priority`. Of course, selecting
entries by `facility` or `priority` is still possible with *socklog* and
[svlogd](https://smarden.org/runit/svlogd.8.html), just like the default
*socklog-unix* configuration shows.

This line selection feature is also used for *socklog*\'s instant [Log
events notification](notify.html).

---

## [Modular, reliable network logging]{#network}

The *socklog-inet* service can be used to replace a *syslog* network
server, receiving log messages on UDP port 514. Usually *syslog* clients
try to transmit their logs to that port using UDP.

*socklog* does not support this UDP network logging on the client side.
*socklog* provides a different modular, reliable [Network logging
concept](network.html) instead. Logs are transmitted through network
using a TCP connection in the default configuration, log transmission
failures are handled. The modular design of
[runit](https://smarden.org/runit/) and *socklog* makes it possible to
insert encryption, authentication, compression, and more, easily.

---

## [Sortable logs]{#sortable}

If [svlogd](https://smarden.org/runit/svlogd.8.html) is told to prepend
log lines with time-stamps, the resulting logs are sortable. So it is
possible to merge different logs and sort them chronologically without
special effort.

---

## [Small code size]{#smallcode}

One of the *socklog* project\'s principles is to keep the code size
minimal. As of version 0.8.0 of *socklog*, the `socklog.c` source
contains only about 400 lines of code; the `uncat.c` source is about 200
lines of code.

This minimizes the possibility of bugs introduced by programmer\'s
fault, and makes it more easy for security related people to proofread
the source code.

Of course this also means that *socklog* never will evolve to a
multi-/over-featured, bloated project.

---

[Gerrit Pape \<pape@smarden.org\>](mailto:pape@smarden.org)
