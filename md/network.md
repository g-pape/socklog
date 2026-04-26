% socklog - network logging

[G. Pape](https://smarden.org/pape/)\
[socklog](index.html)

---

# socklog - network logging

---

[Logging through TCP](#tcp)\
[Logging through UDP](#udp)

---

Logging via network connection cannot be made reliable. There is always
a possibility for failures. The network connection itself may be down or
the receiving machine has crashed for example. So there must be a
decision, what to do in such cases.

## [The *socklog* network logging concept]{#tcp}

-   logging is still done locally through
    [svlogd](https://smarden.org/runit/svlogd.8.html).
-   the log is transmitted through a network connection (if possible)
    when [svlogd](https://smarden.org/runit/svlogd.8.html) decides that
    `current` is big enough, using
    [svlogd](https://smarden.org/runit/svlogd.8.html)\'s *processor*
    feature.
-   since this transmission cannot be made reliable, there is a tool
    [tryto](tryto.1.html) that limits the time to try this transmission.
    See below for examples.
-   if the transmission of `current` fails, the log will be saved
    locally, [svlogd](https://smarden.org/runit/svlogd.8.html) remembers
    the failure in its `state` and [tryto](tryto.1.html) includes the
    error messages into the transmission on the next run, so the remote
    machine gets the information of log transmission failures.

There are no restrictions on how to transmit the log data, a separate
process of your choice will do the work, e.g. *netcat* or
[tcpclient](https://cr.yp.to/ucspi-tcp/tcpclient.html). This modularity
lets you easily insert authentication, compression, encryption and other
things.

## Example setup

### Log Server (machine receiving log data)

Setup a *socklog-ucspi-tcp* service as described in
[Configuration](configuration.html#ucspi-tcp) with the following
`socklog-ucspi-tcp/run` and `socklog-ucspi-tcp/log/run` scripts:

socklog-ucspi-tcp/run:

      #!/bin/sh
      PORT=10116
      exec 2>&1
      exec tcpsvd -vl0 -unobody 0 "$PORT" socklog ucspi TCPREMOTEIP

socklog-ucspi-tcp/log/run:

      #!/bin/sh
      exec chpst -ulog svlogd -t main/main main/10.0.0.236

and socklog-ucspi-tcp/log/main/10.0.0.236/config

      # cat 10.0.0.236/config
      -*
      +10.0.0.236:*

You will then find all log data from remote hosts that was successfully
transmitted in `main/main/`. Log data from `10.0.0.236` will
additionally be saved in `main/10.0.0.236/`.

### Log client (machine sending log data)

Change the *socklog* configuration to use a processor to transmit the
log data:

socklog-unix/log/run:

      #!/bin/sh
      exec chpst -ulog svlogd ./main/main

socklog-unix/log/main/main/config

      s4096
      n20
      !tryto -pv nc 10.0.0.16 10116

and restart the log service:

      # sv restart socklog-unix/log

On each rotation of [svlogd](https://smarden.org/runit/svlogd.8.html)\'s
`current`, the data will be transmitted to `10.0.0.16:10116` using
[tryto](tryto.1.html) and *netcat*, failures will be noticed and
notified on the next run.

---

## [Logging through UDP]{#udp}

Transmitting log messages through UDP should only be used within private
networks. Logging through UDP doesn\'t ensure that the log messages
actually reach the log server, and doesn\'t provide authentication and
authorization. It\'s the old-style UNIX syslog remote logging, and
supported by *socklog* for compatibility reasons.

## Example setup

### Log Server (machine receiving log data)

Setup a *socklog-inet* service as described in
[Configuration](configuration.html#inet) with the following
`socklog-inet/run` and `socklog-inet/log/run` scripts, and the following
`socklog-inet/log/main/config` log configuration:

socklog-inet/run:

      #!/bin/sh
      exec 2>&1
      exec chpst -Unobody socklog inet 0 514

socklog-inet/log/run:

      #!/bin/sh
      exec chpst -ulog svlogd -t main/main main/10.0.0.236

and socklog-inet/log/main/10.0.0.236/config:

      # cat 10.0.0.236/config
      -*
      +10.0.0.236:*

You will then find all log messages from remote hosts that were
successfully transmitted in `main/main/`. Log messages from `10.0.0.236`
will additionally be saved in `main/10.0.0.236/`.

### Log client (machine sending log data)

Tell socklog to write raw syslog messages without converting syslog
priority and facility to names by adding the -R option:

socklog-unix/run:

      #!/bin/sh
      exec 2>&1
      exec chpst -Unobody socklog -R unix /dev/log

Then change the configuration of *socklog*\'s main log directory to tell
[svlogd](https://smarden.org/runit/svlogd.8.html) to transmit log
messages through UDP:

/var/log/socklog/main/config:

      s9999
      n2
      U10.0.0.16:514

Restart the service, and tell the log service to reload its
configuration:

      # sv restart socklog-unix
      # sv hup socklog-unix/log

Now each log message will be sent through UDP to `10.0.0.16:514`, and
not written to the log directory. If
[svlogd](https://smarden.org/runit/svlogd.8.html) has trouble
transmitting data through UDP, and is able to detect an error, it logs
an error message followed by the log message to the log directory.

Of course you can configure *socklog* to log through UDP while keeping
local logging enabled, and also select log messages to be transmitted
through UDP by pattern. See the documentation of
[svlogd](https://smarden.org/runit/svlogd.8.html) for details.

---

[Gerrit Pape \<pape@smarden.org\>](mailto:pape@smarden.org)
