% socklog - examples

[G. Pape](https://smarden.org/pape/)\
[socklog](index.html)

---

# socklog - examples

---

## listening on unix domain dgram socket `/dev/log`:

-   a sample `socklog-unix/run` script:

          #!/bin/sh
          exec 2>&1
          exec chpst -Unobody socklog unix /dev/log
          

-   a sample `socklog-unix/log/run` script:

          #!/bin/sh
          exec chpst -ulog svlogd \
            main/main main/auth main/cron main/daemon main/debug main/ftp \
            main/kern main/local main/mail main/news main/syslog main/user
          

-   sample log `config` files:

          # cat main/config
          s999999
          n10

          # cat auth/config
          s999999
          n5
          -*
          +auth.*
          +authpriv.*

          # cat debug/config
          s999999
          n5
          -*
          +*.debug*
          

-   if you want additional
    [tai64n](https://cr.yp.to/daemontools/tai64n.html) time-stamps
    prepended to each message, use this `socklog-unix/log/run` script as
    example:

          #!/bin/sh
          exec chpst -ulog svlogd -t \
            main/main main/auth main/cron main/daemon main/debug main/ftp \
            main/kern main/local main/mail main/news main/syslog main/user
          

---

## listening on udp network socket `0.0.0.0`:`514`:

-   a sample `socklog-inet/run` script is:

          #!/bin/sh
          exec 2>&1
          exec chpst -Unobody socklog inet 0 514
          

-   a sample `socklog-inet/log/run` script is:

          #!/bin/sh
          exec chpst -ulog svlogd -t main/main
          

---

## reading kernel messages from `/dev/klog` on BSD:

-   a sample `socklog-klog/run` script on BSD:

          #!/bin/sh
          exec </dev/klog
          exec 2>&1
          exec chpst -unobody socklog ucspi
          

-   a sample `socklog-klog/log/run`:

          #!/bin/sh
          exec chpst -ulog svlogd -t main/main
          

---

## A client for [socklog network logging](network.html):

-   a sample `socklog-unix/log/run` script and log `config` file that
    transmits the logs to a network logging server running the
    *socklog-ucspi-tcp* service on `10.0.0.16:10116` is (you need to
    have the *netcat* package installed):

          #!/bin/sh
          exec chpst -ulog svlogd ./main/main

          # cat main/config
          s4096
          n20
          !tryto -pv nc -q0 10.0.0.16 10116
          

---

## [log events notification](notify.html) with *wall*:

-   a sample `socklog-notify/run` script that uses *wall* for
    notification is:

          #!/bin/sh -e
          PIPE=/var/log/socklog/.notify
          if [ ! -p "$PIPE" ]; then mkfifo -m0620 "$PIPE"; chown log:adm "$PIPE"; fi
          exec <> "$PIPE"
          exec chpst -ulog uncat -s49999 -t180 sh -c 'head | wall'
          

---

[Gerrit Pape \<pape@smarden.org\>](mailto:pape@smarden.org)
